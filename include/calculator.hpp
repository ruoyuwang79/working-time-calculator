#include <fstream>
#include <string>
#include <math.h> 

using namespace std;

// TODO
int calcuFromMd(string filename) {
    return 0;
};

int calcuFromIn() {
    int hours, mins, totalMins;
    for (int i = 1; i < 8; i++) {
        cout << "day " << i << " hours: ";
        cin >> hours;
        cout << "day " << i << " minutes: ";
        cin >> mins;
        totalMins += hours * 60 + mins;
    }
    return totalMins;
};

void printResult(int timeInMin) {
    int dayAvgTime = round(double(timeInMin) / 7);
    cout << "|<=== Week Summary ===>|" << endl;
    cout << "week total: " << timeInMin / 60 << "h" << timeInMin % 60 << endl;
    cout << "day average: " << dayAvgTime / 60 << "h" << dayAvgTime % 60 << endl;
};
