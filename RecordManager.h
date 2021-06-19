#ifndef _RECORDMANAGER_H_
#define _RECORDMANAGER_H_

#include <vector>
#include "CatalogManager.h"
#include "Interpreter.h"
#include "bufferManager.h"

class Tuple{
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
    void ReadFrom(void *source);
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

    bool Satisfy(SelectCondition &condition, Tuple &tuple);
public:
    void SelectTuple(std::vector<SelectCondition> &conditions, void* source, TableInfo info);
    void GetSelectRS(std::vector<Tuple> result);
};

#endif