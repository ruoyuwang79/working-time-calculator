#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

#include "calculator.h"

using namespace std;

class InputParser{
    public:
        InputParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(string(argv[i]));
        }

        const string& getCmdOption(const string &option) const{
            vector<string>::const_iterator itr;
            itr =  find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const string empty_string("");
            return empty_string;
        }
        
        bool cmdOptionExists(const string &option) const{
            return find(this->tokens.begin(), this->tokens.end(), option)
                   != this->tokens.end();
        }
    private:
        vector <string> tokens;
};

int main(int argc, char * argv[]) {
    InputParser args(argc, argv);
    
    if(args.cmdOptionExists("-h")) {
        cout << "usage: ./main.exe [-h] [-f FILENAME]" << endl;
        cout << "-h            help" << endl;
        cout << "-f FILENAME   directly read first 7 items from .md file" << endl;
        return 0;
    }

    const string &filename = args.getCmdOption("-f");
    if (!filename.empty()){
        printResult(calcuFromMd(filename));
    } else {
        printResult(calcuFromIn());
    }

    return 0;
}
