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
//std::vector<std::string> ShowAllTable(bool b);
std::vector<Tuple> SelectTuples(std::vector<SelectCondition>& conditions, std::string table_name);
bool DeleteTuples(std::vector<SelectCondition>& conditions, std::string table_name);
bool CreateIndex(std::string table_name, std::string index_name, std::string column_name)；
bool DropIndex(std::string index_name);
bool ShowTable(std::string table_name);
#endif