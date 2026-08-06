from __future__ import annotations

from dataclasses import dataclass
from threading import Lock

import numpy as np
from numpy.typing import NDArray


UInt16Matrix = NDArray[np.uint16]


@dataclass(frozen=True, slots=True)
class ReaderStats:
    fps: float = 0.0
    total_frames: int = 0
    crc_errors: int = 0
    framing_errors: int = 0
    source_id: int = 0


@dataclass(frozen=True, slots=True)
class FrameSnapshot:
    sequence: int
    matrix: UInt16Matrix | None
    stats: ReaderStats


class LatestFrameStore:
    """Transfers only the newest matrix from the reader to the GUI thread."""

    def __init__(self) -> None:
        self._lock = Lock()
        self._sequence = 0
        self._matrix: UInt16Matrix | None = None
        self._stats = ReaderStats()

    def publish_matrix(self, matrix: UInt16Matrix, stats: ReaderStats) -> None:
        with self._lock:
            self._sequence += 1
            self._matrix = matrix
            self._stats = stats

    def publish_stats(self, stats: ReaderStats) -> None:
        with self._lock:
            self._stats = stats

    def snapshot(self) -> FrameSnapshot:
        with self._lock:
            return FrameSnapshot(self._sequence, self._matrix, self._stats)
