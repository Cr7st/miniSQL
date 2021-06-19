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
struct S f(){
    struct S s1,s2,s3;
    s1.i = 1;
    s2.i = 2;
    s3.i = 3;
    obj.push_back(s1);
    obj.push_back(s2);
    obj.push_back(s3);
    return s1;
}

int main(){
    std::cout << f().i << std::endl;
    std::cout << obj.size() << std::endl;
    std::cout << obj.at(1).i << std::endl;
    std::cout << obj.at(2).i << std::endl;
}