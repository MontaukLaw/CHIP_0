from __future__ import annotations

from PySide6.QtCore import QTimer
from PySide6.QtWidgets import (
    QAbstractItemView,
    QFrame,
    QHBoxLayout,
    QLabel,
    QMainWindow,
    QPushButton,
    QTableView,
    QVBoxLayout,
    QWidget,
)

from .config import GUI_REFRESH_INTERVAL_MS
from .frame_store import LatestFrameStore
from .matrix_model import MatrixTableModel
from .serial_reader import SerialReaderThread


class MainWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("STM32 CDC 数据矩阵")
        self.resize(1500, 850)

        self._store = LatestFrameStore()
        self._reader: SerialReaderThread | None = None
        self._displayed_sequence = -1

        self._matrix_model = MatrixTableModel(self)
        self._table = self._create_table()
        self._status_label = QLabel("未连接")
        self._status_label.setObjectName("metric")
        self._fps_label = QLabel("FPS  0.00")
        self._fps_label.setObjectName("metric")
        self._frame_label = QLabel("帧  0")
        self._frame_label.setObjectName("metric")
        self._source_label = QLabel("源  0x00")
        self._source_label.setObjectName("metric")
        self._error_label = QLabel("CRC  0  ·  帧错误  0")
        self._error_label.setObjectName("errorLabel")
        self._connect_button = QPushButton("连接")
        self._disconnect_button = QPushButton("断开")
        self._disconnect_button.setEnabled(False)

        self._build_layout()
        self._connect_button.clicked.connect(self.start_reader)
        self._disconnect_button.clicked.connect(self.stop_reader)

        self._refresh_timer = QTimer(self)
        self._refresh_timer.setInterval(GUI_REFRESH_INTERVAL_MS)
        self._refresh_timer.timeout.connect(self._refresh_gui)
        self._refresh_timer.start()

        QTimer.singleShot(0, self.start_reader)

    def _create_table(self) -> QTableView:
        table = QTableView()
        table.setModel(self._matrix_model)
        table.setAlternatingRowColors(True)
        table.setWordWrap(False)
        table.setEditTriggers(QAbstractItemView.EditTrigger.NoEditTriggers)
        table.setSelectionMode(QAbstractItemView.SelectionMode.SingleSelection)
        table.verticalHeader().setDefaultSectionSize(24)
        table.horizontalHeader().setDefaultSectionSize(58)
        table.horizontalHeader().setMinimumSectionSize(42)
        return table

    def _build_layout(self) -> None:
        central = QWidget()
        root = QVBoxLayout(central)
        root.setContentsMargins(16, 16, 16, 16)
        root.setSpacing(12)

        toolbar = QFrame()
        toolbar.setObjectName("toolbar")
        toolbar_layout = QHBoxLayout(toolbar)
        toolbar_layout.setContentsMargins(16, 10, 16, 10)

        title = QLabel("32 × 128 实时数据矩阵")
        title.setObjectName("title")
        toolbar_layout.addWidget(title)
        toolbar_layout.addStretch(1)
        for label in (
            self._status_label,
            self._source_label,
            self._fps_label,
            self._frame_label,
            self._error_label,
        ):
            toolbar_layout.addWidget(label)
        toolbar_layout.addSpacing(10)
        toolbar_layout.addWidget(self._connect_button)
        toolbar_layout.addWidget(self._disconnect_button)

        root.addWidget(toolbar)
        root.addWidget(self._table, 1)
        self.setCentralWidget(central)

    def start_reader(self) -> None:
        if self._reader is not None and self._reader.isRunning():
            return

        self._reader = SerialReaderThread(self._store, parent=self)
        self._reader.connection_changed.connect(self._on_connection_changed)
        self._reader.reader_error.connect(self._on_reader_error)
        self._reader.crc_error.connect(self._on_crc_error)
        self._reader.finished.connect(self._reader_finished)
        self._status_label.setText("正在连接…")
        self._connect_button.setEnabled(False)
        self._reader.start()

    def stop_reader(self) -> None:
        if self._reader is not None:
            self._reader.stop()
        self._disconnect_button.setEnabled(False)

    def _refresh_gui(self) -> None:
        snapshot = self._store.snapshot()
        if snapshot.sequence != self._displayed_sequence and snapshot.matrix is not None:
            self._matrix_model.set_matrix(snapshot.matrix)
            self._displayed_sequence = snapshot.sequence

        stats = snapshot.stats
        self._fps_label.setText(f"FPS  {stats.fps:.2f}")
        self._frame_label.setText(f"帧  {stats.total_frames}")
        self._source_label.setText(f"源  0x{stats.source_id:02X}")
        self._error_label.setText(
            f"CRC  {stats.crc_errors}  ·  帧错误  {stats.framing_errors}"
        )

    def _on_connection_changed(self, connected: bool, port_name: str) -> None:
        self._status_label.setText(port_name if connected else "未连接")
        self._disconnect_button.setEnabled(connected)

    def _on_reader_error(self, message: str) -> None:
        self._status_label.setText(f"错误：{message}")

    def _on_crc_error(self, message: str) -> None:
        self.statusBar().showMessage(message, 5000)

    def _reader_finished(self) -> None:
        if self._reader is not None:
            self._reader.deleteLater()
            self._reader = None
        self._connect_button.setEnabled(True)
        self._disconnect_button.setEnabled(False)

    def closeEvent(self, event) -> None:  # noqa: N802
        if self._reader is not None:
            self._reader.stop()
            self._reader.wait(1000)
        event.accept()
