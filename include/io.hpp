using namespace std;

inline void input_json_parse(string argv_1, json &jsonData)
{
  // Open the JSON input_file for reading
  ifstream input_file(argv_1);
  if (!input_file.is_open())
  {
    std::cerr << "Failed to open input.json." << std::endl;
    return;
  }
  // Parse the JSON data from the input_file
  try
  {
    input_file >> jsonData;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
    return;
  }
  input_file.close();

  for (auto it = jsonData.begin(); it != jsonData.end(); ++it)
  {
    std::cout << it.key() << " = " << it.value() << std::endl;
  }
}

inline void output_single_file(int write_file, int k, int interval,
                               int write_file_loop_cutoff, fstream &posfile_x,
                               const vector<double> &x)
{
  int N = x.size();
  if (write_file == 1)
  {
    if (k % interval == 0)
    {
      if (k >= write_file_loop_cutoff)
      {
        for (int i = 0; i < N; i++)
        {
          posfile_x << x[i] << " ";
        }
      }

      posfile_x << endl;
    }
  }
}

inline void output_OP_file(int write_file, int k, int interval_OP,
                           int write_file_loop_cutoff, fstream &posfile_OP,
                           const double &x)
{
  if (write_file == 1)
  {
    if (k % interval_OP == 0)
    {
      if (k >= write_file_loop_cutoff)
      {
        posfile_OP << x << "\t";
      }
    }
  }
}

void output_pos(int write_file, int k, int interval, int write_file_loop_cutoff, int N, fstream &posfile_s, fstream &posfile_x, fstream &posfile_y, fstream &posfile_m, vector<double> &s_present, vector<double> &x_present, vector<double> &y_present)
{
  if (write_file == 1)
  {
    if (k % interval == 0)
    {
      if (k >= write_file_loop_cutoff)
      {
        for (int i = 0; i < N; i++)
        {
          posfile_s << s_present[i] << " ";
          posfile_x << x_present[i] << " ";
          posfile_y << y_present[i] << " ";
        }
      }

      posfile_s << endl;
      posfile_x << endl;
      posfile_y << endl;
    }
  }
}

inline void output_kernel(json &jsonData, int write_kernel_file, double duration,
                          double time_limit, int k, int Obs_time_steps,
                          fstream &kernel_x, fstream &kernel_y, fstream &kernel_s,
                          string x_kernel_file, string y_kernel_file,
                          string z_kernel_file, vector<vector<double>> &x_recorded,
                          vector<vector<double>> &y_recorded,
                          vector<vector<double>> &s_recorded, int N, int delay_steps)
{
  if (write_kernel_file == 1 &&
      (duration > 0.95 * 86400 * time_limit || k == Obs_time_steps - 1))
  {
    if (duration > 0.95 * 86400 * time_limit)
    {
      cout << "Time limit almost reached, writing kernel." << endl;
    }
    else if (k == Obs_time_steps - 1)
    {
      cout << "Last loop of numerics, writing kernel." << endl;
    }
    kernel_x.open(x_kernel_file, ios::out);
    kernel_y.open(y_kernel_file, ios::out);
    kernel_s.open(z_kernel_file, ios::out);
    for (int k = 0; k < delay_steps + 1; k++)
    {
      for (int i = 0; i < N; i++)
      {
        kernel_x << x_recorded[k][i] << " ";
        kernel_y << y_recorded[k][i] << " ";
        kernel_s << s_recorded[k][i] << " ";
      }
      kernel_x << endl;
      kernel_y << endl;
      kernel_s << endl;
    }
    kernel_x.close();
    kernel_y.close();
    kernel_s.close();
    jsonData["write_kernel_file"] = -1;
  }
}

inline void output_screenshot(json &jsonData, int write_screen_shot, double duration,
                              double time_limit, int k, int Obs_time_steps,
                              fstream &screenshot_x, fstream &screenshot_y,
                              fstream &screenshot_s, string x_screen_file,
                              string y_screen_file, string z_screen_file,
                              vector<double> &x_present, vector<double> &y_present,
                              vector<double> &s_present, int N)
{
  if (write_screen_shot == 1 &&
      (duration > 0.95 * 86400 * time_limit || k == Obs_time_steps - 1))
  {
    if (duration > 0.95 * 86400 * time_limit)
    {
      cout << "time limit almost reached, writing screenshot." << endl;
      cout << "time limit is " << 0.95 * 86400 * time_limit << endl;
      cout << "current time is " << duration << endl;
    }
    else if (k == Obs_time_steps - 1)
    {
      cout << "Last loop of numerics, writing screenshot" << endl;
      cout << "k=" << k << endl;
      cout << "Obs_time=" << Obs_time_steps << endl;
    }
    screenshot_x.open(x_screen_file, ios::out);
    screenshot_y.open(y_screen_file, ios::out);
    screenshot_s.open(z_screen_file, ios::out);
    for (int i = 0; i < N; i++)
    {
      screenshot_x << x_present[i] << " ";
      screenshot_y << y_present[i] << " ";
      screenshot_s << s_present[i] << " ";
    }
    screenshot_x << endl;
    screenshot_y << endl;
    screenshot_s << endl;
    screenshot_x.close();
    screenshot_y.close();
    screenshot_s.close();
    jsonData["write_screen_shot"] = -1;
  }
}

void k_5_percent(int Obs_time_steps, int k, double duration)
{
  // Writing k every 5 percent
  int percent_show = Obs_time_steps / 20; // shows result every whenever 1/20 of the program is complete.
  if (percent_show == 0)
  { // Otherwise k % percent_show will not work
    percent_show = 1;
  }
  if (k % percent_show == 0)
  {
    cout << "k=" << k << endl;
    cout << (double)100 * k / Obs_time_steps << " percent complete" << endl;
    cout << "Elapsed time (high resolution clock): " << duration << " seconds" << std::endl; // this time suits best for SLURM.
    cout << "(= " << duration / 60 << " minutes" << " = " << duration / 3600 << " hours)" << endl;
  }
}

