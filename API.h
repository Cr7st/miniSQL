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

std::vector<Tuple> SelectTuples(std::vector<SelectCondition> &conditions, std::string table_name);
#endif