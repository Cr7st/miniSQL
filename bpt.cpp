#include "bpt.h"
using namespace std;

//data_node *data ;

void bpt::query( bpt_node *now, int first, int rear){
    if ( now->is_leaf ) {
        for (int i = 0; i < now->key_num; i++)
            cout << "(" << now->key[i] << ")";
        cout<<"||"<<now->key_num;
    }
    else {
        for (int i = 0; i <= now->key_num; i++)
            query((bpt_node *) now->pointer[i], first, rear );
        cout<<"!!"<<now->key_num;
    }
}

bpt_node *bpt::new_bpt_node()
{
    node_count ++ ;

    bpt_node *p = new bpt_node ;
    p -> is_leaf = false ;
    p -> is_root = false ;
    p -> key_num = 0 ;
    p -> pointer[ 0 ] = NULL ;
    p -> father = NULL ;
    return p ;
}

bpt::bpt( string file_name = "index.txt" )
{
    root = new_bpt_node() ;
    root -> is_root = true ;
    root -> is_leaf = true ;
    name = file_name;
    node_count = 0 ;
    splite_count = 0 ;
    //data = NULL ;
}

/*
data_node *new_data_node( int key , char *value )
{
    data_node *p = new data_node ;
    p -> key = key ;
    p -> value = value ;
    p -> next = NULL ;
    return p ;
}
void insert_data( data_node *father , data_node *data )
{
    data -> next = father -> next ;
    father -> next = ( void *) data ;
}
*/

bpt_node *bpt::find_leaf( int key )
{
    bpt_node *now = root ;
    while ( !now -> is_leaf )
    {
        for ( int i = 0 ; i <= now -> key_num ; i ++ )
            if ( i == now -> key_num || key < now -> key[ i ] )
            {
                now = ( bpt_node *) now -> pointer[ i ] ;
                break ;
            }
    }
    return now ;
}

void bpt::split( bpt_node *node )
{
    splite_count ++ ;
    //cout << endl << "split!:" << node -> key_num << ' ' << node -> key[ 0 ] << ' ' << node -> key[ 1 ] << ' ' << node -> key[ 2 ] << endl;
    bpt_node *nodd = new_bpt_node() ;
    int mid_key = node -> key[ M / 2 ] ;

    nodd -> key_num = M - M / 2 - 1 ;
    for ( int i = 0 ; i <= nodd -> key_num ; i ++ )
    {
        if ( i != nodd -> key_num )
            nodd -> key[ i ] = node -> key[ i + ( M / 2 + 1 ) ] ;
        nodd -> pointer[ i ] = node -> pointer[ i + ( M / 2 + 1 ) ] ;
        if ( !node -> is_leaf ) {
            bpt_node *tmp = ( bpt_node * ) nodd -> pointer[ i ];
            tmp->father = nodd;
            nodd -> pointer[ i ] = ( void* )tmp;
        }
    }
    node -> key_num = M / 2 ;

    if ( node -> is_leaf )
    {
        node -> key_num ++ ;
        nodd -> pointer[ 0 ] = node -> pointer[ 0 ] ;
        node -> pointer[ 0 ] = nodd ;
        nodd -> is_leaf = true ;
        mid_key = node -> key[ M / 2 + 1 ] ;
    }

    if ( node -> is_root )
    {
        node -> is_root = false ;
        root = new_bpt_node() ;
        root -> is_root = true ;
        root -> key[ 0 ] = nodd -> key[0] ;
        root -> pointer[ 0 ] = node ;
        root -> pointer[ 1 ] = nodd ;
        root -> key_num = 1 ;

        node -> father = nodd -> father = root ;
    } else
    {
        nodd -> father = node -> father ;
        insert_in_node( ( bpt_node * ) node -> father , nodd -> key[1] , ( void *) nodd ) ;
    }
    /*
    cout << "midnum: " << mid_key << endl;
    cout << "father: " << (( bpt_node * )node -> father )-> key_num << ' '<< ((bpt_node*)node -> father )-> key[ 0 ] << endl;
    cout << "node: " << node -> key_num << ' ' << node -> key[ 0 ] << ' ' << node -> key[ 1 ] << endl;
    cout << "nodd: " << nodd -> key_num << ' ' << nodd -> key[ 0 ] << ' ' << nodd -> key[ 1 ] << endl << endl ;
    */
}

void bpt::insert_in_node( bpt_node *node , int key , void *value )
{
    int x = 0 ;
    while ( x < node -> key_num && node -> key[ x ] < key ) x ++ ;
    for ( int i = node -> key_num ; i > x ; i -- )
        node -> key[ i ] = node -> key[ i - 1 ] ;
    for ( int i = node -> key_num + 1 ; i > x + 1 ; i -- )
        node->pointer[i] = node->pointer[i - 1];
    node -> key[ x ] = key ;
    node -> pointer[ x + 1 ] = value ;
    node -> key_num ++ ;
    //( bpt_node* )value -> father = node;

    if ( node -> key_num >= M ) // need to split
        split( node ) ;
}

void bpt::delete_in_node( bpt_node *node , int key )
{
    int x = 0 ;
    while ( key != node -> key[ x ] ) x ++ ;
    for ( int i = x ; i < node -> key_num - 1 ; i ++ )
        node -> key[ i ] = node -> key[ i + 1 ] ;
    for ( int i = x + 1 ; i < node -> key_num ; i ++ )
        node -> pointer[ i ] = node -> pointer[ i + 1 ] ;
    node -> key_num -- ;
}

bool bpt::insert_in_bpt( int key , void *value )
{
    bpt_node *leaf = find_leaf( key ) ;
    for ( int i = 0 ; i < leaf -> key_num ; i ++ )
        if ( key == leaf -> key[ i ] )
            return false ;
    insert_in_node( leaf , key , value ) ;
    return true ;
}

bool bpt::delete_in_bpt( int key )
{
    bpt_node *leaf = find_leaf( key ) ;
    for ( int i = 0 ; i < leaf -> key_num ; i ++ )
        if ( key == leaf -> key[ i ] )
        {
            delete_in_node( leaf , key ) ;
            return true ;
        }
    return false ;
}

char *bpt::query_in_bpt( int key )
{
    bpt_node *leaf = find_leaf( key ) ;
    for ( int i = 0 ; i < leaf -> key_num ; i ++ )
        if ( key == leaf -> key[ i ] )
            return ( char * ) leaf -> pointer[ i + 1 ] ;
    return NULL ;
}