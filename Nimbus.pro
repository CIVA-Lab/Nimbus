QT       += core gui opengl xml

TARGET = Nimbus
TEMPLATE = app
ICON = Nimbus.icns

CONFIG(release)
{
  CONFIG += optimize_full
}

win32 {
#CONFIG += console
RC_FILE = Nimbus.rc
DEFINES += QGLVIEWER_STATIC
LIBS += -lopengl32 -lglu32
}

unix:!macx {
DEFINES += NO_VBLANK_SYNC
LIBS += -lGLU
}

INCLUDEPATH += 3rdparty/rply

SOURCES += main.cpp\
        MainWindow.cpp \
    Viewer.cpp \
    DisplayOptionsDialog.cpp \
    CreatePointCloudDialog.cpp \
    3rdparty/rply/rply.c \
    PointCloud.cpp \
    PLYLoader.cpp \
    PointGenerator.cpp \
    StereoOptionsDialog.cpp \
    InfoDialog.cpp

HEADERS  += MainWindow.h \
    Viewer.h \
    DisplayOptionsDialog.h \
    CreatePointCloudDialog.h \
    3rdparty/rply/rply.h \
    PointCloud.h \
    PLYLoader.h \
    PointGenerator.h \
    StereoOptionsDialog.h \
    InfoDialog.h

FORMS    += MainWindow.ui \
    DisplayOptionsDialog.ui \
    CreatePointCloudDialog.ui \
    StereoOptionsDialog.ui \
    InfoDialog.ui

OTHER_FILES += \
    Nimbus.rc \
    README.md

RESOURCES += \
    Nimbus.qrc

CONFIG(static):{
  message(Static compile enabled)
  DEFINES += NIMBUS_STATIC
  DEFINES += QGLVIEWER_STATIC
}

# libQGLViewer source and headers
DEFINES *= NO_VECTORIAL_RENDER

INCLUDEPATH += 3rdparty

HEADERS *= 3rdparty/QGLViewer/qglviewer.h \
    3rdparty/QGLViewer/camera.h \
    3rdparty/QGLViewer/manipulatedFrame.h \
    3rdparty/QGLViewer/manipulatedCameraFrame.h \
    3rdparty/QGLViewer/frame.h \
    3rdparty/QGLViewer/constraint.h \
    3rdparty/QGLViewer/keyFrameInterpolator.h \
    3rdparty/QGLViewer/mouseGrabber.h \
    3rdparty/QGLViewer/quaternion.h \
    3rdparty/QGLViewer/vec.h \
    3rdparty/QGLViewer/domUtils.h \
    3rdparty/QGLViewer/config.h

SOURCES *= 3rdparty/QGLViewer/qglviewer.cpp \
    3rdparty/QGLViewer/camera.cpp \
    3rdparty/QGLViewer/manipulatedFrame.cpp \
    3rdparty/QGLViewer/manipulatedCameraFrame.cpp \
    3rdparty/QGLViewer/frame.cpp \
    3rdparty/QGLViewer/saveSnapshot.cpp \
    3rdparty/QGLViewer/constraint.cpp \
    3rdparty/QGLViewer/keyFrameInterpolator.cpp \
    3rdparty/QGLViewer/mouseGrabber.cpp \
    3rdparty/QGLViewer/quaternion.cpp \
    3rdparty/QGLViewer/vec.cpp

FORMS *= 3rdparty/QGLViewer/ImageInterface.ui
