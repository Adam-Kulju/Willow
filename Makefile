EXE := willow

SOURCES := src/willow.cpp

CXX := clang++

CXXFLAGS := -O3 -march=native -std=c++2a -ffast-math -flto

LINKER :=

SUFFIX :=

ifeq ($(OS), Windows_NT)
	DETECTED_OS := Windows
	SUFFIX := .exe
	LINKER :=
else
	DETECTED_OS := $(shell uname -s)
	SUFFIX :=
	LINKER := -lm
	CXXFLAGS += -pthread
endif

ifneq (,$(findstring clang,$(shell $(CXX) --version)))
    ifeq ($(DETECTED_OS), Windows)
        LINKER += -fuse-ld=lld
    endif
endif

OUT := $(EXE)$(SUFFIX)


$(EXE): $(SOURCES)
	$(CXX) $^ $(CXXFLAGS) -o $(OUT) $(LINKER) 
