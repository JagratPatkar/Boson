#include <iostream>
#include <memory>
#include <map>
#include "scope.h"
#include "type.h"

class ScopeList {
    Scope* head;
    public:
    ScopeList() {
        head = new Scope();
    }
    void createScope();
    void deleteScope();
    unique_ptr<::Type> searchScope(const string&);
    bool addScopeVariable(const string&,unique_ptr<::Type>);
};