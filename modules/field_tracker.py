import cv2
import numpy as np
from PyQt6.QtCore import QObject, pyqtSignal

class FieldTracker(QObject):
    # Signal to emit detected corners
    field_detected = pyqtSignal(object)  # Will emit FieldCorners object
    error_occurred = pyqtSignal(str)

    def __init__(self):
        super().__init__()
        self.field_width_m = 6.10  # Standard doubles court width (meters)
        self.field_height_m = 13.40  # Standard doubles court length (meters)
        self.default_world_points = self._calculate_default_world_points()

    def _calculate_default_world_points(self):
        """Calculate the default world coordinates for a badminton court"""
        # Order: TL, TR, BR, BL
        return [
            (0.0, self.field_height_m),  # TL
            (self.field_width_m, self.field_height_m),  # TR
            (self.field_width_m, 0.0),  # BR
            (0.0, 0.0),  # BL
        ]

    def detect_field(self, mask):
        """Detect field corners from a binary mask"""
        # Find contours in the binary mask
        contours, _ = cv2.findContours(mask.clone(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        if not contours:
            return None

        # Find the largest contour (the field)
        largest_contour = max(contours, key=cv2.contourArea)
        area = cv2.contourArea(largest_contour)

        if area < 100:  # Minimum area to be considered a field
            return None

        # Approximate the contour to a polygon
        peri = cv2.arcLength(largest_contour, True)
        approx = cv2.approxPolyDP(largest_contour, 0.02 * peri, True)

        if len(approx) < 4:
            return None

        # Convert to 2D points
        points = [pt[0].astype(float) for pt in approx]

        # Simple heuristic: sort by y-coordinate to get top and bottom points
        points.sort(key=lambda p: p[1])  # Sort by y-coordinate

        # Take the top 2 and bottom 2 points
        top_points = points[:2]
        bottom_points = points[2:]

        # Sort top and bottom points by x-coordinate
        top_points.sort(key=lambda p: p[0])
        bottom_points.sort(key=lambda p: p[0])

        # Order: TL, TR, BR, BL
        ordered_points = [
            top_points[0],  # TL
            top_points[1],  # TR
            bottom_points[1],  # BR
            bottom_points[0],  # BL
        ]

        # Calculate homography if we have valid points
        if len(ordered_points) == 4:
            image_points = np.array(ordered_points, dtype=np.float32)
            world_points = np.array(self.default_world_points, dtype=np.float32)

            H, mask = cv2.findHomography(image_points, world_points, cv2.RANSAC, 5.0)

            if H is not None and H.shape == (3, 3):
                # Create FieldCorners object
                corners = FieldCorners(
                    image_points=image_points,
                    world_points=world_points,
                    homography=H,
                    valid=True
                )
                return corners

        return None

    def map_to_world(self, image_point, homography):
        """Map a single image point to world coordinates using homography"""
        if homography is None or not homography.shape == (3, 3):
            return None

        # Convert to homogeneous coordinates
        point = np.array([[image_point[0], image_point[1], 1]], dtype=np.float32)

        # Apply homography
        world_point = np.dot(homography, point.T)

        # Convert back to Cartesian coordinates
        world_point = world_point / world_point[2]

        return (world_point[0], world_point[1])

    def set_field_dimensions(self, width_m, height_m):
        """Update the field dimensions"""
        self.field_width_m = width_m
        self.field_height_m = height_m
        self.default_world_points = self._calculate_default_world_points()

    def auto_detect_field_color(self, frame):
        """Simple auto-detection of field color from the frame"""
        # Convert to HSV for better color detection
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

        # Define HSV ranges for badminton court colors (typically green or blue)
        # These ranges can be adjusted
        lower_green = np.array([40, 50, 50])
        upper_green = np.array([80, 255, 255])

        lower_blue = np.array([100, 50, 50])
        upper_blue = np.array([130, 255, 255])

        # Create masks
        green_mask = cv2.inRange(hsv, lower_green, upper_green)
        blue_mask = cv2.inRange(hsv, lower_blue, upper_blue)

        # Combine masks
        combined_mask = cv2.bitwise_or(green_mask, blue_mask)

        # Find contours in the mask
        contours, _ = cv2.findContours(combined_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        if contours:
            # Return the largest contour
            largest_contour = max(contours, key=cv2.contourArea)

            # Get bounding rectangle
            x, y, w, h = cv2.boundingRect(largest_contour)

            # Extract region of interest
            roi = frame[y:y + h, x:x + w]

            # Calculate average HSV value for the ROI
            roi_hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
            avg_hsv = np.mean(roi_hsv, axis=(0, 1))

            # Return the average HSV values
            return avg_hsv

        return None

class FieldCorners:
    """Class to store field corner information"""
    def __init__(self, image_points, world_points, homography, valid):
        self.image_points = image_points
        self.world_points = world_points
        self.homography = homography
        self.valid = valid

    def __repr__(self):
        return f"FieldCorners(valid={self.valid}, corners={len(self.image_points)})"