//#include "sufloat.h"
#include "stdio.h"

typedef struct Int_64 {
        int     i64[2];
        } INT_64;
typedef int SuFloat;            // 32-bit single / float
typedef INT_64 SuDoubl;         // 64-bit double
typedef INT_64 SuInt64;         // 64-bit Int64

extern void cSetDW(void *p, int ofs, int val);
extern int  cReadDW(void *p, int ofs);
extern void sPrt(char *buf, char *fmt, ...);
extern char ssTemp[32];

void a1int_double(int v_low32, int v_high32, SuDoubl *dd)
{
	cSetDW(dd, 0, v_low32);		cSetDW(dd, 4, v_high32);
}

void a1int_single(int v_low32, SuFloat *sn)
{
	cSetDW(sn, 0, v_low32);
}

void a1double_int(SuDoubl *dd, int *v_low32)
{
	cSetDW(v_low32, 0, cReadDW(dd, 0));
}

void a1double_int64(SuDoubl *dd, SuInt64 *v_64)
{
	cSetDW(v_64, 0, cReadDW(dd, 0));
	cSetDW(v_64, 4, cReadDW(dd, 4));
}

void a1double_single(SuDoubl *dd, SuFloat *sn)
{
	cSetDW(sn, 0, cReadDW(dd, 0));
}

void a1single_int(SuFloat *sn, int *v_low32)
{
	cSetDW(v_low32, 0, cReadDW(sn, 0));
}

void a2int_double(int v_low32, int v_high32, SuDoubl *dd)
{
	cSetDW(dd, 0, v_low32);		cSetDW(dd, 4, v_high32);
}

void a2int_int64(int v_low32, int v_high32, SuInt64 *i64)
{
	cSetDW(i64, 0, v_low32);		cSetDW(i64, 4, v_high32);
}

char *sDoubleToStr(SuDoubl *v)
{
  sPrt(ssTemp, "double from $%x:$%x", cReadDW(v, 4), cReadDW(v, 0));
  return(ssTemp);
}

char *sSingleToStr(SuFloat *v)
{
//  sPrt(ssTemp, "%7.7f", v);
  sPrt(ssTemp, "single from $%x", cReadDW(v, 4));
  return(ssTemp);
}