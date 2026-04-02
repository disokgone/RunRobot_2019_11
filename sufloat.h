typedef struct Int_64 {
        int     i64[2];
        } INT_64;
typedef int SuFloat;            // 32-bit single / float
typedef INT_64 SuDoubl;         // 64-bit double
typedef INT_64 SuInt64;         // 64-bit Int64

extern void a1int_double(int v_low32, int v_high32, SuDoubl *dd);
extern void a1int_single(int v_low32, SuFloat *sn);
extern void a1double_int(SuDoubl *dd, int *v_low32);
extern void a1double_int64(SuDoubl *dd, SuInt64 *v_64);
extern void a1double_single(SuDoubl *dd, SuFloat *sn);
extern void a1single_int(SuFloat *sn, int *v_low32);
extern void a2int_double(int v_low32, int v_high32, SuDoubl *dd);
extern void a2int_int64(int v_low32, int v_high32, SuInt64 *i64);
extern char *sDoubleToStr(SuDoubl *v);
extern char *sSingleToStr(SuFloat *v);
