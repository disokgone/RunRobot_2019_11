struct blob_array {
        unsigned int    data_len, fofs; // 資料的長度, 檔案位置
        char    name[24];       // 用於識別資料的名稱, 可自行定義使用方式, 不拘於文字
};

extern int  add_new_blob(int blob_id, char *item_name, void *data, int len);
extern int  change_blob_size(int blob_id, unsigned int new_size);
extern void clear(void *p, int len);
extern void close_blob(int blob_id);    // 釋放記憶體並關閉 FILE *
extern int  flush_blob(int blob_id);    // 寫回檔頭並關閉檔案
extern void *get_blob_data(int blob_id, int item_ndx);  // 頭 4 bytes = data len, 用完請自行釋放該記憶體 !
extern void *get_blob_ptr(int blob_id, int item_ndx);
extern void *get_info_array(int blob_id);        // 傳回 struct blob_array * answer[], 用完請自行釋放該記憶體 !
extern int  open_blob_file(char *fn, int mode, int item_cnt); // mode: 1=唯讀, 2=讀寫, 傳回 1-31 = OK. (if > 31, 請減 32 = DOS Error code, 小於零是內部錯誤碼)
extern void *su_malloc(int len);

 