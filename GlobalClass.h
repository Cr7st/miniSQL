#ifndef _GLOBALCLASS_H_
#define _GLOBALCLASS_H_

#include <string>

enum class DataType{INT, FLOAT, CHAR};

union Data{
    int i;
    double f;
    char *str;
};

class DataClass{
private:
    DataType type;
    int bytes;
    union Data data;
public:
    DataClass();
    DataClass(int i);
    DataClass(double f);
    DataClass(std::string str);
    bool operator==(const DataClass &rhs);
    bool operator<(const DataClass &rhs);
    bool operator<=(const DataClass &rhs);
    bool operator>=(const DataClass &rhs);
    bool operator>(const DataClass &rhs);
    friend std::ostream& operator<<(std::ostream &out, const DataClass &obj);
    friend class Tuple;
    friend class RM;
};


typedef struct{
    std::string attr;
    std::string op;   //1:<, 2:=, 3:>, 4:!=, 5:<=, 6:==, 7:>=
    DataClass value;
    friend class RM;
}SelectCondition;

#endif