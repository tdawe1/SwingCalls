# Sierra Chart ACSIL build (same flags as SC remote MinGW).
#   make
#   make install   # copies DLL into the SC Data folder

CXX      ?= x86_64-w64-mingw32-g++
SC_ROOT  ?= $(HOME)/.wine/drive_c/SierraChart
SC_INC   ?= $(SC_ROOT)/ACS_Source
SC_DATA  ?= $(SC_ROOT)/Data

CXXFLAGS ?= -D_WIN64 -D_WIN32_WINNT=0x0601 -march=x86-64 -mtune=x86-64 \
            -O2 -shared -static -static-libgcc -static-libstdc++ \
            -s -fexceptions -std=gnu++17 -Wno-deprecated -I$(SC_INC)
LIBS     ?= -lgdi32

TARGET   := SwingCalls_64.dll
SRC      := SwingCalls.cpp

.PHONY: all install clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LIBS)

install: $(TARGET)
	cp -f $(TARGET) $(SC_DATA)/$(TARGET)
	cp -f $(SRC) $(SC_INC)/$(SRC)

clean:
	rm -f $(TARGET)
