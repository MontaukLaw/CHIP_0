from __future__ import annotations

from dataclasses import dataclass
import zlib

from .config import (
    CRC_OFFSET,
    FRAME_SIZE,
    HEADER_MAGIC,
    PAYLOAD_OFFSET,
    SOURCE_ID_OFFSET,
    TAIL_MAGIC,
    TAIL_OFFSET,
)


@dataclass(frozen=True, slots=True)
class CdcFrame:
    source_id: int
    payload: bytes
    received_crc: int
    calculated_crc: int

    @property
    def crc_valid(self) -> bool:
        return self.received_crc == self.calculated_crc


class FrameParser:
    """Incrementally extracts fixed-size frames from an arbitrary byte stream."""

    def __init__(self) -> None:
        self._buffer = bytearray()
        self.framing_errors = 0

    def feed(self, data: bytes) -> list[CdcFrame]:
        if data:
            self._buffer.extend(data)

        frames: list[CdcFrame] = []
        while True:
            header_position = self._buffer.find(HEADER_MAGIC)
            if header_position < 0:
                self._preserve_partial_header()
                break

            if header_position > 0:
                del self._buffer[:header_position]

            if len(self._buffer) < FRAME_SIZE:
                break

            if self._buffer[TAIL_OFFSET:FRAME_SIZE] != TAIL_MAGIC:
                self.framing_errors += 1
                del self._buffer[0]
                continue

            raw_frame = bytes(self._buffer[:FRAME_SIZE])
            del self._buffer[:FRAME_SIZE]

            payload = raw_frame[PAYLOAD_OFFSET:CRC_OFFSET]
            received_crc = int.from_bytes(
                raw_frame[CRC_OFFSET:TAIL_OFFSET], byteorder="little"
            )
            frames.append(
                CdcFrame(
                    source_id=raw_frame[SOURCE_ID_OFFSET],
                    payload=payload,
                    received_crc=received_crc,
                    calculated_crc=zlib.crc32(payload) & 0xFFFFFFFF,
                )
            )

        return frames

    def _preserve_partial_header(self) -> None:
        if self._buffer.endswith(HEADER_MAGIC[:1]):
            self._buffer[:] = HEADER_MAGIC[:1]
        else:
            self._buffer.clear()
