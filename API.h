#ifndef _API_H_
#define _API_H_

#include <vector>
#include "CatalogManager.h"
#include "Interpreter.h"
#include "RecordManager.h"
#include "bufferManager.h"
#include "BPTree.h"
#include "GlobalClass.h"
#include "ERROR/Error.h"

void CreateTable(std::string table_name, std::vector<std::string> &column_names, 
                             std::vector<std::string> &data_types, int PK_index);
bool OpenTable(std::string file_name);
bool InsertTuple(std::string table_name, std::vector<DataClass>& list);
bool DropTable(std::string table_name);
std::vector<std::string> ShowAllTable(bool b);
std::vector<std::string> SelectTable(std::string table_name, std::vector<std::string>& query, std::vector<SelectCondition> condition);
bool UpdateTable(TB_Update_Info tb_update_info);


class TableIndexHeadInfo
{
public:
    TableIndexHeadInfo(BPTree& _tree) : tree(_tree) {}
    //number of columns
    size_t GetColumnCount() const;
    //name of each column
    std::vector<std::string> GetColumnNames() const;
    //type of each column
    std::vector<type
private:
    BPTree& tree;
};
#endif