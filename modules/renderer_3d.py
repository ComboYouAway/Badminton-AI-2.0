import numpy as np
from PyQt6.QtWidgets import QWidget, QVBoxLayout, QGridLayout, QPushButton
from PyQt6.QtCore import Qt, pyqtSignal
from PyQt6.QtGui import QPaintEvent, QResizeEvent, QMouseEvent
from PyQt6.QtOpenGL import QOpenGLWidget
from PyQt6.QtOpenGLWidgets import QOpenGLWidget
from PyQt6.QtCore import QCoreApplication
from PyQt6.QtGui import QSurfaceFormat
from OpenGL import GL

class Renderer3D(QOpenGLWidget):
    # Signal when field lines change
    field_lines_changed = pyqtSignal(list)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.field_lines = []
        self.players = []
        self.zoom = 1.0
        self.rotation = 0.0
        self.top_view = False

    def set_field_lines(self, lines):
        """Set the field lines (list of 3D points)"""
        self.field_lines = lines
        self.update()

    def set_players(self, players):
        """Set the player positions (list of 3D positions with colors)"""
        self.players = players
        self.update()

    def set_zoom(self, value):
        """Set zoom level"""
        self.zoom = max(0.1, value)
        self.update()

    def set_rotation(self, value):
        """Set rotation angle"""
        self.rotation = value
        self.update()

    def set_top_view(self, top):
        """Toggle top-down view"""
        self.top_view = top
        self.update()

    def initializeGL(self):
        """Initialize OpenGL"""
        GL.glEnable(GL.GL_DEPTH_TEST)
        GL.glClearColor(0.1, 0.1, 0.1, 1.0)

    def resizeGL(self, w, h):
        """Handle resize"""
        GL.glViewport(0, 0, w, h)

    def paintGL(self):
        """Render the 3D scene"""
        GL.glClear(GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT)
        GL.glPushMatrix()

        # Apply camera transform
        if self.top_view:
            # Top-down view
            GL.glRotatef(0, 1, 0, 0)
        else:
            GL.glRotatef(self.rotation, 0, 1, 0)

        GL.glScalef(self.zoom, self.zoom, self.zoom)

        # Draw field lines
        GL.glBegin(GL.GL_LINES)
        GL.glColor3f(1.0, 1.0, 1.0)
        for i in range(0, len(self.field_lines) - 1, 2):
            p1 = self.field_lines[i]
            p2 = self.field_lines[i + 1]
            GL.glVertex3f(p1[0], p1[1], p1[2])
            GL.glVertex3f(p2[0], p2[1], p2[2])
        GL.glEnd()

        # Draw players as small colored spheres
        for player in self.players:
            pos = player.get('position', [0, 0, 0])
            color = player.get('color', [1.0, 0.0, 0.0])
            self.draw_sphere(pos, 0.15, color)

        GL.glPopMatrix()

    def draw_sphere(self, center, radius, color, segments=8):
        """Draw a simple sphere"""
        GL.glBegin(GL.GL_LINE_STRIP)
        GL.glColor3f(color[0], color[1], color[2])
        for i in range(segments + 1):
            theta = 2 * np.pi * i / segments
            for j in range(segments + 1):
                phi = np.pi * j / segments
                x = radius * np.sin(phi) * np.cos(theta)
                y = radius * np.cos(phi)
                z = radius * np.sin(phi) * np.sin(theta)
                GL.glVertex3f(center[0] + x, center[1] + y, center[2] + z)
        GL.glEnd()

    def mousePressEvent(self, event):
        """Handle mouse press for rotation"""
        self.last_mouse_pos = event.position()

    def mouseMoveEvent(self, event):
        """Handle mouse move for rotation"""
        if self.last_mouse_pos is not None:
            delta = event.position() - self.last_mouse_pos
            self.rotation += delta.x() * 0.5
            self.update()
        self.last_mouse_pos = event.position()

    def wheelEvent(self, event):
        """Handle mouse wheel for zoom"""
        delta = event.angleDelta().y() / 120.0
        self.zoom = max(0.1, min(10.0, self.zoom + delta * 0.1))
        self.update()