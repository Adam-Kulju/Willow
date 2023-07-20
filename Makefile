EXE := willow

SOURCES := src/willow.cpp

CXX := clang++

CXXFLAGS := -O3 -march=native -std=c++20 -ffast-math -flto

LINKER := -fuse-ld=lld

SUFFIX :=

ifeq ($(OS), Windows_NT)
	SUFFIX := .exe
	LINKER := 
else
	SUFFIX :=
	LINKER := -lm
endif

OUT := $(EXE)$(SUFFIX)


$(EXE): $(SOURCES)
	$(CXX) $^ $(CXXFLAGS) -o $(OUT) $(LINKER) 
