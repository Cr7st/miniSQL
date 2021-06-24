/*#include <iostream>
#include <cstdio>
#include <cstring>
#include <time.h>
#include "API.h"
using namespace std;

int main()
{
<<<<<<< HEAD
    vector<string> data_types;
    vector<string> column_names;
    string table_name = string("student");
    data_types.push_back(string("char16"));
    data_types.push_back(string("int"));
    data_types.push_back(string("float"));
    column_names.push_back(string("name"));
    column_names.push_back(string("score"));
    column_names.push_back(string("height"));
    //CreateTable(table_name, column_names, data_types, 0);
    //OpenTable(string("student.db"));
    vector<DataClass> list;
    DataClass d1("ZZH");
    DataClass d2(9);
    DataClass d3(1.73);
    list.push_back(d1);
    list.push_back(d2);
    list.push_back(d3);
    vector<SelectCondition> conditions;
    //SelectTuples(conditions, table_name);
    SelectCondition s;
    s.op = "<";
    s.attr = "score";
    s.value = DataClass(int(9));
    vector<Tuple> res;
    conditions.push_back(s);
    //DeleteTuples(conditions, table_name);
    //InsertTuple(table_name, list);
    DropTable(table_name);
}
=======
    printf( "Usage:\n" ) ;
    printf( "Insert operation: insert [key] [value]\n" ) ;
    printf( "Delete operation: delete [key]\n" ) ;
    printf( "Query  operation: query [key]\n" ) ;
}

int test()
{
    bpt test("index.txt");
    //initial_bpt() ;
    int f_num ;
    char tmp_c[ 10 ]= "haha";
    insert_count = 0 ;

    //抗压测试
    clock_t start = clock() , finish ;
    for( int f_num=1e6; f_num >= 0 ; f_num -- )
    {
        tmp_c[3] = f_num%24 + 'a';
        char *f_value = ( char * ) new char[ strlen( tmp_c ) + 1 ] ;
        strcpy( f_value , tmp_c ) ;
        if ( test.insert_in_bpt( f_num , f_value ) )
        {
            //printf( "Insert %d %s OK!\n" , f_num , f_value ) ;
            insert_count ++ ;
        } else
            printf( "Ignore %d %s\n" , f_num , f_value ) ;
        //break;
    }
    finish = clock();
    printf( "%f seconds\n", (double)(finish - start) / CLOCKS_PER_SEC );

    printf( "B plus tree build OK.\n" ) ;
    //printf( "node_cout is : %d\n" , node_count ) ;
    printf( "insert_count is : %d\n" , insert_count ) ;
    //printf( "splite_count is : %d\n" , splite_count ) ;

    extern bpt_node *root ;
    //query( root );

    while ( 1 )
    {
        usage() ;
        scanf( "%s" , tmp_c ) ;
        if ( strcmp( tmp_c , "insert" ) == 0 )
        {
            scanf( "%d%s" , &f_num , tmp_c ) ;
            char *f_value = ( char * ) new char[ strlen( tmp_c ) + 1 ] ;
            strcpy( f_value , tmp_c ) ;
            if ( test.insert_in_bpt( f_num , f_value ) )
            {
                //printf( "Insert %d %s OK!\n" , f_num , f_value ) ;
                insert_count ++ ;
            } else ;
            //printf( "Ignore %d %s\n" , f_num , f_value ) ;

        } else
        if ( strcmp( tmp_c , "delete" ) == 0 )
        {
            scanf( "%d" , &f_num ) ;
            if ( test.delete_in_bpt( f_num ) )
                printf( "Delete %d OK!\n" , f_num ) ;
            else
                printf( "can't delete key%d\n" , f_num ) ;
        } else
        if ( strcmp( tmp_c , "query" ) == 0 )
        {
            scanf( "%d" , &f_num ) ;
            char *tmp_c = test.query_in_bpt( f_num ) ;
            if ( tmp_c == NULL )
                printf( "can't find key%d\n" , f_num ) ;
            else
                printf( "value of key%d is : %s\n" , f_num , tmp_c ) ;
        } else usage() ;
    }

    return 0;
}*/
