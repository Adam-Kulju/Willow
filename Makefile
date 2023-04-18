EXE := willow

SOURCES := src/willow.c

CXX := gcc

CXXFLAGS := -O3 -march=native

LDFLAGS :=

SUFFIX :=

ifeq ($(OS), Windows_NT)
	SUFFIX := .exe
	LDFLAGS := -lm
else
	SUFFIX :=
endif

OUT := $(EXE)$(SUFFIX)

all: $(EXE)

($EXE): ($SOURCES)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(OUT) $^
