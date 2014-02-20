#-------------------------------------------------
#
# Project created by QtCreator 2014-01-24T16:12:20
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = eros
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        bnetsettingswindow.cpp\
        chatwidget.cpp\
        crashhandler.cpp\
        config.cpp\
        matchmakingplayerinfo.cpp\
        matchmakingsearchprogresswidget.cpp\
        profile.cpp\
        settingswindow.cpp

HEADERS  += main.h\
        mainwindow.h\
        bnetsettingswindow.h\
        chatwidget.h\
        crashhandler.h\
        config.h\
        matchmakingplayerinfo.h\
        matchmakingsearchprogresswidget.h\
        profile.h\
        settingswindow.h




FORMS    += mainwindow.ui\
            settingswindow.ui\
            bnetsettingswindow.ui\
            chatwidget.ui\
            matchmakingplayerinfo.ui\
            matchmakingsearchprogresswidget.ui\


RESOURCES += \
    resources.qrc


OTHER_FILES +=

macx {
    LIBS += -L/usr/local/lib/
    INCLUDEPATH += /usr/local/include/
}
linux {
    INCLUDEPATH += /opt/google-breakpad/src/
    QMAKE_CXXFLAGS += -std=gnu++0x
}
LIBS += -lbreakpad_client -lQSimpleFileWatcher -leros -lprotobuf



