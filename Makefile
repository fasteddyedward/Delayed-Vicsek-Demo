CXX := g++
CXXFLAGS := -O3 -fopenmp -Iinclude -Wfatal-errors
TARGET := build/delayed_vicsek
SRC := src/main.cpp

.PHONY: all clean run-example_local submit-slurm


all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run-example_local: $(TARGET)
	$(TARGET) "example_local/N=200, D_0=0.01, J=1.0, v_0=0.5, aligned_init=1, delta_t=0.5, dt=0.01/input.json"

	
submit-slurm:
	bash scripts/generate_and_submit_slurm.sh

clean:
	rm -r build