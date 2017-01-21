#! /usr/bin/python
# -*- encoding: UTF-8 -*-
#
# This is a tiny example that shows how to show live images from Nao using PyQt.
# You must have python-qt4 installed on your system.
#

import sys

from PyQt4.QtGui import QWidget, QImage, QApplication, QPainter, QPen
from PyQt4.QtCore import Qt, QPoint
from naoqi import ALProxy, ALModule

# To get the constants relative to the video.
import vision_definitions

class ImageWidget(QWidget):
    """
    Tiny widget to display camera images from Naoqi.
    """
    def __init__(self, IP, PORT, CameraID, parent=None):
        """
        Initialization.
        """
        QWidget.__init__(self, parent)
        self._image = QImage()
        self.setWindowTitle('Nao')

        self._imgWidth = 160
        self._imgHeight = 120
        self._cameraID = CameraID
        self.resize(self._imgWidth, self._imgHeight)
        
        # Pico detection
        self._picoDetection = None
        self._picoFlag = False

        # Proxys.
        self._videoProxy = None
        self._picoProxy = None

        # Our video module name.
        self._imgClient = ""

        # This will contain this alImage we get from Nao.
        self._alImage = None

        self._register(IP, PORT)

        # Trigget 'timerEvent' every 100 ms.
        self.startTimer(100)

    def _register(self, IP, PORT):        
        """
        Register our module to the memory event.
        """
        self._memoryProxy = ALProxy("ALMemory", IP, PORT)
        
        """
        Register our module to the pico module.
        """
        self._picoProxy = ALProxy("PicoModule", IP, PORT)
        self._picoProxy.call("addClassifier", "face", "/home/mirko/Devel/ADORE/PicoClass/pico/rnt/cascades/facefinder", -1, -1, -1 ,-1, -1)
        self._picoProxy.call("setActiveCamera",0)
        self._picoProxy.call("subscribe", "mirko")

        """
        Register our module to the video device.
        """
        self._videoProxy = ALProxy("ALVideoDevice", IP, PORT)
        resolution = vision_definitions.kQQVGA  # 160 * 120
        colorSpace = vision_definitions.kRGBColorSpace
        self._imgClient = self._videoProxy.subscribe("_client", resolution, colorSpace, 5)

        # Select camera.
        self._videoProxy.setParam(vision_definitions.kCameraSelectID, self._cameraID)
        
        
    def _unregister(self):
        """
        Unregister our naoqi module from everything.
        """
        if self._imgClient != "":
            self._videoProxy.unsubscribe(self._imgClient)
            
        self._picoProxy.unsubscribe("mirko")
        self._picoProxy.call("removeClassifier", "face")

    def paintEvent(self, event):
        """
        Draw the QImage on screen.
        """
        painter = QPainter(self)
        painter.drawImage(painter.viewport(), self._image)
        if self._picoFlag or True:
            painter.setPen(QPen(Qt.red))
            for k in range(4, len(self._picoDetection)) :
                x = self._picoDetection[k][1]
                y = self._picoDetection[k][2]
                r = self._picoDetection[k][3]/2
                x_s = float(self._picoDetection[2]) / self.size().width()
                y_s = float(self._picoDetection[3]) / self.size().height()
                painter.drawEllipse(QPoint(x/x_s, y/y_s), r/x_s, r/y_s)
                


    def _updateImage(self):
        """
        Retrieve a new image from Nao.
        """
        self._alImage = self._videoProxy.getImageRemote(self._imgClient)
        self._image = QImage(self._alImage[6],           # Pixel array.
                             self._alImage[0],           # Width.
                             self._alImage[1],           # Height.
                             QImage.Format_RGB888)
        temp = self._memoryProxy.getData("picoDetections")
        if temp == self._picoDetection :
            self._picoFlag = False
        else :
            if self._picoDetection != None : 
                self._picoFlag = True
            self._picoDetection = temp


    def timerEvent(self, event):
        """
        Called periodically. Retrieve a nao image, and update the widget.
        """
        self._updateImage()
        self.update()


    def __del__(self):
        """
        When the widget is deleted, we unregister our naoqi video module.
        """
        self._unregister()



if __name__ == '__main__':
    IP = "169.254.28.144"  # Replace here with your NaoQi's IP address.
    PORT = 9559
    CameraID = 0

    # Read IP address from first argument if any.
    if len(sys.argv) > 1:
        IP = sys.argv[1]

    # Read CameraID from second argument if any.
    if len(sys.argv) > 2:
        CameraID = int(sys.argv[2])


    app = QApplication(sys.argv)
    myWidget = ImageWidget(IP, PORT, CameraID)
    myWidget.show()
    sys.exit(app.exec_())
