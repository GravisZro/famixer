TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp \
    other.c \
    fuse_mixer.cpp \
    mixer_interface.cpp \
    alsapp/kernel/pcm_interface.cpp \
    alsapp/kernel/midi_interface.cpp \
    alsapp/kernel/controller_interface.cpp \
    alsapp/kernel/timer_interface.cpp \
    alsapp/kernel/hwdep_interface.cpp

LIBS += -lfuse
LIBS += -lasound

QMAKE_CFLAGS += -D_FILE_OFFSET_BITS=64
QMAKE_CXXFLAGS +=  -std=c++14 $$QMAKE_CFLAGS

HEADERS += \
    errno_t.h \
    fuse_mixer.h \
    mixer_interface.h \
    alsapp/kernel/kernel_interface.h \
    alsapp/kernel/pcm_interface.h \
    alsapp/kernel/midi_interface.h \
    alsapp/kernel/controller_interface.h \
    alsapp/kernel/timer_interface.h \
    alsapp/kernel/hwdep_interface.h \
    alsapp/fs_permissions.h
