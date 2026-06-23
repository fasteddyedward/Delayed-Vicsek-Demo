#!/bin/bash
set -euo pipefail

# Submit a SLURM array job for the delayed Vicsek demo.

#

# Usage:
# bash scripts/submit_array_sbatch.sh example/parameters.txt [job_name]

#

# Optional environment variables:
# PARTITION=batch
# MEM=1G
# TIME_LIMIT=1-00:00:00
# EXECUTABLE=build/delayed_vicsek

if [ "$#" -lt 1 ]; then
echo "Usage: bash scripts/submit_array_sbatch.sh example/parameters.txt [job_name]"
exit 1
fi

parameters_file="$1"
job_name="${2:-delayed_vicsek}"

partition="${PARTITION:-batch}"
mem="${MEM:-1G}"
time_limit="${TIME_LIMIT:-1-00:00:00}"
executable="${EXECUTABLE:-build/delayed_vicsek}"

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

if ! command -v sbatch >/dev/null 2>&1; then
echo "Error: sbatch was not found. This script must be run on a SLURM system."
exit 1
fi

if [ ! -f "$parameters_file" ]; then
echo "Error: parameters file not found: $parameters_file"
exit 1
fi

make

if [ ! -x "$executable" ]; then
echo "Error: executable not found or not executable: $executable"
exit 1
fi

parameters_file_abs="$(cd "$(dirname "$parameters_file")" && pwd)/$(basename "$parameters_file")"
executable_abs="$repo_root/$executable"
output_dir="$(dirname "$parameters_file_abs")"

num_jobs=$(wc -l < "$parameters_file_abs")

if [ "$num_jobs" -eq 0 ]; then
echo "Error: parameters file is empty: $parameters_file_abs"
exit 1
fi

array_max=$((num_jobs - 1))
sbatch_file="$output_dir/sbatch_array_${job_name}.sh"

cat > "$sbatch_file" <<'EOF'
#!/bin/bash
set -euo pipefail

cd "$SLURM_SUBMIT_DIR"

task_id="$SLURM_ARRAY_TASK_ID"
line_number=$((task_id + 1))

input_json=$(sed -n "${line_number}p" "$PARAMETERS_FILE")

if [ -z "$input_json" ]; then
echo "No input JSON found for array task $task_id"
exit 1
fi

echo "SLURM job ID: $SLURM_JOB_ID"
echo "SLURM array task ID: $SLURM_ARRAY_TASK_ID"
echo "Host: $(hostname)"
echo "Executable: $EXECUTABLE"
echo "Parameters file: $PARAMETERS_FILE"
echo "Input JSON: $input_json"
echo "Working directory: $(pwd)"
echo

time "$EXECUTABLE" "$input_json"
EOF

chmod +x "$sbatch_file"

echo "Submitting array job with $num_jobs task(s)."
echo "Array range: 0-$array_max"
echo "Partition: $partition"
echo "Memory: $mem"
echo "Time limit: $time_limit"

sbatch 
--job-name="$job_name" 
--partition="$partition" 
--mem="$mem" 
--time="$time_limit" 
--chdir="$repo_root" 
--output="$output_dir/slurm-%A_%a.out" 
--error="$output_dir/slurm-%A_%a.err" 
--array="0-$array_max" 
--export=ALL,PARAMETERS_FILE="$parameters_file_abs",EXECUTABLE="$executable_abs" 
"$sbatch_file"
