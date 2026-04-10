#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <unordered_map>
#include <algorithm>

using namespace std;

struct Date{
    string y, m, d;
};

struct Streak{
    int all;
    unordered_map<string, int> habit;
} bestStreak;

int DateToNum(string);
string DateFormatting(string);
void TrackToday();
void TrackTheDay(string = "");
int getDayOfWeek(Date);
void ManageMyHabits();
void PersonalDashboard(string = "");
bool IsLeapYear(string);
void SaveData();

fstream file;

// all habits
vector<string> habits;
vector<pair<string, int>> dates;
// habits -> date -> tracking
unordered_map<string, unordered_map<string, bool>> tracking_data;
unordered_map<string, int> progressBydate;

const string DAY[] = {"Sun","Mon","Tue", "Wed","Thu","Fri", "Sat"};
const string MONTH[] = {"January", "February", "March", "April", "May", "June", "July", 
                            "August", "September","October", "November", "December"};
const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Gen AI code
int paletteIdx = 0;
vector<string> palette = {
        "\033[38;2;255;160;160m", // Soft Coral
        "\033[38;2;255;210;170m", // Peach
        "\033[38;2;255;255;190m", // Lemon
        "\033[38;2;180;255;210m", // Mint
        "\033[38;2;180;255;255m", // Aqua
        "\033[38;2;180;210;255m", // Sky
        "\033[38;2;210;190;255m", // Lavender
        "\033[38;2;255;190;230m", // Pink
        "\033[38;2;220;255;180m", // Lime
        "\033[38;2;220;225;235m"  // Silver
    };
// ----------

//Track Today: 0-Tracked, 1-Edit Track, 2-Check Remaining 
int statusTrackToday = 1;

template <typename T>
void GetLine(T &input, bool clearBuffer = 0){
    if (cin.rdbuf()->in_avail() > 0 || clearBuffer) cin.ignore(numeric_limits<streamsize>::max(), '\n');

    getline(cin, input);
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

string CurrentDate(bool isFormatted){
    time_t now = time(0);
    tm *ltm = localtime(&now);

    if(isFormatted)
        return MONTH[ltm->tm_mon] + " " + to_string(ltm->tm_mday) + ", " + to_string(1900 + ltm->tm_year);
    else return DateFormatting(to_string((1900 + ltm->tm_year)) + "-" + to_string(1 + ltm->tm_mon) + "-" + to_string(ltm->tm_mday));
}

void ClearScreen(){
    cout << "\033[2J\033[1;1H\n";
}

// Gen AI code
void ClearPreviousLines(int n) {
    for (int i = 0; i < n; ++i) {
        cout << "\033[A\33[2K\r" << flush;
    }
}
// ----------

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

void DateSort(){
    sort(dates.begin(), dates.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });
}

bool IsValidCmd(string cmd, int end){
    if(pow(10, cmd.size() - 1) > end || !cmd.size()) return false;
    if(cmd == "q") return true;
    for(auto c : cmd) if(c - '0' < 0 || c - '0' > 9) return false;
    if(stoi(cmd) > end || stoi(cmd) < 1) return false;

    return true;
}

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

bool IsValidFile(const string &file_name){
    file.open(file_name, ios::in);
    return file.good();
}

bool IsValidStreak(string first_date, string second_date){
    Date prev_date = DateToStruct(first_date);
    Date now_date = DateToStruct(second_date);

    int prev_y = stoi(prev_date.y);
    int prev_m = stoi(prev_date.m);
    int prev_d = stoi(prev_date.d);

    int now_y = stoi(now_date.y);
    int now_m = stoi(now_date.m);
    int now_d = stoi(now_date.d);

    if(now_y - prev_y == 0){
        if(now_m - prev_m == 1 && now_d == 1 && prev_d == daysInMonth[prev_m - 1]){
            return true;
        } else if(now_m == prev_m && now_d - prev_d == 1)
            return true;
    } else if(now_y - prev_y == 1 && now_m == 1 && prev_m == 12 && now_d == 1 && prev_d == 31) 
        return true;

    return false;
}

void ReadData(){
    if(IsValidFile("habits.csv")){
        file.close();
        file.open("habits.csv", ios::in);

        string line;

        // first line
        getline(file, line);
        stringstream ss(line);

        string data;
        // date
        getline(ss, data, ',');
        // habits name
        while(getline(ss, data, ',')){
            habits.push_back(data);
        }

        while(getline(file, line)){
            stringstream ss(line);
            // date
            getline(ss, data, ',');
            string current_line_date = data;
            dates.push_back({current_line_date, DateToNum(current_line_date)});

            progressBydate[current_line_date] = 0;

            int idx = 0;
            // habits data
            while(getline(ss, data, ',')){
                tracking_data[habits[idx]][current_line_date] = (data == "1"? 1:0);
                if(data == "1") progressBydate[current_line_date]++;
                idx++;
            }
        }
        file.close();
    }
}

void PreviewData(){
    for(auto date: dates){
        cout << date.first << '\n';
        for(auto habit: habits){
            cout << tracking_data[habit][date.first] << " : " << habit << '\n';
        }
        cout << '\n';
    }
}

void PrintMenu(bool valid){
    cout << "✦--- Menu ---✦\n";
    cout << "[1] Track Today\n";
    cout << "[2] Track The Day\n";
    cout << "[3] Manage My Habits\n";
    cout << "[4] Personal Dashboard\n";
    cout << "[5] Save & Exit\n";
    cout << "-----------------------------\n";
    if(!valid) cout << "Invalid command! Please follow the intructions.\n";
    cout << "Input your command (1 - 5): ";
}

void PrintDateAndHabitStatus(string input_date=""){
    cout << "==================================================\n";
    if(input_date == "") cout << "\t🗓️  Daily Checklist - " << CurrentDate(1) << '\n';
    else cout << "\t🗓️  Checklist on - " << DateToMDY(input_date) << '\n';
    cout << "==================================================\n";
    cout << "\n✨ Habits Status ✨\n";
    for(int i = 0; i < habits.size(); i++){
        string tmp_date = (input_date==""?CurrentDate(0) : input_date);
        cout << "[" << (tracking_data[habits[i]][tmp_date]? "/":" ") << "] " << " " << habits[i] << '\n'; 
        
    }
    
    cout << "\n--------------------------------------------------\n";
}

void PrintHabitStatus(){
    cout << "✨ Habits Status ✨\n";

    for(int i = 0; i < habits.size(); i++){
        cout << "[" << (tracking_data[habits[i]][CurrentDate(0)]? "/":" ") << "] " << " " << i + 1 << ". " << habits[i] << '\n'; 
    }
}

string PreviousMonth(string y, string m){
    if(stoi(m) == 1){
        return to_string(stoi(y) - 1) + "-12";
    }else return y + "-" + to_string(stoi(m) - 1);
}

string HeatColor(int completed, int total) {
    float ratio = (total == 0) ? 0 : (float)completed / total;

    if (ratio == 0)   return "\033[48;2;44;44;41m";   // gray  (0%)
    if (ratio <= 0.25) return "\033[48;2;159;225;203m\033[30m"; // teal 100
    if (ratio <= 0.50) return "\033[48;2;93;202;165m\033[30m";  // teal 200
    if (ratio <= 0.75) return "\033[48;2;29;158;117m\033[97m";  // teal 400
    return             "\033[48;2;15;110;86m\033[97m";          // teal 600 (100%)
}

string ResetColor(){
    return "\033[0m";
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

int WeeklyAvg(string input_date = ""){
    float sum = 0;

    string now_date = (input_date == "" ? CurrentDate(0): input_date);
    while(getDayOfWeek(DateToStruct(now_date)) != 1){
        sum += progressBydate[now_date];
        now_date = Yesterday(now_date);
    }
    sum += progressBydate[now_date];
    return int(sum / (7 * habits.size()) * 100);
}

int WeeklyHabitAvg(string input_habit, string input_date = ""){
    float sum = 0;

    string now_date = (input_date == "" ? CurrentDate(0): input_date);
    while(getDayOfWeek(DateToStruct(now_date)) != 1){
        sum += tracking_data[input_habit][now_date];
        now_date = Yesterday(now_date);
    }
    sum += tracking_data[input_habit][now_date];
    return int(sum / 7 * 100);
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

bool IsLeapYear(string input_year){
    int year = stoi(input_year);

    if(year % 400 == 0) return true;
    if(year % 4 == 0 && year % 100) return true;

    return false;

}

void PrintCalendar(string month, string year){
    int space = 0;  
    int firstDay = getDayOfWeek(DateToStruct(year + "-" + month + "-" + "01"));
    if(!firstDay) firstDay = 7;

    string shades[] = {"\033[48;2;44;44;41m", "\033[48;2;159;225;203m\033[30m",
                       "\033[48;2;93;202;165m\033[30m", "\033[48;2;29;158;117m\033[97m",
                       "\033[48;2;15;110;86m\033[97m"};
    
    for(int i = 0; i < 7; i++){
        cout << "\033[107m\033[30m" << setw(4) << DAY[i] << ResetColor();
        space++;
        if(space%7 == 0) cout << "\n";
    }
    for(int i = 1; i < firstDay; i++){
        cout << setw(4) << " ";
        space++;
        if(space%7 == 0) cout << "\n";
    }
    int days = daysInMonth[stoi(month) - 1];
    if(month == "02" && IsLeapYear(year)) days = 29;
    
    for(int i = 1; i <= days; i++){
        string now_date = DateFormatting(year + "-" + month + "-" + to_string(i));
        cout << HeatColor(progressBydate[now_date], habits.size()) << setw(3) << i << " "<< ResetColor();
        space++;
        if(space%7 == 0) cout << "\n";
    }

    cout << "\n\n less  ";
    for (auto& s : shades) cout << s << "  " << "\033[0m" << " ";
    cout << " more\n";
}

void CalculateStreak(Streak &streak){
    streak.all = 1;

    DateSort();

    for(int i = 0; i < habits.size(); i++) {
        streak.habit[habits[i]] = 1;

        for(int j = 1; j < dates.size(); j++){
            if(dates[j].second > DateToNum(CurrentDate(0))) break;
            if(IsValidStreak(dates[j-1].first, dates[j].first) && tracking_data[habits[i]][dates[j - 1].first] && tracking_data[habits[i]][dates[j].first])
                streak.habit[habits[i]]++; 
            else streak.habit[habits[i]] = 1;

            bestStreak.habit[habits[i]] = max(bestStreak.habit[habits[i]], streak.habit[habits[i]]);
        }
    }

    for(int j = 1; j < dates.size(); j++){
        if(IsValidStreak(dates[j-1].first, dates[j].first) && progressBydate[dates[j-1].first] && progressBydate[dates[j].first]){
            streak.all++;
        }else streak.all = 1;
        
        bestStreak.all = max(bestStreak.all, streak.all);
    }
}

/*
1. Track Today
2. Track The Day
3. Manage My Habits
4. Personal Dashboard
5. Exit
*/

void TrackToday(){

    ClearScreen();

    PrintDateAndHabitStatus();

    string now_date = CurrentDate(0);

    //New day concern
    if(!BinarySearchDate(now_date)) dates.push_back({now_date, DateToNum(now_date)}), progressBydate[now_date] = 0;   

    if(statusTrackToday){
        cout << "[!] Entry Mode: Type 'y' for Success, 'n' for Miss.\n\n";
        
        for(int i = 0; i < habits.size(); i++){
            if(statusTrackToday == 2 && tracking_data[habits[i]][now_date]) continue;

            cout << "> [" << i+1 << "/" << habits.size() << "] Did you complete " << habits[i] << " ?: ";
            string input; GetLine(input);
            cout << '\n';
            
            tracking_data[habits[i]][now_date] = (input == "y" || input == "Y"? 1 : 0);
            if(input == "y" || input == "Y") progressBydate[now_date]++;
        }

        statusTrackToday = 0;
    }

    ClearScreen();
    PrintDateAndHabitStatus();

    bool completed = (progressBydate[now_date] == habits.size());
    int cmdNum = 2;

    if(!completed) cout << "[1] Edit today's status · [2] Check the remaining · [q] Back to Menu\nInput command: ";
    else cout << "[1] Edit today's status · [q] Back to Menu\nInput command: ", cmdNum--;

    string cmd;
    GetLine(cmd);

    while(!IsValidCmd(cmd, cmdNum)){
        ClearScreen();
        PrintDateAndHabitStatus();

        cout << "Invalid Command! Please follow the instructions.\n";
        if(!completed) cout << "[1] Edit today's status · [2] Check the remaining · [q] Back to Menu\nInput command: ";
        else cout << "[1] Edit today's status · [q] Back to Menu\nInput command: ";
        GetLine(cmd);
    }

    if(cmd == "1"){ // Edit today's status
        statusTrackToday = 1;
        progressBydate[now_date] = 0;
        TrackToday();  
    }else if(cmd == "2") { //Check the remaining
        statusTrackToday = 2;
        TrackToday();  
    }

    SaveData();
}

void TrackTheDay(string fixed_date){
    ClearScreen();

    string input_date;

    if(fixed_date == ""){
        cout << "[q] Back to Menu\n";
        cout << "What's a date you want to track? (year-month-day, e.g. 2026-05-18) : ";
        cin >> input_date;
        
        if(input_date == "q") return;
        
        
        while(!IsValidDate(input_date)){
            ClearScreen();
            cout << "[q] Back to Menu\n";
            cout << "Invalid Date! Please input valid date.\n";
            cout << "What's a date you want to track? (year-month-day, e.g. 2026-05-18) : ";
            cin >> input_date;

            if(input_date == "q") return;
        }

        input_date = DateFormatting(input_date);
        
        if(!BinarySearchDate(input_date)) {
            dates.push_back({input_date, DateToNum(input_date)});
            DateSort();
            progressBydate[input_date] = 0;
        }

    } else input_date = fixed_date;

    ClearScreen();
    PrintDateAndHabitStatus(input_date);

    cout << "[!] Entry Mode: Type 'y' for Success, 'n' for Miss.\n\n";
        
    for(int i = 0; i < habits.size(); i++){
        cout << "> [" << i+1 << "/" << habits.size() << "] Did you complete " << habits[i] << " ?: ";
        string input; cin >> input;
        cout << '\n';
        
        tracking_data[habits[i]][input_date] = (input == "y" || input == "Y"? 1 : 0);
        if(input == "y" || input == "Y") progressBydate[input_date]++;
    }

    ClearScreen();
    PrintDateAndHabitStatus(input_date);

    cout << "[1] Edit this day's status · [2] Track another day · [q] Back to Menu\nInput command: ";
    string cmd; 
    GetLine(cmd, 1);

    while(!IsValidCmd(cmd, 3)){
        ClearScreen();
        PrintDateAndHabitStatus(input_date);

        cout << "Invalid Command! Please follow the instructions.\n";
        cout << "[1] Edit this day's status · [2] Track another day · [q] Back to Menu\nInput command: ";
        GetLine(cmd);
    }

    if(cmd == "1"){
        progressBydate[input_date] = 0;
        TrackTheDay(input_date);
    }else if(cmd == "2"){
        TrackTheDay();
    }

    SaveData();
}

void ManageMyHabits(){
    ClearScreen();
    PrintHabitStatus();
    cout << "\n--------------------------------------------------\n";

    cout << "[1] Add new habit · [2] Remove a habit · [q] Back to Menu\nInput command: ";
    string cmd;
    GetLine(cmd);   

    while(!IsValidCmd(cmd, 3)){
        ClearScreen();
        PrintHabitStatus();
        cout << "\n--------------------------------------------------\n";

        cout << "Invalid Command! Please follow the instructions.\n";
        cout << "[1] Add new habit · [2] Remove a habit · [q] Back to Menu\nInput command: ";
        GetLine(cmd);
    }

    if(cmd == "1"){
        ClearScreen();
        PrintHabitStatus();
        cout << "\n--------------------------------------------------\n";

        cout << "What's your new habit? : ";

        string new_habit;
        GetLine(new_habit);

        cout << "You want to add \"" << new_habit << "\"? (y/n) : ";
        GetLine(cmd);

        if(cmd == "y"){
            habits.push_back(new_habit);
        }
        
        ManageMyHabits();
    }else if(cmd == "2"){
        ClearScreen();
        PrintHabitStatus();
        cout << "\n--------------------------------------------------\n";

        cout << "Habit you want to remove (1 - " << habits.size() << ") : ";
        GetLine(cmd);
        while(!IsValidCmd(cmd, habits.size())) { 
            ClearScreen();
            PrintHabitStatus();
            cout << "\n--------------------------------------------------\n";

            cout << "Invalid order! Please input existing habit.\n";
            cout << "Habit you want to remove (1 - " << habits.size() << ") : ";
            GetLine(cmd);
        }
        int order = stoi(cmd);

        cout << "You want to remove \"" << habits[order - 1] << "\"? (y/n) : ";
        GetLine(cmd);

        if(cmd == "y"){
            habits.erase(habits.begin() + (order-1));
        }
        
        ManageMyHabits();
    }
}

void PersonalDashboard(string input_date){
    ClearScreen();
    ClearPreviousLines(1);

    if(input_date == "") input_date = CurrentDate(0);
    Date current = DateToStruct(DateFormatting(input_date)); 

    //--------------- Streak Stat ---------------
    Streak streak;
    CalculateStreak(streak);

    //Reset if missed yesterday
    if(!progressBydate[CurrentDate(0)] && !progressBydate[Yesterday(CurrentDate(0))]) streak.all = 0;
    for(auto habit:habits){
        if(!tracking_data[habit][CurrentDate(0)] && !tracking_data[habit][Yesterday(CurrentDate(0))])
            streak.habit[habit] = 0; 
    }

    int thisWeekAvg = WeeklyAvg();

    cout << left << setw(25) << "streak";
    cout << left << setw(25) << "today's progress";
    cout << "weekly avg\n";

    cout << "\033[38;2;255;165;0m" << "\033[48;2;44;44;41m" << left << setw(25) << to_string(streak.all) + " day(s)";
    cout << "\033[38;2;29;158;117m" << left << setw(25) << to_string(progressBydate[CurrentDate(0)]) + "/" + to_string(habits.size());
    cout << "\033[38;2;175;183;255m"<< left << setw(18) << to_string(thisWeekAvg) + "%";
    cout << "\033[0m\n";

    cout << left << setw(25) << "personal best: " + to_string(bestStreak.all);
    cout << left << setw(25) << to_string(int(progressBydate[CurrentDate(0)] / (float)habits.size() * 100)) + "% complete";

    string lastWeek = CurrentDate(0);
    for(int i = 7; i > 0; i--) {
        lastWeek = Yesterday(lastWeek);
        if(getDayOfWeek(DateToStruct(lastWeek)) == 0) break;
    }
    int compareLastWeek = thisWeekAvg - WeeklyAvg(lastWeek);
    cout << left << setw(10) << (compareLastWeek >= 0? "↑ " : "↓ ") + to_string(abs(compareLastWeek)) + "% vs last week";

    cout << "\n\n";

    //--------------- Habit Status ---------------
    cout << "\033[48;2;53;59;70m" << "  TODAY\'S HABITS  \n" << "\033[0m";
    for(auto habit : habits){
        cout << (tracking_data[habit][CurrentDate(0)] ? "\033[32m/" : "\033[31mX")  << "\033[0m " ; 
        cout << left << setw(50) << habit;
        cout << (tracking_data[habit][CurrentDate(0)] ? "\033[32mdone" : "\033[31mmissed")  << "\033[0m"<< '\n'; 
    }   
    cout << "\n";

    //--------------- Weekly Completion ---------------
    cout << "\033[48;2;53;59;70m" << "  WEEKLY COMPLETION  \n" << "\033[0m";

    paletteIdx = 0;
    for(auto habit : habits){
        cout << setw(30) << habit;
        int completionRate = WeeklyHabitAvg(habit);
        const int barSize = 21; 
        int completionBar = ceil(float(completionRate) * barSize / 100);
        for(int i = 0; i < completionBar; i++) cout << palette[paletteIdx] << "■";
        for(int i = 0; i < barSize - completionBar; i++) cout << "\033[38;2;40;40;40m" << "■";
        cout << " " << palette[paletteIdx] << left << setw(4) << to_string(completionRate) + "%";
        cout << "\033[0m" << " | " << streak.habit[habit] << "d streak" << '\n';

        paletteIdx++; paletteIdx %= palette.size();
    }
    cout << "\n";

    //--------------- Best & Needs Work Habit ---------------
    cout << "\033[48;2;53;59;70m" << "BEST HABIT";
    cout << "\033[0m" << setw(30) << " ";
    cout << "\033[48;2;53;59;70m" << "NEEDS WORK\n\033[0m";

    string best, worst;
    int mn = 1e9, mx = 0;
    for(auto habit : habits){
        int habitAvg = WeeklyHabitAvg(habit);
        if(habitAvg >= mx) mx = habitAvg, best = habit;
        if(habitAvg <= mn) mn = habitAvg, worst = habit;
    }

    cout << "\033[38;2;66;143;255m" << left << setw(40) << best << "\033[0m";
    cout << "\033[38;2;255;66;66m" << worst << "\033[0m\n";

    cout << left << setw(41) << to_string(WeeklyHabitAvg(best)) + "% this week · " + to_string(streak.habit[best]) + "d streak"; 
    cout << to_string(WeeklyHabitAvg(worst)) + "% this week · " + to_string(streak.habit[worst]) + "d streak"; 
    cout << "\n\n";

    //--------------- Heat Map ---------------
    cout << "\033[48;2;53;59;70m" << MONTH[stoi(current.m) - 1] << ", " << current.y << "\033[0m" << '\n';
    PrintCalendar(current.m, current.y);

    //-------------------------------------------------------

    cout << "\n--------------------------------------------------------------------\n";
    if(input_date == CurrentDate(0))
        cout << "[1] Previous Month · [q] Back to Menu\nInput command: ";
    else cout << "[1] Previous Month · [2] This Month · [q] Back to Menu\nInput command: ";
    string cmd;
    GetLine(cmd);   

    bool invalidCheck = 0;
    while(!IsValidCmd(cmd, 3)){
        ClearPreviousLines((invalidCheck? 3: 2));

        cout << "Invalid Command! Please follow the instructions.\n";
        if(input_date == CurrentDate(0))
            cout << "[1] Previous Month · [q] Back to Menu\nInput command: ";
        else cout << "[1] Previous Month · [2] This Month · [q] Back to Menu\nInput command: ";
        invalidCheck = 1;
        GetLine(cmd);
    }

    if(cmd == "1"){
        PersonalDashboard(PreviousMonth(current.y, current.m) + "-" + current.d);
    }else if(cmd == "2" && input_date != CurrentDate(0)){
        PersonalDashboard();
    }
}

void SaveData(){
    if(habits.empty()) return;
    
    file.open("habits.csv", ios::out);

    file << "date,";
    for(int i = 0; i < habits.size() - 1; i++) file << habits[i] << ",";
    file << habits.back() << '\n';

    sort(dates.begin(), dates.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });
    for(int i = 0; i < dates.size(); i++){
        file << dates[i].first << ",";

        for(int j = 0; j < habits.size() - 1; j++) file << tracking_data[habits[j]][dates[i].first] << ",";
        file << tracking_data[habits.back()][dates[i].first];

        if(i < dates.size() - 1) file << '\n';
    }

    file.close();
}

int main(){
    ReadData();

    //PreviewData();

    //Menu
    while(1){
        ClearScreen();
        
        PrintMenu(1);
        
        string cmd;
        GetLine(cmd);
        
        while(!IsValidCmd(cmd, 7)){
            ClearScreen();
            PrintMenu(0);
            GetLine(cmd);
        }
        
        ClearScreen();
        
        if(cmd == "1"){
            TrackToday();
        }else if(cmd == "2"){
            TrackTheDay();
        }else if(cmd == "3"){
            ManageMyHabits();
        }else if(cmd == "4"){
            PersonalDashboard();
        }else if(cmd == "5"){
            break;
        }
    }

    //TrackToday();
    //ManageMyHabits();
    //PersonalDashboard();

    return 0;
}