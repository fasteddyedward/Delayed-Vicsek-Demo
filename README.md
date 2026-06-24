# Delayed Vicsek Model — C++ Scientific Computing Demo

This repository is a compact scientific-computing demo for a delayed Vicsek-type active matter model. It demonstrates how an agent-based dynamical system can be implemented in C++, configured through JSON input files, executed locally, and submitted as a SLURM array job for parameter studies on an HPC cluster.

The project is intended as a public portfolio example of simulation-code organization, reproducible execution, and HPC workflow design. It is not intended to reproduce the full research codebase used in my PhD work.

## Overview

The Vicsek model is a minimal model for collective motion in active matter. In the standard model, self-propelled particles align their direction of motion with nearby particles while moving at approximately constant speed. This repository implements a delayed Vicsek-type variant in which the dynamics can depend on delayed particle states.

The code supports configurable parameters such as particle number, noise strength, interaction strength, self-propulsion speed, delay time, time step, box size, interaction range, and initial conditions.

## What this repository demonstrates

* C++ implementation of an agent-based dynamical model
* Time-delayed interaction handling through stored particle histories
* Periodic boundary conditions
* JSON-based input configuration
* Local compilation and execution through a Makefile
* SLURM array-job workflow for HPC parameter sweeps
* Organized simulation output folders
* Output of particle positions, orientations, interaction terms, delayed histories, snapshots, and collective order parameters
* Basic post-processing and visualization workflow through Jupyter/Python

## Repository structure

```text
.
├── analysis/                     # Jupyter/Python scripts for visualization and post-processing
├── example/                      # Example input files, generated parameter lists, and sample output folders
├── include/                      # Header files for dynamics, interaction rules, I/O, and observables
├── scripts/                      # SLURM and workflow scripts
│   ├── generate_and_submit_slurm.sh
│   └── submit_array_sbatch.sh
├── src/                          # Main C++ source file
│   └── main.cpp
├── Makefile                      # Build, local-run, and SLURM workflow shortcuts
├── .gitignore
└── README.md
```

## Requirements

For local compilation:

* Linux or WSL
* `g++`
* `make`
* OpenMP support
* `bc` for shell-based parameter calculations

On Ubuntu/WSL, the core build tools can be installed with:

```bash
sudo apt update
sudo apt install build-essential bc
```

For HPC execution:

* SLURM
* access to a cluster partition specified in `scripts/generate_and_submit_slurm.sh`

For analysis and visualization:

* Python 3
* NumPy
* Matplotlib
* Jupyter, if using the notebook in `analysis/`

## Build

From the repository root:

```bash
make
```

This compiles:

```text
src/main.cpp
```

into:

```text
build/delayed_vicsek
```

To clean generated build files:

```bash
make clean
```

## Run a local example

From the repository root:

```bash
make run-example
```

This builds the executable if needed and runs the default example configuration.

The executable can also be run directly:

```bash
./build/delayed_vicsek
```

or with an explicit input file:

```bash
./build/delayed_vicsek path/to/input.json
```

The no-argument mode is useful for quick local testing and VS Code debugging. The explicit input-file mode is useful for reproducible runs and scripted workflows.

## Run parameter sweeps on SLURM

The SLURM workflow is split into two scripts:

```text
scripts/generate_and_submit_slurm.sh
scripts/submit_array_sbatch.sh
```

The generator script creates input folders, writes `input.json` files, records the generated input paths in `example/parameters.txt`, compiles the code, and submits one SLURM array job.

Run:

```bash
make submit-slurm
```

or directly:

```bash
bash scripts/generate_and_submit_slurm.sh
```

The script generates a parameter list such as:

```text
example/parameters.txt
```

Each line points to one generated `input.json`. The SLURM array task ID selects one line from this file and runs:

```bash
build/delayed_vicsek path/to/input.json
```

This design separates parameter generation from job execution and provides a compact example of an HPC batch workflow.

## Important SLURM settings

The main SLURM settings are defined in:

```text
scripts/generate_and_submit_slurm.sh
```

Typical fields include:

```bash
partition="batch"
slurm_time_limit="2-00:00:00"
```

The script estimates the memory requirement for generated parameter sets and submits the array job using the maximum requested memory across the sweep.

If your cluster uses different partition names, memory rules, or time-limit formats, edit the SLURM settings before submission.

## Input configuration

Each simulation is controlled by an `input.json` file. Example fields include:

```json
{
    "D_0": 0.01,
    "N": 200,
    "J": 1.0,
    "Obs_time_steps": 20000,
    "delta_t": 0.1,
    "dt": 0.01,
    "v_0": 0.5,
    "L_box_x": 10,
    "L_box_y": 10,
    "range": 1.0,
    "aligned_init": 1,
    "engine": "Vicsek_XY_BU",
    "noise_type": "uniform"
}
```

The full generated input file also contains output filenames and workflow metadata used by the C++ code.

## Output

The simulation can write files such as:

```text
x.txt
y.txt
s.txt
v_x.txt
v_y.txt
f_x.txt
f_y.txt
f_s.txt
x_kernel.txt
y_kernel.txt
s_kernel.txt
x_screenshot.txt
y_screenshot.txt
s_screenshot.txt
Order_parameters.txt
output.json
```

The exact output depends on the recording flags in the input JSON, for example:

```json
"write_file": 1,
"write_kernel_file": 1,
"write_screen_shot": 1,
"interval": 100,
"interval_OP": 100
```

## Analysis

The `analysis/` folder contains post-processing and visualization tools. For example, the notebook can be used to visualize recorded particle positions and orientations.

A typical workflow is:

```text
generate input.json
→ run C++ simulation
→ write output files
→ visualize trajectories / orientations / order parameters
```

Additional lightweight Python scripts may be added for plotting order parameters and generating example figures.

## Notes on generated files

Simulation output folders, SLURM logs, generated parameter files, and compiled binaries should generally not be treated as source code. The repository is organized so that source files, scripts, and small examples are version-controlled, while large generated outputs can be excluded through `.gitignore`.

## References

Vicsek, T., Czirók, A., Ben-Jacob, E., Cohen, I., & Shochet, O. (1995). Novel type of phase transition in a system of self-driven particles. *Physical Review Letters*, 75(6), 1226.

## Author

Pin-Chuan (Edward) Chen

This repository was prepared as a public demonstration of scientific software development, numerical simulation, and HPC workflow organization.
