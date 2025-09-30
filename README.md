
## Vicsek Model — Continuous Limit Variant

This is a demo project implementing a modified version of the **Vicsek model**.

Instead of updating agent orientations using the standard rule:

$$
\theta_i(t + 1) = \text{Arg} \left[ \sum_{j \in \mathcal{N}_i} \vec{v}_j(t) \right] + \eta_i(t)
$$
$$
x_i(t+1) = x_i(t) + v_0 \cos(\theta_i(t+1))
$$
$$
y_i(t+1) = y_i(t) + v_0 \sin(\theta_i(t+1))
$$

we introduce update rules that allow both **position** and **orientation** changes to approach the **continuous limit** as  ${\rm d}t \to 0$.

$$
\frac{{\rm d}\theta_i(t)}{{\rm d}t} =  \sum_{j \in \mathcal{N}_i} J \sin\left(\theta_j(t)-\theta_i(t)\right) + \eta_i(t)
$$
$$
\frac{{\rm d}x_i(t)}{{\rm d}t} = v_0 \cos(\theta_i(t))
$$
\frac{{\rm d}y_i(t)}{{\rm d}t} = v_0 \sin(\theta_i(t))
$$
---

## Running the Code on SLURM

To run simulations on a SLURM-based cluster:

1. Edit and execute the script:

   ./submit_input_params.sh
2. Submit multiple jobs by modifying the input parameter arrays, for example:

    delta_t_array=(0.1 0.2 0.3)

## Running Locally Without SLURM
You can also run main.cpp directly:
1. Execute submit_input_params.sh with a single parameter set.
2. When prompted:

    "Do you want to compile files? (y/n or m for move):"
    
    press Ctrl+C to exit.
3. Run main.cpp, which will automatically load parameters from the folder name listed in:

    example/parameters.txt

## References
[1] Vicsek, T., Czirók, A., Ben-Jacob, E., Cohen, I., & Shochet, O. (1995). Novel type of phase transition in a system of self-driven particles. Physical review letters, 75(6), 1226.


