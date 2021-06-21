#ifndef _API_H_
#define _API_H_

#include "CatalogManager.h"
#include "Interpreter.h"
#include "RecordManager.h"
#include "bufferManager.h"
#include "bpt.h"
#include "ERROR/Error.h"

void CreateTable(std::string table_name, std::vector<std::string> &column_names, 
                             std::vector<std::string> &data_types, int PK_index);

bool OpenTable(std::string file_name);
#endif