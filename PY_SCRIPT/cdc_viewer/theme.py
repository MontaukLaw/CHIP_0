APP_STYLE = """
QMainWindow, QWidget {
    background-color: #0b0f14;
    color: #e6edf3;
    font-family: "Segoe UI";
    font-size: 13px;
}
QFrame#toolbar {
    background-color: #111821;
    border: 1px solid #253140;
    border-radius: 10px;
}
QLabel#title {
    font-size: 20px;
    font-weight: 600;
    color: #f0f6fc;
}
QLabel#metric {
    color: #9fb0c3;
    padding: 4px 8px;
}
QLabel#errorLabel {
    color: #ff7b72;
    padding: 4px 8px;
}
QPushButton {
    background-color: #dce6f2;
    color: #111821;
    border: none;
    border-radius: 7px;
    padding: 8px 18px;
    font-weight: 600;
}
QPushButton:hover { background-color: #ffffff; }
QPushButton:pressed { background-color: #b8c7d9; }
QPushButton:disabled { background-color: #52606f; color: #9aa7b5; }
QTableView {
    background-color: #070a0e;
    alternate-background-color: #0d131a;
    color: #dce6f2;
    gridline-color: #202b37;
    border: 1px solid #253140;
    border-radius: 8px;
    selection-background-color: #1f6feb;
    selection-color: #ffffff;
}
QHeaderView::section {
    background-color: #151d27;
    color: #9fb0c3;
    border: none;
    border-right: 1px solid #253140;
    border-bottom: 1px solid #253140;
    padding: 4px;
}
QScrollBar:horizontal, QScrollBar:vertical {
    background: #0b0f14;
    border: none;
    height: 12px;
    width: 12px;
}
QScrollBar::handle:horizontal, QScrollBar::handle:vertical {
    background: #405166;
    border-radius: 5px;
    min-width: 30px;
    min-height: 30px;
}
"""
