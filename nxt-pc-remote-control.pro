TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    main.cpp

HEADERS += \
    window.h \
    network.h \
    idiom.h

RESOURCES += \
    resources.qrc

LIBS += -lbluetooth

