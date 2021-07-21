#include <iostream>
#include <memory>
#include <map>
#include "type.h"

#ifndef SCOPE
#define SCOPE 

class Scope{
    map<string, unique_ptr<::Type> > table;
    
    public:
    Scope* next;
    Scope(Scope* ref) : next(ref) {}
    Scope() { next = nullptr; }
    bool doesVariableExist(const string& name) { return table[name] != 0; }
    bool addVariable(const string&, unique_ptr<::Type>);
    unique_ptr<::Type> returnType(const string&);
};
#endif