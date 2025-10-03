#include<iostream>
#include<vector>
#include<cmath>
#include<ctime> 
#include<fstream>
#include<cctype>  
#include<cstdio>  
#include<algorithm>
#include<limits>
#include <conio.h> 
#include <filesystem>
#include <windows.h>
using namespace std;
class Task;
class Event;
void Head();
int strToint(string A);
vector<Task> readAllTasks(const string& filename);
string getTasksFilePath();
string getExecutableDirectory();
bool prepareTasksFileForWrite(const string& path);
string formatWindowsError(DWORD code);
bool saveAllTasks(const vector<Task>& list, const string& path);



int levenshtein(const std::string& s1In, const std::string& s2In) {
    const size_t m = s1In.size();
    const size_t n = s2In.size();
    if (m == 0) return (int)n;
    if (n == 0) return (int)m;
    std::string s1(m, '\0'), s2(n, '\0');
    for (size_t i = 0; i < m; ++i) s1[i] = (char)tolower((unsigned char)s1In[i]);
    for (size_t j = 0; j < n; ++j) s2[j] = (char)tolower((unsigned char)s2In[j]);
    std::vector<int> prev(n + 1), curr(n + 1);
    for (size_t j = 0; j <= n; ++j) prev[j] = (int)j;
    for (size_t i = 1; i <= m; ++i) {
        curr[0] = (int)i;
        for (size_t j = 1; j <= n; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            curr[j] = std::min({
                prev[j] + 1,        
                curr[j - 1] + 1,    
                prev[j - 1] + cost  
            });
        }
        std::swap(prev, curr);
    }
    return prev[n];
}


string fuzzy_match(const std::string& input, const std::vector<std::string>& commands, int max_dist = 2) {
    for (const auto& cmd : commands) {
        if (levenshtein(input, cmd) <= max_dist)
            return cmd;
    }
    return "";
}

class Task{
    private:
        int serial=0;
        string goal;
        bool done=0;
        string date;
        vector<string> subtasks; 
    public:
        void newTask(int s,string g,string dat){
            serial=s;
            goal=g;
            date=dat;
        }
        void markDone(){
            done=true;
        }
        void extendDeadline(const string& newDate) {
            date = newDate;
        }
        void addSubtask(const string& sub) {
            subtasks.push_back(sub);
        }
        void view(){
            cout<<"Sno. : "<<serial<<endl;
            cout<<"Goal : "<<goal<<endl;
            cout<<"DeadLine : "<<date<<endl;
            if (!subtasks.empty()) {
                cout<<"Subtasks:"<<endl;
                for (size_t i = 0; i < subtasks.size(); ++i) {
                    cout<<"  - "<<subtasks[i]<<endl;
                }
            }
            cout<<endl;
        }
        string getdate() const {
            return date;
        }
        string entry() const{
            string ret="";
            ret = string(1,char('0'+serial)) + "," + goal + "," + date;
            
            if (!subtasks.empty()) {
                ret += ",[";
                for (size_t i = 0; i < subtasks.size(); ++i) {
                    if (i > 0) ret += "|";
                    ret += subtasks[i];
                }
                ret += "]";
            }
            ret += "\n";
            return ret;
        }
        void setSubtasks(const vector<string>& subs) {
            subtasks = subs;
        }
        friend void serialize(vector<Task>& list,int ser);
};

void serialize(vector<Task>& list, int ser) {
    for(int i=ser; i < list.size(); i++) {
        list[i].serial--;
    }
}

void runTaskManager(vector<Task>& list) {
    int listsize = list.size();
    Task obj;
    bool AddingTask=true;
    while (AddingTask)
    {   
        string goal;
        string date;
        char yn;
        cout<<"Any New Goal :: ";
        getline(cin,goal);
        cout<<"Can u please provide Last Date[DD/MM] :: ";
        getline(cin,date);
        
        obj.newTask(list.size()+1,goal,date);
        list.push_back(obj);
        while(true){
            cout << "Anything More ?[y/n] :: ";
            cin >> yn;
            cin.ignore();
            if (yn == 'n' || yn == 'N'){
                AddingTask=false;
                break;
            }
            else if(yn == 'y' || yn=='Y'){
                break;
            }
            else{
                cout<<"Sorry ! Cant Interpret Your Choice !!"<<endl;
            }
        }
    }
    
    saveAllTasks(list, getTasksFilePath());
}

string getPassword() {
    string password;
    char ch;
    while ((ch = _getch()) != '\r') {  
        if (ch == '\b') {  
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";  
            }
        } else {
            password += ch;
            cout << '*';  
        }
    }
    cout << endl;
    return password;
}

int main() {
    string command;
    vector<Task> list = readAllTasks(getTasksFilePath());
    Head();
    main:
    cout<<"Would you like me to check overdue tasks? [y/n] : ";
    char checkOverdue;
    cin >> checkOverdue;
    cin.ignore(); 
    if (checkOverdue == 'y' || checkOverdue == 'Y') {
        cout << "\nChecking overdue tasks...\n";
        vector<time_t> deadlines;
        bool overdue = false;
        for (Task &t : list)
        {
            
            struct tm tm = {};
            
            
            string date = t.getdate();
            int day = 0, month = 0;
            if (sscanf(date.c_str(), "%d/%d", &day, &month) == 2) {
                time_t now = time(0);
                struct tm *now_tm = localtime(&now);
                tm.tm_year = now_tm->tm_year; 
                tm.tm_mon = month - 1;
                tm.tm_mday = day;
                tm.tm_hour = 0;
                tm.tm_min = 0;
                tm.tm_sec = 0;
                deadlines.push_back(mktime(&tm));
            }
            else
            {
                cout << "Invalid date format for task: " << t.getdate() << endl;
            }
        }
        
        time_t now = time(0);
        for (size_t i = 0; i < deadlines.size(); ++i)
        {
            if (difftime(deadlines[i], now) < 0)
            {
                cout << "Task " << (i + 1) << " is overdue: " << list[i].getdate() << endl;
                overdue = true;
            }
        }
        if (!overdue) {
            cout << "Nothing is Overdue Sir.\n";
            cout << "You are all set for New ones!\n\n";            
        }
    } else if(checkOverdue == 'n' || checkOverdue == 'N') {
        cout << "Skipping overdue task check.\n\n";
    }
    else {
        cout << "Invalid input! Please enter 'y' or 'n'.\n";
        goto main; 
    }

    cout<<"WHAT DO YOU WANT TO DO TODAY ?"<<endl;
    
    std::vector<std::string> valid_commands = {
        "exit", "quit", "Add Task", "add task", "add tasks", "Add Tasks", "task manager", "Task Manager",
        "Show Tasks", "show tasks", "Show task", "show task",
        "Mark Done", "mark done", "Mark done", "mark Done", "mark as done", "Mark as done",
        "Extend Deadline", "extend deadline", "extend", "update deadline",
        "Add Subtask", "add subtask", "add subtasks", "Add Subtasks",
        "help", "Help"
    };
    while(true){
        cout<<">> ";
        getline(cin, command);

        std::string matched = fuzzy_match(command, valid_commands);
        if (matched.empty()) matched = command; 

        if(matched == "exit" || matched == "quit" || matched == "Quit" || matched == "Exit") {
            cout<<"See You Later !!"<<endl;
            break;
        }else if(matched == "Add Task" || matched == "add task" || matched == "add tasks" || matched == "Add Tasks" || matched == "task manager" || matched == "Task Manager") {
            runTaskManager(list);
        }else if(matched == "Show Tasks" || matched == "show tasks" || matched == "Show task" || matched == "show task") {
            if(list.empty()) {
                cout<<"No tasks available.\n";
            } else {
                cout<<" Your TODO List is Here :: \n\n";
                for(size_t i = 0; i < list.size(); ++i) {
                    list[i].view();
                }
            }
        }
        else if(matched == "Mark Done" || matched == "mark done" || matched == "Mark done" || matched == "mark Done" || matched == "mark as done" || matched == "Mark as done") {
            if(list.empty()) {
                cout<<"No tasks available to mark as done.\n";
            } else {
                int taskNumber;
                cout<<"Enter the task number to mark as done: ";
                cin>>taskNumber;
                cin.ignore();
                if(taskNumber > 0 && taskNumber <= list.size()) {
                    list[taskNumber - 1].markDone();
                    
                    list.erase(list.begin() + (taskNumber - 1));
                    
                    serialize(list, taskNumber - 1);
                    
                    saveAllTasks(list, getTasksFilePath());
                    cout<<"Removing From the List....\n";
                    cout<<"Task "<<taskNumber<<" marked as done.\n";
                } else {
                    cout<<"Sorry Cant Interpret that Serial No.\n";
                }
            }
        } else if(matched == "Extend Deadline" || matched == "extend deadline" || matched == "extend" || matched == "update deadline") {
            if(list.empty()) {
                cout<<"No tasks available to extend deadline.\n";
            } else {
                int taskNumber;
                cout<<"Enter the task number to extend deadline: ";
                cin>>taskNumber;
                cin.ignore();
                if(taskNumber > 0 && taskNumber <= list.size()) {
                    string newDate;
                    cout<<"Enter new deadline [DD/MM]: ";
                    getline(cin, newDate);
                    list[taskNumber - 1].extendDeadline(newDate);
                    
                    saveAllTasks(list, getTasksFilePath());
                    cout<<"Deadline updated for Task "<<taskNumber<<".\n";
                } else {
                    cout<<"Sorry Cant Interpret that Serial No.\n";
                }
            }
        }
        else if(matched == "Add Subtask" || matched == "add subtask" || matched == "add subtasks" || matched == "Add Subtasks") {
            if(list.empty()) {
                cout<<"No tasks available to add subtasks.\n";
            } else {
                int taskNumber;
                bool validInput = false;
                do {
                    cout<<"Enter the task number to add a subtask: ";
                    if(!(cin >> taskNumber)) {
                        cout << "Invalid input. Please enter a number.\n";
                        cin.clear();  
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');  
                        continue;
                    }
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');  
                    
                    if(taskNumber > 0 && taskNumber <= list.size()) {
                        validInput = true;
                    } else {
                        cout << "Sorry! Task number must be between 1 and " << list.size() << ".\n";
                    }
                } while(!validInput);

                string subtask;
                cout<<"Enter subtask description: ";
                getline(cin, subtask);
                list[taskNumber - 1].addSubtask(subtask);
                
                
                saveAllTasks(list, getTasksFilePath());
                cout<<"Subtask added to Task "<<taskNumber<<".\n";
            }
        }
        else if(matched == "help" || matched == "Help") {
            cout<<"Available Commands : \n";
            cout<<"Add Task\t";
            cout<<"Show Tasks\t";
            cout<<"Mark Done\t";
            cout<<"Extend Deadline\n";
            cout<<"Add Subtask\t";
            cout<<"Help\t";
            cout<<"Exit\t";
            cout<<endl;
        } else {
            cout<<"Unknown Command! Type 'help' for available commands.\n";
        }
    }
}

int strToint(string A){
    int value = 0;
    for(char ch : A){
        if (ch < '0' || ch > '9') break;
        value = value * 10 + (ch - '0');
    }
    return value;
}

void Head(){
    time_t now = time(0);
    tm *ltm = localtime(&now);
    cout<<endl;
    cout<<"   \\        /  |```  |     |```  |```|  |\\  /|  |```     /```  ___  |``\\"<<endl;
    cout<<"    \\  /\\  /   |=    |     |     |   |  | \\/ |  |=       `--.   |   |__/"<<endl;
    cout<<"     \\/  \\/    |___  |___  |___  |___|  |    |  |___     ___/  _|_  |  \\" <<endl;
    cout<<endl;
    cout<< "Today's Date: " 
         << ltm->tm_mday << "/" 
         << 1 + ltm->tm_mon << "/" 
         << 1900 + ltm->tm_year << endl<<endl;
}

vector<Task> readAllTasks(const string& filename) {
    vector<Task> tasks;
    ifstream file(filename);
    string line;
    vector<string> lines;
    if (!file.is_open()) {
        cout << "No Tasks are currently Added"<< endl;
        return tasks; 
    }
    lines.reserve(32);
    while (getline(file, line)) {
        lines.push_back(line);
    }
    file.close();
    if (lines.empty()) return tasks;
    
    auto isNumber = [](const string& s){
        if (s.empty()) return false;
        for (char c : s) if (!isdigit(static_cast<unsigned char>(c))) return false;
        return true;
    };
    size_t limit = lines.size();
    if (isNumber(lines.back())) {
        size_t expected = (size_t)strToint(lines.back());
        if (expected > 0) tasks.reserve(expected);
        limit = lines.size() - 1;
    } else {
        tasks.reserve(lines.size());
    }
    for (size_t i = 0; i < limit; ++i) {
        string entry = lines[i];
        size_t p1 = entry.find(',');
        size_t p2 = entry.find(',', p1 + 1);
        if (p1 == string::npos || p2 == string::npos) continue;
        int serial = strToint(entry.substr(0, p1));
        string goal = entry.substr(p1 + 1, p2 - p1 - 1);
        string date;
        vector<string> subtasks;
        size_t subStart = entry.find(",[", p2);
        if (subStart != string::npos) {
            date = entry.substr(p2 + 1, subStart - (p2 + 1));
            size_t subEnd = entry.find("]", subStart);
            string subStr = entry.substr(subStart + 2, subEnd - (subStart + 2));
            size_t pos = 0, found;
            while ((found = subStr.find('|', pos)) != string::npos) {
                subtasks.push_back(subStr.substr(pos, found - pos));
                pos = found + 1;
            }
            if (pos < subStr.size())
                subtasks.push_back(subStr.substr(pos));
        } else {
            date = entry.substr(p2 + 1);
        }
        Task t;
        t.newTask(serial, goal, date);
        if (!subtasks.empty()) t.setSubtasks(subtasks);
        tasks.push_back(t);
    }

    
    return tasks;
}

string getExecutableDirectory() {
    char buffer[MAX_PATH];
    DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    if (length == 0 || length == MAX_PATH) {
        return string(".");
    }
    string fullPath(buffer, length);
    size_t pos = fullPath.find_last_of("/\\");
    if (pos == string::npos) {
        return string(".");
    }
    return fullPath.substr(0, pos);
}

string getTasksFilePath() {
    string baseDir = getExecutableDirectory();
    std::error_code ec;
    std::filesystem::create_directories(baseDir, ec);
    std::filesystem::path filePath = std::filesystem::path(baseDir) / "Tasks.txt";
    return filePath.string();
}

bool prepareTasksFileForWrite(const string& path) {
    
    std::filesystem::path p(path);
    std::error_code ec;
    std::filesystem::create_directories(p.parent_path(), ec);
    
    DWORD attrs = GetFileAttributesA(path.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES) {
        DWORD cleared = attrs & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
        if (cleared != attrs) {
            SetFileAttributesA(path.c_str(), cleared);
        }
    }
    return true;
}

string formatWindowsError(DWORD code) {
    if (code == 0) code = GetLastError();
    LPVOID msgBuf;
    DWORD size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&msgBuf,
        0, NULL);
    string msg;
    if (size && msgBuf) {
        msg.assign((LPSTR)msgBuf, size);
        LocalFree(msgBuf);
    } else {
        msg = "Unknown error (code " + to_string(code) + ")";
    }
    return msg;
}

bool saveAllTasks(const vector<Task>& list, const string& path) {
    if (!prepareTasksFileForWrite(path)) {
        DWORD err = GetLastError();
        cout << "Error: Unable to prepare tasks file for writing at: " << path << "\n";
        cout << formatWindowsError(err) << "\n";
        return false;
    }
    ofstream entryFile(path, ios::trunc);
    if (!entryFile.is_open()) {
        DWORD err = GetLastError();
        cout << "Error: Unable to open tasks file for writing at: " << path << "\n";
        cout << formatWindowsError(err) << "\n";
        return false;
    }
    for (size_t i = 0; i < list.size(); i++)
    {
        entryFile << list[i].entry();
    }
    entryFile << list.size();
    entryFile.flush();
    entryFile.close();
    SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_ARCHIVE);
    return true;
}

