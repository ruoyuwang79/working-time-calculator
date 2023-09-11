#include "calculator.h"

using namespace std;

// TODO
int calcuFromMd(string filename) {
    return 0;
}

int calcuFromIn() {
    int hours, mins, totalMins;
    totalMins = 0;
    for (int i = 1; i < 8; i++) {
        hours = 0;
        mins = 0;
        cout << "day " << i << " hours (default 0): ";
        string input;
        getline(cin, input);
        if (!input.empty()) {
            istringstream stream(input);
            stream >> hours;
        }
        cout << "day " << i << " minutes (default 0): ";
        getline(cin, input);
        if (!input.empty()) {
            istringstream stream(input);
            stream >> mins;
        }
        totalMins += hours * 60 + mins;
    }
    return totalMins;
}

void printResult(int timeInMin) {
    int dayAvgTime = round(double(timeInMin) / 7);
    cout << "|<=== Week Summary ===>|" << endl;
    cout << "week total: " << timeInMin / 60 << "h" << timeInMin % 60 << "min" << endl;
    cout << "day average: " << dayAvgTime / 60 << "h" << dayAvgTime % 60 << "min" << endl;
}
