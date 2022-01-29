#ifdef BOB_INCLUDE_FLOAT_SUPPORT

#include <string.h>
#include <float.h>
#include "bob.h"

/* method handlers */
static BobValue BIF_rref(BobInterpreter *c);
static BobValue BIF_det(BobInterpreter *c);
static BobValue BIF_transpose(BobInterpreter *c);

/* virtual property methods */
static BobValue BIF_rows(BobInterpreter *c,BobValue obj);
static BobValue BIF_cols(BobInterpreter *c,BobValue obj);

/* Matrix methods */
static BobCMethod methods[] = {
    BobMethodEntry( "rref",         BIF_rref        ),
    BobMethodEntry( "det",          BIF_det         ),
    BobMethodEntry( "transpose",    BIF_transpose   ),
    BobMethodEntry( 0,              0               )
};

/* Matrix properties */
static BobVPMethod properties[] = {
    BobVPMethodEntry( "rows",           BIF_rows,       NULL        ),
    BobVPMethodEntry( "cols",           BIF_cols,       NULL        ),
    BobVPMethodEntry( 0,                0,      0       )
};

/* prototypes */
static BobFloatType Det(BobInterpreter *c,BobValue obj);

/* BobInitMatrix - initialize the 'Matrix' object */
void BobInitMatrix(BobInterpreter *c)
{
    c->floatObject = BobEnterType(BobGlobalScope(c),"Matrix",&BobMatrixDispatch);
    BobEnterMethods(c,c->floatObject,methods);
    BobEnterVPMethods(c,c->floatObject,properties);
}

/* BIF_transpose - built-in method 'transpose' */
static BobValue BIF_transpose(BobInterpreter *c)
{
    BobIntegerType nRows,nCols,i,j;
    BobValue obj,new;
    BobParseArguments(c,"V=*",&obj,&BobMatrixDispatch);
    nRows = BobMatrixRows(obj);
    nCols = BobMatrixCols(obj);
    BobCPush(c,obj);
    new = BobMakeMatrix(c,nCols,nRows);
    obj = BobPop(c);
    for (i = 0; i < nRows; ++i)
        for (j = 0; j < nCols; ++j)
            BobSetMatrixElement(new,j,i,BobMatrixElement(obj,i,j));
    return new;
}

#define ABS(x)      ((x) < 0 ? -(x) : (x))
#define NEAR_0(x)   (ABS(x) < 10.0 * DBL_EPSILON)

/* BIF_rref - built-in method 'rref' */
static BobValue BIF_rref(BobInterpreter *c)
{
    BobIntegerType nRows,nCols,limit,row,col,i,j;
    BobValue obj,r;
    int tmp = -1;

    /* parse the argument list */
    BobParseArguments(c,"V=*|i",&obj,&BobMatrixDispatch,&tmp);

    /* get the matrix dimensions */
    nRows = BobMatrixRows(obj);
    nCols = BobMatrixCols(obj);

    /* set the column limit */
    if ((limit = (BobIntegerType)tmp) < 0)
        limit = (nCols > nRows ? nRows : nCols);
    else if (limit > nCols)
        BobCallErrorHandler(c,BobErrIndexOutOfBounds,BobMakeInteger(c,limit));

    /* make the result matrix */
    BobCPush(c,obj);
    r = BobMakeMatrix(c,nRows,nCols);
    obj = BobPop(c);

    /* copy the input matrix to the result matrix */
    memcpy(BobMatrixAddress(r),BobMatrixAddress(obj),nRows * nCols * sizeof(BobFloatType));

    row = col = 0;
    while (row < nRows && col < limit) {
        BobFloatType pivotValue = ABS(BobMatrixElement(r,row,col));
        BobIntegerType pivotRow = row;

        /* find the row with the maximum value in this column */
        for (i = row + 1; i < nRows; ++i) {
            BobFloatType value = ABS(BobMatrixElement(r,i,col));
            if (value > pivotValue) {
                pivotValue = value;
                pivotRow = i;
            }
        }
        
        /* make sure it's greater than zero */
        if (!NEAR_0(pivotValue)) {

            /* get the pivot value */
            pivotValue = BobMatrixElement(r,pivotRow,col);

            /* swap the row with the pivot value with the current row */
            if (row != pivotRow) {
                for (j = 0; j < nCols; ++j) {
                    BobFloatType tmp = BobMatrixElement(r,row,j);
                    BobSetMatrixElement(r,row,j,BobMatrixElement(r,pivotRow,j));
                    BobSetMatrixElement(r,pivotRow,j,tmp);
                }
            }

            /* divide the current row by the pivot value */
            for (j = 0; j < nCols; ++j)
                BobSetMatrixElement(r,row,j,BobMatrixElement(r,row,j) / pivotValue);

            /* reduce current column values of the remaining rows to zero */
            for (i = 0; i < nRows; ++i) {
                if (i != row) {
                    BobFloatType scale = BobMatrixElement(r,i,col);
                    for (j = 0; j < nCols; ++j) {
                        BobFloatType pv = BobMatrixElement(r,row,j);
                        BobFloatType rv = BobMatrixElement(r,i,j);
                        BobSetMatrixElement(r,i,j,rv - scale * pv);
                    }
                }
            }

            /* move on to the next row */
            ++row;
        }

        /* move on to the next column */
        ++col;
    }

    /* return the result matrix */
    return r;
}

/* BIF_det - built-in method 'det' */
static BobValue BIF_det(BobInterpreter *c)
{
    BobValue obj;

    /* parse the argument list */
    BobParseArguments(c,"V=*",&obj,&BobMatrixDispatch);

    /* make sure the matrix is square */
    if (BobMatrixRows(obj) != BobMatrixCols(obj))
        BobCallErrorHandler(c,BobErrIncompatible);

    /* return the result matrix */
    return BobMakeFloat(c,Det(c,obj));
}

/* Det - compute the determinant of a submatrix */
static BobFloatType Det(BobInterpreter *c,BobValue obj)
{
    BobIntegerType size = BobMatrixRows(obj);
    BobFloatType value = 0.0;
    BobIntegerType i,j;
    BobValue tmp;

    /* check for the trivial cases */
    switch (size) {
    case 0:
        return 0.0;
    case 1:
        return BobMatrixElement(obj,0,0);
    case 2:
        return BobMatrixElement(obj,0,0) * BobMatrixElement(obj,1,1)
             - BobMatrixElement(obj,0,1) * BobMatrixElement(obj,1,0);
    }
    
    /* make a temporary matrix */
    BobCheck(c,2);
    BobPush(c,obj);
    tmp = BobMakeMatrix(c,size - 1,size - 1);
    obj = BobPop(c);

    /* compute the determinant using the first row */
    for (i = 0, j = 0; j < size; ++j) {
        BobFloatType element,minor;
        BobIntegerType ii,jj,k,l;

        /* get the entry */
        element = BobMatrixElement(obj,i,j);

        /* compute the cofactor if the entry is non-zero */
        if (!NEAR_0(element)) {
            
            /* build the submatrix */
            for (ii = k = 0; ii < size; ++ii)
                if (ii != i) {
                    for (jj = l = 0; jj < size; ++jj)
                        if (jj != j) {
                            BobFloatType e = BobMatrixElement(obj,ii,jj);
                            BobSetMatrixElement(tmp,k,l,e);
                            ++l;
                        }
                    ++k;
                }
                            
            /* compute the i,j minor */
            BobPush(c,obj);
            BobPush(c,tmp);
            minor = element * Det(c,tmp);
            tmp = BobPop(c);
            obj = BobPop(c);
    
            /* add the i,j cofactor */
            if ((i + j) & 1)
                value -= minor;
            else
                value += minor;
        }
    }

    /* return the determinant value */
    return value;
}

/* BIF_rows - built-in property 'rows' */
static BobValue BIF_rows(BobInterpreter *c,BobValue obj)
{
    return BobMakeInteger(c,BobMatrixRows(obj));
}

/* BIF_cols - built-in property 'cols' */
static BobValue BIF_cols(BobInterpreter *c,BobValue obj)
{
    return BobMakeInteger(c,BobMatrixCols(obj));
}

static int GetMatrixProperty(BobInterpreter *c,BobValue obj,BobValue tag,BobValue *pValue);
static int SetMatrixProperty(BobInterpreter *c,BobValue obj,BobValue tag,BobValue value);
static int MatrixPrint(BobInterpreter *c,BobValue obj,BobStream *s);
static long MatrixSize(BobValue obj);
static BobValue MatrixCopy(BobInterpreter *c,BobValue obj);

BobDispatch BobMatrixDispatch = {
    "Matrix",
    &BobMatrixDispatch,
    GetMatrixProperty,
    SetMatrixProperty,
    BobDefaultNewInstance,
    MatrixPrint,
    MatrixSize,
    MatrixCopy,
    BobDefaultScan,
    BobDefaultHash
};

/* GetMatrixProperty - Matrix get property handler */
static int GetMatrixProperty(BobInterpreter *c,BobValue obj,BobValue tag,BobValue *pValue)
{
    if (BobIntegerP(tag)) {
        BobIntegerType nCols = BobMatrixCols(obj);
        BobIntegerType i,row,col;
        if ((i = BobIntegerValue(tag)) < 0)
            BobCallErrorHandler(c,BobErrIndexOutOfBounds,tag);
        row = i / nCols; col = i % nCols;
        if (row >= BobMatrixRows(obj))
            BobCallErrorHandler(c,BobErrIndexOutOfBounds,tag);
        *pValue = BobMakeFloat(c,BobMatrixElement(obj,row,col));
        return TRUE;
    }
    return BobGetVirtualProperty(c,obj,c->floatObject,tag,pValue);
}

/* SetMatrixProperty - Matrix set property handler */
static int SetMatrixProperty(BobInterpreter *c,BobValue obj,BobValue tag,BobValue value)
{
    if (BobIntegerP(tag)) {
        BobIntegerType nCols = BobMatrixCols(obj);
        BobIntegerType i,row,col;
        if (!BobIntegerP(value))
            BobTypeError(c,value);
        if ((i = BobIntegerValue(tag)) < 0)
            BobCallErrorHandler(c,BobErrIndexOutOfBounds,tag);
        row = i / nCols; col = i % nCols;
        if (row >= BobMatrixRows(obj))
            BobCallErrorHandler(c,BobErrIndexOutOfBounds,tag);
        BobSetMatrixElement(obj,row,col,BobFloatValue(value));
        return TRUE;
    }
    return BobSetVirtualProperty(c,obj,c->floatObject,tag,value);
}

void FormatAndMeasureFloat(char *buf,int *pBefore,int *pAfter,BobFloatType value)
{
    char *dot;
    BobFloatToString(buf,value);
    if ((dot = strchr(buf,'.')) != NULL) {
        *pBefore = dot - buf;
        *pAfter = &buf[strlen(buf)] - dot;
    }
    else {
        *pBefore = strlen(buf);
        *pAfter = 0;
    }
}

/* MatrixPrint - Matrix print handler */
static int MatrixPrint(BobInterpreter *c,BobValue obj,BobStream *s)
{
    int before,beforeMax,after,afterMax,cnt;
    BobIntegerType nRows,nCols,i,j;
    char buf[64];

    nRows = BobMatrixRows(obj);
    nCols = BobMatrixCols(obj);

    beforeMax = afterMax = 0;
    for (i = 0; i < nRows; ++i) {
        for (j = 0; j < nCols; ++j) {
            FormatAndMeasureFloat(buf,&before,&after,BobMatrixElement(obj,i,j));
            if (before > beforeMax)
                beforeMax = before;
            if (after > afterMax)
                afterMax = after;
        }
    }

    if (BobStreamPutS("[{\n",s) == BobStreamEOF)
        return BobStreamEOF;

    BobCheck(c,1);
    for (i = 0; i < nRows; ) {
        for (j = 0; j < nCols; ) {
            BobPush(c,obj);
            FormatAndMeasureFloat(buf,&before,&after,BobMatrixElement(obj,i,j));
            if ((cnt = beforeMax - before) > 0)
                while (--cnt >= 0)
                    BobStreamPutC(' ',s);
            BobStreamPutS(buf,s);
            if ((cnt = afterMax - after) > 0)
                while (--cnt >= 0)
                    BobStreamPutC(' ',s);
            if (++j < nCols)
                BobStreamPutS("  ",s);
            obj = BobPop(c);
        }
        if (++i < nRows) {
            BobPush(c,obj);
            BobStreamPutS(" ;\n",s);
            obj = BobPop(c);
        }
    }
    
    if (BobStreamPutS("\n}]",s) == BobStreamEOF)
        return BobStreamEOF;

    return TRUE;
}

/* MatrixSize - Matrix size handler */
static long MatrixSize(BobValue obj)
{
    return sizeof(BobMatrix);
}

/* MatrixCopy - Matrix copy handler */
static BobValue MatrixCopy(BobInterpreter *c,BobValue obj)
{
    BobValue newObj = BobDefaultCopy(c,obj);
    if (obj != newObj) {
        BobMatrix *m = (BobMatrix *)newObj;
        int dataSize = m->nRows * m->nCols * sizeof(BobFloatType);
        BobFloatType *p = (BobFloatType *)((char *)m + dataSize);
        BobIntegerType i;
        for (i = 0; i < m->nRows; ++i) {
            m->rows[i] = p;
            p += m->nCols;
        }    
    }
    return newObj;
}

/* BobMakeMatrix - make a new matrix value */
BobValue BobMakeMatrix(BobInterpreter *c,BobIntegerType nRows,BobIntegerType nCols)
{
    int headerSize = sizeof(BobMatrix) + (nRows - 1) * sizeof(BobFloatType *);
    int dataSize = nRows * nCols * sizeof(BobFloatType);
    BobValue new = BobAllocate(c,headerSize + dataSize);
    BobMatrix *m = (BobMatrix *)new;
    BobFloatType *p;
    int i;
    
    /* initialize the matrix */
    BobSetDispatch(new,&BobMatrixDispatch);
    m->nRows = nRows;
    m->nCols = nCols;

    /* get a pointer to the row data */
    p = (BobFloatType *)((char *)m + headerSize);

    /* initialize the row data */
    memset(p,0,dataSize);

    /* initialize the row pointers */
    for (i = 0; i < nRows; ++i) {
        m->rows[i] = p;
        p += nCols;
    }

    /* return the new matrix */
    return new;
}

static BobValue MatrixAdd(BobInterpreter *c,BobValue p1,BobValue p2);
static BobValue MatrixAddScalar(BobInterpreter *c,BobValue p1,BobFloatType fval);
static BobValue MatrixSubtract(BobInterpreter *c,BobValue p1,BobValue p2);
static BobValue MatrixSubtractFromScalar(BobInterpreter *c,BobValue p1,BobFloatType fval);
static BobValue MatrixMultiplyByScalar(BobInterpreter *c,BobValue p1,BobFloatType fval);
static BobValue MatrixMultiply(BobInterpreter *c,BobValue p1,BobValue p2);

BobValue BobMatrixUnaryOp(BobInterpreter *c,int op,BobValue p1)
{
    switch (op) {
    case '-':
        return MatrixSubtractFromScalar(c,p1,0.0);
    }
    BobTypeError(c,p1);
    return c->nilValue; /* never reached */
}

BobValue BobMatrixBinaryOp(BobInterpreter *c,int op,BobValue p1,BobValue p2)
{
    switch (op) {
    case '+':
        if (BobMatrixP(p1)) {
            if (BobMatrixP(p2))
                return MatrixAdd(c,p1,p2);
            else if (BobFloatP(p2))
                return MatrixAddScalar(c,p1,BobFloatValue(p2));
            else if (BobIntegerP(p2))
                return MatrixAddScalar(c,p1,(BobFloatType)BobIntegerValue(p2));
        }
        else if (BobFloatP(p1))
            return MatrixAddScalar(c,p2,BobFloatValue(p1));
        else if (BobIntegerP(p1))
            return MatrixAddScalar(c,p2,(BobFloatType)BobIntegerValue(p1));
        break;
    case '-':
        if (BobMatrixP(p1)) {
            if (BobMatrixP(p2))
                return MatrixSubtract(c,p1,p2);
            else if (BobFloatP(p2))
                return MatrixAddScalar(c,p1,-BobFloatValue(p2));
            else if (BobIntegerP(p2))
                return MatrixAddScalar(c,p1,-(BobFloatType)BobIntegerValue(p2));
        }
        else if (BobFloatP(p1))
            return MatrixSubtractFromScalar(c,p2,BobFloatValue(p1));
        else if (BobIntegerP(p1))
            return MatrixSubtractFromScalar(c,p2,(BobFloatType)BobIntegerValue(p1));
        break;
    case '*':
        if (BobMatrixP(p1)) {
            if (BobMatrixP(p2))
                return MatrixMultiply(c,p1,p2);
            else if (BobFloatP(p2))
                return MatrixMultiplyByScalar(c,p1,BobFloatValue(p2));
            else if (BobIntegerP(p2))
                return MatrixMultiplyByScalar(c,p1,(BobFloatType)BobIntegerValue(p2));
        }
        else if (BobFloatP(p1))
            return MatrixMultiplyByScalar(c,p2,BobFloatValue(p1));
        else if (BobIntegerP(p1))
            return MatrixMultiplyByScalar(c,p2,(BobFloatType)BobIntegerValue(p1));
        break;
    }
    BobTypeError(c,BobMatrixP(p1) ? p1 : p2);
    return c->nilValue; /* never reached */
}

static BobValue MatrixAddScalar(BobInterpreter *c,BobValue p1,BobFloatType fval)
{
    BobMatrix *m = (BobMatrix *)p1;
    BobMatrix *r;
    int i,j;

    r = (BobMatrix *)BobMakeMatrix(c,m->nRows,m->nCols);

    for (i = 0; i < r->nRows; ++i)
        for (j = 0; j < r->nCols; ++j)
            BobSetMatrixElement(r,i,j,fval + BobMatrixElement(m,i,j));

    /* return successfully */
    return (BobValue)r;
}

static BobValue MatrixAdd(BobInterpreter *c,BobValue p1,BobValue p2)
{
    BobMatrix *a = (BobMatrix *)p1;
    BobMatrix *b = (BobMatrix *)p2;
    BobMatrix *r;
    int i,j;

    /* make sure the matrices are compatible */
    if (a->nRows != b->nRows || a->nCols != b->nCols)
        BobCallErrorHandler(c,BobErrIncompatible);

    r = (BobMatrix *)BobMakeMatrix(c,a->nRows,a->nCols);

    for (i = 0; i < r->nRows; ++i)
        for (j = 0; j < r->nCols; ++j)
            BobSetMatrixElement(r,i,j,BobMatrixElement(a,i,j) + BobMatrixElement(b,i,j));

    /* return successfully */
    return (BobValue)r;
}

static BobValue MatrixSubtractFromScalar(BobInterpreter *c,BobValue p1,BobFloatType fval)
{
    BobMatrix *m = (BobMatrix *)p1;
    BobMatrix *r;
    int i,j;

    r = (BobMatrix *)BobMakeMatrix(c,m->nRows,m->nCols);

    for (i = 0; i < r->nRows; ++i)
        for (j = 0; j < r->nCols; ++j)
            BobSetMatrixElement(r,i,j,fval - BobMatrixElement(m,i,j));

    /* return successfully */
    return (BobValue)r;
}

static BobValue MatrixSubtract(BobInterpreter *c,BobValue p1,BobValue p2)
{
    BobMatrix *a = (BobMatrix *)p1;
    BobMatrix *b = (BobMatrix *)p2;
    BobMatrix *r;
    int i,j;

    /* make sure the matrices are compatible */
    if (a->nRows != b->nRows || a->nCols != b->nCols)
        BobCallErrorHandler(c,BobErrIncompatible);

    r = (BobMatrix *)BobMakeMatrix(c,a->nRows,a->nCols);

    for (i = 0; i < r->nRows; ++i)
        for (j = 0; j < r->nCols; ++j)
            BobSetMatrixElement(r,i,j,BobMatrixElement(a,i,j) - BobMatrixElement(b,i,j));

    /* return successfully */
    return (BobValue)r;
}

static BobValue MatrixMultiplyByScalar(BobInterpreter *c,BobValue p1,BobFloatType fval)
{
    BobMatrix *m = (BobMatrix *)p1;
    BobMatrix *r;
    int i,j;

    r = (BobMatrix *)BobMakeMatrix(c,m->nRows,m->nCols);

    for (i = 0; i < r->nRows; ++i)
        for (j = 0; j < r->nCols; ++j)
            BobSetMatrixElement(r,i,j,fval * BobMatrixElement(m,i,j));

    /* return successfully */
    return (BobValue)r;
}

static BobValue MatrixMultiply(BobInterpreter *c,BobValue p1,BobValue p2)
{
    BobMatrix *a = (BobMatrix *)p1;
    BobMatrix *b = (BobMatrix *)p2;
    BobMatrix *r;
    int i,j,k;
    
    /* make sure the matrices are compatible */
    if (a->nCols != b->nRows)
        BobCallErrorHandler(c,BobErrIncompatible);

    r = (BobMatrix *)BobMakeMatrix(c,a->nRows,b->nCols);
    
    for (i = 0; i < r->nRows; ++i)
        for (j = 0; j < r->nCols; ++j) {
            double sum = 0.0;
            for (k = 0; k < a->nCols; ++k)
                sum += BobMatrixElement(a,i,k) * BobMatrixElement(b,k,j);
            BobSetMatrixElement(r,i,j,sum);
        }

    /* return successfully */
    return (BobValue)r;
}

#endif

