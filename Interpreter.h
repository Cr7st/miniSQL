#include <string>
#include <vector>
#include <stdio.h>
#include "RecordManager.h"
#include "CatalogManager.h"

typedef struct{
    std::string attr;
    std::string op;   //1:<, 2:=, 3:>, 4:!=, 5:<=, 6:==, 7:>=
    DataClass value;
    friend class RM;
}SelectCondition;

