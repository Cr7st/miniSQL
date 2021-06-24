#include <iostream>
#include <cstdio>
#include <cstring>
#include <time.h>
#include "API.h"
using namespace std;

int main()
{
    vector<string> data_types;
    vector<string> column_names;
    string table_name = string("student");
    data_types.push_back(string("char16"));
    data_types.push_back(string("int"));
    data_types.push_back(string("float"));
    column_names.push_back(string("name"));
    column_names.push_back(string("score"));
    column_names.push_back(string("height"));
    //CreateTable(table_name, column_names, data_types, 0);
    //OpenTable(string("student.db"));
    vector<DataClass> list;
    DataClass d1("ZZH");
    DataClass d2(9);
    DataClass d3(1.73);
    list.push_back(d1);
    list.push_back(d2);
    list.push_back(d3);
    vector<SelectCondition> conditions;
    //SelectTuples(conditions, table_name);
    SelectCondition s;
    s.op = "<";
    s.attr = "score";
    s.value = DataClass(int(9));
    vector<Tuple> res;
    conditions.push_back(s);
    //DeleteTuples(conditions, table_name);
    //InsertTuple(table_name, list);
    DropTable(table_name);
}