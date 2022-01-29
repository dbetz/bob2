/* bobsymbol.c - 'Symbol' handler */
/*
        Copyright (c) 2001, by David Michael Betz
        All rights reserved
*/

#include <string.h>
#include "bob.h"

/* property handlers */
static BobValue BIF_printName(BobInterpreter *c,BobValue obj);

/* Vector methods */
static BobCMethod methods[] = {
BobMethodEntry( 0,                  0                   )
};

/* Vector properties */
static BobVPMethod properties[] = {
BobVPMethodEntry( "printName",      BIF_printName,      0                   ),
BobVPMethodEntry( 0,                0,                  0                   )
};

/* BobInitSymbol - initialize the 'Symbol' object */
void BobInitSymbol(BobInterpreter *c)
{
    c->symbolObject = BobEnterType(BobGlobalScope(c),"Symbol",&BobSymbolDispatch);
    BobEnterMethods(c,c->symbolObject,methods);
    BobEnterVPMethods(c,c->symbolObject,properties);
}

/* BIF_printName - built-in property 'printName' */
static BobValue BIF_printName(BobInterpreter *c,BobValue obj)
{
    return BobMakeString(c,BobSymbolPrintName(obj),BobSymbolPrintNameLength(obj));
}

/* SYMBOL */

#define SymbolHashValue(o)              (((BobSymbol *)o)->hashValue)
#define SetSymbolHashValue(o,v)         (((BobSymbol *)o)->hashValue = (v))
#define SetSymbolPrintNameLength(o,v)   (((BobSymbol *)o)->printNameLength = (v))

static int GetSymbolProperty(BobInterpreter *c,BobValue obj,BobValue tag,BobValue *pValue);
static int SetSymbolProperty(BobInterpreter *c,BobValue obj,BobValue tag,BobValue value);
static int SymbolPrint(BobInterpreter *c,BobValue val,BobStream *s);
static long SymbolSize(BobValue obj);
static BobValue SymbolCopy(BobInterpreter *c,BobValue obj);
static BobIntegerType SymbolHash(BobValue obj);

static BobValue MakeSymbol(BobInterpreter *c,unsigned char *printName,int length,BobIntegerType hashValue);
static BobValue AllocateSymbolSpace(BobInterpreter *c,long size);

BobDispatch BobSymbolDispatch = {
    "Symbol",
    &BobSymbolDispatch,
    GetSymbolProperty,
    SetSymbolProperty,
    BobDefaultNewInstance,
    SymbolPrint,
    SymbolSize,
    SymbolCopy,
    BobDefaultScan,
    SymbolHash
};

/* GetSymbolProperty - Symbol get property handler */
static int GetSymbolProperty(BobInterpreter *c,BobValue obj,BobValue tag,BobValue *pValue)
{
    return BobGetVirtualProperty(c,obj,c->symbolObject,tag,pValue);
}

/* SetSymbolProperty - Symbol set property handler */
static int SetSymbolProperty(BobInterpreter *c,BobValue obj,BobValue tag,BobValue value)
{
    return BobSetVirtualProperty(c,obj,c->symbolObject,tag,value);
}

/* SymbolPrint - Symbol print handler */
static int SymbolPrint(BobInterpreter *c,BobValue val,BobStream *s)
{
    char *p = BobSymbolPrintName(val);
    long size = BobSymbolPrintNameLength(val);
    while (--size >= 0)
        if (BobStreamPutC(*p++,s) == BobStreamEOF)
            return BobStreamEOF;
    return 0;
}

/* SymbolSize - Symbol size handler */
static long SymbolSize(BobValue obj)
{
    BobSymbol *sym = (BobSymbol *)obj;
    return sizeof(BobSymbol) + BobRoundSize(sym->printNameLength - 1);
}

/* SymbolCopy - Symbol copy handler */
static BobValue SymbolCopy(BobInterpreter *c,BobValue obj)
{
    return obj;
}

/* SymbolHash - Symbol hash handler */
static BobIntegerType SymbolHash(BobValue obj)
{
    return SymbolHashValue(obj);
}

/* BobMakeSymbol - make a new symbol */
BobValue BobMakeSymbol(BobInterpreter *c,unsigned char *printName,int length)
{
    return MakeSymbol(c,printName,length,BobHashString(printName,length));
}

/* MakeSymbol - make a new symbol */
static BobValue MakeSymbol(BobInterpreter *c,unsigned char *printName,int length,BobIntegerType hashValue)
{
    long allocSize = sizeof(BobSymbol) + BobRoundSize(length - 1);
    BobValue sym = AllocateSymbolSpace(c,allocSize);
    memcpy(BobSymbolPrintName(sym),printName,length);
    BobSetDispatch(sym,&BobSymbolDispatch);
    SetSymbolHashValue(sym,hashValue);
    SetSymbolPrintNameLength(sym,length);
    return sym;
}

/* BobIntern - intern a symbol given its print name as a string */
BobValue BobIntern(BobInterpreter *c,BobValue printName)
{
    return BobInternString(c,BobStringAddress(printName),BobStringSize(printName));
}

/* BobInternCString - intern a symbol given its print name as a c string */
BobValue BobInternCString(BobInterpreter *c,char *printName)
{
    return BobInternString(c,(unsigned char *)printName,strlen(printName));
}

/* BobInternString - intern a symbol given its print name as a string/length */
BobValue BobInternString(BobInterpreter *c,unsigned char *printName,int length)
{
    BobIntegerType hashValue = BobHashString(printName,length);
    BobIntegerType i;
    BobValue p = BobObjectProperties(c->symbols);
    if (BobHashTableP(p)) {
        i = hashValue & (BobHashTableSize(p) - 1);
        p = BobHashTableElement(p,i);
    }
    else
        i = -1;
    for (; p != c->nilValue; p = BobPropertyNext(p)) {
        BobValue sym = BobPropertyTag(p);
        if (length == BobSymbolPrintNameLength(sym)
        &&  memcmp(printName,BobSymbolPrintName(sym),length) == 0)
            return sym;
    }
    BobCPush(c,MakeSymbol(c,printName,length,hashValue));
    BobAddProperty(c,c->symbols,BobTop(c),c->nilValue,hashValue,i);
    return BobPop(c);
}

/* BobGlobalValue - get the value of a global symbol */
int BobGlobalValue(BobScope *scope,BobValue sym,BobValue *pValue)
{
    BobInterpreter *c = scope->c;
    BobValue obj,property;
    for (obj = BobScopeObject(scope); BobObjectP(obj); obj = BobObjectClass(obj))
        if ((property = BobFindProperty(c,obj,sym,NULL,NULL)) != NULL) {
            *pValue = BobPropertyValue(property);
            return TRUE;
        }
    return FALSE;
}

/* BobSetGlobalValue - set the value of a global symbol */
void BobSetGlobalValue(BobScope *scope,BobValue sym,BobValue value)
{
    BobSetProperty(scope->c,BobScopeObject(scope),sym,value);
}

/* AllocateSymbolSpace - allocate symbol space */
static BobValue AllocateSymbolSpace(BobInterpreter *c,long size)
{
    BobValue p;
    if (!c->symbolSpace || c->symbolSpace->bytesRemaining < size) {
        BobSymbolBlock *b;
        if (!(b = (BobSymbolBlock *)BobAlloc(c,sizeof(BobSymbolBlock))))
            BobInsufficientMemory(c);
        b->bytesRemaining = BobSBSize;
        b->nextByte = b->data;
        b->next = c->symbolSpace;
        c->symbolSpace = b;
    }
    c->symbolSpace->bytesRemaining -= size;
    p = (BobValue)c->symbolSpace->nextByte;
    c->symbolSpace->nextByte += size;
    return p;
}
