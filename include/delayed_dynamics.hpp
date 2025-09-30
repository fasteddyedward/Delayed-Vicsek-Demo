#include <random>
#include <vector>
#include "../include/interp_functions.hpp"
// #include "../include/debug.hpp"
using namespace std;

void boundary_update_XY(vector<double> &x_present, vector<double> &y_present, double L_box_x, double L_box_y, int N, int boundary_shift, double Delta_L, uniform_real_distribution<> &dis, mt19937 &gen)
{

    if (boundary_shift == 0)
    {
#pragma omp parallel for
        for (int i = 0; i < N; i++)
        {
            x_present[i] = x_present[i] - L_box_x * round(x_present[i] / L_box_x);
            y_present[i] = y_present[i] - L_box_y * round(y_present[i] / L_box_y);
        }
    }
    else if (boundary_shift == 1)
    {
#pragma omp parallel for
        for (int i = 0; i < N; i++)
        {
            double rand = dis(gen);
            if (rand > 0.5)
            {
                x_present[i] = x_present[i] - Delta_L * round(y_present[i] / L_box_y); // shift x when particle crosses y boundary
                x_present[i] = x_present[i] - L_box_x * round(x_present[i] / L_box_x);

                y_present[i] = y_present[i] - Delta_L * round(x_present[i] / L_box_x); // shift y when particle crosses x boundary
                y_present[i] = y_present[i] - L_box_y * round(y_present[i] / L_box_y);
            }
            else
            {
                y_present[i] = y_present[i] - Delta_L * round(x_present[i] / L_box_x); // shift y when particle crosses x boundary
                y_present[i] = y_present[i] - L_box_y * round(y_present[i] / L_box_y);

                x_present[i] = x_present[i] - Delta_L * round(y_present[i] / L_box_y); // shift x when particle crosses y boundary
                x_present[i] = x_present[i] - L_box_x * round(x_present[i] / L_box_x);
            }
        }
    }
}

void rand_init(mt19937 &gen, int random_seed)
{
    if (random_seed == 1)
    {
        random_device rd;
        gen = mt19937(rd()); // Here the seed is also random
        cout << "random seed mt19937(rd())" << endl;
    }
    else if (random_seed == 0)
    {
        gen = mt19937(2); // Here the seed is fixed
        cout << "seed=mt19937(2)" << endl;
    }
}

void noise_init(string noise_type, mt19937 &gen, uniform_real_distribution<> &dis, normal_distribution<double> &normrnd, int N, double D_0, double dt, vector<double> &noise_x2, vector<double> &noise_y2, vector<double> &noise_s2)
{
    for (int i = 0; i < N; i++)
    {
        if (noise_type.compare("uniform") == 0)
        {
            noise_s2[i] = (dis(gen) - 0.5) * 2 * M_PI * (D_0 * sqrt(dt));
        }
        else if (noise_type.compare("Gaussian") == 0)
        {
            noise_s2[i] = normrnd(gen) * sqrt(1.0 / 3) * pow(D_0 * M_PI, 1) * sqrt(dt); // Gaussian white noise.
            // Here the sigma (stddev) of noise distribution over time interval dt is sqrt(1/3)*(D_0*pi)*sqrt(dt), which is the same as the uniform noise for [-pi*D_0, pi*D_0];
        }
    }
}

void config_init(vector<double> &s_init, vector<double> &x_init, vector<double> &y_init, vector<vector<double>> &s_recorded, vector<vector<double>> &x_recorded, vector<vector<double>> &y_recorded, string noise_type, vector<double> noise_s2, vector<double> noise_x2, vector<double> noise_y2, double L_box_x, double L_box_y, string load_file_name, int aligned_init, int N, double D_0, double v_0, double delta_t, double dt, int delay_steps, normal_distribution<double> normrnd, mt19937 &gen, uniform_real_distribution<> &dis, const json &jsonData)
{

    if (strcmp(load_file_name.c_str(), "No Loading") == 0)
    {
        // Initial positions
        // #pragma omp parallel for
        if (aligned_init == 1)
        {
            for (int i = 0; i < N; i++)
            {
                // s_init[i]=0;
                s_init[i] = -M_PI;
                x_init[i] = (dis(gen) - 0.5) * L_box_x;
                y_init[i] = (dis(gen) - 0.5) * L_box_y;
            }
        }
        else if (aligned_init == 0)
        {
            for (int i = 0; i < N; i++)
            {
                s_init[i] = (dis(gen) - 0.5) * 2 * M_PI;
                x_init[i] = (dis(gen) - 0.5) * L_box_x;
                y_init[i] = (dis(gen) - 0.5) * L_box_y;
            }
        }

#pragma omp parallel for
        for (int i = 0; i < N; i++)
        {
            s_recorded[0][i] = s_init[i];
            x_recorded[0][i] = x_init[i];
            y_recorded[0][i] = y_init[i];
        }

        for (int k = 0; k < delay_steps; k++)
        {
            noise_init(noise_type, gen, dis, normrnd, N, D_0, dt, noise_x2, noise_y2, noise_s2);
#pragma omp parallel for
            for (int i = 0; i < N; i++)
            {
                s_recorded[k + 1][i] = s_recorded[k][i] + noise_s2[i];
                x_recorded[k + 1][i] = x_recorded[k][i] + v_0 * cos(s_recorded[k + 1][i]) + noise_x2[i];
                y_recorded[k + 1][i] = y_recorded[k][i] + v_0 * sin(s_recorded[k + 1][i]) + noise_y2[i];
            }
            boundary_update_XY(x_recorded[k + 1], y_recorded[k + 1], L_box_x, L_box_y, N, 0, 0, dis, gen);
        }
    }
    else
    {
        cout << "Interpolation start." << endl;
        string load_file_s = jsonData["load_file_name"].get<string>() + "/" + "kernel_s.txt";
        string load_file_x = jsonData["load_file_name"].get<string>() + "/" + "kernel_x.txt";
        string load_file_y = jsonData["load_file_name"].get<string>() + "/" + "kernel_y.txt";

        cout << "load_file_x=" << load_file_x << endl;
        double dt_loaded = jsonData["dt"];
        int interval_loaded = 1; // Use this if we are loading x_kernel.txt.
        cout << "interval_loaded=" << interval_loaded << endl;
        int delay_steps = delta_t / dt; // for output

        interp(load_file_x, delta_t, dt_loaded, dt, interval_loaded, N, x_recorded);
        interp(load_file_y, delta_t, dt_loaded, dt, interval_loaded, N, y_recorded);
        interp(load_file_s, delta_t, dt_loaded, dt, interval_loaded, N, s_recorded);

        for (int k = 0; k < delay_steps; k++)
        {
            boundary_update_XY(x_recorded[k], y_recorded[k], L_box_x, L_box_y, N, 0, 0, dis, gen);
        }
        cout << "Interpolation successful" << endl;
        cout << "x.size()=" << x_recorded.size() << endl;
        cout << "x[0].size()=" << x_recorded[0].size() << endl;
    }
}

void xys_update_Vicsek(vector<double> &s_present, vector<double> &x_present, vector<double> &y_present, vector<double> &noise_s2, double v_0, double L_box_x, double L_box_y, int N, double dt)
{
#pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        s_present[i] = s_present[i] + noise_s2[i];
        s_present[i] = s_present[i] - 2 * M_PI * round(s_present[i] / (2 * M_PI));
        x_present[i] = x_present[i] + v_0 * cos(s_present[i]) * dt;
        y_present[i] = y_present[i] + v_0 * sin(s_present[i]) * dt;
    }
}

void xys_update_Vicsek_XY(vector<double> &s_present, vector<double> &x_present, vector<double> &y_present, vector<double> &F_s, vector<double> &F_x, vector<double> &F_y, vector<double> &noise_s2, vector<double> &noise_x2, vector<double> &noise_y2, double v_0, double L_box_x, double L_box_y, int N, double dt)
{
#pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        s_present[i] = s_present[i] + F_s[i] * dt + noise_s2[i];
        x_present[i] = x_present[i] + F_x[i] * dt + noise_x2[i];
        y_present[i] = y_present[i] + F_y[i] * dt + noise_y2[i];
    }
}

void xys_update_Vicsek_XY_recorded(vector<vector<double>> &s_recorded, vector<vector<double>> &x_recorded, vector<vector<double>> &y_recorded, vector<double> &F_s, vector<double> &F_x, vector<double> &F_y, vector<double> &noise_s2, vector<double> &noise_x2, vector<double> &noise_y2, double v_0, double L_box_x, double L_box_y, int N, double dt)
{

    vector<double> s(N, 0), x(N, 0), y(N, 0);
#pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        s[i] = s_recorded[s_recorded.size() - 1][i] + F_s[i] * dt + noise_s2[i];
        x[i] = x_recorded[x_recorded.size() - 1][i] + F_x[i] * dt + noise_x2[i];
        y[i] = y_recorded[y_recorded.size() - 1][i] + F_y[i] * dt + noise_y2[i];
    }
    s_recorded.push_back(s);
    x_recorded.push_back(x);
    y_recorded.push_back(y);
    s_recorded.erase(s_recorded.begin());
    x_recorded.erase(x_recorded.begin());
    y_recorded.erase(y_recorded.begin());
}

// Updates only the x & y, so we have the freedom to update s before/after the average in the Viscek Engine.
void xy_update_Vicsek(vector<double> &s_present, vector<double> &x_present, vector<double> &y_present, double v_0, int N, double dt)
{
#pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        x_present[i] = x_present[i] + v_0 * cos(s_present[i]) * dt;
        y_present[i] = y_present[i] + v_0 * sin(s_present[i]) * dt;
    }
}

void s_update_Vicsek(vector<double> &s_present, vector<double> &noise_s2, double v_0, int N)
{
#pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        s_present[i] = s_present[i] + noise_s2[i];
        s_present[i] = s_present[i] - 2 * M_PI * round(s_present[i] / (2 * M_PI)); // I think we don't need this
    }
}

void frame_rotate_update(vector<double> &s_present, vector<double> &x_present, vector<double> &y_present, double L_box_x, double L_box_y, int N, int frame_rotate, double rot_angle)
{
    // This function rotates the entire ensemble counter-clockwise by rot_angle, then performs the periodic boundary condition
    if (frame_rotate == 1)
    {
#pragma omp parallel for
        for (int i = 0; i < N; i++)
        {
            s_present[i] = s_present[i] + rot_angle;
            x_present[i] = x_present[i] * cos(rot_angle) - y_present[i] * sin(rot_angle);
            y_present[i] = y_present[i] * sin(rot_angle) + y_present[i] * cos(rot_angle);
            // boundary condition restored, so that the particles outside the box after rotation come back in.
            x_present[i] = x_present[i] - L_box_x * round(x_present[i] / L_box_x);
            y_present[i] = y_present[i] - L_box_y * round(y_present[i] / L_box_y);
        }
    }
}