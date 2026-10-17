import cv2
import numpy as np
from PyQt6.QtCore import QThread, pyqtSignal

class YoloTracker(QThread):
    """YOLOv8n player detection with max 4 players and simple ID tracking"""
    detection_ready = pyqtSignal(object)  # Emits list of Detection objects
    error_occurred = pyqtSignal(str)

    MAX_PLAYERS = 4
    CONFIDENCE_THRESHOLD = 0.5
    NMS_THRESHOLD = 0.4

    def __init__(self, model_path="yolov8n.pt"):
        super().__init__()
        self.model_path = model_path
        self.running = False
        self.model = None
        self.player_tracker = PlayerTracker(max_players=self.MAX_PLAYERS)
        self.confidence_threshold = self.CONFIDENCE_THRESHOLD
        self.nms_threshold = self.NMS_THRESHOLD

    def run(self):
        """Main thread loop - runs YOLO detection"""
        try:
            # Lazy import ultralytics
            try:
                from ultralytics import YOLO
                self.model = YOLO(self.model_path)
            except ImportError:
                self.error_occurred.emit("ultralytics nicht installiert. Führe: pip install ultralytics")
                return
            except Exception as e:
                self.error_occurred.emit(f"YOLO Modell konnte nicht geladen werden: {str(e)}")
                return

            self.running = True
            while self.running:
                # YOLO detection is triggered by detection_requested signal
                # This thread waits for frames
                self.msleep(100)

        except Exception as e:
            self.error_occurred.emit(f"YOLO Tracker Fehler: {str(e)}")

    def detect(self, frame):
        """Run YOLO detection on a frame"""
        if self.model is None:
            return []

        try:
            results = self.model(frame, verbose=False)
            detections = []

            for r in results:
                boxes = r.boxes
                if boxes is None:
                    continue
                for box in boxes:
                    cls = int(box.cls[0])
                    conf = float(box.conf[0])
                    if cls == 0 and conf >= self.confidence_threshold:  # class 0 = person
                        x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
                        detections.append(Detection(
                            box=(int(x1), int(y1), int(x2), int(y2)),
                            confidence=conf,
                            class_id=cls
                        ))

            # Apply NMS and limit to max players
            detections = self._apply_nms(detections)
            detections = self.player_tracker.update(detections)

            self.detection_ready.emit(detections)
            return detections

        except Exception as e:
            self.error_occurred.emit(f"YOLO Detection Fehler: {str(e)}")
            return []

    def _apply_nms(self, detections):
        """Apply Non-Maximum Suppression"""
        if not detections:
            return detections

        boxes = np.array([[d.box[0], d.box[1], d.box[2], d.box[3]] for d in detections])
        scores = np.array([d.confidence for d in detections])
        indices = cv2.dnn.NMSBoxes(boxes.tolist(), scores.tolist(),
                                   self.confidence_threshold, self.nms_threshold)

        if len(indices) == 0:
            return []

        return [detections[i[0]] for i in indices]

    def stop(self):
        """Stop the tracker"""
        self.running = False
        self.wait()


class Detection:
    """Single detection result"""
    def __init__(self, box, confidence, class_id=0):
        self.box = box  # (x1, y1, x2, y2)
        self.confidence = confidence
        self.class_id = class_id
        self.id = None  # Set by player tracker


class PlayerTracker:
    """Simple ID tracker for max 4 players"""
    def __init__(self, max_players=4):
        self.max_players = max_players
        self.players = {}
        self.next_id = 1

    def update(self, detections):
        """Match detections to existing players"""
        matched = []
        used_ids = set()

        for det in detections:
            # Find closest existing player (simple centroid matching)
            det_center = ((det.box[0] + det.box[2]) / 2,
                         (det.box[1] + det.box[3]) / 2)

            best_id = None
            best_dist = float('inf')

            for pid, player in self.players.items():
                if pid in used_ids:
                    continue
                dist = np.sqrt((det_center[0] - player['center'][0])**2 +
                               (det_center[1] - player['center'][1])**2)
                if dist < best_dist and dist < 100:  # Max matching distance
                    best_dist = dist
                    best_id = pid

            if best_id is not None:
                det.id = best_id
                self.players[best_id] = {'center': det_center, 'box': det.box}
                used_ids.add(best_id)
            elif len(self.players) < self.max_players:
                det.id = self.next_id
                self.players[self.next_id] = {'center': det_center, 'box': det.box}
                used_ids.add(self.next_id)
                self.next_id += 1

            matched.append(det)

        return matched[:self.max_players]