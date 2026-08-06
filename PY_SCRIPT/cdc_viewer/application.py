from __future__ import annotations

import sys

from PySide6.QtWidgets import QApplication

from .main_window import MainWindow
from .theme import APP_STYLE


def run() -> int:
    app = QApplication(sys.argv)
    app.setApplicationName("STM32 CDC Matrix Viewer")
    app.setStyleSheet(APP_STYLE)

    window = MainWindow()
    window.show()
    return app.exec()
