QT       += core gui serialport printsupport multimediawidgets concurrent
TARGET = uv
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    json_utils.cpp \
    main.cpp \
    main_window.cpp \
    qcustomplot.cpp \
    qrc_files_restorer.cpp \
    stm_spectrometr.cpp

HEADERS += \
    json_utils.h \
    main_window.h \
    qcustomplot.h \
    qrc_files_restorer.h \
    stm_spectrometr.h

FORMS += \
    main_window.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
