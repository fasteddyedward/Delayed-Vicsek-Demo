#include <iostream>
#include <vector>
#include <cmath>
#include <typeinfo>
#include <random>
#include <string.h>
#include <fstream>
#include <sstream>
#include <omp.h>
#include <chrono>
#include <sys/stat.h>
#include "../include/write_json.hpp"
#include "../include/delayed_engines.hpp"
#include "../include/order_params.hpp"
#include "../include/io.hpp"
#include "../include/delayed_dynamics.hpp"
using namespace std;
// Include the JSON library (e.g., nlohmann/json)
using json = nlohmann::json;

int main(int argc, char *argv[])
{

    // /////////////////////////////////////////////////////////////////////
    // // === Option 1: Press F5 directly or ctrl+alt+n to debug and run the file directly  ===
    // cout<< "Starting Option 1: run with local debug" << endl;
    // cout<< "If not working, did you run ./submit_input_params already so that run_local_debug/parameters.txt is created?"<<endl;
    // int index = 0;  // Local test index
    // ifstream file("run_local_debug/parameters.txt");

    // // === Option 2: Use this block for SLURM or CLI runs ===
    // cout << "Starting Option 2: run with submit_input_params.sh" << endl;
    // cout << "If not working, are you sure that you compiled the main.cpp again?"<<endl;
    // if (argc != 2) {
    //     cerr << "Usage: " << argv[0] << " <index>" << endl;
    //     return 1;
    // }
    // int index = atoi(argv[1]);
    // ifstream file("parameters.txt");
    // /////////////////////////////////////////////////////////////////

    // Here we choose whether to run locally or on SLURM
    int index;
    ifstream file;
    if (argc == 1)
    {
        // === Option 1: run the file directly  ===
        cout << "Starting Option 1: run with example" << endl;
        cout << "If not working, did you run ./submit_input_params already so that example/parameters.txt is created?" << endl;
        index = 0;
        file.open("example/parameters.txt");
    }
    else if (argc == 2)
    {
        // === Option 2: run the file on SLURM  ===
        cout << "Starting Option 2: run with submit_input_params.sh" << endl;
        cout << "If not working, are you sure that you compiled the main.cpp again?" << endl;
        index = atoi(argv[1]);
        file.open("parameters.txt");
        cout << file.good() << endl;
    }

    if (!file.is_open())
    {
        cerr << "Failed to open parameter file." << endl;
        return 1;
    }

    vector<string> lines;
    string line;
    while (getline(file, line))
    {
        lines.push_back(line);
    }

    if (index < 0 || index >= lines.size())
    {
        cerr << "Index out of range." << endl;
        return 1;
    }

    // Define a JSON object
    json jsonData;
    string input_json_file = lines[index] + "/input.json";
    cout << "Running with input file: " << input_json_file << endl;

    input_json_parse(input_json_file, jsonData);

    fstream s_file, f_s_file, s_kernel_file, s_screenshot_file, x_file, f_x_file, x_kernel_file, x_screenshot_file, y_file, f_y_file, y_kernel_file, y_screenshot_file, m_file, Order_parameters_file, v_s_file, v_x_file, v_y_file;

    s_file.open(jsonData["output_folder"].get<string>() + "/" + jsonData["s_file"].get<string>(), ios::out);
    f_s_file.open(jsonData["output_folder"].get<string>() + "/" + jsonData["f_s_file"].get<string>(), ios::out);
    v_s_file.open(jsonData["output_folder"].get<string>() + "/" + jsonData["v_s_file"].get<string>(), ios::out);

    x_file.open(jsonData["output_folder"].get<string>() + "/" + jsonData["x_file"].get<string>(), ios::out);
    f_x_file.open(jsonData["output_folder"].get<string>() + "/" + jsonData["f_x_file"].get<string>(), ios::out);
    v_x_file.open(jsonData["output_folder"].get<string>() + "/" + jsonData["v_x_file"].get<string>(), ios::out);

    y_file.open(jsonData["output_folder"].get<string>() + "/" + jsonData["y_file"].get<string>(), ios::out);
    f_y_file.open(jsonData["output_folder"].get<string>() + "/" + jsonData["f_y_file"].get<string>(), ios::out);
    v_y_file.open(jsonData["output_folder"].get<string>() + "/" + jsonData["v_x_file"].get<string>(), ios::out);

    Order_parameters_file.open(jsonData["output_folder"].get<string>() + "/" + jsonData["Order_parameters_file"].get<string>(), ios::out);
    double cpu_time_used;
    clock_t start, end;
    auto start2 = chrono::high_resolution_clock::now();
    double NaN = numeric_limits<double>::quiet_NaN();

    int N = jsonData["N"].get<int>();
    vector<vector<double>> s_end(N, vector<double>(1));
    vector<vector<double>> x_end(N, vector<double>(1));
    vector<vector<double>> y_end(N, vector<double>(1));
    normal_distribution<double> normrnd(0.0, 1.0);
    vector<double> s_init(N);
    vector<double> x_init(N);
    vector<double> y_init(N);
    double time_Vicsek = 0.0, time_har = 0.0, cpu_time_Vicsek = 0.0;
    vector<double> F_s(N, 0);
    vector<double> noise_s2(N);
    vector<double> F_x(N, 0);
    vector<double> noise_x2(N);
    vector<double> F_y(N, 0);
    vector<double> noise_y2(N);

    // Recorded: these are the files that record over the time delta_t/dt (+1)
    int delay_steps = round(jsonData["delta_t"].get<double>() / jsonData["dt"].get<double>());
    vector<vector<double>> s_recorded(delay_steps + 1, vector<double>(N));
    vector<vector<double>> x_recorded(delay_steps + 1, vector<double>(N));
    vector<vector<double>> y_recorded(delay_steps + 1, vector<double>(N));

    // Creating a random number generator
    mt19937 gen;
    uniform_real_distribution<> dis(0.0, 1.0);
    rand_init(gen, jsonData["random_seed"].get<int>());

    // Initial configuration by randomly distributing or loading
    config_init(
        s_init, x_init, y_init,
        s_recorded, x_recorded, y_recorded,
        jsonData["noise_type"].get<string>(),
        noise_s2, noise_x2, noise_y2,
        jsonData["L_box_x"].get<double>(),
        jsonData["L_box_y"].get<double>(),
        jsonData["load_file_name"].get<string>(),
        jsonData["aligned_init"].get<int>(),
        jsonData["N"].get<int>(),
        jsonData["D_0"].get<double>(),
        jsonData["v_0"].get<double>(),
        jsonData["delta_t"].get<double>(),
        jsonData["dt"].get<double>(),
        delay_steps,
        normrnd, gen, dis, jsonData);

    // Initializing used time
    start = clock();
    start2 = chrono::high_resolution_clock::now();

    // Main Part
    for (int k = 0; k < jsonData["Obs_time_steps"].get<int>(); k++)
    {

        // Assigning uniform/Gaussian white noise.
        noise_init(
            jsonData["noise_type"].get<string>(),
            gen, dis, normrnd,
            jsonData["N"].get<int>(),
            jsonData["D_0"].get<double>(),
            jsonData["dt"].get<double>(),
            noise_x2, noise_y2, noise_s2);

        // Vicsek_XY with forward update
        if (jsonData["engine"].get<string>().compare("Vicsek_XY_FU") == 0)
        {
            engine_Vicsek_XY_delayed_forward_update(
                s_recorded[delay_steps], x_recorded[delay_steps], y_recorded[delay_steps],
                s_recorded[0], x_recorded[0], y_recorded[0],
                jsonData["J"].get<double>(),
                jsonData["L_box_x"].get<double>(),
                jsonData["L_box_y"].get<double>(),
                jsonData["v_0"].get<double>(),
                jsonData["range"].get<double>(),
                jsonData["N"].get<int>(),
                F_s, F_x, F_y, time_Vicsek);
        }
        // Vicsek_XY with backward update
        else if (jsonData["engine"].get<string>().compare("Vicsek_XY_BU") == 0)
        {
            engine_Vicsek_XY_delayed_backward_update(
                s_recorded[delay_steps], x_recorded[delay_steps], y_recorded[delay_steps],
                s_recorded[0], x_recorded[0], y_recorded[0],
                jsonData["J"].get<double>(),
                jsonData["L_box_x"].get<double>(),
                jsonData["L_box_y"].get<double>(),
                jsonData["v_0"].get<double>(),
                jsonData["range"].get<double>(),
                jsonData["N"].get<int>(),
                F_s, F_x, F_y, time_Vicsek);
        }
        else
        {
            cout << "Warning: you did not choose an engine to run with! Check for typo Vicsek_XY_FU or Vicsek_XY_BU" << endl;
            return 1;
        }

        xys_update_Vicsek_XY_recorded(
            s_recorded, x_recorded, y_recorded,
            F_s, F_x, F_y,
            noise_s2, noise_x2, noise_y2,
            jsonData["v_0"].get<double>(),
            jsonData["L_box_x"].get<double>(),
            jsonData["L_box_y"].get<double>(),
            jsonData["N"].get<int>(),
            jsonData["dt"].get<double>());

        // Moving the particles back into the box; Also the shifting boundary effects are here.
        boundary_update_XY(
            x_recorded[delay_steps], y_recorded[delay_steps],
            jsonData["L_box_x"].get<double>(),
            jsonData["L_box_y"].get<double>(),
            jsonData["N"].get<int>(),
            jsonData["boundary_shift"].get<int>(),
            jsonData["Delta_L"].get<double>(),
            dis, gen);

        // Calculating used time
        end = clock();
        auto end2 = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end2 - start2;

        // Calculating the velocities
        vector<double> v_x_present(jsonData["N"].get<int>()),
            v_y_present(jsonData["N"].get<int>()),
            v_s_present(jsonData["N"].get<int>());
        v_x_present =
            get_vel(x_recorded, jsonData["dt"].get<double>(), jsonData["L_box_x"]);
        v_y_present =
            get_vel(y_recorded, jsonData["dt"].get<double>(), jsonData["L_box_y"]);
        v_s_present =
            get_vel(s_recorded, jsonData["dt"].get<double>(), 2 * M_PI);
        vector<double> v_mean = mean_vel(v_x_present, v_y_present);

        // Outputting x_file, y_file, v_x_file, v_y_file
        output_single_file(jsonData["write_file"].get<int>(), k,
                           jsonData["interval"].get<int>(),
                           jsonData["write_file_loop_cutoff"].get<int>(), x_file,
                           x_recorded[delay_steps]);
        output_single_file(jsonData["write_file"].get<int>(), k,
                           jsonData["interval"].get<int>(),
                           jsonData["write_file_loop_cutoff"].get<int>(), y_file,
                           y_recorded[delay_steps]);
        output_single_file(jsonData["write_file"].get<int>(), k,
                           jsonData["interval"].get<int>(),
                           jsonData["write_file_loop_cutoff"].get<int>(), s_file,
                           s_recorded[delay_steps]);

        output_single_file(
            jsonData["write_file"].get<int>(), k, jsonData["interval"].get<int>(),
            jsonData["write_file_loop_cutoff"].get<int>(), v_x_file, v_x_present);
        output_single_file(
            jsonData["write_file"].get<int>(), k, jsonData["interval"].get<int>(),
            jsonData["write_file_loop_cutoff"].get<int>(), v_y_file, v_y_present);
        output_single_file(
            jsonData["write_file"].get<int>(), k, jsonData["interval"].get<int>(),
            jsonData["write_file_loop_cutoff"].get<int>(), v_s_file, v_s_present);

        output_single_file(
            jsonData["write_file"].get<int>(), k, jsonData["interval"].get<int>(),
            jsonData["write_file_loop_cutoff"].get<int>(), f_x_file, F_x);
        output_single_file(
            jsonData["write_file"].get<int>(), k, jsonData["interval"].get<int>(),
            jsonData["write_file_loop_cutoff"].get<int>(), f_y_file, F_y);
        output_single_file(
            jsonData["write_file"].get<int>(), k, jsonData["interval"].get<int>(),
            jsonData["write_file_loop_cutoff"].get<int>(), f_s_file, F_s);

        // Order Parameters: v_mean  E_spring speed_mean  polarization
        if (k == 0)
        {
            Order_parameters_file
                << "polarization \n";
        }

        output_OP_file(jsonData["write_file"].get<int>(), k,
                       jsonData["interval_OP"].get<int>(),
                       jsonData["write_file_loop_cutoff"].get<int>(),
                       Order_parameters_file, polarization(v_x_present, v_y_present));

        // Writing k every 5 percent
        k_5_percent(
            jsonData["Obs_time_steps"].get<int>(),
            k,
            duration.count());

        output_kernel(jsonData, jsonData["write_kernel_file"].get<int>(),
                      duration.count(), jsonData["time_limit"].get<double>(), k,
                      jsonData["Obs_time_steps"].get<int>(), x_kernel_file,
                      y_kernel_file, s_kernel_file, jsonData["x_kernel_file"].get<string>(),
                      jsonData["y_kernel_file"].get<string>(),
                      jsonData["s_kernel_file"].get<string>(),
                      x_recorded,
                      y_recorded, s_recorded, jsonData["N"].get<int>(), delay_steps);

        output_screenshot(
            jsonData, jsonData["write_screen_shot"].get<int>(), duration.count(),
            jsonData["time_limit"].get<double>(), k,
            jsonData["Obs_time_steps"].get<int>(), x_screenshot_file,
            y_screenshot_file, s_screenshot_file, jsonData["x_screenshot_file"].get<string>(),
            jsonData["y_screenshot_file"].get<string>(),
            jsonData["s_screenshot_file"].get<string>(),
            x_recorded[delay_steps],
            y_recorded[delay_steps],
            s_recorded[delay_steps],
            jsonData["N"].get<int>());
    }
    s_file.close();
    f_s_file.close();
    v_s_file.close();
    x_file.close();
    f_x_file.close();
    v_x_file.close();
    y_file.close();
    f_y_file.close();
    v_y_file.close();
    return 0;
}
