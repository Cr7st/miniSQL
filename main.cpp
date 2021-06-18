#include <iostream>
#include <string>
#include <vector>
#include "CatalogManager.h"
#include "RecordManager.h"

int main(){
    int *i = new int;
    *i = 1;
    int &r = *i;
    r = 2;
    std::cout << *i << std::endl;
    delete &r;
}