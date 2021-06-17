#include <vector>
#include <iostream>
#include "RecordManager.h"
#ifndef _CATALOGMANAGER_H_
#define _CATALOGMANAGER_H_

enum class DataType{INT, FLOAT, CHAR};

union Data{
    int i;
    double f;
    char *str;
};

class DataBaseClass{
protected:
    DataType type;
    union Data data;

    friend std::ostream& operator<<(std::ostream &out, const DataBaseClass &obj);
    friend class Tuple;
};

class IntData: public DataBaseClass{
public:
    //IntData();
    IntData(int i = 0);
};

class FloatData: public DataBaseClass{
public:
    //FloatData();
    FloatData(double f = 0);
};

class CharData: public DataBaseClass{
private:
    int length;
public:
    int get_length() const;
    CharData();
    CharData(const char str[], int n = 1);
    ~CharData();
};

class ColumnInfo{
public:
    std::string column_names;
    int bytes;
    DataType type;
    bool is_PK;
    bool has_index;

    friend class TableInfo;
    friend class CatalogManager;
    friend class Tuple;
};

class TableInfo{
public:
    std::vector<int> PK_index;
    std::vector<int> index_on;
    std::vector<ColumnInfo> columns;

public:
    /**
     * @brief Get the number of columns/attributes of the table
     */
    int n_columns() const;

    /**
     * @brief Caculate the size of one tuple of this kind of table
     * @return the number of bytes
     */
    int CalTupleSize() const;

    /**
     * @brief To get the exact memory content of the data
     * @param destination: The destination address of the memory
     * @return Return the number of bytes required by all the data
     */
    int GetWriteSource(void *destination);

    friend class CatalogManager;
    friend class Tuple;
};

class CM{ //short for CatalogManager
public:
    bool CreateTable(TableInfo table);
    bool SetIndexOn(std::string column_name);
};

#endif