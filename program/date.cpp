#include "date.h"

string CurrentDate(bool isFormatted){
    time_t now = time(0);
    tm *ltm = localtime(&now);

    if(isFormatted)
        return MONTH[ltm->tm_mon] + " " + to_string(ltm->tm_mday) + ", " + to_string(1900 + ltm->tm_year);
    else return DateFormatting(to_string((1900 + ltm->tm_year)) + "-" + to_string(1 + ltm->tm_mon) + "-" + to_string(ltm->tm_mday));
}

bool BinarySearchDate(string input_date){
    if(!dates.size()) return false;
    int tar = DateToNum(input_date);
    int l = 0, r = dates.size() - 1;
    while(l < r){
        int m = (l + r) >> 1;

        if(dates[m].second >= tar) r = m;
        else l = m + 1;
    }

    if(tar == dates[l].second) return true;
    else return false;
}

Date DateToStruct(string input_date){
    string tmp, y = "", m = "", d = "";
    int idx = 0;
    while(idx < input_date.size()){
        tmp = "";
        while(input_date[idx] != '-' && idx < input_date.size()){
            tmp += input_date[idx];
            idx++;
        } 
        if(y == ""){
            y = tmp;
        }else if(m == ""){
            m = to_string(stoi(tmp));
        }
        idx++;
    }
    d = to_string(stoi(tmp));

    return {y, m, d};
}

int DateToNum(string input_date){
    string tmp;
    int sum = 0, idx = 0;
    while(idx < input_date.size()){
        tmp = "";
        while(input_date[idx] != '-' && idx < input_date.size()){
            tmp += input_date[idx];
            idx++;
        } 
        sum += stoi(tmp) * (idx == 4? 366 : (idx == 7? 31 : 1));
        idx++;
    }

    return sum;
}

string DateToMDY(string input_date){
    Date tmp_date = DateToStruct(input_date);
    return MONTH[stoi(tmp_date.m) - 1] + " " + tmp_date.d + ", " + tmp_date.y ;
}

string DateFormatting(string input_date){
    Date tmp_date = DateToStruct(input_date);
    return tmp_date.y + "-" + (stoi(tmp_date.m) < 10? "0" + tmp_date.m : tmp_date.m) + "-" + (stoi(tmp_date.d) < 10? "0" + tmp_date.d : tmp_date.d); 
}

// GenAI code
int getDayOfWeek(Date input_date) {
    int day = stoi(input_date.d), month = stoi(input_date.m), year = stoi(input_date.y);
    if (month < 3) {
        month += 12;
        year -= 1;
    }

    int q = day;
    int m = month;
    int K = year % 100;
    int J = year / 100; 

    int h = (q + (13 * (m + 1)) / 5 + K + (K / 4) + (J / 4) - (2 * J)) % 7;

    if (h < 0) {
        h += 7;
    }

    //Sat = 0 -> Fri = 6
    return h;
}
// ----------------------

bool IsValidDate(string input_date){

    // check if not a number
    for(auto c : input_date) if(c != '-' && (c - '0' < 0 || c - '0' > 9)) return false;

    Date tmp_date = DateToStruct(input_date);

    // check boundary
    if(tmp_date.y.size() != 4 || stoi(tmp_date.y) < 1000) return false;
    if(!tmp_date.m.size() || stoi(tmp_date.m) < 1 || stoi(tmp_date.m) > 12) return false;
    if(!tmp_date.d.size() || stoi(tmp_date.d) < 1 || stoi(tmp_date.d) > 31) return false;

    if(DateToNum(input_date) > DateToNum(CurrentDate(0))) return false;

    return true;
}

bool IsLeapYear(string input_year){
    int year = stoi(input_year);

    if(year % 400 == 0) return true;
    if(year % 4 == 0 && year % 100) return true;

    return false;

}

void DateSort(){
    sort(dates.begin(), dates.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });
}

string PreviousMonth(string y, string m){
    if(stoi(m) == 1){
        return to_string(stoi(y) - 1) + "-12";
    }else return y + "-" + to_string(stoi(m) - 1);
}

string Yesterday(string input_date){
    Date today = DateToStruct(input_date);
    
    if(stoi(today.d) != 1) return DateFormatting(today.y + "-" + today.m + "-" + to_string(stoi(today.d) - 1));
    else {
        if(stoi(today.m) == 1){
            return DateFormatting(to_string(stoi(today.y)-1) + "-12-31");
        }else{
            if(stoi(today.m) == 3 && IsLeapYear(today.y))
                return DateFormatting(today.y + "-02-29");
            else  
                return DateFormatting(today.y + "-" + to_string(stoi(today.m) - 1) + "-" + to_string(daysInMonth[stoi(today.m) - 2]));

        }
    }
}