#ifndef _GLOBALCLASS_H_
#define _GLOBALCLASS_H_

#include <string>

constexpr int FILE_PAGESIZE = 8192;	                    // 内存页(==文件页)大小
constexpr int MEM_PAGEAMOUNT = 4096;                    // 内存页数量
constexpr int MAX_FILENAME_LEN = 256;                   // 文件名（包含路径）最大长度

constexpr int RecordColumnCount = 12 * 4;
// 记录字段数量限制,假设所有字段都是字符数组，一个字符数组字段需要4个字符->CXXX
constexpr int ColumnNameLength = 16;                   // 单个字段名称长度限制
constexpr int bptree_t = 40;                            // B+tree's degree, bptree_t >= 2
constexpr int MaxKeyCount = 2 * bptree_t;              // the max number of keys in a b+tree node
constexpr int MaxChildCount = 2 * bptree_t;            // the max number of child in a b+tree node

enum class DataType{INT, FLOAT, CHAR};

union Data{
    int i;
    double f;
    char str[30 ];
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