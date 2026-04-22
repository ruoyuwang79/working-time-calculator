#include "calculator.h"

#include <algorithm>
#include <ctime>
#include <regex>
#include <set>
#include <vector>

using namespace std;

namespace {
struct Date {
    int y;
    int m;
    int d;
};

bool parseDate(const string& text, Date& out) {
    smatch match;
    regex full(R"(^\s*(\d{4})\.(\d{1,2})\.(\d{1,2})\s*$)");
    if (regex_match(text, match, full)) {
        out = {stoi(match[1]), stoi(match[2]), stoi(match[3])};
        return true;
    }
    return false;
}

bool parsePossiblyShortDate(const string& text, const Date& base, Date& out) {
    smatch match;
    regex full(R"(^\s*(\d{4})\.(\d{1,2})\.(\d{1,2})\s*$)");
    regex monthDay(R"(^\s*(\d{1,2})\.(\d{1,2})\s*$)");
    regex yearDay(R"(^\s*(\d{4})\.(\d{1,2})\s*$)");
    regex dayOnly(R"(^\s*(\d{1,2})\s*$)");

    if (regex_match(text, match, full)) {
        out = {stoi(match[1]), stoi(match[2]), stoi(match[3])};
        return true;
    }
    if (regex_match(text, match, yearDay)) {
        int y = stoi(match[1]);
        int d = stoi(match[2]);
        if (y == base.y) {
            out = {base.y, base.m, d};
            return true;
        }
    }
    if (regex_match(text, match, monthDay)) {
        out = {base.y, stoi(match[1]), stoi(match[2])};
        return true;
    }
    if (regex_match(text, match, dayOnly)) {
        out = {base.y, base.m, stoi(match[1])};
        return true;
    }
    return false;
}

int dayDiffInclusive(const Date& start, const Date& end) {
    tm a = {};
    a.tm_year = start.y - 1900;
    a.tm_mon = start.m - 1;
    a.tm_mday = start.d;
    a.tm_hour = 12;

    tm b = {};
    b.tm_year = end.y - 1900;
    b.tm_mon = end.m - 1;
    b.tm_mday = end.d;
    b.tm_hour = 12;

    time_t ta = mktime(&a);
    time_t tb = mktime(&b);
    if (ta == -1 || tb == -1 || tb < ta) {
        return -1;
    }

    double days = difftime(tb, ta) / (60 * 60 * 24);
    return static_cast<int>(days) + 1;
}



long long dayKey(const Date& d) {
    tm t = {};
    t.tm_year = d.y - 1900;
    t.tm_mon = d.m - 1;
    t.tm_mday = d.d;
    t.tm_hour = 12;
    time_t v = mktime(&t);
    return static_cast<long long>(v / (60 * 60 * 24));
}
string trim(const string& input) {
    const string ws = " \t\r\n";
    size_t begin = input.find_first_not_of(ws);
    if (begin == string::npos) {
        return "";
    }
    size_t end = input.find_last_not_of(ws);
    return input.substr(begin, end - begin + 1);
}

string durationText(int mins) {
    int h = mins / 60;
    int m = mins % 60;
    if (h == 0) {
        return to_string(m) + "min";
    }
    if (m == 0) {
        return to_string(h) + "h";
    }
    return to_string(h) + "h" + to_string(m) + "min";
}

int sumTimeRangesInSection(const vector<string>& lines, int begin, int end) {
    regex rangePattern(R"((\d{1,2}):(\d{1,2})\s*-\s*(\d{1,2}):(\d{1,2}))");
    int total = 0;

    for (int i = begin; i < end; ++i) {
        auto start = sregex_iterator(lines[i].begin(), lines[i].end(), rangePattern);
        auto finish = sregex_iterator();

        for (auto it = start; it != finish; ++it) {
            int sh = stoi((*it)[1]);
            int sm = stoi((*it)[2]);
            int eh = stoi((*it)[3]);
            int em = stoi((*it)[4]);
            int diff = (eh * 60 + em) - (sh * 60 + sm);
            if (diff < 0) {
                diff += 12 * 60;
            }
            if (diff < 0) {
                diff += 24 * 60;
            }
            total += diff;
        }
    }

    return total;
}

void replaceOrInsertSummary(vector<string>& lines, int sectionBegin, int sectionEnd, const string& prefix, const string& replacement) {
    for (int i = sectionBegin; i < sectionEnd; ++i) {
        if (lines[i].find(prefix) != string::npos) {
            lines[i] = replacement;
            return;
        }
    }
    lines.insert(lines.begin() + sectionEnd, replacement);
}
}

int calcuFromMd(string filename) {
    ifstream input(filename);
    if (!input.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return 0;
    }

    vector<string> lines;
    string line;
    while (getline(input, line)) {
        lines.push_back(line);
    }
    input.close();

    vector<int> headings;
    for (int i = 0; i < static_cast<int>(lines.size()); ++i) {
        if (trim(lines[i]).rfind("## ", 0) == 0) {
            headings.push_back(i);
        }
    }

    struct DaySection {
        int begin;
        int end;
        int mins;
    };

    vector<DaySection> daySections;
    int weeklyHeading = -1;
    int weeklyEnd = static_cast<int>(lines.size());
    int totalMins = 0;
    int dayCountForAverage = 0;
    Date weekStart = {0, 0, 0};
    Date weekEndDate = {0, 0, 0};
    bool hasWeekRange = false;
    vector<Date> explicitDays;

    for (int i = 0; i < static_cast<int>(headings.size()); ++i) {
        int begin = headings[i];
        int end = (i + 1 < static_cast<int>(headings.size())) ? headings[i + 1] : static_cast<int>(lines.size());
        string raw = trim(lines[begin]);
        if (raw.size() < 2) {
            continue;
        }
        string headingText = trim(raw.substr(2));

        if (headingText.find('~') != string::npos) {
            weeklyHeading = begin;
            weeklyEnd = end;
            size_t pos = headingText.find('~');
            string startText = trim(headingText.substr(0, pos));
            string endText = trim(headingText.substr(pos + 1));
            Date startDate, endDate;
            if (parseDate(startText, startDate) && parsePossiblyShortDate(endText, startDate, endDate)) {
                int diff = dayDiffInclusive(startDate, endDate);
                if (diff > 0) {
                    dayCountForAverage = diff;
                    weekStart = startDate;
                    weekEndDate = endDate;
                    hasWeekRange = true;
                }
            }
            continue;
        }

        Date dayDate;
        if (!parseDate(headingText, dayDate)) {
            continue;
        }

        explicitDays.push_back(dayDate);
        int dayMins = sumTimeRangesInSection(lines, begin + 1, end);
        totalMins += dayMins;
        daySections.push_back({begin, end, dayMins});
    }

    for (int i = static_cast<int>(daySections.size()) - 1; i >= 0; --i) {
        const DaySection& section = daySections[i];
        string summary = "- 小结：<font color='green'>" + durationText(section.mins) + "</font>";
        replaceOrInsertSummary(lines, section.begin + 1, section.end, "小结：", summary);
    }

    if (hasWeekRange) {
        set<long long> days;
        long long startKey = dayKey(weekStart);
        long long endKey = dayKey(weekEndDate);
        for (long long k = startKey; k <= endKey; ++k) {
            days.insert(k);
        }
        for (const Date& d : explicitDays) {
            days.insert(dayKey(d));
        }
        dayCountForAverage = static_cast<int>(days.size());
    }
    if (dayCountForAverage <= 0) {
        dayCountForAverage = max(1, static_cast<int>(daySections.size()));
    }

    int avgMins = static_cast<int>(round(static_cast<double>(totalMins) / dayCountForAverage));
    string weekSummary = "- 周总结：<font color='green'>" + durationText(totalMins) + "</font>，日平均：<font color='green'>" + durationText(avgMins) + "</font>";

    if (weeklyHeading >= 0) {
        replaceOrInsertSummary(lines, weeklyHeading + 1, weeklyEnd, "周总结：", weekSummary);
    }

    ofstream output(filename);
    if (!output.is_open()) {
        cerr << "Failed to write file: " << filename << endl;
        return totalMins;
    }
    for (size_t i = 0; i < lines.size(); ++i) {
        output << lines[i];
        if (i + 1 < lines.size()) {
            output << "\n";
        }
    }

    return totalMins;
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
