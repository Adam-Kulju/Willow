EXE := willow

SOURCES := src/willow.cpp

CXX := clang++

CXXFLAGS := -O3 -march=native -std=c++2a -ffast-math -flto

LINKER :=

SUFFIX :=

ifeq ($(OS), Windows_NT)
	SUFFIX := .exe
	LINKER :=
else
	SUFFIX :=
	LINKER := -lm
endif

ifneq (,$(findstring clang,$(shell $(CXX) --version)))
	LINKER += -fuse-ld=lld
endif

OUT := $(EXE)$(SUFFIX)


$(EXE): $(SOURCES)
	$(CXX) $^ $(CXXFLAGS) -o $(OUT) $(LINKER) 
