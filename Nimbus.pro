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
LIBS *= -framework QGLViewer
}

unix:!macx {
INCLUDEPATH += extern/QGLViewer/include
LIBS += -Lextern/QGLViewer/lib -lQGLViewer
}

INCLUDEPATH += 3rdparty/rply
INCLUDEPATH += extern/laslib/include

LIBS += -Lextern/laslib/lib -llas

SOURCES += main.cpp\
        MainWindow.cpp \
    Viewer.cpp \
    DisplayOptionsDialog.cpp \
    CreatePointCloudDialog.cpp \
    3rdparty/rply/rply.c \
    PointCloud.cpp \
    PLYLoader.cpp \
    LASLoader.cpp

HEADERS  += MainWindow.h \
    Viewer.h \
    DisplayOptionsDialog.h \
    CreatePointCloudDialog.h \
    3rdparty/rply/rply.h \
    PointCloud.h \
    PLYLoader.h \
    LASLoader.h

FORMS    += MainWindow.ui \
    DisplayOptionsDialog.ui \
    CreatePointCloudDialog.ui














