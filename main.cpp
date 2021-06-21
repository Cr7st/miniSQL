#include <iostream>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

void Init();
void Run();
string ConsoleGetCommand();
vector<string> FileGetCommand(string file_name);
void Help();

int main(){
    Init();
    Run();
}

void Init()
{
    string space8 = "        ";
    string longline(60, '=');
    cout << longline << endl;
    cout << space8 << "    __  ___ _         _  _____  ____    __ " << endl;
    cout << space8 << "   /  |/  /(_)____   (_)/ ___/ / __ \\  / / " << endl;
    cout << space8 << "  / /|_/ // // __ \\ / / \\__ \\ / / / / / /  " << endl;
    cout << space8 << " / /  / // // / / // / ___/ // /_/ / / /___" << endl;
    cout << space8 << "/_/  /_//_//_/ /_//_/ /____/ \\___\\_\\/_____/" << endl;
    cout << "\n" << space8 << "    Welcome to MiniSQL Database System!" << endl;
    cout << "\n" << space8 << "    All the command should end with \";\"" << endl;
    cout << space8 << "    \"quit;\": command to quit the system" << endl;
    cout << space8 << "       \"help;\": command to get help" << endl;
    cout << "      \"file;\": command to read SQL query from a file" << endl;
    cout << "\n" << space8 << "  Bowen Zheng, Yunqi Chen and Zehao Zhu" << endl;
    cout << longline << endl;                                                                                             
}

void Run()
{
    string command;
    vector<string> command_set;
    string file_name;
    while (1){
        command = ConsoleGetCommand();
        if (command == "quit;")
            break;
        else if (command == "help;")
            Help();
        else if (command == "file;"){
            cout << "Please enter the query file name:" << endl;
            cin >> file_name;
            file_name = file_name + ".sql";
            command_set = FileGetCommand(file_name);
            for (int i = 0; i < command_set.size(); i++){
                //Parse(command_set[i]);
            }
        }
        else{
            //Parse(command);
        }
    }
}

string ConsoleGetCommand()
{
    cout << "MiniSQL: ";
    string tmp;
    string res;
    getline(cin, tmp);
    while (tmp[tmp.length() - 1] != ';'){
        cout << "          ";
        res = res + " ";
        res = res + tmp;
        getline(cin, tmp);
    }
    res = res + " " + tmp;
    return res;
}