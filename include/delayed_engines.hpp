#include <vector>
#include <cmath>
#include <chrono>
#include <cmath>
using namespace std;

void engine_Vicsek_XY_delayed_backward_update(vector<double> &s_present, vector<double> &x_present, vector<double> &y_present, vector<double> &s_delay, vector<double> &x_delay, vector<double> &y_delay, double J, double L_box_x, double L_box_y, double v_0, double range, int N, vector<double> &F_s, vector<double> &F_x, vector<double> &F_y, double &time_Vicsek)
{
    auto start = std::chrono::high_resolution_clock::now();
#pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        F_s[i] = 0;
        F_x[i] = 0;
        F_y[i] = 0;
    }

    vector<double> NN(N, 0); // number of nearest neighbors
    vector<double> R_ATT(N, 0);
    vector<double> X_ATT(N, 0);
    vector<double> Y_ATT(N, 0);
    vector<double> X_CELL(N, 0);
    vector<double> Y_CELL(N, 0);

    int n_cel_x = floor(L_box_x / range), n_cel_y = floor(L_box_y / range);
    vector<vector<int>> hoc(n_cel_x, vector<int>(n_cel_y, -1));
    vector<int> ll(N, -1), icel_x(N, 0), icel_y(N, 0);
    if (fmod(L_box_x, range) != 0 | fmod(L_box_y, range) != 0)
    {
        cout << "We need exact multiples for L_box/range!" << endl;
        exit(-1);
    }

    for (int i = 0; i < N; i++)
    {
        X_CELL[i] = x_delay[i] + L_box_x / 2;
        Y_CELL[i] = y_delay[i] + L_box_y / 2;
        if (floor(X_CELL[i] / range) == n_cel_x || floor(Y_CELL[i] / range) == n_cel_y)
        {
            cout << "i=" << i << endl;
            cout << "x_delay=" << x_delay[i] << endl;
            cout << "y_delay=" << y_delay[i] << endl;
            cout << "s_delay=" << s_delay[i] << endl;
            cout << "X_CELL:" << X_CELL[i] << endl;
            cout << "Y_CELL:" << Y_CELL[i] << endl;
            cout << "warning2:going over cell" << endl;
            if (floor(X_CELL[i] / range) == n_cel_x)
            {
                X_CELL[i] = 0;
            }
            if (floor(Y_CELL[i] / range) == n_cel_y)
            {
                Y_CELL[i] = 0;
            }
        }
        icel_x[i] = floor(X_CELL[i] / range);
        icel_y[i] = floor(Y_CELL[i] / range);
        ll[i] = hoc[icel_x[i]][icel_y[i]];
        hoc[icel_x[i]][icel_y[i]] = i;
    }

#pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        for (int dx_cel_index = -1; dx_cel_index <= 1; dx_cel_index++)
        {
            for (int dy_cel_index = -1; dy_cel_index <= 1; dy_cel_index++)
            {
                int x_cel_index = fmod(icel_x[i] + dx_cel_index, n_cel_x);
                int y_cel_index = fmod(icel_y[i] + dy_cel_index, n_cel_y);
                if (x_cel_index == -1)
                {
                    x_cel_index = x_cel_index + n_cel_x;
                }
                if (y_cel_index == -1)
                {
                    y_cel_index = y_cel_index + n_cel_y;
                }
                int j = hoc[x_cel_index][y_cel_index];
                while (j != -1)
                {
                    if (i != j)
                    {
                        X_ATT[i] = x_delay[i] - x_delay[j];
                        Y_ATT[i] = y_delay[i] - y_delay[j];
                        R_ATT[i] = sqrt(pow(X_ATT[i], 2) + pow(Y_ATT[i], 2));
                        if (R_ATT[i] < range)
                        {
                            NN[i]++;
                            F_s[i] = F_s[i] + sin(s_delay[j] - s_delay[i]);
                        }
                    }
                    j = ll[j];
                }
            }
        }

        if (NN[i] != 0)
        {
            F_s[i] = F_s[i] * J / NN[i];
        }
        F_x[i] = v_0 * cos(s_delay[i]);
        F_y[i] = v_0 * sin(s_delay[i]);
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    time_Vicsek = time_Vicsek + duration.count();
}

void engine_Vicsek_XY_delayed_forward_update(vector<double> &s_present, vector<double> &x_present, vector<double> &y_present, vector<double> &s_delay, vector<double> &x_delay, vector<double> &y_delay, double J, double L_box_x, double L_box_y, double v_0, double range, int N, vector<double> &F_s, vector<double> &F_x, vector<double> &F_y, double &time_Vicsek)
{
    auto start = std::chrono::high_resolution_clock::now();
#pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        F_s[i] = 0;
        F_x[i] = 0;
        F_y[i] = 0;
    }

    vector<double> NN(N, 0); // number of nearest neighbors
    vector<double> R_ATT(N, 0);
    vector<double> X_ATT(N, 0);
    vector<double> Y_ATT(N, 0);
    vector<double> X_CELL(N, 0);
    vector<double> Y_CELL(N, 0);

    int n_cel_x = floor(L_box_x / range), n_cel_y = floor(L_box_y / range);
    vector<vector<int>> hoc(n_cel_x, vector<int>(n_cel_y, -1));
    vector<int> ll(N, -1), icel_x(N, 0), icel_y(N, 0);
    if (fmod(L_box_x, range) != 0 | fmod(L_box_y, range) != 0)
    {
        cout << "We need exact multiples for L_box/range!" << endl;
        exit(-1);
    }

    // #pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        X_CELL[i] = x_delay[i] + L_box_x / 2;
        Y_CELL[i] = y_delay[i] + L_box_y / 2;
        if (floor(X_CELL[i] / range) == n_cel_x || floor(Y_CELL[i] / range) == n_cel_y)
        {
            cout << "i=" << i << endl;
            cout << "x_delay=" << x_delay[i] << endl;
            cout << "y_delay=" << y_delay[i] << endl;
            cout << "s_delay=" << s_delay[i] << endl;
            cout << "X_CELL:" << X_CELL[i] << endl;
            cout << "Y_CELL:" << Y_CELL[i] << endl;
            cout << "warning2:going over cell" << endl;
            if (floor(X_CELL[i] / range) == n_cel_x)
            {
                X_CELL[i] = 0;
            }
            if (floor(Y_CELL[i] / range) == n_cel_y)
            {
                Y_CELL[i] = 0;
            }
        }
        icel_x[i] = floor(X_CELL[i] / range);
        icel_y[i] = floor(Y_CELL[i] / range);
        ll[i] = hoc[icel_x[i]][icel_y[i]];
        hoc[icel_x[i]][icel_y[i]] = i;
    }
#pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        for (int dx_cel_index = -1; dx_cel_index <= 1; dx_cel_index++)
        {
            for (int dy_cel_index = -1; dy_cel_index <= 1; dy_cel_index++)
            {
                int x_cel_index = fmod(icel_x[i] + dx_cel_index, n_cel_x);
                int y_cel_index = fmod(icel_y[i] + dy_cel_index, n_cel_y);
                if (x_cel_index == -1)
                {
                    x_cel_index = x_cel_index + n_cel_x;
                }
                if (y_cel_index == -1)
                {
                    y_cel_index = y_cel_index + n_cel_y;
                }
                int j = hoc[x_cel_index][y_cel_index];
                while (j != -1)
                {
                    if (i != j)
                    {
                        X_ATT[i] = x_delay[i] - x_delay[j];
                        Y_ATT[i] = y_delay[i] - y_delay[j];
                        R_ATT[i] = sqrt(pow(X_ATT[i], 2) + pow(Y_ATT[i], 2));
                        if (R_ATT[i] < range)
                        {
                            NN[i]++;
                            F_s[i] = F_s[i] + sin(s_delay[j] - s_delay[i]);
                        }
                    }
                    j = ll[j];
                }
            }
        }

        if (NN[i] != 0)
        {
            F_s[i] = F_s[i] * J / NN[i];
        }

        F_x[i] = v_0 * cos(s_present[i]);
        F_y[i] = v_0 * sin(s_present[i]);
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    time_Vicsek = time_Vicsek + duration.count();
}