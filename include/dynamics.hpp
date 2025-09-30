#include <random>
using namespace std;

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

void config_init(vector<double> &s_init, vector<double> &x_init, vector<double> &y_init, vector<double> &s_present, vector<double> &x_present, vector<double> &y_present, double L_box_x, double L_box_y, string load_file_name, int aligned_init, int N, mt19937 &gen, uniform_real_distribution<> &dis, const json &jsonData)
{
    if (strcmp(load_file_name.c_str(), "No Loading") == 0)
    {
        // Initial positions
        if (aligned_init == 1)
        {
            for (int i = 0; i < N; i++)
            {
                // s_init[i]=0;
                s_init[i] = -M_PI;
                // s_init[i]=s_init[i]-2*M_PI*round(s_init[i]/(2*M_PI));
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
            s_present[i] = s_init[i];
            x_present[i] = x_init[i];
            y_present[i] = y_init[i];
        }
    }
    else
    {
        string load_file_s = jsonData["load_file_name"].get<string>() + "/" + "s_screenshot.txt";
        string load_file_x = jsonData["load_file_name"].get<string>() + "/" + "x_screenshot.txt";
        string load_file_y = jsonData["load_file_name"].get<string>() + "/" + "y_screenshot.txt";
        cout << "File loading successful" << endl;
        vector<double> s_load = readVectorFromFile(load_file_s, 1);
        vector<double> x_load = readVectorFromFile(load_file_x, 1);
        vector<double> y_load = readVectorFromFile(load_file_y, 1);
        cout << "s_loadsize=" << s_load.size() << endl;
        cout << "s_presentsize=" << s_present.size() << endl;
#pragma omp parallel for
        for (int i = 0; i < N; i++)
        {
            s_present[i] = s_load[i];
            x_present[i] = x_load[i];
            y_present[i] = y_load[i];
        }

        cout << "s_present_size=" << s_present.size() << endl;
        cout << "y_present_size=" << y_present.size() << endl;
        cout << "x_present_size=" << x_present.size() << endl;
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
        s_present[i] = s_present[i] - 2 * M_PI * round(s_present[i] / (2 * M_PI));
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