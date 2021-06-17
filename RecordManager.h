#ifndef _RECORDMANAGER_H_
#define _RECORDMANAGER_H_

#include <vector>
#include "CatalogManager.h"
#include "Buffer.h"
#include "Interpreter.h"

class Tuple{
protected:
    std::vector<DataBaseClass*> data_list;
    int tuple_size;
public:
    /**
     * @brief To get the exact memory content of the data
     * @param destination: The destination address of the memory
     */
    void WriteTo(void *destination);

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
    Tuple(TableInfo info);

    Tuple();
    /**
     * @brief Use WriteTo() to get the source content and write into the block
     *        Then set the block to be dirty.
     * @param block: The destination block
     */
    //void WriteToBlock(Block block);
};

class RM{ //short for RecordManager
    void SelectTuples(std::vector<Tuple> &tuples, std::vector<SelectCondition> conditions);
};

#endif