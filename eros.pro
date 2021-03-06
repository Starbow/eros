#-------------------------------------------------
#
# Project created by QtCreator 2014-01-24T16:12:20
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = eros
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        bnetsettingswindow.cpp\
        chatwidget.cpp\
        config.cpp\
        matchmakingplayerinfo.cpp\
        matchmakingsearchprogresswidget.cpp\
        profile.cpp\
        settingswindow.cpp\
        erostitlebar.cpp\
        util.cpp\
        directorywatcher.cpp

HEADERS  += mainwindow.h\
        bnetsettingswindow.h\
        chatwidget.h\
        config.h\
        matchmakingplayerinfo.h\
        matchmakingsearchprogresswidget.h\
        profile.h\
        settingswindow.h\
        erostitlebar.h\
        util.h\
        directorywatcher.h




FORMS    += mainwindow.ui\
            settingswindow.ui\
            bnetsettingswindow.ui\
            chatwidget.ui\
            matchmakingplayerinfo.ui\
            matchmakingsearchprogresswidget.ui\
            erostitlebar.ui

RESOURCES += \
    resources.qrc


OTHER_FILES +=

unix {
    LIBS += -L/usr/local/lib/
    INCLUDEPATH += /usr/local/include/
}
macx {
    ICON = res/img/client/icons/eros.icns
    QMAKE_INFO_PLIST = res/data/info.plist
}
linux {
    INCLUDEPATH += /opt/google-breakpad/src/
    QMAKE_CXXFLAGS += -std=gnu++0x
}
!macx {
    LIBS += -lbreakpad_client
    SOURCES += crashhandler.cpp
    HEADERS += crashhandler.h
}


LIBS += -leros -lprotobuf
CONFIG += c++11

TRANSLATIONS += translations/eros_en_GB.ts\
                translations/eros_de_DE.ts\
                translations/eros_ko_KR.ts\
                translations/eros_es_419.ts\
				translations/eros_es_ES.ts\
				translations/eros_fr_FR.ts\
                translations/eros_blank.ts