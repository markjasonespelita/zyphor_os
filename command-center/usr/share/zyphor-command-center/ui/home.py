from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QLabel,
    QFrame, QScrollArea, QHBoxLayout
)
from PyQt6.QtCore import Qt


class HomePage(QWidget):
    def __init__(self):
        super().__init__()

        self.setStyleSheet("""
            QWidget {
                background-color: #0f172a;
                color: white;
            }
        """)

        main = QVBoxLayout(self)
        main.setContentsMargins(40, 30, 40, 30)

        # Container Card
        container = QFrame()
        container.setStyleSheet("""
            QFrame {
                background-color: #111827;
                border: 1px solid #1f2937;
                border-radius: 16px;
            }
        """)

        layout = QVBoxLayout(container)
        layout.setContentsMargins(30, 30, 30, 30)
        layout.setSpacing(20)

        # Header
        distro = QLabel("ZYPHOR OS")
        distro.setStyleSheet("""
            color: #3b82f6;
            font-size: 13px;
            font-weight: bold;
            letter-spacing: 2px;
        """)

        title = QLabel("🚀 What's New")
        title.setStyleSheet("""
            font-size: 32px;
            font-weight: bold;
        """)

        # IMPORTANT ==================== (Date and Version)

        version = QLabel("Version 1.11.1-u14 • Updated On: June 13, 2026")

        # /IMPORTANT =================== (Date and Version)

        notice = QLabel("Upgrade now to get the latest features, improvements, and fixes.")
        version.setStyleSheet("""
            color: #9ca3af;
            font-size: 15px;
        """)
        notice.setStyleSheet("""
            color: #9ca3af;
            font-size: 15px;
        """)

        layout.addWidget(distro)
        layout.addWidget(title)
        layout.addWidget(version)
        layout.addWidget(notice)

        # ================================================================

        # Feature Card
        feature = QFrame()
        feature.setStyleSheet("""
            QFrame {
                background-color: #161f2f;
                border: 1px solid #2a3445;
                border-radius: 12px;
            }
        """)

        feature_layout = QVBoxLayout(feature)

        section_title = QLabel("New Features")
        section_title.setStyleSheet("""
            color: #3b82f6;
            font-size: 22px;
            font-weight: bold;
        """)

        feature_layout.addWidget(section_title)

        changes = [
            "The ZyShell startup banner has been refreshed to provide a cleaner and more consistent display of system version information.",
            "New OS banner format: Zyphor OS vX.X.X-uX (ZyShell) X.X.X"
        ]

        for item in changes:
            lbl = QLabel(f"• {item}")
            lbl.setWordWrap(True)
            lbl.setStyleSheet("""
                font-size: 15px;
                line-height: 1.5;
            """)
            feature_layout.addWidget(lbl)

        layout.addWidget(feature)

        # ================================================================

        footer = QLabel(
            "Thank you for using Zyphor OS ❤️\n"
            "Open Source • Community Driven • Built with Consistency"
        )

        footer.setAlignment(Qt.AlignmentFlag.AlignCenter)
        footer.setStyleSheet("""
            color: #9ca3af;
            padding-top: 15px;
        """)

        layout.addWidget(footer)

        main.addWidget(container)