#ifndef _CATALOGMANAGER_H_
#define _CATALOGMANAGER_H_

#include <vector>
#include <iostream>
#include "GlobalClass.h"

// class IntData: public DataClass{
// public:
//     //IntData();
//     IntData(int i = 0);
// };

// class FloatData: public DataClass{
// public:
//     //FloatData();
//     FloatData(double f = 0);
// };

// class CharData: public DataClass{
// private:
//     int length;
// public:
//     int get_length() const;
//     CharData();
//     CharData(const char str[], int n = 1);
//     ~CharData();
// };

struct ColumnInfo{
public:
    std::string column_name;
    int bytes;
    DataType type;
    bool is_PK;
    bool is_unique;
    bool has_index;
};

struct TableInfoMem{
    char name[28];
    char n_col[4];
    char columns_info[32 * 15];
    char indicies_info[32 * 16];
};

/**
 * @brief: The class to store the information of a table,
 *         which includes the primarykey, the columns info and on which column there is an index
 */
class TableInfo{
private:
    int PK_index;
    std::vector<int> index_on;
    std::vector<std::string> index_names;
    std::vector<ColumnInfo> columns;
    std::string table_name;

    /**
     * @brief To write the essential information to the destination address
     *        It should be written to the file head
     * @param destination: The destination address of the memory
     */
    void WriteTo(void *destination);

    struct TableInfoMem* GetPatchedData();

    void ReadFrom(void *source);
public:
    friend bool CreateIndex(std::string table_name, std::string index_name,std::string column_name);
    /**
     * @brief Get the number of columns/attributes of the table
     */
    int n_columns() const;

    /**
     * @brief Caculate the size of one tuple of this kind of table
     * @return the number of bytes
     */
    int CalTupleSize() const;

    bool SetIdxOn(int index, std::string index_name);

    ColumnInfo operator[](int idx) const; 

    friend class FileHeadInfo;
    friend class CatalogManager;
    friend class Tuple;
    friend class CM;
    friend class RM;
};

class CM{ //short for CatalogManager
private:
    std::vector<TableInfo> ex_tables; // the info of tables now exit in the memory
public: 
    /**
     * @brief Initialize an TableInfo object with the information given from interpreter
     * @param column_names: the names of all the columns
     * @param data_types: the declaration of the types of the columns
     *                    should be in the format of:
     *                    "uniqueint"
     *                    "uniqueflota"
     *                    "uniquechar..." here "..." is the length, in number
     *                    prefix unique is only needed when the column is declared to be unique
     * @return return the reference of the initialized TableInfo
     */
    TableInfo InitTableInfo(std::string table_name, std::vector<std::string> &column_names, 
                             std::vector<std::string> &data_types, int PK_index);

    /**
     * @brief What to check:
     *          - If there are columns have the same name
     * @return If the TableInfo is legal, return true, else throw an error
     */
    bool NewInfoCheck(TableInfo &table);

    TableInfo& LookUpTableInfo(std::string name);

    void CreateTable(TableInfo &table, void *destination);
    
    /**
     * 注意：对SetIdxOn之后对表信息进行了修改，需要将表头信息重新写
     */
    bool SetIdxOn(TableInfo &table, int index, std::string index_name);

    void WriteTo(TableInfo &info, void *destination);

    bool DropTable(std::string name);
    bool DropIndex(TableInfo &table, std::string index_name);

    void OpenTableFile(void *source);

    void CloseTable(std::string name);
    void CloseTable(TableInfo &table);
};

#endif