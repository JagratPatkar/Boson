#include <map>
#include <memory>

#include <utility>
using namespace std;

#ifndef SYMBOL_TABLE
#define SYMBOL_TABLE
template <class T1, class T2>
class SymbolTable
{
protected:
    map<T1, T2> Table;

public:
    void addElement(T1 t1, T2 t2) { Table.insert(make_pair(t1, move(t2))); }
    T2 getElement(T1 t) { return Table[t]; }
    bool doesElementExist(T1 t) { return Table.find(t) != Table.end(); }
    void clearTable() { Table.clear(); }
};

#endif