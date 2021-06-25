//
// Created by 86152 on 2021/6/20.
//
#include "Interpreter.h"

using namespace std;
extern CM CatalogManager;

DataClass convert(string value){
    int n = 0, i = 0;
    for(i=0; i<value.length(); i++)
        if(value[i] == '.')  n++;
        else if(value[i] < '0' || value[i] > '9') break;
    DataClass ptr;
    if(value[0] == '\'' && value[value.size()-1] == '\'' || value[0] == '\"' && value[value.size()-1] == '\"')
        ptr = DataClass(value.substr(1, value.length() - 2));
    else if(i==value.length() && n==0) ptr = DataClass(atoi(value.c_str()));
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
    for(i=0; i<a.length()-1 && (a[i]==' ' || a[i]==',' || a[i]==')'); i++) ;
    return a.substr(i);
}

void Select(string command){
    int i, j;
    vector<string> query;
    vector<SelectCondition> condition;
    string table;
    command = DeleteSpace(command.substr(6));
    while(command.substr(0,4) != "from"){
        for(i=0; command[i]!=',' && command[i]!=' '; i++) ;
        query.push_back(command.substr(0, i));
        command = DeleteSpace(command.substr(i + (command[i] == ' ')));
    }
    command = DeleteSpace(command.substr(4));
    for(i=0; command[i]!=' ' && command[i]!=';'; i++) ;
    char c = command[i];
    table = command.substr(0, i);
    command = DeleteSpace(command.substr(i));
    if(c!=';'){
        command = DeleteSpace(command.substr(5));
        while(1)
        {
            for (i = 0; i < command.length() && command[i] != '!' && command[i] != '=' && command[i] != '<' &&
                        command[i] != '>'; i++);
            string attr = command.substr(0, i);
            trim(attr);
            command = DeleteSpace(command.substr(i));
            string op = command.substr(0,1);
            if (command[1] == '=') op += "=";
            command = DeleteSpace(command.substr(1 + (command[1] == '=')));
            for (i=0; i < command.length() && command[i] != ';'  && command[i] != ',' && command[i] != ' '; i++);
            string value = command.substr(0, i);
            trim(value);
            DataClass ptr = convert(value);
            SelectCondition tmp = {attr, op, ptr};
            condition.push_back(tmp);
            if (command[i] == ';') break;
            command = DeleteSpace(command.substr(i+1));
            if(command.substr(0, 3) == "and")
                command = DeleteSpace(command.substr(3));
        }
    }
    PrintResult res;
    try {
        std::vector<Tuple> result_set = SelectTuples(condition, table);
        res.SelectTuple(table, result_set);
    }
    catch(SQLError::TABLE_ERROR e){
        e.PrintError();
    }
    catch(...){
        std::cout << "An error took place!" << std::endl;
    }
}

void Insert(string command){
    int i, j;
    vector<string> attr;
    vector<DataClass> content;
    string table;
    command = DeleteSpace(command.substr(11));
    for (i = 0; command[i] != ' '; i++);
    table = command.substr(0, i);
    /*command = DeleteSpace(command.substr(i+1));
    while(1) {
        for (i = 0; command[i] != ',' && command[i] != ' '&& command[i] != ')'; i++);
        attr.push_back(command.substr(0, i));
        if(command[i] == ')') break;
        command = DeleteSpace(command.substr(i + (command[i] == ' ')));
    }*/
    for (i = 0; command[i] != '('; i++);
    command = DeleteSpace(command.substr(i+1));
    while(1) {
        for (i = 0; command[i] != ','&& command[i] != ')'; i++);
        DataClass ptr = convert(command.substr(0, i));
        content.push_back(ptr);
        if(command[i] == ')') break;
        command = DeleteSpace(command.substr(i + (command[i] == ' ')));
    }
    // if(attr.size() != content.size())
    //     std::cout<<"The number of parameters is wrong\n";
    PrintResult res;
    try{
        res.InsertTuple(InsertTuple(table, content));
    }
    catch(SQLError::TABLE_ERROR e){
        e.PrintError();
    }
    catch(SQLError::KEY_INSERT_ERROR e){
        e.PrintError();
    }
    catch(...){
        std::cout << "An error took place!" << std::endl;
    }
}

void Delete(string command){
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
    for(i=0; command[i]!=' ' && command[i]!=';'; i++) ;
    char c = command[i];
    table = command.substr(0, i);
    command = DeleteSpace(command.substr(i));
    if(c!=';'){
        command = DeleteSpace(command.substr(5));
        while(1)
        {
            for (i = 0; i < command.length() && command[i] != '!' && command[i] != '=' && command[i] != '<' &&
                        command[i] != '>'; i++);
            string attr = command.substr(0, i);
            trim(attr);
            command = DeleteSpace(command.substr(i));
            string op = command.substr(0,1);
            if (command[1] == '=') op += "=";
            command = DeleteSpace(command.substr(1 + (command[1] == '=')));
            for (i=0; i < command.length() && command[i] != ';'  && command[i] != ',' && command[i] != ' '; i++);
            string value = command.substr(0, i);
            trim(value);
            DataClass ptr = convert(value);
            SelectCondition tmp = {attr, op, ptr};
            condition.push_back(tmp);
            if (command[i] == ';') break;
            command = DeleteSpace(command.substr(i+1));
            if(command.substr(0, 3) == "and")
                command = DeleteSpace(command.substr(3));
        }
    }
    try{
        DeleteTuples(condition, table);
    }
    catch(SQLError::TABLE_ERROR e){
        e.PrintError();
    }
    catch(...){
        std::cout << "An error took place!" << std::endl;
    }
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
        int primary=-1;
        command = DeleteSpace(command.substr(5));
        for (i = 0; command[i] != '('; i++);
        string table = command.substr(0, i);
        command = DeleteSpace(command.substr(i+1));
        trim(table);
        for(int j=0; ; j++){
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
            command = DeleteSpace(command.substr(i));
            if(primary == -1 && command.substr(0, 7)=="primary") {
                primary = j;
                command = DeleteSpace(command.substr(7));
            }
            else if(command.substr(0, 6)=="unique") {
                type = "unique" + type;
                command = DeleteSpace(command.substr(6));
            }
            types.push_back(type);
            if(command[0] == ';') break;
        }
        try{
            CreateTable(table, property, types, primary);
        }
        catch(SQLError::TABLE_ERROR e){
            e.PrintError();
        }
        catch(...){
            std::cout << "An error took place!" << std::endl;
        }
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
            try{
                CreateIndex(table, index, column);
            }
            catch(SQLError::TABLE_ERROR e){
                e.PrintError();
            }
            catch(...){
                std::cout << "An error took place!" << std::endl;
            }
        }
        else
            std::cout<<"Sorry, MiniSQL can't interpret your command, try again\n";
    }
    else
        std::cout<<"Sorry, MiniSQL can't interpret your command, try again\n";
}

void Drop(string command){
    int i, j;
    command = DeleteSpace(command.substr(4));
    try{
        if(command.substr(0, 8)=="database"){
            std::cout<<"Successfully Drop!";
        }
        else if(command.substr(0, 5)=="table"){
            command = DeleteSpace(command.substr(5));
            command.erase(command.length() - 1);
            trim(command);
            if(DropTable(command))
                std::cout<<"Successfully Drop!";
        }
        else if(command.substr(0, 5)=="index"){
            command = DeleteSpace(command.substr(5));
            command.erase(command.length() - 1);
            trim(command);
            if(DropIndex(command))
                std::cout<<"Successfully Drop!";
        }
        else
            std::cout<<"Sorry, MiniSQL can't interpret your command, try again\n";
    }
    catch(SQLError::TABLE_ERROR e){
        e.PrintError();
    }
    catch(...){
        std::cout << "An error took place!" << std::endl;
    }
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
    else if(command.substr(0,4) == "drop")
        Drop(command);
    else
        std::cout<<"Sorry, MiniSQL can't interpret your command, try again\n";
}



void PrintResult::CreateTable(bool is_created)
{
    if(is_created)
    {
        std::cout<<"table create succeeded!"<<std::endl;
    }
    else
    {
        std::cout<<"table create failed!"<<std::endl;
    }
}

void PrintResult::DropTable(bool is_dropped)
{
    if(is_dropped)
    {
        std::cout<<"table drop succeeded!"<<std::endl;
    }
    else
    {
        std::cout<<"table drop failed!"<<std::endl;
    }
}

void PrintResult::InsertTuple(bool is_inserted)
{
    if(is_inserted)
    {
        std::cout<<"tuple insert succeeded!"<<std::endl;
    }
    else
    {
        std::cout<<"tuple insert failed!"<<std::endl;
    }
}

void PrintResult::SelectTuple(std::string table_name, std::vector<Tuple> tuple)
{
    std::cout<<"====="<<table_name<<"====="<<std::endl;
    TableInfo &table_info = CatalogManager.LookUpTableInfo(table_name);
    if(tuple.size() == 0)
    {
        std::cout<<" ----- empty -----"<<std::endl;
        return;
    }
    else
    {
        //打印列名
        for(int i  = 0;i<table_info.n_columns();i++)
        {
            std::cout<< "|" <<table_info[i].column_name<<"\t";
        }
        std::cout<< std::endl;
        //分割线
        //打印每一条记录
        for(int i = 0;i<tuple.size();i++)
        {
            for(int j = 0; j<table_info.n_columns();j++)
            {
                if(tuple[i].data_list[j].type == DataType::INT)
                    std::cout<<tuple[i].data_list[j].data.i<<"\t";
                else if(tuple[i].data_list[j].type == DataType::FLOAT)
                    std::cout<<tuple[i].data_list[j].data.f<<"\t";
                else if(tuple[i].data_list[j].type == DataType::CHAR){
                    std::string s(tuple[i].data_list[j].data.str);
                    std::cout << s << "\t";
                }
            }
            std::cout<<endl;
        }
        std::cout << "Retrived " << tuple.size() << " tuples" << std::endl;
   }
}

void PrintResult::DeleteTuple(int DeleteNum)
{
    if( DeleteNum == 1)
    {
        std::cout<<DeleteNum<<" tuple delete succeed!"<<std::endl;
    }
    else if(DeleteNum > 1)
    {
        std::cout<<DeleteNum<<" tuples delete succeed!"<<std::endl;
    }
    else
    {
        std::cout<<"0 tuple delete!"<<std::endl;
    }
}

void PrintResult::CreateIndex(bool is_created)
{
    if(is_created == 1)
    {
        std::cout<<"Index create succeeded!"<<std::endl;
    }
    else
    {
        std::cout<<"Index create failed!"<<std::endl;
    }
}
void PrintResult::DropIndex(bool is_dropped)
{
    if(is_dropped)
    {
         std::cout<<"Index drop succeeded!"<<std::endl;
    }
    else
    {
        std::cout<<"Index drop failed!"<<std::endl;
    }
}