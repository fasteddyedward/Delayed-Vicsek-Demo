## Copied from analysis/Order_Parameters/2025.1.29 polarization/p2_write_P5.py
## Adding in variance of polarization and making the length larger.
import os
import json
import numpy as np
import posixpath
  
  
# === Settings ===
print("Proj5")
run="run35"

base_path="F:/Linux ITP/2024.6.24 All2all cutoff/analysis/Order_Parameters/2026.2.18 polarization"
run_folder = f"F:/Linux ITP/2024.6.24 All2all cutoff/P5_dist_dep_delay/{run}"

# base_path="/scratch/fermi/chen/2024.6.24 All2all cutoff/analysis/Order_Parameters/2025.1.29 polarization"
# run_folder = f"/scratch/fermi/chen/2024.6.24 All2all cutoff/P5_dist_dep_delay/{run}"

# v_file_path = posixpath.join(base_path,f"p_P5_{run}.txt")
v_file_path = posixpath.join(base_path,f"p_P5_{run}.txt")

print(v_file_path)

# === Output file setup ===
with open(v_file_path, 'w') as v_file:
    header = "N D_0 v_0 range delta_t dt c v_mean v_var E_spring speed_mean polarization pol_var\n"
    v_file.write(header)

    # === Loop over subfolders ===
    for subfolder in sorted(os.listdir(run_folder)):
        subfolder_path = os.path.join(run_folder, subfolder)
        print(f"Processing subfolder: {subfolder_path}")
        if not os.path.isdir(subfolder_path):
            continue

        json_path = os.path.join(subfolder_path, "input.json")
        if not os.path.isfile(json_path):
            continue

        try:
            with open(json_path, "r") as f:
                json_data = json.load(f)
        except Exception as e:
            print(f"Could not read JSON in {subfolder}: {e}")
            continue

        op_file_path = os.path.join(subfolder_path, "Order_parameters.txt")
        if not os.path.isfile(op_file_path):
            print(f"Missing data file in {subfolder}")
            continue

        try:
            data_v = np.loadtxt(op_file_path, skiprows=1)
        except Exception as e:
            print(f"Error reading data file: {op_file_path}, {e}")
            continue

        if data_v.ndim == 1:
            data_v = np.expand_dims(data_v, axis=0)

        v = np.sqrt(data_v[:, 0] ** 2)
        range_start = int(0.9 * len(v))
        if range_start < 1:
            continue

        v_range = slice(range_start, len(v))

        v_mean = np.mean(v[v_range])
        v_var = np.var(v[v_range])
        E_spring_mean = np.mean(data_v[v_range, 1])
        speed_mean = np.mean(data_v[v_range, 2])
        pol = np.mean(data_v[v_range, 3])
        pol_var=np.var(data_v[v_range,3])

        line = f"{json_data['N']} {json_data['D_0']:.2f} {json_data['v_0']:.2f} {json_data['range']:.2f} {json_data['delta_t']:.2f} {json_data['dt']:.5f} {json_data['c']:.2f} {v_mean:.4f} {v_var:.4f} {E_spring_mean:.4f} {speed_mean:.4f} {pol:.4f} {pol_var:.4f}\n"
        v_file.write(line)

print("Processing complete.")
  
  