#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

struct Date{
    string y, m, d;
};

struct Streak{
    int all;
    unordered_map<string, int> habit;
};

extern vector<string> habits;
extern vector<pair<string, int>> dates;

extern unordered_map<string, unordered_map<string, bool>> tracking_data;
extern unordered_map<string, int> progressBydate;

extern const string DAY[];
extern const string MONTH[]; 
extern const int daysInMonth[];

#endif