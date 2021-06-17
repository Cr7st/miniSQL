#include <iostream>
#include <string>
#include <vector>
#include "CatalogManager.h"
#include "RecordManager.h"

int main(){
    ColumnInfo i;
    i.type = DataType::INT;
    i.bytes = sizeof(int);
    ColumnInfo f;
    f.type = DataType::FLOAT;
    f.bytes = sizeof(double);
    ColumnInfo s;
    s.type = DataType::CHAR;
    s.bytes = 5;
    TableInfo tb;
    tb.columns.push_back(i);
    tb.columns.push_back(f);
    tb.columns.push_back(s);
    Tuple *t = new Tuple(tb);
    void* ptr = malloc(4096);
    t->WriteTo(ptr);
    delete t;
    Tuple p(tb);
    p.ReadFrom(ptr);
    return 0;
}