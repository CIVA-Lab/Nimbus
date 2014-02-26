#-------------------------------------------------
#
# Project created by QtCreator 2013-05-23T17:53:50
#
#-------------------------------------------------

QT       += core gui opengl xml

TARGET = Nimbus
TEMPLATE = app
ICON = Nimbus.icns

macx {
INCLUDEPATH *= extern/macx/include
LIBS *= -Lextern/macx/lib -lQGLViewer
}

win32 {
#CONFIG += console
CONFIG += static
DEFINES += QGLVIEWER_STATIC
INCLUDEPATH += extern/win32/include
LIBS += -L../extern/win32/lib -llas -llaszip -lQGLViewer
RC_FILE = Nimbus.rc
}

unix:!macx {
DEFINES += NO_VBLANK_SYNC
INCLUDEPATH += ../local/include
LIBS += -L../local/lib -llas -llaszip -lboost_thread-mt -lQGLViewer
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
    PointGenerator.cpp

HEADERS  += MainWindow.h \
    Viewer.h \
    DisplayOptionsDialog.h \
    CreatePointCloudDialog.h \
    3rdparty/rply/rply.h \
    PointCloud.h \
    PLYLoader.h \
    PointGenerator.h

FORMS    += MainWindow.ui \
    DisplayOptionsDialog.ui \
    CreatePointCloudDialog.ui

OTHER_FILES += \
    Nimbus.rc

RESOURCES += \
    Nimbus.qrc

CONFIG(static):{
  message(Static compile enabled)
  DEFINES += NIMBUS_STATIC
  QTPLUGIN += \
      qico
}


