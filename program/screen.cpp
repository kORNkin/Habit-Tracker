#include "screen.h"
#include "date.h"

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

bool IsValidCmd(string cmd, int end){
    if(pow(10, cmd.size() - 1) > end || !cmd.size()) return false;
    if(cmd == "q") return true;
    for(auto c : cmd) if(c - '0' < 0 || c - '0' > 9) return false;
    if(stoi(cmd) > end || stoi(cmd) < 1) return false;

    return true;
}

string HeatColor(int completed, int total) {
    float ratio = (total == 0) ? 0 : (float)completed / total;

    if (ratio == 0)   return "\033[48;2;44;44;41m";   // gray  (0%)
    if (ratio <= 0.25) return "\033[48;2;159;225;203m\033[30m"; // teal 100
    if (ratio <= 0.50) return "\033[48;2;93;202;165m\033[30m";  // teal 200
    if (ratio <= 0.75) return "\033[48;2;29;158;117m\033[97m";  // teal 400
    return             "\033[48;2;15;110;86m\033[97m";          // teal 600 (100%)
}

void PrintMenu(bool valid){
    cout << "✦--- Menu ---✦\n";
    cout << "[1] Track Today\n";
    cout << "[2] Track The Day\n";
    cout << "[3] Manage My Habits\n";
    cout << "[4] Personal Dashboard\n";
    cout << "[5] Exit\n";
    cout << "-----------------------------\n";
    if(!valid) cout << "Invalid command! Please follow the intructions.\n";
    cout << "Input your command (1 - 5): ";
}

void PrintDateAndHabitStatus(string input_date){
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

string ResetColor(){
    return "\033[0m";
}
