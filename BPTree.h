//
// Created by 86152 on 2021/6/20.
//

#ifndef BPTREE_H
#define BPTREE_H
#include "bufferManager.h"
#include "RecordManager.h"
#include "GlobalClass.h"
#include <queue>


// 索引文件头信息结点,保存在文件头预留空间
class IndexHeadNode
{
public:
    FileAddr    root;                                       // the address of the root
    FileAddr    MostLeftNode;                               // the address of the most left node
    int         KeyTypeIndex;                               // 关键字字段的位置
    char        RecordTypeInfo[RecordColumnCount];          // 记录字段类型信息，
    char        RecordColumnName[RecordColumnCount/4* ColumnNameLength];
};

// define B+tree Node
enum class NodeType {ROOT, INNER, LEAF};
class BTNode
{
public:
    NodeType node_type;                              // node type
    int count_valid_key;                             // the number of key has stored in the node

    DataClass key[MaxKeyCount];                        // array of keys
    FileAddr children[MaxChildCount];                // if the node is not a leaf node, children store the children pointer
    // otherwise it store record address;

    FileAddr next;                                   // if leaf node
    void PrintSelf();
};

class BPTree
{
    //friend std::vector<RecordHead> ShowTable(std::string table_name, std::string path);
    //friend RecordHead GetDbfRecord(std::string table_name, FileAddr fd, std::string path);
public:
    // 参数：索引文件名称， 关键字类型， 记录各个类型信息数组， 记录各个字段名称信息数组
    BPTree(std::string idx_name);
    BPTree(const std::string idx_name, const std::string tb_name, int KeyTypeIndex, char (&_RecordTypeInfo)[RecordColumnCount],
           char (&_RecordColumnName)[RecordColumnCount / 4 * ColumnNameLength], TableInfo& info);          // 创建索引文件的B+树
    ~BPTree() { }
    FileAddr Search(DataClass search_key);                                        // 查找关键字是否已经存在
    bool Insert(DataClass k, FileAddr k_fd);                                      // 插入关键字k
    FileAddr UpdateKey(DataClass k, DataClass k_new);                               // 返回关键字对应的记录地址
    FileAddr Delete(DataClass k);                                                 // 返回该关键字记录在数据文件中的地址
    void PrintBPTreeStruct();                                                    // 层序打印所有结点信息
    void PrintAllLeafNode();
    IndexHeadNode *GetPtrIndexHeadNode();
    BTNode *FileAddrToMemPtr(FileAddr node_fd);                                 // 文件地址转换为内存指针

    std::vector<FileAddr*> RightSearch(DataClass low_key);
    std::vector<FileAddr*> LeftSearch(DataClass high_key);
    std::vector<FileAddr*> AllSearch();
    std::vector<FileAddr*> ExcludeSearch(DataClass key);
    std::vector<FileAddr*> RangeSearch(DataClass low_key, DataClass high_key);

private:
    FileAddr DeleteKeyAtInnerNode(FileAddr x, int i, DataClass key);              // x的下标为i的结点为叶子结点
    FileAddr DeleteKeyAtLeafNode(FileAddr x, int i, DataClass key);               // x的下标为i的结点为叶子结点
    void InsertNotFull(FileAddr x, DataClass k, FileAddr k_fd);
    void SplitChild(FileAddr x, int i, FileAddr y);                             // 分裂x的孩子结点x.children[i] , y
    FileAddr Search(DataClass search_key, FileAddr node_fd);                      // 判断关键字是否存在
    FileAddr SearchInnerNode(DataClass search_key, FileAddr node_fd);             // 在内部节点查找
    FileAddr SearchLeafNode(DataClass search_key, FileAddr node_fd);              // 在叶子结点查找


private:
    int file_id;
    std::string str_idx_name, table_name;
    IndexHeadNode idx_head;
};

#endif //MINISQL_BPTREE_H
