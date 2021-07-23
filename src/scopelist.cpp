#include "scopelist.h"
#include "scope.h"

void ScopeList::createScope(){
    Scope* temp = new Scope(head);
    head = temp;
}

void ScopeList::deleteScope(){
    Scope* temp = head->next;
    delete head;
    head = temp;
}

unique_ptr<::Type> ScopeList::searchScope(const string& name){
    Scope* temp = head;
    do {
        if(temp->doesVariableExist(name)){
            return temp->returnType(name);
        }
    }while(temp->next != nullptr);
    return nullptr;
}

bool ScopeList::addScopeVariable(const string& name,unique_ptr<::Type> ty){
    return head->addVariable(name,move(ty));
}