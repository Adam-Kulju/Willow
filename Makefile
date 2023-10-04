EXE := willow

SOURCES := src/willow.cpp

CXX := clang++

CXXFLAGS := -O3 -march=native -std=c++2a -ffast-math -flto

LINKER :=

SUFFIX :=

ifneq ($(OS), Darwin)
	SUFFIX := .exe
	LINKER :=
else
	SUFFIX :=
	LINKER := -lm
	CXXFLAGS += -pthread
endif

ifneq (,$(findstring clang,$(shell $(CXX) --version)))
	LINKER += -fuse-ld=lld
endif

OUT := $(EXE)$(SUFFIX)


$(EXE): $(SOURCES)
	$(CXX) $^ $(CXXFLAGS) -o $(OUT) $(LINKER) 
