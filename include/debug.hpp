#include <vector>
using namespace std; 


void print_array_2(vector<vector <double>> x){
    cout<<"outputting array 2:"<<endl;
    for (int k=0;k<x.size();k++){
        for (int i=0;i<x[0].size();i++){
            cout<<x[k][i]<<" ";
            }
            cout<<endl;
    }
}


void print_array_1(vector<double> x){
    cout<<"outputting array 1:"<<endl;
        for (int i=0;i<x.size();i++){
            cout<<x[i]<<" ";
    }
}