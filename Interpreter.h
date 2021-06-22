#include <string>
#include <vector>
#include <stdio.h>
#include "RecordManager.h"
#include "CatalogManager.h"
#include "API.h"

typedef struct{
    std::string attr;
    std::string op;   //1:<, 2:=, 3:>, 4:!=, 5:<=, 6:==, 7:>=
    DataClass value;
    friend class RM;
}SelectCondition;

void Interpreter(std::string command);

void Insert(std::string command);

void Select(std::string command);

std::string DeleteSpace(std::string a);

void trim(std::string &s);