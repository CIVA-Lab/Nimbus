#-------------------------------------------------
#
# Project created by QtCreator 2013-05-23T17:53:50
#
#-------------------------------------------------

QT       += core gui opengl xml

TARGET = Nimbus
TEMPLATE = app
ICON = Nimbus.icns

LIBS *= -framework QGLViewer

INCLUDEPATH += 3rdparty/rply

SOURCES += main.cpp\
        MainWindow.cpp \
    Viewer.cpp \
    DisplayOptionsDialog.cpp \
    CreatePointCloudDialog.cpp \
    3rdparty/rply/rply.c

HEADERS  += MainWindow.h \
    Viewer.h \
    DisplayOptionsDialog.h \
    CreatePointCloudDialog.h \
    3rdparty/rply/rply.h

FORMS    += MainWindow.ui \
    DisplayOptionsDialog.ui \
    CreatePointCloudDialog.ui








