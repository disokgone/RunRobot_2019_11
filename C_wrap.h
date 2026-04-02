extern void _fastcall add_watch(int hash, int addr, char *name);	// hash = high_16_data_len + mid_8_ID + low_8_type_flag
extern void _fastcall del_watch(int id_type);	// id_type = mid_8_ID + low_8_type_flag
extern void _fastcall dbp(int n1, int n2, char *sInfo);		// 顯示可被中斷除錯的訊息 (可在 Delphi 側, 在此設立中斷點, 方便跳入 C 程式除錯)
extern void _fastcall xChangeFileExt(char *fn, char *ext);
extern void _fastcall xfclose(int handle);
extern int  _fastcall xfcreate(char *fname);
extern int  _fastcall xFileExists(char *fname);
extern char _fastcall xDeleteFile(char *fname);
extern int  _fastcall xfopen(char *fname, int mode);
extern int  _fastcall xfread(void *buf, int len, int count, int handle);
extern int  _fastcall xGetTodayDate(void);	// 取得今天日期 (yy: 23 bits, mm: 4 bits, dd: 5 bits)
extern void _fastcall xGetRandomFileName(char *fn);	// 使用今天日期與時間產生亂數檔名
extern void _fastcall xfree(void *p);
extern int  _fastcall xfwrite(void *buf, int len, int count, int handle);
extern long _fastcall xfseek(int handle, int ofs, int orig);
extern void _fastcall xHexDump(void *p, int len);			// iDebug.pas
extern void * _fastcall xmalloc(int size);
extern void _fastcall xmemmove(void *pDst, void *pSrc, int len);
extern void _fastcall xmemset(void *pDst, int chr, int len);
extern char _fastcall xRenameFile(char *oldName, char *newName);
extern void _fastcall cSay(char *pStr);
extern void _fastcall cSayN(int n, char *pStr);
extern void _fastcall xSayStrHex(char *pStr, int val, int len);		// iDebug.pas
extern void * _fastcall xstrcat(char *pDst, char *pSrc);
extern int  _fastcall xstrcmp(char *str1, char *str2);
extern void * _fastcall xstrcopy(char *pDst, char *pSrc);
extern void * _fastcall xstricomp(void *pstr1, void *pstr2);
extern void * _fastcall xstrncpy(void *pDst, void *pSrc, int max_len);
extern void * _fastcall xstrupr(char *str);
extern void * _fastcall xxErr(char *str);
extern int  _fastcall xstrlen(char *str);
extern int  _fastcall xvsprintf(char *buf, char * fmt, va_list ap);
extern int  _fastcall ReadDW(void *p, int ofs);
extern void _fastcall SetDW(void *p, int ofs, int val);
