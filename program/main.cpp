#include "globals.h"
#include "date.h"
#include "screen.h"
#include "fileManagement.h"

#include <cstdlib>
#include <cmath>
#include <algorithm>

using namespace std;

void TrackToday();
void TrackTheDay(string = "");
void ManageMyHabits();
void PersonalDashboard(string = "");
void SaveData();

Streak bestStreak;

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
        while(cmd == "q" || !IsValidCmd(cmd, habits.size())) { 
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

    SaveData();
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
    while(!IsValidCmd(cmd, (input_date == CurrentDate(0)? 1: 2))){
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

int main(){
    ReadData();

    //PreviewData();

    //Menu
    while(1){
        ClearScreen();
        
        PrintMenu(1);
        
        string cmd;
        GetLine(cmd);
        
        while(!IsValidCmd(cmd, 5)){
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