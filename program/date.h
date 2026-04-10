#ifndef DATE_H
#define DATE_H

#include "globals.h" // We need the Date struct defined here
#include <string>
#include <ctime>
#include <chrono>

string CurrentDate(bool isFormatted);
bool BinarySearchDate(string input_date);
Date DateToStruct(string input_date);
int DateToNum(string input_date);
string DateToMDY(string input_date);
string DateFormatting(string input_date);
int getDayOfWeek(Date input_date);
bool IsValidDate(string input_date);
bool IsLeapYear(string input_year);
void DateSort();
string PreviousMonth(string y, string m);
string Yesterday(string input_date);

#endif