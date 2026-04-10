#ifndef SCREEN_H
#define SCREEN_H

#include "globals.h"
#include <iostream>
#include <iomanip>

template <typename T>
void GetLine(T &input, bool clearBuffer = 0){
    if (cin.rdbuf()->in_avail() > 0 || clearBuffer) cin.ignore(numeric_limits<streamsize>::max(), '\n');

    getline(cin, input);
}

void ClearScreen();
void ClearPreviousLines(int n);
bool IsValidCmd(string cmd, int end);
string HeatColor(int completed, int total);
void PrintMenu(bool valid);
void PrintDateAndHabitStatus(string input_date="");
void PrintHabitStatus();
void PrintCalendar(string month, string year);
string ResetColor();

#endif