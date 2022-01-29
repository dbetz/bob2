int BobSaveWorkspace(BobInterpreter *c,BobStream *s);
BobInterpreter *BobRestoreWorkspace(BobInterpreter *c,BobStream *s);

static int WriteTypeName(BobDispatch *d,BobStream *s);
static BobDispatch *ReadTypeName(BobInterpreter *c,BobStream *s);
static void AddType(BobInterpreter *c,BobDispatch *d);
static int WriteLong(long n,BobStream *s);
static int ReadLong(long *pn,BobStream *s);
static int WriteFloat(BobFloatType n,BobStream *s);
static int ReadFloat(BobFloatType *pn,BobStream *s);
static int WritePointer(BobInterpreter *c,BobValue p,BobStream *s);
static int ReadPointer(BobInterpreter *c,BobValue *pp,BobStream *s);

static BobValue BIF_SaveWorkspace(BobInterpreter *c);

/* BIF_SaveWorkspace - built-in function 'SaveWorkspace' */
static BobValue BIF_SaveWorkspace(BobInterpreter *c)
{
    char *fname;
    BobStream *s;
    int sts;

    /* check the arguments list */
    BobCheckArgCnt(c,1);
    BobCheckType(c,1,BobStringP);

    /* open the output file */
    fname = (char *)BobStringAddress(BobGetArg(c,1));
    if ((s = BobOpenFileStream(fname,"wb")) == NULL)
        return c->falseValue;

    /* save the workspace to the file */
    sts = BobSaveWorkspace(c,s);

    /* close the file */
    BobCloseStream(s);

    /* return status */
    return sts ? c->trueValue : c->falseValue;
}

/* BobSaveWorkspace - save a workspace image */
int BobSaveWorkspace(BobInterpreter *c,BobStream *s)
{
    unsigned char *next;
    BobDispatch *d;
    BobValue obj;
    
    /* first compact memory */
    BobCollectGarbage(c);
    
    /* write the file header */
    WriteLong((long)(c->newSpace->top - c->newSpace->base),s);
    WriteLong((long)(c->stackTop - c->stack),s);
    WriteLong((long)(c->newSpace->free - c->newSpace->base),s);
    WriteLong((long)c->nextTypeTag,s);

    /* write the type names */
    for (d = c->types; d != NULL; d = d->next)
        if (!WriteTypeName(d,s))
            return FALSE;

    /* write the root objects */
    WritePointer(c,c->nilValue,s);
    WritePointer(c,c->trueValue,s);
    WritePointer(c,c->falseValue,s);
    WritePointer(c,c->unboundValue,s);
    WritePointer(c,c->objectValue,s);
    WritePointer(c,c->symbols,s);
    
    /* write the heap */
    for (next = c->newSpace->base; next < c->newSpace->free; next += ValueSize(obj)) {
        obj = (BobValue)next;
        if (!BobQuickGetDispatch(obj)->write(c,obj,s))
            return FALSE;
    }
    return TRUE;
}

/* BobRestoreWorkspace - restore a saved workspace image */
BobInterpreter *BobRestoreWorkspace(BobInterpreter *c,BobStream *s)
{
    long size,stackSize,typeCount,typeTableSize,tmp;
    unsigned char *firstFree;
    BobDispatch **typeTable;
    BobDispatch *d;
    BobValue obj;

    /* free the current memory spaces */
    FreeMemorySpaces(c);
    
    /* read the file header */
    ReadLong(&size,s);
    ReadLong(&stackSize,s);
    ReadLong(&tmp,s);
    ReadLong(&typeCount,s);

    /* allocate the type table */
    typeTableSize = typeCount * sizeof(BobDispatch *);
    if ((typeTable = (BobDispatch **)malloc(typeTableSize)) == NULL) {
        BobFreeInterpreter(c);
        return NULL;
    }
    memset(typeTable,0,typeTableSize);
    c->nextTypeTag = (int)typeCount;
    
    /* compute first free address after the load */
    firstFree = c->newSpace->base + tmp;
    
    /* read the type names */
    for (tmp = typeCount; --typeCount >= 0; )
        if ((typeTable[tmp] = ReadTypeName(c,s)) == NULL) {
            BobFreeInterpreter(c);
            free(typeTable);
            return NULL;
        }

    /* create the new memory spaces */
    AllocateMemorySpaces(c,size,stackSize);
    
    /* read the root objects */
    ReadPointer(c,&c->nilValue,s);
    ReadPointer(c,&c->trueValue,s);
    ReadPointer(c,&c->falseValue,s);
    ReadPointer(c,&c->unboundValue,s);
    ReadPointer(c,&c->objectValue,s);
    ReadPointer(c,&c->symbols,s);

    /* read the heap */
    while (c->newSpace->free < firstFree) {
        ReadLong(&tmp,s);
        if (tmp < 0 || tmp >= typeCount) {
            BobFreeInterpreter(c);
            free(typeTable);
            return NULL;
        }
        d = typeTable[(int)tmp];
        obj = (BobValue)c->newSpace->free;
        BobSetDispatch(obj,d);
        if (!d->read(c,obj,s)) {
            BobFreeInterpreter(c);
            free(typeTable);
            return NULL;
        }
        c->newSpace->free += ValueSize(obj);
    }

    /* free the type table */
    free(typeTable);

    /* initialize the interpreter */
    InitInterpreter(c);

    /* return the interpreter */
    return c;
}

static int IntegerWrite(BobInterpreter *c,BobValue obj,BobStream *s);
static int IntegerRead(BobInterpreter *c,BobValue obj,BobStream *s);

static int IntegerWrite(BobInterpreter *c,BobValue obj,BobStream *s)
{
    return WriteLong(BobTypeTag(obj),s)
        && WriteLong((long)BobIntegerValue(obj),s);
}

static int IntegerRead(BobInterpreter *c,BobValue obj,BobStream *s)
{
    long value;
    if (!ReadLong(&value,s))
        return FALSE;
    BobSetHeapIntegerValue(obj,(BobIntegerType)value);
    return TRUE;
}

static int FloatWrite(BobInterpreter *c,BobValue obj,BobStream *s);
static int FloatRead(BobInterpreter *c,BobValue obj,BobStream *s);

static int FloatWrite(BobInterpreter *c,BobValue obj,BobStream *s)
{
    return WriteLong(BobTypeTag(obj),s)
        && WriteFloat(BobFloatValue(obj),s);
}

static int FloatRead(BobInterpreter *c,BobValue obj,BobStream *s)
{
    BobFloatType value;
    if (!ReadFloat(&value,s))
        return FALSE;
    BobSetFloatValue(obj,value);
    return TRUE;
}

static int StringWrite(BobInterpreter *c,BobValue obj,BobStream *s);
static int StringRead(BobInterpreter *c,BobValue obj,BobStream *s);

static int StringWrite(BobInterpreter *c,BobValue obj,BobStream *s)
{
    unsigned char *p = BobStringAddress(obj);
    long size = BobStringSize(obj);
    if (!WriteLong(BobTypeTag(obj),s) || !WriteLong(size,s))
        return FALSE;
    for (; --size >= 0; ++p)
        if (BobStreamPutC(*p,s) == BobStreamEOF)
            return FALSE;
    return TRUE;
}

static int StringRead(BobInterpreter *c,BobValue obj,BobStream *s)
{
    unsigned char *p = BobStringAddress(obj);
    long size;
    int ch;
    if (!ReadLong(&size,s))
        return FALSE;
    BobSetStringSize(obj,size);
    while (--size >= 0) {
        if ((ch = BobStreamGetC(s)) == BobStreamEOF)
            return FALSE;
        *p++ = ch;
    }
    return TRUE;
}

static int VectorWrite(BobInterpreter *c,BobValue obj,BobStream *s);
static int VectorRead(BobInterpreter *c,BobValue obj,BobStream *s);

static int VectorWrite(BobInterpreter *c,BobValue obj,BobStream *s)
{
    long i,size = BobVectorSize(obj);
    if (!WriteLong(BobTypeTag(obj),s) || !WriteLong(size,s))
        return FALSE;
    for (i = 0; i < size; ++i)
        if (!WritePointer(c,BobVectorElement(obj,i),s))
            return FALSE;
    return TRUE;
}

static int VectorRead(BobInterpreter *c,BobValue obj,BobStream *s)
{
    BobValue value;
    long i,size;
    if (!ReadLong(&size,s))
        return FALSE;
    BobSetVectorSize(obj,size);
    for (i = 0; i < size; ++i) {
        if (!ReadPointer(c,&value,s))
            return FALSE;
        BobSetVectorElement(obj,i,value);
    }
    return TRUE;
}

static int CMethodWrite(BobInterpreter *c,BobValue obj,BobStream *s);
static int CMethodRead(BobInterpreter *c,BobValue obj,BobStream *s);

static int CMethodWrite(BobInterpreter *c,BobValue obj,BobStream *s)
{
    char *p = BobCMethodName(obj);
    if (!WriteLong(BobTypeTag(obj),s))
        return FALSE;
    for (; *p != '\0'; ++p)
        if (BobStreamPutC(*p,s) == BobStreamEOF)
            return FALSE;
    return BobStreamPutC('\0',s) != BobStreamEOF;
}

static int CMethodRead(BobInterpreter *c,BobValue obj,BobStream *s)
{
    char name[256],*p = name;
    BobCMethodHandler *handler;
    int ch;
    while ((ch = BobStreamGetC(s)) != '\0') {
        if (ch == BobStreamEOF)
            return FALSE;
        *p++ = ch;
    }
    *p = '\0';
    if ((handler = (*c->findCMethodHandler)(name)) == NULL)
        return FALSE;
    SetCMethodName(obj,name);
    SetCMethodHandler(obj,handler);
    return TRUE;
}

static int ForeignPtrWrite(BobInterpreter *c,BobValue obj,BobStream *s);
static int ForeignPtrRead(BobInterpreter *c,BobValue obj,BobStream *s);

static int ForeignPtrWrite(BobInterpreter *c,BobValue obj,BobStream *s)
{
    if (!WriteLong(BobTypeTag(obj),s))
        return FALSE;
    return TRUE;
}

static int ForeignPtrRead(BobInterpreter *c,BobValue obj,BobStream *s)
{
    SetForeignPtr(obj,NULL);
    return TRUE;
}

/* WriteTypeName - write a type name to an image file */
static int WriteTypeName(BobDispatch *d,BobStream *s)
{
    char *p = d->typeName;
    int ch;
    do {
        ch = *p++;
        if (BobStreamPutC(ch,s) != ch)
            return FALSE;
    } while (ch != '\0');
    return TRUE;
}

static int ForeignPtrWrite(BobInterpreter *c,BobValue obj,BobStream *s);
static int ForeignPtrRead(BobInterpreter *c,BobValue obj,BobStream *s);

/* ReadTypeName - read a type name and return the matching type dispatch structure */
static BobDispatch *ReadTypeName(BobInterpreter *c,BobStream *s)
{
    char typeName[MaxTypeName + 1],*p = typeName;
    BobDispatch *d;
    int ch;

    /* read the type name */
    do {
        if (p > &typeName[MaxTypeName])
            return NULL;
        if ((ch = BobStreamGetC(s)) == BobStreamEOF)
            return NULL;
        *p++ = ch;
    } while (ch != '\0');

    /* find the type dispatch structure */
    for (d = c->types; d != NULL; d = d->next)
        if (strcmp(typeName,d->typeName) == 0)
            return d;

    /* type not found */
    return NULL;
}

/* AddType - add a type dispatch structure to the type list */
static void AddType(BobInterpreter *c,BobDispatch *d)
{
    /* add to the interpreter list */
    if (c) {
        d->typeTag = c->nextTypeTag++;
        d->next = c->types;
        c->types = d;
    }

    /* add to the global list */
    else {
        d->typeTag = nextTypeTag++;
        d->next = types;
        types = d;
    }
}

/* WriteLong - write a long value to an image file */
static int WriteLong(long n,BobStream *s)
{
    return BobStreamPutC((int)(n >> 24),s) != BobStreamEOF
    &&     BobStreamPutC((int)(n >> 16),s) != BobStreamEOF
    &&     BobStreamPutC((int)(n >>  8),s) != BobStreamEOF
    &&     BobStreamPutC((int)(n)      ,s) != BobStreamEOF;
}

/* ReadLong - read a long value from an image file */
static int ReadLong(long *pn,BobStream *s)
{
    int c;
    if ((c = BobStreamGetC(s)) == BobStreamEOF)
        return FALSE;
    *pn = (long)c << 24;
    if ((c = BobStreamGetC(s)) == BobStreamEOF)
        return FALSE;
    *pn |= (long)c << 16;
    if ((c = BobStreamGetC(s)) == BobStreamEOF)
        return FALSE;
    *pn |= (long)c << 8;
    if ((c = BobStreamGetC(s)) == BobStreamEOF)
        return FALSE;
    *pn |= (long)c;
    return TRUE;
}

/* Warning!!  Not byte order independent!! */
/* WriteFloat - write a float value to an image file */
static int WriteFloat(BobFloatType n,BobStream *s)
{
    int count = sizeof(BobFloatType);
    char *p = (char *)&n;
    while (--count >= 0)
        if (BobStreamPutC(*p++,s) == BobStreamEOF)
            return FALSE;
    return TRUE;
}

/* Warning!!  Not byte order independent!! */
/* ReadFloat - read a float value from an image file */
static int ReadFloat(BobFloatType *pn,BobStream *s)
{
    int count = sizeof(BobFloatType);
    char *p = (char *)pn;
    int c;
    while (--count >= 0) {
        if ((c = BobStreamGetC(s)) == BobStreamEOF)
            return FALSE;
        *p++ = c;
    }
    return TRUE;
}

/* WritePointer - write a pointer to an image file */
static int WritePointer(BobInterpreter *c,BobValue p,BobStream *s)
{
    return BobPointerP(p) ? WriteLong((long)((unsigned char *)p - c->newSpace->base),s)
                          : WriteLong((long)p,s);
}

/* ReadPointer - read a pointer from an image file */
static int ReadPointer(BobInterpreter *c,BobValue *pp,BobStream *s)
{
    long n;
    if (!ReadLong(&n,s))
        return FALSE;
    *pp = BobPointerP(n) ? (BobValue)(c->newSpace->base + n) : (BobValue)n;
    return TRUE;
}


