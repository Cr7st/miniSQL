#include <string>
#include <vector>
#include "RecordManager.h"
#include "CatalogManager.h"

class SelectCondition{
private:
    std::string on_attr;
    std::string op;
    DataClass cmp_value;
    friend class RM;
};