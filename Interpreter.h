#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <string>
#include <vector>
#include <stdio.h>
#include <cstdlib>
#include <algorithm>
#include "RecordManager.h"
#include "CatalogManager.h"
#include "GlobalClass.h"
#include "API.h"

std::string DeleteSpace(std::string a);

void trim(std::string &s);

DataClass convert(std::string value);

void Interpreter(std::string command);

void Insert(std::string command);

void Select(std::string command);

void Delete(std::string command);

void Create(std::string command);

void Drop(std::string command);

class PrintResult
{
public:
    void CreateTable(bool is_created);
    void DropTable(bool is_dropped);
    void ShowTable(std::string command);
    void SelectTuple(std::string table_name, std::vector<Tuple> tuple);
    void InsertTuple(bool is_inserted);
    void DeleteTuple(int DeleteNum);
    void CreateIndex(bool is_created);
    void DropIndex(bool is_dropped);
};


#endif