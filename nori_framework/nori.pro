SOURCES += src/common.cpp \
	src/object.cpp \
	src/proplist.cpp \
	src/isotropic.cpp \
	src/scene.cpp \
        src/random.cpp \
	src/ao.cpp \
        src/area.cpp \
	src/mesh.cpp \
	src/kdtree.cpp \
	src/obj.cpp \
	src/perspective.cpp \
	src/rfilter.cpp \
	src/block.cpp \
	src/bitmap.cpp \
	src/parser.cpp \
	src/mirror.cpp \
	src/medium.cpp \
	src/independent.cpp \
	src/main.cpp \
	src/gui.cpp \
        src/designer.cpp \
        src/designer/meshentry.cpp \
        src/designer/viewer.cpp \
    src/dielectric.cpp

# EX1
SOURCES += $$PWD/src/depth.cpp

# EX2
HEADERS += $$PWD/include/charts/qcustomplot.h
SOURCES += $$PWD/src/charts/qcustomplot.cpp \
           $$PWD/src/directional.cpp \
           $$PWD/src/naive.cpp \
           $$PWD/src/diffuse.cpp \
           $$PWD/src/phong.cpp \
           $$PWD/src/hemisampling.cpp \
           $$PWD/src/variance.cpp

# EX3
SOURCES += $$PWD/src/light_integrator.cpp
            
HEADERS += $$PWD/include/nori/*.h \
   include/nori/gui.h \
   $$PWD/include/nori/designer/*.h

RESOURCES += data/resources.qrc

FORMS += src/designer.ui

include(acg_exercise.pri)

INCLUDEPATH += $$PWD/include \

DEPENDPATH += include data
OBJECTS_DIR = build
RCC_DIR = build
MOC_DIR = build
UI_DIR = build
DESTDIR = .

debug{
        OBJECTS_DIR = build_debug
        RCC_DIR = build_debug
        MOC_DIR = build_debug
        UI_DIR = build_debug
        DESTDIR = bin-debug
}
QT += xml xmlpatterns opengl

unix:macx {
        message(Mac OS)
        OBJECTIVE_SOURCES += src/support_osx.m
        QMAKE_LFLAGS += -framework Cocoa -lobjc
        QMAKE_CXXFLAGS_X86_64 += -mmacosx-version-min=10.7
}

unix:!mac {
        message(Linux)
        MACHINE = $$system(uname -m)
        contains(MACHINE, x86_64) {
            message(64bit)
            INCLUDEPATH += $$PWD/include/OpenEXR
            QMAKE_RPATHDIR = += $$PWD/lib
            LIBS += -L$$PWD/lib
            LIBS += -lIex-2_0
        }
        else {
            message(32bit)
            QMAKE_CXXFLAGS += -DEIGEN_DONT_ALIGN
        }
}

unix {
        QMAKE_CXXFLAGS_RELEASE += -O3 -march=nocona -msse2 -mfpmath=sse -fstrict-aliasing -DNDEBUG #-Wno-enum-compare -Wno-unused-local-typedefs
        #Use homebrew or macport to install OpenEXR on MacOSX, use the package manager on Linux
        QMAKE_LIBDIR += /usr/local/lib
        INCLUDEPATH += /usr/local/include/OpenEXR
        QMAKE_LIBDIR += /usr/lib
        INCLUDEPATH += /usr/include/OpenEXR
        QMAKE_LIBDIR += /opt/local/lib
        INCLUDEPATH += /opt/local/include/OpenEXR
        LIBS += -lIlmImf -lIex
        # Remove if you have Boost >=1.49
        INCLUDEPATH += $$PWD/include/boost1.49_min
}

win32 {
        message(Windows)
        # You will have to update the following three lines based on where you have installed OpenEXR
        QMAKE_LIBDIR += ./openexr/lib/x64
        INCLUDEPATH += ./openexr/include
        LIBPATH += C:\your\path\to\nori\openexr\lib\i386

        INCLUDEPATH += ./include/boost1.49_min

        QMAKE_CXXFLAGS += /O2 /fp:fast /GS- /GL /D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS
        QMAKE_LDFLAGS += /LTCG
        SOURCES += src/support_win32.cpp

        LIBS += IlmImf.lib Iex.lib IlmThread.lib Imath.lib Half.lib
}

TARGET = nori
CONFIG += console
CONFIG -= app_bundle
