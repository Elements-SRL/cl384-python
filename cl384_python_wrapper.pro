TEMPLATE = lib
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

TARGET = cl384_python_wrapper

SOURCES += \
        main.cpp

#DEFINES += PYTHON_3_10
DEFINES += PYTHON_3_11

contains(DEFINES, PYTHON_3_10) {
    PYTHON_PATH = $$(PYTHON_3_10_7_PATH)
    PYTHON_LIB = python310
}

contains(DEFINES, PYTHON_3_11) {
    PYTHON_PATH = $$(PYTHON_3_11_7_PATH)
    PYTHON_LIB = python311
}

include($$(E384COMMLIB_PATH)includecommlib.pri)

INCLUDEPATH += "$$PYTHON_PATH\include" \
        $$(PYBIND_11_PATH)include

LIBS += -L"$$PYTHON_PATH\libs" -l$$PYTHON_LIB
