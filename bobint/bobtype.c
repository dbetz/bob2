/* bobtype.c - derived types */
/*
        Copyright (c) 2001, by David Michael Betz
        All rights reserved
*/

#include <stdlib.h>
#include <string.h>
#include "bob.h"

/* TYPE */

static BobValue TypeNewInstance(BobInterpreter *c,BobValue parent)
{
    BobDispatch *d = (BobDispatch *)BobCObjectValue(parent);
    return (*d->newInstance)(c,parent);
}

static int TypePrint(BobInterpreter *c,BobValue val,BobStream *s)
{
    BobDispatch *d = BobTypeDispatch(val);
    char *p = d->typeName;
    BobStreamPutS("<Type-",s);
    while (*p)
        if (BobStreamPutC(*p++,s) == BobStreamEOF)
            return BobStreamEOF;
    BobStreamPutC('>',s);
    return 0;
}

/* BobInitTypes - initialize derived types */
void BobInitTypes(BobInterpreter *c)
{
    /* make all of the derived types */
    if (!(c->typeDispatch = BobMakeDispatch(c,"Type",&BobCObjectDispatch)))
        BobInsufficientMemory(c);

    /* initialize the 'Type' type */
    c->typeDispatch->dataSize = sizeof(BobCPtrObject) - sizeof(BobCObject);
    c->typeDispatch->object = BobEnterType(BobGlobalScope(c),"Type",c->typeDispatch);
    c->typeDispatch->newInstance = TypeNewInstance;
    c->typeDispatch->print = TypePrint;

    /* fixup the 'Type' class */
    BobSetObjectClass(c->typeDispatch->object,c->typeDispatch->object);
}

/* BobAddTypeSymbols - add symbols for the built-in types */
void BobAddTypeSymbols(BobInterpreter *c)
{
    BobEnterType(BobGlobalScope(c),"CObject",         &BobCObjectDispatch);
    BobEnterType(BobGlobalScope(c),"Symbol",          &BobSymbolDispatch);
    BobEnterType(BobGlobalScope(c),"CMethod",         &BobCMethodDispatch);
    BobEnterType(BobGlobalScope(c),"Property",        &BobPropertyDispatch);
    BobEnterType(BobGlobalScope(c),"CompiledCode",    &BobCompiledCodeDispatch);
    BobEnterType(BobGlobalScope(c),"Environment",     &BobEnvironmentDispatch);
    BobEnterType(BobGlobalScope(c),"StackEnvironment",&BobStackEnvironmentDispatch);
    BobEnterType(BobGlobalScope(c),"MovedEnvironment",&BobMovedEnvironmentDispatch);
}

/* BobEnterType - enter a type */
BobValue BobEnterType(BobScope *scope,char *name,BobDispatch *d)
{
    BobInterpreter *c = scope->c;
    BobCheck(c,2);
    BobPush(c,BobMakeCPtrObject(c,c->typeDispatch,d));
    BobPush(c,BobInternCString(c,name));
    BobSetGlobalValue(scope,BobTop(c),c->sp[1]);
    BobDrop(c,1);
    return BobPop(c);
}
