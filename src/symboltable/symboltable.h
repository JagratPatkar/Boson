#include <map>
#include <utility>
using namespace std;


#ifndef SYMBOL_TABLE
#define SYMBOL_TABLE
template <class T1, class T2>
class SymbolTable
{
    map<T1, T2> Table;

public:
    void addElement(T1 t1, T2 t2) { Table[t1] = t2; }
    T2 getElement(T1 t) { return Table[t]; }
    bool doesElementExist(T1 t) { return Table.find(t) != Table.end(); }
    void clearTable() { Table.clear(); }
};
#endif