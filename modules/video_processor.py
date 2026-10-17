import cv2
from PyQt6.QtCore import QThread, pyqtSignal
import numpy as np

class VideoProcessor(QThread):
    # Signal to emit frames
    frame_ready = pyqtSignal(np.ndarray)
    error_occurred = pyqtSignal(str)

    def __init__(self, source=0):
        super().__init__()
        self.source = source  # 0 for webcam, or path to video file
        self.running = False
        self.cap = None

    def run(self):
        """Main thread loop - captures frames and emits them"""
        try:
            self.cap = cv2.VideoCapture(self.source)
            if not self.cap.isOpened():
                self.error_occurred.emit(f"Cannot open video source: {self.source}")
                return

            self.running = True
            while self.running:
                ret, frame = self.cap.read()
                if not ret:
                    # For video files, loop back to start
                    if isinstance(self.source, str) and self.source != 0:
                        self.cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                        continue
                    else:
                        self.error_occurred.emit("Failed to grab frame")
                        break

                # Emit the frame
                self.frame_ready.emit(frame)

                # Small delay to prevent overwhelming the GUI
                self.msleep(30)  # ~30 FPS

        except Exception as e:
            self.error_occurred.emit(f"Video processor error: {str(e)}")
        finally:
            if self.cap:
                self.cap.release()

    def stop(self):
        """Stop the video capture thread"""
        self.running = False
        self.wait()  # Wait for thread to finish

    def isOpened(self):
        """Check if video source is opened"""
        return self.cap is not None and self.cap.isOpened()