TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

CONFIG(debug, debug|release) {
    UI_DIR      = ./tmp/debug/ui
    MOC_DIR     = ./tmp/debug/moc
    OBJECTS_DIR = ./tmp/debug/obj
} else:CONFIG(release, debug|release) {
    UI_DIR      = ./tmp/release/ui
    MOC_DIR     = ./tmp/release/moc
    OBJECTS_DIR = ./tmp/release/obj
}
DESTDIR=bin

INCLUDEPATH += ../src/

SOURCES += \
	main.cpp \
    ../src/Ini.cpp

HEADERS += \
    ../src/Ini.h
