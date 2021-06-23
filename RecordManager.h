#ifndef __RECORDMANAGER_H__
#define __RECORDMANAGER_H__
#include "ERROR/Error.h"
#include "CatalogManager.h"
#include "GlobalClass.h"
#include <vector>
#include <string>
#include <cstring>

class Tuple{
friend void PrintResult::SelectTuple(std::string table_name, std::vector<Tuple> tuple);
protected:
    std::vector<DataClass> data_list;
    int tuple_size;
private:
    /**
     * @brief Patch all the data as the source
     * @return return the address of the patched data source
     */
    void* GetWriteSource();

    /**
     * @brief To get the memory content of the source address
     * @param source: the address of the source memory
     */
    void ReadFrom(const char *source);
public:
    ~Tuple();

    /**
     * @brief Initialize the tuple with the information of the table
     *         What to initialize: number of attributes
     *                             type of attributes
     *                             the number of bytes needed to store this tuple
     * @param info: The infomation of the table
     */
    Tuple(TableInfo &info);

    Tuple(TableInfo &Info, std::vector<DataClass> &list);

    Tuple();
    /**
     * @brief Use WriteTo() to get the source content and write into the block
     *        Then set the block to be dirty.
     * @param block: The destination block
     */
    //void WriteToBlock(Block block);
    friend class RM;
};

class RM{ //short for RecordManager
private:
    std::vector<Tuple> select_rs;

    bool Satisfy(SelectCondition &condition, Tuple &tuple, const TableInfo &info);
public:
    void SelectTuple(std::vector<SelectCondition> &conditions, const void* source, TableInfo &info);
    void GetSelectRS(std::vector<Tuple> &result);
    void InsertTuple(void *destination, TableInfo &table, std::vector<DataClass> &list);
    void* GetSource(Tuple *t);
    bool InsertCheck(const void *source, TableInfo &table, std::vector<DataClass> &list);
    bool DeleteCheck(std::vector<SelectCondition> &conditions, const void *source, TableInfo &info);

};

#endif