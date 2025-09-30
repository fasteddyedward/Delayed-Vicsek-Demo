vector<double> readVectorFromFile(const string &filename, const int &extract_lines)
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
    vector<double> matrix;
    file.open(filename);
    int line_count = 1;
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
                matrix.push_back(value);
            }
        }
        file.close();
    }
    else
    {
        cerr << "Error: Unable to open the file " << filename << endl;
    }
    cout << "size of loaded matrix= " << matrix.size() << endl;
    return matrix;
}
