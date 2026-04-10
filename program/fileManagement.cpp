#include "fileManagement.h"
#include "date.h"

bool IsValidFile(const string &file_name){
    file.open(file_name, ios::in);
    return file.good();
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