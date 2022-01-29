/* bobenter.c - functions for entering symbols */
/*
        Copyright (c) 2001, by David Michael Betz
        All rights reserved
*/

#include "bob.h"

/* BobEnterVariable - add a built-in variable to the symbol table */
void BobEnterVariable(BobScope *scope,char *name,BobValue value)
{
    BobInterpreter *c = scope->c;
    BobCheck(c,2);
    BobPush(c,value);
    BobPush(c,BobInternCString(c,name));
    BobSetGlobalValue(scope,BobTop(c),c->sp[1]);
    BobDrop(c,2);
}

/* BobEnterFunction - add a built-in function to the symbol table */
void BobEnterFunction(BobScope *scope,BobCMethod *function)
{
    BobInterpreter *c = scope->c;
    BobCPush(c,BobInternCString(c,function->name));
    BobSetGlobalValue(scope,BobTop(c),(BobValue)function);
    BobDrop(c,1);
}

/* BobEnterFunctions - add built-in functions to the symbol table */
void BobEnterFunctions(BobScope *scope,BobCMethod *functions)
{
    for (; functions->name != 0; ++functions)
        BobEnterFunction(scope,functions);
}

/* BobEnterObject - add a built-in object to the symbol table */
BobValue BobEnterObject(BobScope *scope,char *name,BobValue parent,BobCMethod *methods)
{
    BobInterpreter *c = scope->c;

    /* make the object and set the symbol value */
    if (name) {
        BobCheck(c,2);
        BobPush(c,BobMakeObject(c,parent));
        BobPush(c,BobInternCString(c,name));
        BobSetGlobalValue(scope,BobTop(c),c->sp[1]);
        BobDrop(c,1);
    }
    else
        BobCPush(c,BobMakeObject(c,parent));

    /* enter the methods */
    if (methods)
        BobEnterMethods(c,BobTop(c),methods);

    /* return the object */
    return BobPop(c);
}

/* BobEnterCObjectType - add a built-in cobject type to the symbol table */
BobDispatch *BobEnterCObjectType(BobScope *scope,BobDispatch *parent,char *typeName,BobCMethod *methods,BobVPMethod *properties,long size)
{
    BobInterpreter *c = scope->c;
    BobDispatch *d;

    /* make the type */
    if (!(d = BobMakeCObjectType(c,parent,typeName,methods,properties,size)))
        return NULL;

    /* add the type symbol */
    BobCPush(c,BobInternCString(c,typeName));
    BobSetGlobalValue(scope,BobTop(c),d->object);
    BobDrop(c,1);

    /* return the new object type */
    return d;
}

/* BobEnterCPtrObjectType - add a built-in pointer cobject type to the symbol table */
BobDispatch *BobEnterCPtrObjectType(BobScope *scope,BobDispatch *parent,char *typeName,BobCMethod *methods,BobVPMethod *properties)
{
    BobInterpreter *c = scope->c;
    BobDispatch *d;

    /* make the type */
    if (!(d = BobMakeCPtrObjectType(c,parent,typeName,methods,properties)))
        return NULL;

    /* add the type symbol */
    BobCPush(c,BobInternCString(c,typeName));
    BobSetGlobalValue(scope,BobTop(c),d->object);
    BobDrop(c,1);

    /* return the new object type */
    return d;
}

/* BobEnterMethods - add built-in methods to an object */
void BobEnterMethods(BobInterpreter *c,BobValue object,BobCMethod *methods)
{
    BobCheck(c,2);
    BobPush(c,object);
    for (; methods->name != 0; ++methods) {
        BobPush(c,BobInternCString(c,methods->name));
        BobSetProperty(c,c->sp[1],BobTop(c),(BobValue)methods);
        BobDrop(c,1);
    }
    BobDrop(c,1);
}

/* BobEnterMethod - add a built-in method to an object */
void BobEnterMethod(BobInterpreter *c,BobValue object,BobCMethod *method)
{
    BobCheck(c,2);
    BobPush(c,object);
    BobPush(c,BobInternCString(c,method->name));
    BobSetProperty(c,c->sp[1],BobTop(c),(BobValue)method);
    BobDrop(c,2);
}

/* BobEnterVPMethods - add built-in virtual property methods to an object */
void BobEnterVPMethods(BobInterpreter *c,BobValue object,BobVPMethod *methods)
{
    BobCheck(c,2);
    BobPush(c,object);
    for (; methods->name != 0; ++methods) {
        BobPush(c,BobInternCString(c,methods->name));
        BobSetProperty(c,c->sp[1],BobTop(c),(BobValue)methods);
        BobDrop(c,1);
    }
    BobDrop(c,1);
}

/* BobEnterVPMethod - add a built-in method to an object */
void BobEnterVPMethod(BobInterpreter *c,BobValue object,BobVPMethod *method)
{
    BobCheck(c,2);
    BobPush(c,object);
    BobPush(c,BobInternCString(c,method->name));
    BobSetProperty(c,c->sp[1],BobTop(c),(BobValue)method);
    BobDrop(c,2);
}

/* BobEnterProperty - add a property to an object */
void BobEnterProperty(BobInterpreter *c,BobValue object,char *selector,BobValue value)
{
    BobCheck(c,3);
    BobPush(c,object);
    BobPush(c,value);
    BobPush(c,BobInternCString(c,selector));
    BobSetProperty(c,c->sp[2],BobTop(c),c->sp[1]);
    BobDrop(c,3);
}

