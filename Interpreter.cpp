//
// Created by 86152 on 2021/6/20.
//
#include "Interpreter.h"
using namespace std;

void trim(string &s)
{
    if (s.empty()) return;
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
}

string DeleteSpace(string a){
    int i;
    for(i=0; i<a.length() && a[i]!=' '; i++) ;
    return a.substr(i);
}

void Select(string command){
    int i, j;
    vector<string> query;
    vector<SelectCondition> condition;
    string table;
    command = DeleteSpace(command.substr(6));
    while(command.substr(0,4) != "from"){
        for(i=0; command[i]!=',' || command[i]!=' '; i++) ;
        query.push_back(command.substr(0, i));
        command = DeleteSpace(command.substr(i + (command[i] == ' ')));
    }
    command = DeleteSpace(command.substr(4));
    for(i=0; command[i]!=' '; i++) ;
    table = command.substr(0, i);
    command = DeleteSpace(command.substr(i));
    if(command.find("where") != std::string::npos){
        while(1)
        {
            for (i = 0; i < command.length() && command[i] != '!' && command[i] != '=' && command[i] != '<' &&
                        command[i] != '>'; i++);
            if (i == command.length()) break;
            string attr = command.substr(0, i);
            trim(attr);
            command = DeleteSpace(command.substr(i));
            string op = command.substr(0,1);
            if (command[1] == '=') op += "=";
            command = DeleteSpace(command.substr(1 + (command[1] != '=')));
            for (i=0; i < command.length() && command[i] != ',' && command.substr(i, i+3) != "and"; i++);
            string value = command.substr(0, i);
            trim(value);
            SelectCondition tmp = {attr, op, value};
            condition.push_back(tmp);
            if (i == command.length()) break;
            if(command.substr(i, i+3) != "and")
                command = DeleteSpace(command.substr(i+3));
            else
                command = DeleteSpace(command.substr(i+1));
        }
    }
    SelectTuples(condition, table);
}

void Insert(string command){
    int i, j;
    vector<string> attr, content;
    string table;
    command = DeleteSpace(command.substr(11));
    for (i = 0; command[i] != '('; i++);
    table = command.substr(0, i);
    command = DeleteSpace(command.substr(i+1));
    while(command.substr(0, 6) != "values") {
        for (i = 0; command[i] != ',' && command[i] != ' '&& command[i] != ')'; i++);
        attr.push_back(command.substr(0, i));
        if(command[i] == ')') break;
        command = DeleteSpace(command.substr(i + (command[i] == ' ')));
    }
    command = DeleteSpace(command.substr(6));
    while(1) {
        for (i = 0; command[i] != ','&& command[i] != ')'; i++);
        content.push_back(command.substr(0, i));
        if(command[i] == ')') break;
        command = DeleteSpace(command.substr(i + (command[i] == ' ')));
    }
    if(attr.size() != content.size())
        cout<<"The number of parameters is wrong\n";
    else
        InsertApi(attr, content);
}

void Delete(string command){

}

void Create(string command){
    int i, j;
    command = DeleteSpace(command.substr(6));
    if(command.substr(0, 8)=="database"){
        command = DeleteSpace(command.substr(8));
        for (i = 0; command[i] != ','; i++);
        command = command.substr(0, i);
        trim(command);
        CreateDBApi(command);
    }
    else if(command.substr(0, 5)=="table"){
        vector<TableProperty> property;
        string primary;
        command = DeleteSpace(command.substr(5));
        for (i = 0; command[i] != '('; i++);
        table = command.substr(0, i);
        trim(table);
        while(command[i] != ')'){
            TableProperty tmp;
            for (i = 0; command[i] != ' '; i++);
            attr = command.substr(0, i);
            for (i = 0; command[i] != ',' && command[i] != ' ' && command[i] != ')'; i++);
            if(command.find("primary")  != std::string::npos)
                primary = attr;
            type = command.substr(0, i);
            trim(type);
            if(type=="int")
                tmp = {attr, DataType::INT, 4};
            else if(type=="double" || type=="float")
                tmp = {attr, DataType::DOUBLE, 8};
            else {
                int sum = 0;
                if()
            }
            if(command[i] == ')') break;

        }
        property.push_back(tmp);
        CreateDBApi(table, property);
    }
}

void Interpreter(){
    cout<<("Welcome to MiniSQL (GNU/Linux 4.15.0-142-generic x86_64)\n\n"
           " * Documentation:  https://help.ubuntu.com\n"
           " * Management:     https://landscape.canonical.com\n"
           " * Support:        https://ubuntu.com/advantage\n\n"
           "Now you can input some SQL command, inupt help for more information\n");
    string command;
    int i;
    while (1) {
        cout<<">>> ";
        getline(cin, command);
        command = DeleteSpace(command);
        if(command=="exit")
            break;
        else if(command=="help")
            Help();
        else if(command.substr(0,6) == "select")
            Select(command);
        else if(command.substr(0,11) == "insert into")
            Insert(command);
        else if(command.substr(0,6) == "delete")
            Delete(command);
        else if(command.substr(0,6) == "create")
            Create(command);
        else
            cout<<"Sorry, MiniSQL can't interpret your command, try again\n";
    }
}
