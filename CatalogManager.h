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
private:
    std::string column_names;
    int bytes;
    DataType type;
    bool is_PK;
    bool has_index;

    friend class TableInfo;
    friend class CatalogManager;
    friend class Tuple;
    friend class CM;
};

/**
 * @brief: The class to store the information of a table,
 *         which includes the primarykey, the columns info and on which column there is an index
 */
class TableInfo{
private:
    std::vector<int> PK_indices;
    std::vector<int> index_on;
    std::vector<ColumnInfo> columns;

    /**
     * @brief To write the essential information to the destination address
     *        It should be written to the file head
     * @param destination: The destination address of the memory
     */
    void WriteTo(void *destination);
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
    friend class CatalogManager;
    friend class Tuple;
    friend class CM;
};

class CM{ //short for CatalogManager
public:
    TableInfo& InitTableInfo(std::vector<std::string> &column_names, std::vector<std::string> &data_types, int PK_index);
    bool NewInfoCheck(TableInfo &table);
    bool CreateTable(TableInfo &table);
    bool SetIndexOn(std::string column_name);
};

#endif