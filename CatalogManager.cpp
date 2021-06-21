#include "CatalogManager.h"
#include "./ERROR/Error.h"
#include <string>
#include <string.h>
#include <vector>
#include <iterator>

int TableInfo::CalTupleSize() const
{
    int size = 0;
    for (auto vi = columns.begin(); vi != columns.end(); vi++){
        size += vi->bytes;
    }
    return size;
}


int TableInfo::n_columns() const
{
    return columns.size();
}


bool CM::NewInfoCheck(TableInfo &table)
{
    // for (int i = 0; i < ex_tables.size(); i++){
    //     if (table.table_name == ex_tables[i].table_name){
    //         const std::string e = "There is already a table named " + table.table_name + "!";
    //         throw SQLError::TABLE_ERROR(e);
    //         return false;
    //     }
    // }
    for (int i = 0; i < table.n_columns(); i++){
        for (int j = 0; j < table.n_columns(); j++){
            if (i == j) continue;
            else{
                if (table.columns[i].column_name == table.columns[j].column_name){
                    const std::string e = "There is an name conflict between attributes!";
                    throw SQLError::TABLE_ERROR(e);
                    return false;
                }
            }
        }
    }
    return true;
}


TableInfo CM::InitTableInfo(std::string table_name, std::vector<std::string> &column_names, 
                             std::vector<std::string> &data_types, int PK_index)
{
    if (column_names.size() > 15){
        std::string e("To many attributes!");
        throw SQLError::TABLE_ERROR(e);
    }
    TableInfo table;
    if (table_name.length() >= 27){
        std::string e("Table name is too long!");
        throw SQLError::TABLE_ERROR(e);
    }
    table.table_name = table_name;
    for (int i = 0; i < column_names.size(); i++){
        ColumnInfo column;
        column.has_index = false;
        if (column_names[i].length() >= 19){
            throw SQLError::KeyAttr_NameLength_ERROR();
        }
        column.column_name = column_names[i];
        if (data_types[i].find("unique") != data_types[i].npos){
            column.is_unique = true;
            // discard the prefix "unique"
            data_types[i] = data_types[i].substr(6, data_types[i].length() - 6);
        }
        else{
            column.is_unique = false;
        }
        if (data_types[i] == "int"){
            column.type = DataType::INT;
            column.bytes = sizeof(int);
        }
        else if (data_types[i] == "float"){
            column.type = DataType::FLOAT;
            column.bytes = sizeof(double);
        }
        else if (data_types[i].substr(0, 4) == "char"){
            column.type = DataType::CHAR;
            // get the length of char
            std::string str_size = data_types[i].substr(4, data_types[i].length() - 4);
            column.bytes = atoi(str_size.c_str()) * sizeof(char);
        }
        table.columns.push_back(column);
    }
    table.columns[PK_index].is_PK = true;
    table.columns[PK_index].has_index = true;
    table.index_on.push_back(PK_index);
    table.PK_index = PK_index;
    return table;
}


TableInfo& CM::LookUpTableInfo(std::string name)
{
    for (int i = 0; i < ex_tables.size(); i++){
        if (ex_tables[i].table_name == name)
            return ex_tables[i];
    }
    std::string e("can't find this table");
    throw SQLError::TABLE_ERROR(e);
}


bool CM::SetIdxOn(TableInfo &table, int index, void *destination)
{
    if (table.SetIdxOn(index)){
        table.WriteTo(destination);
        return true;
    }
    else return false;
}


bool TableInfo::SetIdxOn(int index)
{
    if (columns.at(index).has_index)
        return false;
    else{
        columns.at(index).has_index = true;
        return true;
    }
}


void CM::OpenTableFile(TableInfo &table)
{
    ex_tables.push_back(table);
}


void CM::CloseTable(std::string name)
{
    for (auto vi = ex_tables.begin(); vi != ex_tables.end(); vi++){
        if (vi->table_name == name)
            ex_tables.erase(vi);
    }
}


struct TableInfoMem* TableInfo::GetPatchedData()
{
    struct TableInfoMem *res = new struct TableInfoMem;
    int i;
    for (i = 0; i < table_name.length(); i++){
        res->name[i] = table_name[i];
    }
    res->name[i] = '\0';
    i = n_columns();
    memcpy(res->n_col, &i, 4);
    int attr_info = 0;
    char* dest_addr;
    int data_type;
    for (i = 0; i < columns.size(); i++){
        if (columns[i].is_PK)
            attr_info = attr_info | 0b001;
        if (columns[i].is_unique)
            attr_info = attr_info | 0b010;
        if (columns[i].has_index)
            attr_info = attr_info | 0b100;
        dest_addr = res->columns_info + 32 * i;
        memcpy(dest_addr, &attr_info, 4);
        switch (columns[i].type){
            case DataType::INT:
                data_type = 1;
                break;
            case DataType::FLOAT:
                data_type = 2;
                break;
            case DataType::CHAR:
                data_type = 3;
                break;
            default: break;
        }
        dest_addr += 4;
        memcpy(dest_addr, &data_type, 4);
        dest_addr += 4;
        memcpy(dest_addr, &(columns[i].bytes), 4);
        dest_addr += 4;
        for (int j = 0; j < table_name.length(); j++){
            *dest_addr = table_name[i];
            dest_addr++;
        }
        *dest_addr = '\0';
    }
    return res;
}

void TableInfo::WriteTo(void *destination)
{
    struct TableInfoMem *res =  GetPatchedData();
    memcpy(destination, res, sizeof(struct TableInfoMem));
    delete res;
}

void TableInfo::ReadFrom(void *source)
{
    char name[28];
    memcpy(name, source, 28);
    table_name = std::string(name);
    int n_col;
    memcpy(&n_col, (char*)source + 28, 4);
    int attr_info;
    int data_type;
    char *source_i = (char*)source;
    for (int i = 0; i < n_col; i++){
        source_i = source_i + 32;
        memcpy(&attr_info, source_i, 4);
        columns[i].is_PK = attr_info & 1;
        columns[i].is_unique = attr_info & 2;
        columns[i].has_index = attr_info & 4;
        memcpy(&data_type, source_i + 4, 4);
        switch (data_type){
            case 1:
                columns[i].type = DataType::INT;
                break;
            case 2:
                columns[i].type = DataType::FLOAT;
                break;
            case 3:
                columns[i].type = DataType::CHAR;
                break;
            default: break;
        }
        memcpy(&(columns[i].bytes), source_i + 8, 4);
        memcpy(name, source_i + 12, 20);
        columns[i].column_name = std::string(name);
    }
}