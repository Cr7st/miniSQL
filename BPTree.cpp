//
// Created by 86152 on 2021/6/20.
//

#include "BPTree.h"

BTree::BTree(const std::string idx_name, int KeyTypeIndex, char(&_RecordTypeInfo)[RecordColumnCount],
             char(&_RecordColumnName)[RecordColumnCount / 4 * ColumnNameLength])
        :str_idx_name(idx_name)
{
    auto &buffer = GetGlobalFileBuffer();
    auto pMemFile = buffer[str_idx_name.c_str()];

    // 如果索引文件不存在则创建
    if (!pMemFile)
    {
        // 创建索引文件
        buffer.CreateFile(str_idx_name.c_str());
        pMemFile = buffer[str_idx_name.c_str()];

        // 初始化索引文件，创建一个根结点
        BTNode root_node;
        assert(sizeof(BTNode) < (FILE_PAGESIZE - sizeof(PAGEHEAD)));
        root_node.node_type = NodeType::ROOT;
        root_node.count_valid_key = 0;
        root_node.next = FileAddr{ 0,0 };
        FileAddr root_node_fd= buffer[str_idx_name.c_str()]->AddRecord(&root_node, sizeof(root_node));

        // 初始化其他索引文件头信息
        idx_head.root = root_node_fd;
        idx_head.MostLeftNode = root_node_fd;
        idx_head.KeyTypeIndex = KeyTypeIndex;
        //strcpy(idx_head.RecordTypeInfo, _RecordTypeInfo.c_str());
        memcpy(idx_head.RecordTypeInfo, _RecordTypeInfo, RecordColumnCount);
        //strcpy(idx_head.RecordColumnName, _RecordColumnName.c_str());
        memcpy(idx_head.RecordColumnName, _RecordColumnName, RecordColumnCount / 4 * ColumnNameLength);


        // 将结点的地址写入文件头的预留空间区
        memcpy(buffer[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve, &idx_head, sizeof(idx_head));

    }
    file_id = pMemFile->fileId;
}

BTree::BTree(std::string idx_name)
{
    str_idx_name = idx_name;
    file_id = GetGlobalFileBuffer()[idx_name.c_str()]->fileId;
}

FileAddr BTree::DeleteKeyAtInnerNode(FileAddr x, int i, KeyAttr key)
{
    auto px = FileAddrToMemPtr(x);
    auto py = FileAddrToMemPtr(px->children[i]);
    FileAddr fd_res;
    if (py->node_type == NodeType::LEAF)
    {
        fd_res = DeleteKeyAtLeafNode(x, i, key);
    }
    else
    {
        int j = py->count_valid_key-1;
        while (py->key[j] > key)j--;
        assert(j>=0);
        fd_res = DeleteKeyAtInnerNode(px->children[i], j, key);
    }

    // 判断删除后的结点个数
    if (py->count_valid_key >= MaxKeyCount / 2)
        return fd_res;

    // 如果删除后的关键字个数不满足B+树的规定，向兄弟结点借用key

    // 如果右兄弟存在且有富余关键字
    if ((i <= px->count_valid_key - 2) && (FileAddrToMemPtr(px->children[i + 1])->count_valid_key > MaxKeyCount / 2))
    {
        auto RBrother = FileAddrToMemPtr(px->children[i + 1]);
        // 借来的关键字
        auto key_bro = RBrother->key[0];
        auto fd_bro = RBrother->children[0];


        // 更新右兄弟的索引结点
        px->key[i + 1] = RBrother->key[1];
        // 跟新右兄弟结点
        for (int j = 1; j <= RBrother->count_valid_key - 1; j++)
        {
            RBrother->key[j - 1] = RBrother->key[j];
            RBrother->children[j - 1] = RBrother->children[j];
        }
        RBrother->count_valid_key -= 1;

        // 更新本叶子结点
        py->key[py->count_valid_key] = key_bro;
        py->children[py->count_valid_key] = fd_bro;
        py->count_valid_key += 1;

        return fd_res;
    }

    // 如果左兄弟存在且有富余关键字
    if (i > 0 && FileAddrToMemPtr(px->children[i - 1])->count_valid_key > MaxKeyCount / 2)
    {
        auto LBrother = FileAddrToMemPtr(px->children[i - 1]);
        // 借来的关键字
        auto key_bro = LBrother->key[LBrother->count_valid_key - 1];
        auto fd_bro = LBrother->children[LBrother->count_valid_key - 1];

        // 更新左兄弟结点
        LBrother->count_valid_key -=1;

        // 更新本结点
        px->key[i] = key_bro;
        for (int j = py->count_valid_key - 1; j >= 0; j--)
        {
            py->key[j + 1] = py->key[j];
            py->children[j + 1] = py->children[j];
        }
        py->key[0] = key_bro;
        py->children[0] = fd_bro;

        py->count_valid_key += 1;

        return fd_res;
    }

    // 若兄弟结点中没有富余的key,则当前结点和兄弟结点合并成一个新的叶子结点，并删除父结点中的key

    // 若右兄弟存在将其合并
    if (i < px->count_valid_key - 1)
    {
        auto RBrother = FileAddrToMemPtr(px->children[i + 1]);
        for (int j = 0; j < RBrother->count_valid_key; j++)
        {
            py->key[py->count_valid_key] = RBrother->key[j];
            py->children[py->count_valid_key] = RBrother->children[j];
            py->count_valid_key++;
        }

        // 更新next
        py->next = RBrother->next;
        // 删除右结点
        GetGlobalFileBuffer()[str_idx_name.c_str()]->DeleteRecord(&px->children[i + 1], sizeof(BTNode));
        // 更新父节点索引
        for (int j = i + 2; j < px->count_valid_key; j++)
        {
            px->key[j - 1] = px->key[j];
            px->children[j - 1] = px->children[j];
        }
        px->count_valid_key--;
    }
    else
    {// 将左结点合并
        auto LBrother = FileAddrToMemPtr(px->children[i - 1]);
        for (int j = 0; j < py->count_valid_key; j++)
        {
            LBrother->key[LBrother->count_valid_key] = py->key[j];
            LBrother->children[LBrother->count_valid_key] = py->children[j];
            LBrother->count_valid_key++;
        }

        // 更新next
        LBrother->next = py->next;

        // 删除本结点
        GetGlobalFileBuffer()[str_idx_name.c_str()]->DeleteRecord(&px->children[i], sizeof(BTNode));
        // 更新父节点索引
        for (int j = i + 1; j < px->count_valid_key; j++)
        {
            px->key[j - 1] = px->key[j];
            px->children[j - 1] = px->children[j];
        }
        px->count_valid_key--;
    }
    return fd_res;
}

// 假设待删除的关键字已经存在
FileAddr BTree::DeleteKeyAtLeafNode(FileAddr x, int i, KeyAttr key)
{
    auto px = FileAddrToMemPtr(x);
    auto py = FileAddrToMemPtr(px->children[i]);
    FileAddr fd_res;
    int j = py->count_valid_key - 1;
    while (py->key[j] != key)j--;
    assert(j >= 0);
    fd_res = py->children[j];
    // 删除叶节点中最小的关键字，更新父节点
    if (j == 0)
    {
        px->key[i] = py->key[j+1];
    }

    j++;
    while (j <= py->count_valid_key - 1)
    {
        py->children[j - 1] = py->children[j];
        py->key[j - 1] = py->key[j];
        j++;
    }
    py->count_valid_key -= 1;
    return fd_res;
}

// 在一个非满结点 x, 插入关键字 k, k的数据地址为 k_fd
void BTree::InsertNotFull(FileAddr x, KeyAttr k, FileAddr k_fd)
{
    auto px = FileAddrToMemPtr(x);
    int i = px->count_valid_key-1;

    // 如果该结点是叶子结点，直接插入
    if (px->node_type == NodeType::LEAF|| px->node_type == NodeType::ROOT)
    {
        while (i >= 0 && k < px->key[i])
        {
            px->key[i + 1] = px->key[i];
            px->children[i + 1] = px->children[i];
            i--;
        }
        px->key[i + 1] = k;
        px->children[i + 1] = k_fd;
        px->count_valid_key += 1;
    }
    else
    {
        while (i >= 0 && k < px->key[i])  i = i - 1;

        // 如果插入的值比内节点的值还小
        if (i < 0){
            i = 0;
            px->key[i] = k;
        }
        assert(i >= 0);

        FileAddr ci = px->children[i];
        auto pci = FileAddrToMemPtr(ci);
        if (pci->count_valid_key == MaxKeyCount)
        {
            SplitChild(x, i, ci);
            if (k >= px->key[i + 1])
                i += 1;
        }
        InsertNotFull(px->children[i], k, k_fd);
    }
}

// 将x下标为i的孩子满结点分裂
void BTree::SplitChild(FileAddr x, int i, FileAddr y)
{
    auto pMemPageX = GetGlobalClock()->GetMemAddr(file_id, x.filePageID);
    auto pMemPageY = GetGlobalClock()->GetMemAddr(file_id, y.filePageID);
    pMemPageX->isModified = true;
    pMemPageY->isModified = true;

    BTNode*px = FileAddrToMemPtr(x);
    BTNode*py = FileAddrToMemPtr(y);
    BTNode z;         // 分裂出来的新结点
    FileAddr z_fd;    // 新结点的文件内地址

    z.node_type = py->node_type;
    z.count_valid_key = MaxKeyCount / 2;

    // 将y结点的一般数据转移到新结点
    for (int k = MaxKeyCount / 2; k < MaxKeyCount; k++)
    {
        z.key[k - MaxKeyCount / 2] = py->key[k];
        z.children[k - MaxKeyCount / 2] = py->children[k];
    }
    py->count_valid_key = MaxKeyCount / 2;

    // 在y的父节点x上添加新创建的子结点 z
    int j;
    for ( j= px->count_valid_key-1; j> i; j--)
    {
        px->key[j+1] = px->key[j];
        px->children[j+1] = px->children[j];
    }

    j++; // after j++, j should be i+1;
    px->key[j] = z.key[0];

    if (py->node_type == NodeType::LEAF)
    {
        z.next = py->next;
        z_fd = GetGlobalFileBuffer()[str_idx_name.c_str()]->AddRecord(&z, sizeof(z));
        py->next = z_fd;
    }
    else
        z_fd = GetGlobalFileBuffer()[str_idx_name.c_str()]->AddRecord(&z, sizeof(z));

    px->children[j] = z_fd;
    px->count_valid_key++;
}

vector<FileAddr> BTree::LeftSearch(KeyAttr high_key) {
    auto pMemPage = GetGlobalClock()->GetMemAddr(file_id, 0);
    auto pfilefd = (FileAddr*)pMemPage->GetFileCond()->reserve;  // 找到根结点的地址
    BTNode* Node = FileAddrToMemPtr(*pfilefd);
    FileAddr Addr;
    while(Node->node_type == NodeType::LEAF) {
        Addr = Node->children[0];
        Node = FileAddrToMemPtr(Addr);
    }
    vector<FileAddr> AddrRes;
    while(Addr != FileAddr{0, 0}){
        Node = FileAddrToMemPtr(Addr);
        for(int i = 0; i <Node->count_valid_key; i++) {
            AddrRes.push_back(Node->children[i]);
            if (Node->key[i] == high_key) return AddrRes;
        }
        Addr = Node->next;
    }
    return AddrRes;
}

vector<FileAddr> BTree::RightSearch(KeyAttr low_key) {
    auto pMemPage = GetGlobalClock()->GetMemAddr(file_id, 0);
    auto pfilefd = (FileAddr*)pMemPage->GetFileCond()->reserve;  // 找到根结点的地址
    FileAddr Addr = Search(low_key, *pfilefd);
    BTNode* Node;
    vector<FileAddr> AddrRes;
    while(Addr != FileAddr{0, 0}){
        Node = FileAddrToMemPtr(Addr);
        for(int i = 0; i <Node->count_valid_key; i++) {
            AddrRes.push_back(Node->children[i]);
        }
        Addr = Node->next;
    }
    return AddrRes;
}

vector<FileAddr> BTree::RangeSearch(KeyAttr low_key, KeyAttr high_key) {
    auto pMemPage = GetGlobalClock()->GetMemAddr(file_id, 0);
    auto pfilefd = (FileAddr*)pMemPage->GetFileCond()->reserve;  // 找到根结点的地址
    FileAddr Addr = Search(low_key, *pfilefd);
    BTNode* Node;
    vector<FileAddr> AddrRes;
    while(Addr != FileAddr{0, 0}){
        Node = FileAddrToMemPtr(Addr);
        for(int i = 0; i <Node->count_valid_key; i++) {
            AddrRes.push_back(Node->children[i]);
            if (Node->key[i] == high_key) return AddrRes;
        }
        Addr = Node->next;
    }
    return AddrRes;
}

FileAddr BTree::Search(KeyAttr search_key)
{
    auto pMemPage = GetGlobalClock()->GetMemAddr(file_id, 0);
    auto pfilefd = (FileAddr*)pMemPage->GetFileCond()->reserve;  // 找到根结点的地址
    return Search(search_key, *pfilefd);
}

FileAddr BTree::Search(KeyAttr search_key, FileAddr node_fd)
{
    BTNode* pNode = FileAddrToMemPtr(node_fd);

    if (pNode->node_type == NodeType::LEAF|| pNode->node_type == NodeType::ROOT)
    {
        return SearchLeafNode(search_key, node_fd);
    }
    else
    {
        return SearchInnerNode(search_key, node_fd);
    }
}

bool BTree::Insert(KeyAttr k, FileAddr k_fd)
{
    // 如果该关键字已经存在则插入失败
    try
    {
        auto key_fd = Search(k);
        if (key_fd != FileAddr{ 0,0 })
            throw SQLError::KEY_INSERT_ERROR();
    }
    catch (const SQLError::BaseError &error)
    {
        SQLError::DispatchError(error);
        std::cout << std::endl;
        return false;
    }

    // 得到根结点的fd
    FileAddr root_fd = *(FileAddr*)GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve;
    auto proot = FileAddrToMemPtr(root_fd);
    if (proot->count_valid_key == MaxKeyCount)
    {
        // 创建新的结点 s ,作为根结点
        BTNode s;
        s.node_type = NodeType::INNER;  // 只有初始化才使用 NodeType::ROOT
        s.count_valid_key = 1;
        s.key[0] = proot->key[0];
        s.children[0] = root_fd;
        FileAddr s_fd = GetGlobalFileBuffer()[str_idx_name.c_str()]->AddRecord(&s, sizeof(BTNode));

        // 将新的根节点文件地址写入
        *(FileAddr*)GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve = s_fd;
        GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->isModified = true;

        //将旧的根结点设置为叶子结点
        auto pOldRoot = FileAddrToMemPtr(root_fd);
        if(pOldRoot->node_type == NodeType::ROOT)
            pOldRoot->node_type = NodeType::LEAF;

        // 先分裂再插入
        SplitChild(s_fd, 0, s.children[0]);
        InsertNotFull(s_fd, k, k_fd);
    }
    else
    {
        InsertNotFull(root_fd, k, k_fd);
    }
    return true;
}

// 更新关键字
FileAddr BTree::UpdateKey(KeyAttr k, KeyAttr k_new)
{
    // 在索引中删除旧的关键字
    auto data_fd = Delete(k);  // 保存关键字对应的记录地址

    //更新新的关键字
    Insert(k_new, data_fd);
    return data_fd;
}

FileAddr BTree::Delete(KeyAttr key)
{
    auto search_res = Search(key);
    if (search_res.offSet == 0)
        return FileAddr{0,0};

    // 得到根结点的fd
    FileAddr root_fd = *(FileAddr*)GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve;
    auto proot = FileAddrToMemPtr(root_fd);


    // 根节点为ROOT 或者 LEAF 直接删除
    if (proot->node_type == NodeType::ROOT|| proot->node_type == NodeType::LEAF)
    {
        // 直接删除
        int j = proot->count_valid_key - 1;
        while (proot->key[j] != key)j--;
        assert(j >= 0);
        FileAddr fd_res = proot->children[j];
        for (j++; j < proot->count_valid_key; j++)
        {
            proot->key[j - 1] = proot->key[j];
            proot->children[j - 1] = proot->children[j];
        }
        proot->count_valid_key--;
        return fd_res;
    }

    int i = proot->count_valid_key - 1;
    while (proot->key[i] > key)i--;
    assert(i >= 0);
    //auto px = FileAddrToMemPtr(root_fd);
    //auto py = FileAddrToMemPtr(px->children[i]);

    auto fd_delete = DeleteKeyAtInnerNode(root_fd, i, key);


    if (proot->count_valid_key == 1)
    {
        // 将新的根节点文件地址写入
        *(FileAddr*)GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve = proot->children[0];
        GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->isModified = true;

        GetGlobalFileBuffer()[str_idx_name.c_str()]->DeleteRecord(&root_fd, sizeof(BTNode));
    }

    return fd_delete;
}

void BTree::PrintBTreeStruct()
{
    std::queue<FileAddr> fds;
    //int n = 0;
    // 得到根结点的fd
    FileAddr root_fd = *(FileAddr*)GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve;
    auto pRoot = FileAddrToMemPtr(root_fd);
    if (pRoot->node_type == NodeType::ROOT||pRoot->node_type == NodeType::LEAF)
    {
        if (pRoot->count_valid_key > 0)
        {
            pRoot->PrintSelf();
        }
        return;
    }

    fds.push(root_fd);
    while (!fds.empty())
    {
        // 打印该结点的所有的关键字
        FileAddr tmp = fds.front();
        fds.pop();
        auto pNode = FileAddrToMemPtr(tmp);
        std::cout << "Node File Address:" << tmp.filePageID << " " << tmp.offSet << std::endl;
        pNode->PrintSelf();
        std::cout << std::endl;

        if (pNode->node_type != NodeType::LEAF)
        {
            for (int i = 0; i < pNode->count_valid_key; i++)
            {
                fds.push(pNode->children[i]);
            }
        }

    }
    //std::cout << "total nodes: " << n << std::endl;
}

void BTree::PrintAllLeafNode()
{
    auto phead = (IndexHeadNode*)GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve;
    auto pRoot = FileAddrToMemPtr(phead->root);
    if (pRoot->count_valid_key <= 0)
    {
        std::cout << "记录为空" << std::endl;
        return;
    }

    auto pNode = FileAddrToMemPtr(phead->MostLeftNode);


    static int n = 0;
    while (pNode->next.offSet != 0)
    {
        for (int i = 0; i < pNode->count_valid_key; i++)
        {
            n++;
            std::cout << pNode->key[i];
        }

        pNode = FileAddrToMemPtr(pNode->next);
    }
    for (int i = 0; i < pNode->count_valid_key; i++)
    {
        n++;
        std::cout << pNode->key[i];
    }
    std::cout << std::endl << n << std::endl;
}

IndexHeadNode * BTree::GetPtrIndexHeadNode()
{
    auto phead = (IndexHeadNode*)GetGlobalFileBuffer()[str_idx_name.c_str()]->GetFileFirstPage()->GetFileCond()->reserve;
    return phead;
}

FileAddr BTree::SearchInnerNode(KeyAttr search_key, FileAddr node_fd)
{
    FileAddr fd_res{0,0};

    BTNode* pNode = FileAddrToMemPtr(node_fd);
    for (int i = pNode->count_valid_key - 1; i >= 0; i--)
    {
        if (pNode->key[i] <= search_key)
        {
            fd_res = pNode->children[i];
            break;
        }
    }

    if (fd_res == FileAddr{ 0,0 })
    {
        return fd_res;
    }
    else
    {
        BTNode* pNextNode = FileAddrToMemPtr(fd_res);
        if (pNextNode->node_type == NodeType::LEAF)
            return SearchLeafNode(search_key, fd_res);
        else
            return SearchInnerNode(search_key, fd_res);
    }
    //return fd_res;
}

FileAddr BTree::SearchLeafNode(KeyAttr search_key, FileAddr node_fd)
{

    BTNode* pNode = FileAddrToMemPtr(node_fd);
    for (int i = 0; i <pNode->count_valid_key; i++)
    {
        if (pNode->key[i] == search_key)
        {
            return pNode->children[i];
        }
    }
    return FileAddr{ 0,0 };
}

BTNode * BTree::FileAddrToMemPtr(FileAddr node_fd)
{
    auto pMemPage = GetGlobalClock()->GetMemAddr(file_id, node_fd.filePageID);
    pMemPage->isModified = true;
    return (BTNode*)((char*)pMemPage->Ptr2PageBegin + node_fd.offSet+sizeof(FileAddr));
}

void BTNode::PrintSelf()
{
    using std::cout;
    using std::endl;
    cout << "Node Type: ";
    switch (node_type)
    {
        case NodeType::ROOT:
            cout << "ROOT";
            break;
        case NodeType::INNER:
            cout << "INNER";
            break;
        case NodeType::LEAF:
            cout << "LEAF";
            break;
        default:
            break;
    }
    cout << "\tcount_valid_key: " << count_valid_key << endl;

    for (int i = 0; i < count_valid_key; i++)
    {
        cout << "index: "<<i<<" key: " << key[i] << "\t" << "child addr: " << children[i].filePageID << " " << children[i].offSet << endl;
    }

}