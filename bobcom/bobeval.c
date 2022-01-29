/* bobeval.c - evaluator functions */
/*
        Copyright (c) 2001, by David Michael Betz
        All rights reserved
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bob.h"
#include "bobcom.h"

/* method handlers */
static BobValue BIF_Load(BobInterpreter *c);
static BobValue BIF_Eval(BobInterpreter *c);
static BobValue BIF_CompileFile(BobInterpreter *c);

/* function table */
static BobCMethod functionTable[] = {
BobMethodEntry( "Load",                 BIF_Load                ),
BobMethodEntry( "Eval",                 BIF_Eval                ),
BobMethodEntry( "CompileFile",          BIF_CompileFile         ),
BobMethodEntry( 0,                      0                       )
};

/* BobUseEval - enter the built-in functions and symbols for eval */
void BobUseEval(BobInterpreter *c)
{
    BobCMethod *method;

    /* create a compiler context */
    if ((c->compiler = BobMakeCompiler(c,4096,256)) == NULL)
        BobInsufficientMemory(c);
    
    /* enter the eval functions */
    for (method = functionTable; method->name != 0; ++method)
        BobEnterFunction(BobGlobalScope(c),method);
}

/* BobUnuseEval - finish using eval */
void BobUnuseEval(BobInterpreter *c)
{
    if (c->compiler) {
        BobFreeCompiler((BobCompiler *)c->compiler);
        c->compiler = NULL;
    }
}

/* BIF_Load - built-in function 'Load' */
static BobValue BIF_Load(BobInterpreter *c)
{
    BobStream *s = NULL;
    char *name;
    BobParseArguments(c,"**S|P?=",&name,&s,BobFileDispatch);
    return BobLoadFile(BobCurrentScope(c),name,s) ? c->trueValue : c->falseValue;
}

/* BIF_Eval - built-in function 'Eval' */
static BobValue BIF_Eval(BobInterpreter *c)
{
    char *str;
    BobCheckArgCnt(c,3);
    BobCheckType(c,3,BobStringP);
    str = (char *)BobStringAddress(BobGetArg(c,3));
    return BobEvalString(BobCurrentScope(c),str);
}

/* BIF_CompileFile - built-in function 'CompileFile' */
static BobValue BIF_CompileFile(BobInterpreter *c)
{
    char *iname,*oname;
    BobCheckArgCnt(c,4);
    BobCheckType(c,3,BobStringP);
    BobCheckType(c,4,BobStringP);
    iname = (char *)BobStringAddress(BobGetArg(c,3));
    oname = (char *)BobStringAddress(BobGetArg(c,4));
    return BobCompileFile(BobCurrentScope(c),iname,oname) ? c->trueValue : c->falseValue;
}

/* BobEvalString - evaluate a string */
BobValue BobEvalString(BobScope *scope,char *str)
{
    BobStream *s = BobMakeStringStream(scope->c,str,strlen(str));
    if (s) {
        BobValue value = BobEvalStream(scope,s);
        BobCloseStream(s);
        return value;
    }
    else
        return scope->c->falseValue;
}

/* BobEvalStream - evaluate a stream */
BobValue BobEvalStream(BobScope *scope,BobStream *s)
{
    BobValue val;
    BobInitScanner(scope->c->compiler,s);
    val = BobCompileExpr(scope);
    return val ? BobCallFunction(scope,val,0) : NULL;
}

/* BobLoadFile - read and evaluate expressions from a file */
int BobLoadFile(BobScope *scope,char *fname,BobStream *os)
{
    BobInterpreter *c = scope->c;
    BobUnwindTarget target;
    BobStream *is;
    int sts;

    /* open the source file */
    if ((is = BobOpenFileStream(c,fname,"r")) == NULL)
        BobCallErrorHandler(c,BobErrFileNotFound,fname);
    
    /* setup the unwind target */
    BobPushUnwindTarget(c,&target);
    if ((sts = BobUnwindCatch(c)) != 0) {
        BobCloseStream(is);
        BobPopAndUnwind(c,sts);
    }
        
    /* announce the file */
    if (os) {
        BobStreamPutS("Loading '",os);
        BobStreamPutS(fname,os);
        BobStreamPutS("'\n",os);
    }

    /* load the source file */
    BobLoadStream(scope,is,os);

    /* return successfully */
    BobPopUnwindTarget(c);
    BobCloseStream(is);
    return TRUE;
}

/* BobLoadStream - read and evaluate a stream of expressions */
void BobLoadStream(BobScope *scope,BobStream *is,BobStream *os)
{
    BobInterpreter *c = scope->c;
    BobValue expr;
    BobInitScanner(c->compiler,is);
    while ((expr = BobCompileExpr(scope)) != NULL) {
        BobValue val = BobCallFunction(scope,expr,0);
        if (os) {
            BobPrint(c,val,os);
            BobStreamPutC('\n',os);
        }
    }
}
