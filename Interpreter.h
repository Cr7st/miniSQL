#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <string>
#include <vector>
#include <stdio.h>
#include <cstdlib>
#include "RecordManager.h"
#include "CatalogManager.h"
#include "GlobalClass.h"
#include "API.h"

void Interpreter(std::string command);

void Insert(std::string command);

void Select(std::string command);

std::string DeleteSpace(std::string a);

void trim(std::string &s);

#endif