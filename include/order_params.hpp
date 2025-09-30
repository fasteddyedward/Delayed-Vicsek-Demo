#include <cmath>
#include <vector>

using namespace std;

inline vector<double> get_vel(vector<vector<double>> &x_recorded, double dt,
                              double L_box_x)
{
  int N = x_recorded[0].size();
  int k = x_recorded.size(); // e.g. delay_steps=0;
  vector<double> v_x(N, numeric_limits<double>::quiet_NaN());
  if (k == 1)
  {
    return v_x;
  }
  for (int i = 0; i < N; i++)
  {
    double diff_x = x_recorded[k - 1][i] - x_recorded[k - 2][i];
    diff_x = diff_x - L_box_x * round(diff_x / L_box_x);
    v_x[i] = diff_x / dt;
  }
  return v_x;
}

inline vector<double> mean_vel(vector<double> &v_x, vector<double> &v_y)
{
  vector<double> v_mean(2, 0);
  int N = v_x.size();
  for (int i = 0; i < N; i++)
  {
    v_mean[0] = v_mean[0] + v_x[i];
    v_mean[1] = v_mean[1] + v_y[i];
  }
  v_mean[0] = v_mean[0] / N;
  v_mean[1] = v_mean[1] / N;
  return v_mean;
}

inline vector<double> mean_vel3D(vector<double> &v_x, vector<double> &v_y,
                                 vector<double> &v_z)
{
  vector<double> v_mean(3, 0);
  int N = v_x.size();
  for (int i = 0; i < N; i++)
  {
    v_mean[0] = v_mean[0] + v_x[i];
    v_mean[1] = v_mean[1] + v_y[i];
    v_mean[2] = v_mean[2] + v_z[i];
  }
  v_mean[0] = v_mean[0] / N;
  v_mean[1] = v_mean[1] / N;
  v_mean[2] = v_mean[2] / N;
  return v_mean;
}

inline double mean_speed(vector<double> &v_x, vector<double> &v_y)
{
  double v_mean = 0.0;
  int N = v_x.size();
  for (int i = 0; i < N; i++)
  {
    v_mean = v_mean + sqrt(pow(v_x[i], 2) + pow(v_y[i], 2));
  }

  v_mean = v_mean / N;
  return v_mean;
}

inline double mean_speed3D(vector<double> &v_x, vector<double> &v_y,
                           vector<double> &v_z)
{
  double v_mean = 0.0;
  int N = v_x.size();
  for (int i = 0; i < N; i++)
  {
    v_mean = v_mean + sqrt(pow(v_x[i], 2) + pow(v_y[i], 2) + pow(v_z[i], 2));
  }

  v_mean = v_mean / N;
  return v_mean;
}

inline double polarization(vector<double> &F_x, vector<double> &F_y)
{
  vector<double> F_mean(2, 0);
  int N = F_x.size();
  for (int i = 0; i < N; i++)
  {
    F_mean[0] = F_mean[0] + F_x[i];
    F_mean[1] = F_mean[1] + F_y[i];
  }
  F_mean[0] = F_mean[0] / N;
  F_mean[1] = F_mean[1] / N;

  double pol = sqrt(F_mean[0] * F_mean[0] + F_mean[1] * F_mean[1]);
  return pol;
}

inline double polarization3D(vector<double> &F_x, vector<double> &F_y, vector<double> &F_z)
{
  vector<double> F_mean(2, 0);
  int N = F_x.size();
  for (int i = 0; i < N; i++)
  {
    F_mean[0] = F_mean[0] + F_x[i];
    F_mean[1] = F_mean[1] + F_y[i];
    F_mean[2] = F_mean[2] + F_z[i];
  }
  F_mean[0] = F_mean[0] / N;
  F_mean[1] = F_mean[1] / N;
  F_mean[2] = F_mean[2] / N;

  double pol = sqrt(F_mean[0] * F_mean[0] + F_mean[1] * F_mean[1] + F_mean[2] * F_mean[2]);
  return pol;
}

inline double magnetization(vector<double> s)
{

  int N = s.size();
  double s_x = 0;
  double s_y = 0;
  for (int i = 0; i < N; i++)
  {
    s_x = s_x + cos(s[i]);
    s_y = s_y + sin(s[i]);
  }

  double m = sqrt(pow(s_x, 2) + pow(s_y, 2)) / N;
  return m;
}