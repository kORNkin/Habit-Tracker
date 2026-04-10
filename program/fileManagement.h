#ifndef FILEMANAGEMENT_H
#define FILEMANAGEMENT_H

#include "globals.h"
#include <fstream>
#include <sstream>

bool IsValidFile(const string &file_name);
void ReadData();
void SaveData();

#endif