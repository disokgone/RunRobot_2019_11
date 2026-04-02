#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//#include <windows.h>

extern void _fastcall xfclose(int handle);
extern int _fastcall xfcreate(char *fname);
extern int _fastcall xfopen(char *fname, int mode);
extern int _fastcall xfread(void *buf, int len, int count, int handle);
extern void _fastcall xfree(void *p);
extern int _fastcall xfwrite(void *buf, int len, int count, int handle);
extern long _fastcall xfseek(int handle, int ofs, int orig);
extern void * _fastcall xmalloc(int size);
extern void _fastcall xmemmove(void *pDst, void *pSrc, int len);
extern void _fastcall xmemset(void *pDst, int chr, int len);
// extern void _fastcall xSay(char *s);
#define MAX_BLOB_COUNT          31
#define MAX_BLOB_ITEMS          8192
#define NO_ERR                  0
#define ERR_BAD_MODE            -1
#define ERR_BAD_BOLB_ID         -2
#define ERR_CANT_OPEN_FILE      -3      // 無法開啟檔案
#define ERR_NO_FREE_BLOB        -4      // blob 表格已經滿了
#define ERR_MUST_RW_MODE        -5      // 不能使用唯讀 ! 必須使用讀寫模式, 否則無法存檔 !
#define ERR_BAD_BOLB_FHWND      -6      // 檔案代碼無效
#define ERR_BOLB_FULL           -7      // 你的 blob 已經滿了, 超過你指定的可用項目數 !
#define ERR_ITEM_HAS_DATA       -8      // 已經有資料 !!
#define ERR_NO_DATA_IN_BOLB     -9      // 空指標 !!
#define ERR_BAD_IO_MODE         -10     // 唯讀模式無法寫出資料 !!

// extern void __fastcall cSay(char *s);
// void xSay(char *fmt, ...);

struct blob_array {
        unsigned int    data_len, fofs; // 資料的長度, 檔案位置
        char    name[24];       // 用於識別資料的名稱, 可自行定義使用方式, 不拘於文字
};

struct blob_info {
        unsigned int flag;      // 1=已從檔案讀到記憶體, 2=尚未存回檔案, 4=資料有變動
        void    *mem_ptr;       // 在存檔前, 全部的資料都存在這裡 !
};

struct small_blob {
        int     f_io;
        int     nItems, ftail, freeItemNdx, mode;
        struct  blob_array *blob_items;
        struct  blob_info *blob_infs;
};

// --------------------------
int  add_new_blob(int blob_id, char *item_name, void *data, int len);
int  change_blob_size(int blob_id, int item_ndx, unsigned int new_size);
void clear(void *p, int len);   // = ZeroMemory
void _fastcall close_blob(int blob_id);   	// 釋放記憶體並關閉 FILE *
void _stdcall copy_temp_str(void *your_ptr);			// 一次拷貝整個 temp_str[128]
int  flush_blob(int blob_id);   // 寫回檔頭 (未關閉檔案)
void * _fastcall get_blob_data(int blob_id, int item_ndx);         // 頭 4 bytes = data len, 用完請自行釋放該記憶體 !
void *get_blob_ptr(int blob_id, int item_ndx);
void * _fastcall get_info_array(int blob_id);      // 傳回 struct blob_array * answer[], 用完請自行釋放該記憶體 !
int _fastcall open_blob_file(char *fn, int mode, int item_cnt);  // mode: 1=唯讀, 2=讀寫, item_cnt = 預留項數空間
int  read_blob_header(int fio, int open_mode, int req_count);  // 讀取基本檔頭
void *su_malloc(int len);

struct small_blob *smb; // small_blob 陣列[MAX_BLOB_COUNT]
int     free_blob = 1;  // 0 號勿用, 從 1 開始較安全
char *SU_BLOB_MAGIC_STR = "SuBlob";        // 7th char = \0. 8th char = version num.
// char *temp_str[128];		// also in runner.c

// byte 8-11=item count, byte 12-15=file size in total.

// --------------------------
int  add_new_blob(int blob_id, char *item_name, void *data, int len)
{ // set data[len] to blob_id as a new blob, 傳回此 new blob index (之後可修改資料)
// item_name[24] 整個 24 bytes 會被拷貝 !
struct small_blob *now_smb;
struct blob_array *b_item;
struct blob_info *b_inf;

        if (blob_id >= free_blob) return(ERR_BAD_BOLB_ID);
        now_smb = smb + blob_id - 1;    // 指到此 blob_id
        if (now_smb->freeItemNdx >= now_smb->nItems) return(ERR_BOLB_FULL);
        b_inf = now_smb->blob_infs + now_smb->freeItemNdx;
        if (b_inf->mem_ptr) return(ERR_ITEM_HAS_DATA); // 已經有資料 !!
        b_item = now_smb->blob_items + now_smb->freeItemNdx;
        xmemmove(b_item->name, item_name, 24);
        b_item->data_len = len;         b_item->fofs = 0;
        if (len) {
                b_inf->mem_ptr = su_malloc(len);        // 只借剛好你指定的空間大小
                xmemmove(b_inf->mem_ptr, data, len);    // 拷貝你指定的資料 !
        }
        b_inf->flag |= 2;       // 尚未存檔 !
        now_smb->freeItemNdx ++;
        return(now_smb->freeItemNdx - 1); //  b_inf->mem_ptr 你可以自由更新此記憶體內容
}
// --------------------------
int  change_blob_size(int blob_id, int item_ndx, unsigned int new_size)
{
struct small_blob *now_smb;
struct blob_array *b_item;
struct blob_info *b_inf;
void *new_buf;
        if (blob_id >= free_blob) return(ERR_BAD_BOLB_ID);
        now_smb = smb + blob_id - 1;    // 指到此 blob_id
        if (item_ndx >= now_smb->freeItemNdx) return(ERR_NO_DATA_IN_BOLB);
        b_item = now_smb->blob_items + item_ndx;
        b_inf = now_smb->blob_infs + item_ndx;
        if (b_inf->mem_ptr) { // 有資料..
                if (new_size > b_item->data_len) {
                        new_buf = su_malloc(new_size);
                        xmemmove(new_buf, b_inf->mem_ptr, b_item->data_len);
                        xfree(b_inf->mem_ptr);
                        b_inf->mem_ptr = new_buf;
                }
        }
        else { // 原本沒有資料..
                if (new_size) b_inf->mem_ptr = su_malloc(new_size);
        }
        b_item->data_len = new_size;
        return(NO_ERR);
}
// --------------------------
void clear(void *p, int len) {
	// ZeroMemory(p, len);		// for Windows
	xmemset(p, 0, len);		// Other systems.
}
// --------------------------
void _fastcall close_blob(int blob_id)
{
int    fio;
struct small_blob *now_smb;
struct blob_info *b_inf;
int     i, n;
        if (blob_id >= free_blob) return;       // ERR_BAD_BOLB_ID
        now_smb = smb + blob_id - 1;    // 指到此 blob_id
        if (now_smb->f_io == NULL) return;      // ERR_BAD_BOLB_FHWND
        xfseek(now_smb->f_io, 0, 2);  	xfclose(now_smb->f_io);
        b_inf = now_smb->blob_infs;
        for (i = 0;i < now_smb->nItems;i ++) {
                if (b_inf->mem_ptr) {  xfree(b_inf->mem_ptr);  b_inf->mem_ptr = NULL; }
                b_inf ++;
        }
        if (now_smb->blob_items) xfree(now_smb->blob_items);
        if (now_smb->blob_infs) xfree(now_smb->blob_infs);
        clear(now_smb, sizeof(struct small_blob));
}
// --------------------------
void _stdcall copy_temp_str(void *your_ptr)
{ // 一次拷貝整個 temp_str[128]
	xmemmove(your_ptr, temp_str, 128 << 2);		// !!@@ 32/64 bit @@!!
}
// --------------------------
int flush_blob(int blob_id)
{ // 寫回檔頭
int    fio;
struct small_blob *now_smb;
struct blob_array *b_item;
struct blob_info *b_inf;
int     i, n;

        if (blob_id >= free_blob) return(ERR_BAD_BOLB_ID);
        now_smb = smb + blob_id - 1;    // 指到此 blob_id
        if (now_smb->f_io == NULL) return(ERR_BAD_BOLB_FHWND);
        if (now_smb->mode == 1) return(ERR_BAD_IO_MODE);
        fio = now_smb->f_io;
        // call packing() to clear garbage !
        xfseek(fio, 0, 0);   	xfwrite(SU_BLOB_MAGIC_STR, 6, 1, fio);
        i = 0x0100;             xfwrite(&i, 2, 1, fio);	// write \0 & version = 1
        // 此時 now_smb->ftail 不需正確 !!
        xfwrite(&now_smb->nItems, 8, 1, fio);    // 寫出資料項數與檔案總長度 (此時 檔案總長度 不需正確, 稍後補正)
        xfwrite(now_smb->blob_items, sizeof(struct blob_array) * now_smb->nItems, 1, fio);
        n = (sizeof(struct blob_array) * now_smb->nItems) + 16; // 16 = 短檔頭長度
        xfseek(fio, n, 0);      now_smb->ftail = n;
        // 逐一掃瞄 blob 內涵並存入檔案 !
        b_item = now_smb->blob_items;
        b_inf = now_smb->blob_infs;
        for (i = 0;i < now_smb->freeItemNdx;i ++) {
                if (b_item->data_len < 1) continue;     // 無資料   
                b_item->fofs = n;       n += b_item->data_len;
                xfwrite(b_inf->mem_ptr, b_item->data_len, 1, fio); // 寫出資料
                b_inf->flag &= 0xF9;    // 已經存檔, (clear 2, 4)
                b_item ++;              b_inf ++;
        }
        now_smb->ftail = n;
        xfseek(fio, 8, 0);              xfwrite(&now_smb->nItems, 4, 1, fio);
        xfwrite(&now_smb->ftail, 4, 1, fio);
        xfwrite(now_smb->blob_items, sizeof(struct blob_array) * now_smb->nItems, 1, fio);
        xfseek(fio, now_smb->ftail, 0);
        return(NO_ERR);
}
// --------------------------
void * _fastcall get_info_array(int blob_id)
{ // 傳回 struct blob_array * answer[], 用完請自行釋放該記憶體 !
struct small_blob *now_smb;
char    *p;
int     n;
        if (blob_id >= free_blob) return((void *) ERR_BAD_BOLB_ID);
        now_smb = smb + blob_id - 1;    // 指到此 blob_id
        n = now_smb->nItems * sizeof(struct blob_array);
        p = (char *) su_malloc(n + 8);
        *((int *) p) = now_smb->nItems;         // 1st int = 項數
        *((int *) (p + 4)) = now_smb->freeItemNdx;      // 2nd int = free index (若此值大於項數,代表已無閒置空間)
        xmemmove(p + 8, now_smb->blob_items, n);
        return((void *) p);
}
// --------------------------
void * _fastcall get_blob_data(int blob_id, int item_ndx)
{ // 頭 4 bytes = data len, 用完請自行釋放該記憶體 !
int    fio;
struct small_blob *now_smb;
struct blob_array *pBL;
int     *p;
int     n;
        if (blob_id >= free_blob) return((void *) ERR_BAD_BOLB_ID);
        now_smb = smb + blob_id - 1;    // 指到此 blob_id
        if (now_smb->f_io == NULL) return((void *) ERR_BAD_BOLB_FHWND);
        fio = now_smb->f_io;
        pBL = now_smb->blob_items + item_ndx;   // index 從 0 開始
        if (pBL->data_len < 1) return(NULL);    // no data !!
        xfseek(fio, pBL->fofs, 0);
        p = (int *) su_malloc(pBL->data_len + 4);
        *p = pBL->data_len;
        xfread(p + 1, pBL->data_len, 1, fio);
        return(p);
}
// --------------------------
void *get_blob_ptr(int blob_id, int item_ndx)
{
struct small_blob *now_smb;
struct blob_info *b_inf;
        if (blob_id >= free_blob) return((void *) ERR_BAD_BOLB_ID);
        now_smb = smb + blob_id - 1;    // 指到此 blob_id
        if (item_ndx >= now_smb->freeItemNdx) return((void *) ERR_NO_DATA_IN_BOLB);
        b_inf = now_smb->blob_infs + item_ndx;
        if (! b_inf->mem_ptr) return((void *) ERR_NO_DATA_IN_BOLB); // 空指標 !!
        return(b_inf->mem_ptr);
}
// --------------------------
int _fastcall open_blob_file(char *fn, int mode, int item_cnt)
{ // mode: 1=唯讀, 2=讀寫, 傳回 1-31 = OK. (if > 31, 請減 32 = DOS Error code, 小於零是內部錯誤碼)
int     fio;

        if (free_blob > (MAX_BLOB_COUNT - 1)) return(ERR_NO_FREE_BLOB);   // blob 表格已經滿了
        if (! smb) smb = (struct small_blob *) su_malloc(sizeof(struct small_blob) * MAX_BLOB_COUNT);
        switch(mode) {
                case 1: // mode: 1=唯讀   
                        fio = xfopen(fn, 0x40);		// 開檔模式 (0=R, 1=W, 2=RW, 0x40=share)
                        if (fio) return(read_blob_header(fio, mode, item_cnt));
                        else return(ERR_CANT_OPEN_FILE);        // 無法開啟檔案
                case 2: // mode: 2=讀寫
						fio = xfcreate(fn);			xfclose(fio);	// 先砍除舊檔
                        fio = xfopen(fn, 0x42);
                        if (fio) return(read_blob_header(fio, mode, item_cnt));
                        else return(ERR_CANT_OPEN_FILE);        // 無法開啟檔案
        }
        return(ERR_BAD_MODE);   // bad mode !
}
// --------------------------
int read_blob_header(int fio, int open_mode, int req_count)
{ // 讀取基本檔頭 (req_count = 預留項數空間)
struct small_blob *now_smb;
unsigned int flen;
int     blob_id;

        blob_id = free_blob - 1;        // 從 1 開始較安全
        flen = xfseek(fio, 0, 2);
        if (flen < 1) { // 起始一個空白的 blob file !
                if (open_mode == 1) return(ERR_MUST_RW_MODE); // 不能使用唯讀 ! 必須使用讀寫模式, 否則無法存檔 !
                now_smb = smb + blob_id;
                now_smb->freeItemNdx = 0;
                now_smb->ftail = 12 + (sizeof(struct blob_array) * req_count);
                now_smb->blob_items = (struct blob_array *) su_malloc(sizeof(struct blob_array) * req_count);
                now_smb->blob_infs = (struct  blob_info *) su_malloc(sizeof(struct  blob_info) * req_count);
                xfseek(fio, 0, 0);      xfwrite(SU_BLOB_MAGIC_STR, 16, 1, fio);
                xfwrite(now_smb->blob_items, sizeof(struct blob_array) * req_count, 1, fio);
        }
        else {
                now_smb = smb + blob_id;
                now_smb->ftail = flen;      	// 真正的檔案尾端
                xfseek(fio, 8, 0);       		// ofs 8 = 項數
                xfread(&now_smb->nItems, 4, 1, fio);
                if (req_count < now_smb->nItems) req_count = now_smb->nItems;
                now_smb->blob_items = (struct blob_array *) su_malloc(sizeof(struct blob_array) * req_count);
                now_smb->blob_infs = (struct  blob_info *) su_malloc(sizeof(struct  blob_info) * req_count);
                xfseek(fio, 16, 0);
                xfread(now_smb->blob_items, sizeof(struct blob_array), req_count, fio);
                now_smb->freeItemNdx = req_count + 1;   // 暫時不允許增加項目
        }
        now_smb->f_io = fio;           now_smb->nItems = req_count;
        free_blob ++;                  now_smb->mode = open_mode;
//  	flen = ftell(fio);
// 		xSay("flen = %d", flen);
        return(blob_id + 1);
}
// --------------------------
void *su_malloc(int len)
{
void *p;

        p = xmalloc(len);       clear(p, len);          return(p);
}
// --------------------------
/* void xSay(char *fmt, ...)
{  // enable this func in Delphi !
char	buf[256];
va_list	ap;

        if (ap == NULL) { cSay(fmt);  return; }
        va_start(ap, fmt);
        vsprintf(buf, fmt, (char *) ap);
        va_end(ap);	cSay(buf);
} */
