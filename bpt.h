//
// Last modified by zzh on 2021/6/12.
//

#ifndef __GP__BPT__
#define __GP__BPT__

#include <iostream>
#include <string>
#include <vector>

const int M = 31 ;

struct bpt_node {
    bool is_leaf;
    bool is_root;
    int key_num;
    int key[M];
    int offset;   //buffer中的寻址
    void *pointer[M + 1];    //非叶节点存M+1个儿子，叶节点存M个value
    void *father;
};

class bpt{
private:
    bpt_node *root;
    std::string name;
    int node_count, splite_count;
public:
    bpt(std::string file_name);
    void query(bpt_node *now, int, int);
    bool insert_in_bpt(int key, void *value);
    void insert_in_node( bpt_node *node , int key , void *value ) ;
    bool delete_in_bpt(int key);
    void delete_in_node( bpt_node *node , int key ) ;
    void split( bpt_node *node );
    char *query_in_bpt(int key);
    bpt_node *new_bpt_node();
    bpt_node *find_leaf( int key );
};

#endif