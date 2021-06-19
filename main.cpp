#include <iostream>
#include <string>
#include <vector>
#include "CatalogManager.h"
#include "RecordManager.h"

class S{
public:
    int i;
};

std::vector<struct S> obj;

int main(){
    for (int i = 0; i < 4; i++){
        struct S s;
        s.i = i;
        obj.push_back(s);
    }
    std::cout << obj.size() << std::endl;
    for (int i = 0; i < 4; i++){
        std::cout << obj[i].i << std::endl;
    }
}