EXE := willow

SOURCES := src/willow.cpp

CXX := clang++

CXXFLAGS := -O3 -march=native -std=c++20 -ffast-math -flto

LINKER :=

SUFFIX :=

ifeq ($(OS), Windows_NT)
	SUFFIX := .exe
	LINKER := -fuse-ld=lld
else
	SUFFIX :=
	LINKER := -lm -fuse-ld=lld
endif

OUT := $(EXE)$(SUFFIX)


$(EXE): $(SOURCES)
	$(CXX) $^ $(CXXFLAGS) -o $(OUT) $(LINKER) 
