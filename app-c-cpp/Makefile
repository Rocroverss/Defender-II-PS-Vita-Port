TARGET      := defender2_vita
TITLE       := Defender II Vita Port
TITLEID     := DDEF00001
VERSION     := 01.00

SOURCES     := $(wildcard src/*.cpp src/engine/*.cpp src/game/*.cpp src/scenes/*.cpp)
OBJS        := $(SOURCES:.cpp=.o)

INCLUDES    := -Iinclude
LIBS        := -lvitagl -lSceGxm_stub -lSceDisplay_stub -lSceCtrl_stub -lSceTouch_stub -lSceLibKernel_stub -lm -lstdc++

CFLAGS      := -Wl,-q -O2 $(INCLUDES)
CXXFLAGS    := $(CFLAGS) -std=c++17 -fno-rtti
ASFLAGS     := $(CFLAGS)

EXTRA_TARGETS   := eboot.bin
PSP2SDK         := $(VITASDK)/arm-vita-eabi

include $(PSP2SDK)/include/psp2/common/makefile.psp2
