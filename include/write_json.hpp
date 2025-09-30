
// #include "json/single_include/nlohmann/json.hpp"
#include "json.hpp"

using json = nlohmann::json;
using namespace std; 
void write_json(const json &data){
    // string y_out_file= data["output_folder"].get<string>()+"/"+jsonData["y_out_file"].get<string>();
    // std::ofstream outputFile("output.json");
    // std::ofstream outputFile(data["output_folder"].get<string>()+"/output.json");
    std::ofstream outputFile(data["output_folder"].get<string>()+"/"+data["output_file"].get<string>());
    if (outputFile.is_open()){
        outputFile << data.dump(4);
        outputFile.close();
        cout<<"Parameters saved to output.json"<<endl;
    }
    else{
        cerr << "Unable to open the files for writing." << endl;
    }
}

