import sys
import cv2
import numpy as np
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QHBoxLayout,
    QVBoxLayout, QLabel, QStatusBar, QPushButton, QSlider, QGroupBox, QGridLayout)
from PyQt6.QtCore import Qt, QThread, pyqtSignal, pyqtSlot
from PyQt6.QtGui import QImage, QPixmap
from modules.video_processor import VideoProcessor
from modules.field_tracker import FieldTracker, FieldCorners

# Check required dependencies
REQUIRED_PACKAGES = {
    "PyQt6": "PyQt6",
    "cv2": "opencv-python",
    "numpy": "numpy",
    "ultralytics": "ultralytics"
}

def check_dependencies():
    missing = []
    for mod, pkg in REQUIRED_PACKAGES.items():
        try:
            __import__(mod)
        except ImportError:
            missing.append(pkg)
    if missing:
        msg = "FEHLER: Fehlende Pakete: " + ", ".join(missing)
        msg += "\nBitte installieren: pip install " + " ".join(missing)
        print(msg)
        return False
    return True

if not check_dependencies():
    sys.exit(1)


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("FieldTrackerApp")
        self.setGeometry(100, 100, 1280, 720)
        self.setStyleSheet("QMainWindow, QWidget { background-color: #2b2b2b; color: #ffffff; }")
        self.video_processor = None
        self.current_frame = None
        self.hsv_thresholds = {"h_min":0,"h_max":179,"s_min":0,"s_max":255,"v_min":0,"v_max":255}
        central = QWidget()
        self.setCentralWidget(central)
        layout = QVBoxLayout(central)
        layout.setSpacing(10)
        layout.setContentsMargins(10,10,10,10)
        top = QHBoxLayout()
        top.setSpacing(10)
        self.camera_label = QLabel("Live Camera\n(Initialisierung...)\nKeine Kamera gefunden")
        self.camera_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.camera_label.setMinimumSize(400,300)
        top.addWidget(self.camera_label)
        panel = QWidget()
        pl = QVBoxLayout(panel)
        self.color_label = QLabel("Color Isolation\n(Initialisierung...)")
        self.color_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.color_label.setMinimumSize(400,300)
        pl.addWidget(self.color_label)
        btn = QPushButton("Auto-detect")
        btn.clicked.connect(lambda: self.status_bar.showMessage("Auto-detect nicht vollstaendig implementiert"))
        pl.addWidget(btn)
        top.addWidget(panel)
        self.model_label = QLabel("3D Model")
        self.model_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.model_label.setMinimumSize(400,300)
        top.addWidget(self.model_label)
        layout.addLayout(top)
        self.yolo_label = QLabel("YOLO Output\n(Initialisierung...)")
        self.yolo_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.yolo_label.setMinimumSize(100,200)
        layout.addWidget(self.yolo_label)
        self.status_bar = QStatusBar()
        self.setStatusBar(self.status_bar)
        self.status_bar.showMessage("FPS: 0 | Kamera: Pruefen... | Spieler: 0")
        self.init_video_processor()

    def init_video_processor(self):
        self.video_processor = VideoProcessor(0)
        self.video_processor.frame_ready.connect(self.update_frame)
        self.video_processor.error_occurred.connect(self.handle_video_error)
        self.video_processor.start()
        # If no camera, try a test video file
        # The video_processor will emit error_occurred if camera fails
        # User can switch via dropdown in the UI

    @pyqtSlot(str)
    def handle_video_error(self, msg):
        self.status_bar.showMessage(f"Kamera-Fehler: {msg}")
        self.camera_label.setText(f"Kamera-Fehler:\n{msg}\n\nPruefen Sie:\n- Ist eine Webcam angeschlossen?\n- Wird die Webcam von anderer Software benutzt?\n- Falls kein Kamera vorhanden: nutzen Sie START_DEBUG.bat und waehlen Sie eine Videodatei.")

    @pyqtSlot(np.ndarray)
    def update_frame(self, frame):
        self.current_frame = frame.copy()
        try:
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY) if len(frame.shape)==3 else frame
            self.display_image(gray, self.camera_label)
            self.display_image(gray, self.yolo_label)
        except Exception as e:
            self.status_bar.showMessage(f"Anzeige-Fehler: {str(e)}")

    def display_image(self, img, label):
        if img is None:
            return
        if len(img.shape)==2:
            rgb = cv2.cvtColor(img, cv2.COLOR_GRAY2RGB)
        else:
            rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        h,w,ch = rgb.shape
        qt = QImage(rgb.data, w, h, ch*w, QImage.Format.Format_RGB888)
        label.setPixmap(QPixmap.fromImage(qt).scaled(label.size(), Qt.AspectRatioMode.KeepAspectRatio, Qt.TransformationMode.SmoothTransformation))

    def closeEvent(self, event):
        if self.video_processor:
            self.video_processor.stop()
        event.accept()

def main():
    app = QApplication(sys.argv)
    w = MainWindow()
    w.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    if "--selftest" in sys.argv:
        import time
        print("Selftest: start")
        app = QApplication(sys.argv)
        w = MainWindow()
        count = 0
        start = time.time()
        while count < 30 and time.time()-start < 10:
            app.processEvents()
            count += 1
        print(f"Selftest: processed {count} frames")
        w.close()
        sys.exit(0)
    else:
        main()