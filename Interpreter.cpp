//
// Created by 86152 on 2021/6/20.
//
#include "Interpreter.h"
using namespace std;

DataClass convert(string value){
    int n = 0, i = 0;
    for(i=0; i<value.length(); i++)
        if(value[i] == '.')  n++;
        else if(value[i] < '0' || value[i] > '9') break;
    DataClass ptr;
    if(i<value.length()) ptr = DataClass(atoi(value.c_str()));
    else if(n==1) ptr = DataClass(atof(value.c_str()));
    else ptr = DataClass(value.c_str());
    return ptr;
}

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
            DataClass ptr = convert(value);
            SelectCondition tmp = {attr, op, ptr};
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
    vector<string> attr;
    vector<DataClass> content;
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
        DataClass ptr = convert(command.substr(0, i));
        content.push_back(ptr);
        if(command[i] == ')') break;
        command = DeleteSpace(command.substr(i + (command[i] == ' ')));
    }
    if(attr.size() != content.size())
        cout<<"The number of parameters is wrong\n";
    else
        InsertTuple(table, content);
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
        //CreateDBApi(command);
    }
    else if(command.substr(0, 5)=="table"){
        vector<string> property, types;
        int primary;
        command = DeleteSpace(command.substr(5));
        for (i = 0; command[i] != '('; i++);
        string table = command.substr(0, i);
        trim(table);
        for(int j=0; command[i] != ')'; j++){
            for (i = 0; command[i] != ' '; i++);
            string attr = command.substr(0, i);
            property.push_back(attr);
            command = DeleteSpace(command.substr(i));
            for (i = 0; command[i] != ',' && command[i] != ' ' && command[i] != ')'; i++);
            string type = command.substr(0, i);
            for (string::iterator it = type.begin(); it < type.end(); it++){
                if (*it == '(' || *it == ')')
                {
                    type.erase(it);
                    it--;
                }
            }
            trim(type);
            types.push_back(type);
            command = DeleteSpace(command.substr(i));
            if(command.substr(0, 7)=="primary")
                primary = j;
        }
        CreateTable(table, property, types, primary);
    }
    else if(command.substr(0, 5)=="index"){
        command = DeleteSpace(command.substr(5));
        for (i = 0; command[i] != ' '; i++);
        string index = command.substr(0, i);
        command = DeleteSpace(command.substr(i));
        if(command.substr(0,2)=="on"){
            command = DeleteSpace(command.substr(2));
            for (i = 0; command[i] != ' ' && command[i] != ')'; i++);
            string table = command.substr(0, i);
            for (j = 0; command[j] == ' ' || command[j] == '('; j++);
            for (i = j; command[i] != ' ' && command[i] != ')'; i++);
            string column = command.substr(i, j);
            CreateIndex(table, index, column);
        }
        else
            cout<<"Sorry, MiniSQL can't interpret your command, try again\n";
    }
    else
        cout<<"Sorry, MiniSQL can't interpret your command, try again\n";
}

void Drop(string command){
    int i, j;
    command = DeleteSpace(command.substr(4));
    if(command.substr(0, 5)=="table"){
        command = DeleteSpace(command.substr(5));
        for (i = 0; command[i] != ' ' && i<command.size(); i++);
        string table = command.substr(0, i);
        DropTable(table);
    }
    else if(command.substr(0, 5)=="index"){
        command = DeleteSpace(command.substr(5));
        for (i = 0; command[i] != ' ' && i<command.size(); i++);
        string index = command.substr(0, i);
        DropIndex(index);
    }
    else
        cout<<"Sorry, MiniSQL can't interpret your command, try again\n";
}

void Interpreter(std::string command){
    if(command.substr(0,6) == "select")
        Select(command);
    else if(command.substr(0,11) == "insert into")
        Insert(command);
    else if(command.substr(0,6) == "delete")
        Delete(command);
    else if(command.substr(0,6) == "create")
        Create(command);
    else if(command.substr(0,6) == "drop")
        Drop(command);
    else
        cout<<"Sorry, MiniSQL can't interpret your command, try again\n";
}
