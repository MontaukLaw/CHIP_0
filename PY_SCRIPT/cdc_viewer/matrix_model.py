from __future__ import annotations

import numpy as np
from PySide6.QtCore import QAbstractTableModel, QModelIndex, Qt

from .config import MATRIX_COLUMNS, MATRIX_ROWS
from .frame_store import UInt16Matrix


class MatrixTableModel(QAbstractTableModel):
    """Virtualized 32x128 numeric table backed directly by a NumPy array."""

    def __init__(self, parent=None) -> None:
        super().__init__(parent)
        self._matrix: UInt16Matrix = np.zeros(
            (MATRIX_ROWS, MATRIX_COLUMNS), dtype=np.uint16
        )

    def rowCount(self, parent=QModelIndex()) -> int:  # noqa: N802
        return 0 if parent.isValid() else MATRIX_ROWS

    def columnCount(self, parent=QModelIndex()) -> int:  # noqa: N802
        return 0 if parent.isValid() else MATRIX_COLUMNS

    def data(self, index: QModelIndex, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid():
            return None
        if role == Qt.ItemDataRole.DisplayRole:
            return str(int(self._matrix[index.row(), index.column()]))
        if role == Qt.ItemDataRole.TextAlignmentRole:
            return Qt.AlignmentFlag.AlignCenter
        return None

    def headerData(self, section, orientation, role=Qt.ItemDataRole.DisplayRole):  # noqa: N802
        if role != Qt.ItemDataRole.DisplayRole:
            return None
        return str(section)

    def set_matrix(self, matrix: UInt16Matrix) -> None:
        self._matrix = matrix
        self.dataChanged.emit(
            self.index(0, 0),
            self.index(MATRIX_ROWS - 1, MATRIX_COLUMNS - 1),
            [Qt.ItemDataRole.DisplayRole],
        )
