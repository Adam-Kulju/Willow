EXE := willow

SOURCES := src/willow.cpp

CXX := clang++

CXXFLAGS := -O3 -march=native -std=c++2a -ffast-math -flto

LINKER :=

SUFFIX :=

ifneq ($(OS), Windows_NT)
	SUFFIX := .exe
	LINKER :=
else
	SUFFIX :=
	LINKER := -lm
	CXXFLAGS += -pthread
endif

ifneq (,$(findstring clang,$(shell $(CXX) --version)))
    ifneq ($(DETECTED_OS),Darwin)
        LINKER += -fuse-ld=lld
    endif
endif

OUT := $(EXE)$(SUFFIX)


$(EXE): $(SOURCES)
	$(CXX) $^ $(CXXFLAGS) -o $(OUT) $(LINKER) 
