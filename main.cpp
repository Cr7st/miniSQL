#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "Interpreter.h"
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
    cout << "\n" << " Copyright(c) 2021 Bowen Zheng, Yunqi Chen and Zehao Zhu" << endl;
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
                Interpreter(command_set[i]);
            }
        }
        else{
            Interpreter(command);
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
        cout << "         ";
        res = res + " ";
        res = res + tmp;
        getline(cin, tmp);
    }
    res = res + " " + tmp;
    res.erase(0, 1);
    return res;
}

void Help()
{
    cout << R"(+------------------------------------------------------------------------------------------------+)" << std::endl;
	cout << R"(|A simple example to create a student databae named STU                                          |)" << std::endl;
	cout << R"(+------------------------------------------------------------------------------------------------+)" << std::endl;
	cout << R"(|Create database  : create database STU;                                                         |)" << std::endl;
	cout << R"(|Use database     : use database STU;                                                            |)" << std::endl;
	cout << R"(|Show database    : show databases;                                                              |)" << std::endl;
	cout << R"(|Create Table     : create table student(id int primary, socre double, name char(20));           |)" << std::endl;
	cout << R"(|Insert Record(1) : insert into student(id,score,name)values(1,95.5,ZhangSan);                   |)" << std::endl;
	cout << R"(|Insert Record(2) : insert into student(id,name)values(2,LiSi); Note:LiSi has no score           |)" << std::endl;
	cout << R"(|Delete Table     : delete from student where id = 1; Note: ZhangSan is deleted                  |)" << std::endl;
	cout << R"(|Select Table(1)  : select * from student where id = 2;                                          |)" << std::endl;
	cout << R"(|Select Table(2)  : select * from student where id > 1 and score < 98;                           |)" << std::endl;
	cout << R"(|Drop database    : drop database STU;                                                           |)" << std::endl;
	cout << R"(|Quit             : quit;                                                                        |)" << std::endl;
	cout << R"(+------------------------------------------------------------------------------------------------+)" << std::endl;
	cout << R"(|Note             : Anytime you want to end MiniSQL use "quit;" command please.                  |)" << std::endl;
	cout << R"(+------------------------------------------------------------------------------------------------+)" << std::endl;
}

vector<string> FileGetCommand(string file_name)
{
    vector<string> command_set;
    ifstream in_file;
    in_file.open(file_name);
    while (!in_file.eof()){
        string tmp;
        string res;
        getline(in_file, tmp);
        while (tmp[tmp.length() - 1] != ';'){
            res = res + " " + tmp;
            getline(in_file, tmp);
        }
        res = res + " " + tmp;
        res.erase(0, 1);
        command_set.push_back(res);
    }
    return command_set;
}