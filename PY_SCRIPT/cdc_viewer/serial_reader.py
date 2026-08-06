from __future__ import annotations

from dataclasses import replace
from threading import Event
import time

import numpy as np
import serial
from serial.tools import list_ports
from PySide6.QtCore import QThread, Signal

from .config import (
    BAUDRATE,
    DEVICE_DESCRIPTION,
    MATRIX_COLUMNS,
    MATRIX_ROWS,
    SERIAL_TIMEOUT_SECONDS,
)
from .frame_store import LatestFrameStore, ReaderStats
from .protocol import FrameParser


class SerialReaderThread(QThread):
    connection_changed = Signal(bool, str)
    reader_error = Signal(str)
    crc_error = Signal(str)

    def __init__(
        self,
        store: LatestFrameStore,
        port_name: str | None = None,
        parent=None,
    ) -> None:
        super().__init__(parent)
        self._store = store
        self._requested_port = port_name
        self._stop_event = Event()

    def stop(self) -> None:
        self._stop_event.set()

    def run(self) -> None:
        error_message: str | None = None
        try:
            port_name = self._requested_port or self._find_port()
            self._read_serial(port_name)
        except (RuntimeError, serial.SerialException, OSError) as error:
            if not self._stop_event.is_set():
                error_message = str(error)
        finally:
            self.connection_changed.emit(False, "")
            if error_message is not None:
                self.reader_error.emit(error_message)

    def _read_serial(self, port_name: str) -> None:
        parser = FrameParser()
        stats = ReaderStats()
        interval_frames = 0
        report_time = time.monotonic()

        with serial.Serial(
            port_name,
            BAUDRATE,
            timeout=SERIAL_TIMEOUT_SECONDS,
        ) as cdc:
            self.connection_changed.emit(True, port_name)
            cdc.reset_input_buffer()

            while not self._stop_event.is_set():
                chunk = cdc.read(cdc.in_waiting or 1)
                for frame in parser.feed(chunk):
                    stats = replace(
                        stats,
                        total_frames=stats.total_frames + 1,
                        source_id=frame.source_id,
                        framing_errors=parser.framing_errors,
                    )
                    interval_frames += 1

                    if not frame.crc_valid:
                        stats = replace(stats, crc_errors=stats.crc_errors + 1)
                        self.crc_error.emit(
                            f"CRC错误：帧 {stats.total_frames}，"
                            f"收到 0x{frame.received_crc:08X}，"
                            f"计算 0x{frame.calculated_crc:08X}"
                        )
                        continue

                    # Decode 4096 little-endian uint16 values in native code.
                    matrix = (
                        np.frombuffer(frame.payload, dtype="<u2")
                        .reshape(MATRIX_ROWS, MATRIX_COLUMNS)
                        .copy()
                    )
                    self._store.publish_matrix(matrix, stats)

                now = time.monotonic()
                elapsed = now - report_time
                if elapsed >= 1.0:
                    stats = replace(
                        stats,
                        fps=interval_frames / elapsed,
                        framing_errors=parser.framing_errors,
                    )
                    self._store.publish_stats(stats)
                    interval_frames = 0
                    report_time = now

    @staticmethod
    def _find_port() -> str:
        matches = [
            port.device
            for port in list_ports.comports()
            if DEVICE_DESCRIPTION.casefold()
            in (port.description or "").casefold()
        ]
        if not matches:
            raise RuntimeError(f'找不到串口“{DEVICE_DESCRIPTION}”')
        return matches[0]
