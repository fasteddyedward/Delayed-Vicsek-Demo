#!/bin/bash
# set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# REPO_ROOT=$(pwd)
echo $REPO_ROOT
cd "$REPO_ROOT"

job_folder="example"
workdir="$REPO_ROOT/$job_folder"
parameters_file="$workdir/parameters.txt"
executable="build/delayed_vicsek"

#### Setting running parameters

engine="Vicsek_XY_BU"          # "Vicsek_XY_BU", "Vicsek_XY_FU"
noise_type="uniform"          # "uniform", "Gaussian"

D_0_array=(0.01)
delta_t_array=(0.1)
v_0_array=(0.5)
range_array=(1.0)
J_array=(1.0)

rho=2
L_box_x=10
L_box_y=$L_box_x
N_array=($(printf "%.0f" "$(echo "$L_box_x * $L_box_y * $rho" | bc)"))

dt=0.01
random_seed=0
aligned_init=1

boundary_shift=0
Delta_L=0

frame_rotate=0
rot_angle=0.0

loop_total=1

#### Running durations

Obs_time_steps=2e4
Obs_time_steps=$(printf "%0.0f" "$Obs_time_steps")

#### SLURM parameters

partition="batch"
time_limit="2-00:00:00"   # SLURM format: days-hours:minutes:seconds

#### Recording files

write_file=1
write_kernel_file=1
write_screen_shot=1
interval_OP=100
interval=100
write_file_loop_cutoff=0

#### Create job folder and reset parameter list

mkdir -p "$workdir"
: > "$parameters_file"

max_mem_mb=1

for J in "${J_array[@]}"; do
for range in "${range_array[@]}"; do
for D_0 in "${D_0_array[@]}"; do
for N in "${N_array[@]}"; do
for delta_t in "${delta_t_array[@]}"; do
for v_0 in "${v_0_array[@]}"; do
for cpu_task in 1; do

```
                        safe=4
                        mem_mb=$(awk "BEGIN { printf \"%.0f\", 24*$N*($delta_t/$dt+1)/1000000*$safe }")

                        if [ "$mem_mb" -lt 1 ]; then
                            mem_mb=1
                        fi

                        if [ "$delta_t" = "0.0" ]; then
                            mem_mb=2048
                        fi

                        if [ "$mem_mb" -gt "$max_mem_mb" ]; then
                            max_mem_mb="$mem_mb"
                        fi

                        load_file_name="No Loading"

                        output_folder_name="N=$N, D_0=$D_0, J=$J, v_0=$v_0, aligned_init=$aligned_init, delta_t=$delta_t, dt=$dt"
                        output_folder_rel="$job_folder/$output_folder_name"
                        output_folder="$REPO_ROOT/$output_folder_rel"

                        input_file="input.json"
                        output_file="output.json"
                        input_json_rel="$output_folder_rel/$input_file"
                        input_json="$REPO_ROOT/$input_json_rel"

                        job_name="$job_folder"

                        mkdir -p "$output_folder"

                        cat > "$input_json" <<EOF
{
"D_0": $D_0,
"N": $N,
"J": $J,
"Obs_time_steps": $Obs_time_steps,
"delta_t": $delta_t,
"dt": $dt,
"v_0": $v_0,
"L_box_x": $L_box_x,
"L_box_y": $L_box_y,
"boundary_shift": $boundary_shift,
"Delta_L": $Delta_L,
"range": $range,
"load_file_name": "$load_file_name",
"loop_total": $loop_total,
"interval": $interval,
"interval_OP": $interval_OP,
"write_file": $write_file,
"write_file_loop_cutoff": $write_file_loop_cutoff,
"write_kernel_file": $write_kernel_file,
"write_screen_shot": $write_screen_shot,
"cpu_task": $cpu_task,
"output_folder": "$output_folder",
"s_file": "s.txt",
"f_s_file": "f_s.txt",
"v_s_file": "v_s.txt",
"s_kernel_file": "s_kernel.txt",
"s_screenshot_file": "s_screenshot.txt",
"x_file": "x.txt",
"f_x_file": "f_x.txt",
"v_x_file": "v_x.txt",
"x_kernel_file": "x_kernel.txt",
"x_screenshot_file": "x_screenshot.txt",
"y_file": "y.txt",
"f_y_file": "f_y.txt",
"v_y_file": "v_y.txt",
"y_kernel_file": "y_kernel.txt",
"y_screenshot_file": "y_screenshot.txt",
"delta_t_file": "delta_t.txt",
"m_file": "m.txt",
"Order_parameters_file": "Order_parameters.txt",
"input_file": "$input_file",
"output_file": "$output_file",
"job_folder": "$job_folder",
"job_name": "$job_name",
"partition": "$partition",
"workdir": "$workdir",
"mem": "${mem_mb}MB",
"random_seed": $random_seed,
"aligned_init": $aligned_init,
"time_limit": "$time_limit",
"engine": "$engine",
"noise_type": "$noise_type",
"frame_rotate": $frame_rotate,
"rot_angle": $rot_angle
}
EOF
                        echo "$input_json_rel" >> "$parameters_file"
                        echo "Generated $input_json_rel with mem=${mem_mb}MB"

                    done
                done
            done
        done
    done
done
```

done

num_jobs=$(wc -l < "$parameters_file")

if [ "$num_jobs" -eq 0 ]; then
echo "No jobs generated."
exit 1
fi

echo "Generated $num_jobs job(s)."
echo "Maximum requested memory: ${max_mem_mb}MB"

#### Compile

make

#### Back up this generator script into the job folder

source_file="$(basename "$0")"
timestamp=$(date +"%Y.%m.%d.%H-%M-%S")
cp "$0" "$workdir/$source_file.$timestamp"

#### Submit one SLURM array job

PARTITION="$partition" 
MEM="${max_mem_mb}MB" 
TIME_LIMIT="$time_limit" 
EXECUTABLE="$executable" 
bash scripts/submit_array_sbatch.sh "$parameters_file" "$job_folder"
