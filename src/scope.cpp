#include "scope.h"

bool Scope::addVariable(const string& name, unique_ptr<::Type> ty) {
    if(doesVariableExist(name)) return false;
    table[name] = move(ty); 
    return true;
}

unique_ptr<::Type> Scope::returnType(const string& name) { return table[name]->getNew(); }