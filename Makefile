CXX := g++
CXXFLAGS := -O3 -fopenmp -Iinclude -Wfatal-errors
TARGET := build/delayed_vicsek
SRC := src/main.cpp

.PHONY: all clean run-example

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run-example: $(TARGET)
	$(TARGET) "example/N=200, D_0=0.01, J=1.0, v_0=0.5, aligned_init=1, delta_t=0.5, dt=0.01/input.json"

clean:
	rm -r build