#include <iostream>
#include <vector>
#include <cmath>
#include <typeinfo>
#include <random>
#include <string.h>
#include <fstream>
#include <sstream>
#include <chrono>
using namespace std;

vector<vector<double>> readDataFromFile_lastnlines(const string &filename, const int &extract_lines)
{
    // Reading how many files there are:
    int num_lines = 0;
    ifstream file(filename);
    if (file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            num_lines = num_lines + 1;
        }
        file.close();
    }
    else
    {
        cerr << "Error: Unable to open the file " << filename << endl;
    }
    // Start appending matrix
    vector<vector<double>> matrix;
    file.open(filename);
    int line_count = 0;
    if (file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            line_count = line_count + 1;
            if (line_count > num_lines - extract_lines)
            {
                vector<double> row;
                istringstream iss(line);
                double value;
                while (iss >> value)
                {
                    row.push_back(value);
                }
                matrix.push_back(row);
            }
        }
        file.close();
    }
    else
    {
        cerr << "Error: Unable to open the file " << filename << endl;
    }
    return matrix;
}

vector<vector<double>> readDataFromFile(const string &filename)
{
    vector<vector<double>> matrix;
    ifstream file(filename);

    if (file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            vector<double> row;
            istringstream iss(line);
            double value;
            while (iss >> value)
            {
                row.push_back(value);
            }
            matrix.push_back(row);
        }
        file.close();
    }
    else
    {
        cerr << "Error: Unable to open the file " << filename << endl;
    }

    return matrix;
}

std::vector<std::vector<double>> transpose(const std::vector<std::vector<double>> data)
{
    // this assumes that all inner vectors have the same size and
    // allocates space for the complete result in advance
    std::vector<std::vector<double>> result(data[0].size(),
                                            std::vector<double>(data.size()));
    for (std::vector<double>::size_type i = 0; i < data[0].size(); i++)
        for (std::vector<double>::size_type j = 0; j < data.size(); j++)
        {
            result[i][j] = data[j][i];
        }
    return result;
}

void interp(string filename_x, double delta_t, double dt_in, double dt_out, int pt_interp, int N, vector<vector<double>> &x_interp)
{
    int delay_steps_in = ceil(delta_t / (dt_in * pt_interp));
    int delay_steps_out = delta_t / dt_out;

    vector<vector<double>> x = readDataFromFile_lastnlines(filename_x, delay_steps_in + 1);

    if (x.size() < delay_steps_out)
    {
        cerr << "The loaded kernel_x is not long enough for new delta_t!" << endl;
    }

    if (x[0].size() < N)
    {
        cerr << "N in the loaded file is too small! N=" << x[0].size() << endl;
    }

    double result;
    int k_in = x.size() - delay_steps_in - 2; // Basically starting from k_in=-1;

    for (int k_out = 0; k_out < delay_steps_out + 1; k_out++)
    {
        if (k_out % pt_interp == 0)
        {
            k_in = k_in + 1;
        }
        double t = (double)(k_out % pt_interp) / (double)pt_interp;

        for (int i = 0; i < N; i++)
        {
            if (pt_interp == 1)
            {
                x_interp[k_out][i] = x[k_in][i];
            }
            else
            {
                x_interp[k_out][i] = x[k_in][i] + (x[k_in + 1][i] - x[k_in][i]) * t;
            }
        }
    }
}