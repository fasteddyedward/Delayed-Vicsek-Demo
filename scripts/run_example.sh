#!/bin/bash
set -e

# Build the executable from the repository root
make

# Run the current example workflow.
# main.cpp currently expects example/parameters.txt relative to src/,
# so we run the executable from inside src/.
make run-example
