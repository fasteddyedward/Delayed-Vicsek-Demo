#!/bin/bash
set -euo pipefail

# Submit one delayed Vicsek simulation to SLURM.

#

# Usage:

# bash scripts/submit_sbatch.sh path/to/input.json [job_name]

#

# Optional environment variables:

# PARTITION=batch
# MEM=1G
# TIME_LIMIT=1-00:00:00
# EXECUTABLE=build/delayed_vicsek

if [ "$#" -lt 1 ]; then
echo "Usage: bash scripts/submit_sbatch.sh path/to/input.json [job_name]"
exit 1
fi

input_json="$1"
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

if [ ! -f "$input_json" ]; then
echo "Error: input JSON file not found: $input_json"
exit 1
fi

make

if [ ! -x "$executable" ]; then
echo "Error: executable not found or not executable: $executable"
exit 1
fi

input_json_abs="$(cd "$(dirname "$input_json")" && pwd)/$(basename "$input_json")"
executable_abs="$repo_root/$executable"
output_dir="$(dirname "$input_json_abs")"

sbatch_file="$output_dir/sbatch_${job_name}.sh"

cat > "$sbatch_file" <<'EOF'
#!/bin/bash
set -e

executable="$1"
input_json="$2"

echo "Job ID: $SLURM_JOB_ID"
echo "Host: $(hostname)"
echo "Executable: $executable"
echo "Input JSON: $input_json"
echo "Working directory: $(pwd)"
echo

time "$executable" "$input_json"
EOF

chmod +x "$sbatch_file"

sbatch 
--job-name="$job_name" 
--partition="$partition" 
--mem="$mem" 
--time="$time_limit" 
--chdir="$repo_root" 
--output="$output_dir/slurm-%j.out" 
--error="$output_dir/slurm-%j.err" 
"$sbatch_file" "$executable_abs" "$input_json_abs"
