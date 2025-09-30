#!/bin/bash
job_folder='example'
file_name='main'

PWD=$(pwd)
workdir="$PWD/$job_folder" # workdir="'/scratch/fermi/chen/2023.7.6 All to all cutoff new interactions/Ex1'"

# region #### Setting parameters
engine='Vicsek_XY_BU' # 'Vicsek_XY_BU', 'Vicsek_XY_FU'
# noise_type="Gaussian" #"uniform", "Gaussian"
noise_type="uniform"
D_0_J_array=(1)
delta_t_array=($(seq 0.0 0.1 1.0))
delta_t_array=(0.0)
v_0_array=(0.5)
range_array=(1.0)
rho=2
L_box_x=2
L_box_y=$L_box_x;
N_array=($(printf "%.0f" $(echo "$L_box_x * $L_box_y * $rho" | bc)));
dt=0.01
J_array=(1.0)
dt=0.001
random_seed=0 # 1 for random seed, 0 for fixed seed for random number
aligned_init=1 # 1 for aligned initial condition, 0 for random initial condition
boundary_shift=0;
Delta_L=0
frame_rotate=0
rot_angle=0.0


## Running Durations
Obs_time_steps=2e6
Obs_time_steps=$(printf "%0.0f" "$Obs_time_steps")
loop_total=1
partition='batch' # student or batch, see sinfo
time_limit=2
timeout='2d' ## 1s, 1m, 1h, 1d...etc
sbatch_file="sbatch_file.sh"

## Recording files
write_file=1 ## 1 to write, 0 to not write
write_kernel_file=1 ## 1 to write kernel_x.txt, 0 to not write# write_file_loop_cutoff=0 ## Start writing files from the nth loop (0-based; 0 for including all loops, and loop_total-1 for no recording)
write_screen_shot=1 ## 1 to screen shot position config at the end of each loop
interval_OP=100 #dt=0.001, Obstime=2e6
# Choose one from below

## Option 0: recording everything; only do this for 1~5 parameter sets.
interval=10000 #dt=0.001 Obstime=2e6
write_file_loop_cutoff=0 ## Start writing files from the nth loop (0-based; 0 for including all loops, and loop_total-1 for no recording)

# region #### Createing Jobfolder Directory if doesn't exist
if [ ! -d "$job_folder" ]; then
    echo "Creating folder $job_folder"
    mkdir "$job_folder"
fi
# endregion

rm "$workdir/parameters.txt"

for J in ${J_array[@]}; do
    for range in ${range_array[@]}; do

        for D_0 in ${D_0_J_array[@]}; do

            for N in ${N_array[@]}; do
                for delta_t in ${delta_t_array[@]}; do
                    for v_0 in ${v_0_array[@]}; do

                        for cpu_task in 1; do
                            # region  ### Calculating mem in MB's
                            safe=4
                            mem=$(awk "BEGIN { printf \"%.0f\", 24*$N*($delta_t/$dt+1)/1000000*$safe}")MB

                            if [ $delta_t == 0.0 ]; then
                                mem=2GB
                            fi

                            # endregion
                            load_file_name="No Loading"
                            output_folder="$workdir/N=$N, D_0=$D_0, J=$J, v_0=$v_0, aligned_init=$aligned_init, delta_t=$delta_t, dt=$dt"

                            job_name="$job_folder"
                            input_file="input.json"
                            output_file="output.json"
                            # endregion

                            echo "$output_folder" >> "$workdir/parameters.txt"


                            # region    #### Writing input parameters ot $input_file
                            input_data='{
                            "D_0": '"$D_0"',
                            "N": '"$N"',
                            "J": '"$J"',
                            "Obs_time_steps": '"$Obs_time_steps"',
                            "delta_t": '"$delta_t"',
                            "dt": '"$dt"',
                            "v_0":'"$v_0"',
                            "L_box_x":'"$L_box_x"',
                            "L_box_y":'"$L_box_y"',
                            "boundary_shift":'"$boundary_shift"',
                            "Delta_L":'"$Delta_L"',
                            "range":'"$range"',
                            "load_file_name": "'"$load_file_name"'",
                            "loop_total": '"$loop_total"',
                            "interval": '"$interval"',
                            "interval_OP": '"$interval_OP"',
                            "write_file": '"$write_file"',
                            "write_file_loop_cutoff": '"$write_file_loop_cutoff"',
                            "write_kernel_file": '"$write_kernel_file"',
                            "write_screen_shot": '"$write_screen_shot"',
                            "cpu_task": '"$cpu_task"',
                            "output_folder": "'"$output_folder"'",
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
                            "input_file": "'"$input_file"'",
                            "output_file": "'"$output_file"'",
                            "file_name": "'"$file_name"'",
                            "job_folder": "'"$job_folder"'",
                            "job_name": "'"$job_name"'",
                            "sbatch_file": "'"$sbatch_file"'",
                            "partition": "'"$partition"'",
                            "workdir": "'"$workdir"'",
                            "mem": "'"$mem"'",
                            "random_seed": '"$random_seed"',
                            "aligned_init": '"$aligned_init"',
                            "time_limit": '"$time_limit"',
                            "engine": "'"$engine"'",
                            "noise_type": "'"$noise_type"'",
                            "frame_rotate": '"$frame_rotate"',
                            "rot_angle": '"$rot_angle"'
                            }'
                            # endregion
                            echo "$input_data" >"$input_file"

                            # region #### Createing Output Directory if doesn't exist
                            if [ ! -d "$output_folder" ]; then
                                echo "Creating folder $output_folder"
                                mkdir "$output_folder"
                            fi
                            # endregion

                            mv $input_file "$output_folder" ## MODIFIED: this $job_folder -> $output_folder
                            # echo $input_file has been moved to "$output_folder"
                            echo mem=$mem
                        done

                    done
                done
                # done
            done
        done
    done
done

#### compiling main.cpp
./compile_file.sh $job_folder $file_name

#### Copying submit_main to the running folder
source_file="$(basename "$0")"
echo "source_file=$source_file"
destination_folder=$job_folder
timestamp=$(date +"%Y.%m.%d.%H:%M:%S")
backup_file="$source_file.$timestamp"
cp -b "$source_file" "$destination_folder/$backup_file"

./submit_sbatch.sh "$sbatch_file" "$job_name" "$partition" "$workdir" "$mem" "$cpu_task" "$timeout" "$file_name" "$output_folder"