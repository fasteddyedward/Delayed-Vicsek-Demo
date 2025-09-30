#!/bin/bash
job_folder='example'
file_name='main'

PWD=$(pwd)
workdir="$PWD/$job_folder" 

#### Setting running parameters
engine='Vicsek_XY_BU' # 'Vicsek_XY_BU', 'Vicsek_XY_FU'
noise_type="uniform" # "Gaussian" #"uniform", "Gaussian"
D_0_array=(0.01) # diffusion constant
delta_t_array=(0.1) # time delay
v_0_array=(0.5) # self-propulsion speed
range_array=(1.0) # interaction range
rho=2
L_box_x=10
L_box_y=$L_box_x;
N_array=($(printf "%.0f" $(echo "$L_box_x * $L_box_y * $rho" | bc)));
dt=0.01
J_array=(1.0)
random_seed=0 # 1 for random seed, 0 for fixed seed for random number
aligned_init=1 # 1 for aligned initial condition, 0 for random initial condition

## Shift entering position when crossing the periodic boundaries
boundary_shift=0 # 0 for no shift, 1 for shift
Delta_L=0 # Shift distance by Delta_L if boundary_shift=1

## Rotate the frame by rot_angle degree at each time step
frame_rotate=0 # 0 for no rotation, 1 for rotation
rot_angle=0.0 # in radians, positive for counter-clockwise, negative for clockwise


## Running Durations
Obs_time_steps=2e4 #  Total observation time steps
Obs_time_steps=$(printf "%0.0f" "$Obs_time_steps")

## SLURM parameters
partition='batch' # student or batch, see sinfo
time_limit=2 ## in days
timeout='2d' ## 1s, 1m, 1h, 1d...etc
sbatch_file="sbatch_file.sh"

## Recording files
write_file=1 ## 1 to write data files, 0 to not write
write_kernel_file=1 ## 1 to write the history function 
write_screen_shot=1 ## 1 to screen shot position config at the end of each loop
interval_OP=100 # recording order parameters every interval_OP time steps
interval=100 # recording data every interval time steps
write_file_loop_cutoff=0 ## Start writing files from the nth loop (0-based; 0 for including all loops, and loop_total-1 for no recording)

#### Createing Jobfolder Directory if doesn't exist
if [ ! -d "$job_folder" ]; then
    echo "Creating folder $job_folder"
    mkdir "$job_folder"
fi

rm "$workdir/parameters.txt"

for J in ${J_array[@]}; do
    for range in ${range_array[@]}; do
        for D_0 in ${D_0_array[@]}; do
            for N in ${N_array[@]}; do
                for delta_t in ${delta_t_array[@]}; do
                    for v_0 in ${v_0_array[@]}; do

                        for cpu_task in 1; do
                            ### Calculating mem in MB's
                            safe=4
                            mem=$(awk "BEGIN { printf \"%.0f\", 24*$N*($delta_t/$dt+1)/1000000*$safe}")MB

                            if [ $delta_t == 0.0 ]; then
                                mem=2GB
                            fi

                            load_file_name="No Loading"
                            output_folder="$workdir/N=$N, D_0=$D_0, J=$J, v_0=$v_0, aligned_init=$aligned_init, delta_t=$delta_t, dt=$dt"
                            job_name="$job_folder"
                            input_file="input.json"
                            output_file="output.json"
                            echo "$output_folder" >> "$workdir/parameters.txt"
                             #### Writing input parameters ot $input_file
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
                            echo "$input_data" >"$input_file"

                            #### Createing Output Directory if doesn't exist
                            if [ ! -d "$output_folder" ]; then
                                echo "Creating folder $output_folder"
                                mkdir "$output_folder"
                            fi

                            mv $input_file "$output_folder" 
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