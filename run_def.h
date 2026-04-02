#define INT64 long long		// 64-bit
#define ADDR unsigned int	// 32-bit
// #define ADDR unsigned long long	// 64-bit

struct TArgs {
    char *name;
    unsigned int flags, line_no, vType, loc_match;
};

struct TArrayStru {	// 陣列 (變數或常數) 架構 (佔 5 個 32-bits)
	unsigned int LwrBound, UprBound;	// 下標/上標的界限值
	unsigned short Ndx;
	unsigned short vType;		// 變數型態代碼 (TypeCode[], 例如 array = $80)
	unsigned short cType;		// 常數型態代碼 (CT_XXXX, 例如 array = $80)
	unsigned char bLwrSolved, bUprSolved;		// 下標/上標 是否還未求出值 ? (T: unsolved)
    void *pData;		// 指到真正擺放資料的地方
};

struct BRK_PTS { // 中斷點
	unsigned int		program_id;
	unsigned int		line_number;
	unsigned int		pass_count, unused;
};

struct TCaller {
	void *pFunc;			// caller 來自於哪個函式 ? (PFuncStru)
	unsigned int LineNo;	// 要 call 之前的行號是多少 ?
	unsigned int LocPos;	// 要 call 之前的行號是多少 ?
	void *pExec;	// call 完之後, 要跳回去執行的第一個指令位置..
	void *pFin;		// 要跳回去執行的程式段的最後一個指令位置..
	unsigned int iResult[3];			// 放傳回值
	unsigned short Result_Type, pad;	// 傳回值的型態
	unsigned int cur_rolling_no;            // 目前執行函數的滾動值
};

struct TConstStru {	// 常數架構 (佔 3 個 32-bits)
	char *pName;	// 字串指標 (若此常數是字串, 若字串長度不到 2 字元, 則直接放在這裡)
    void *PMyConst; // --> struct TMyConst * 
	int i_Val;		// integer 或 single, 反正 32-bit 能放得下的就放這邊 (陣列則放其 array index)
	unsigned short cType;	// 常數型態代碼 (CT_XXXX, 例如 array = $80)
	unsigned char prgID;	// 來自哪個程式
	unsigned char flags;	// 旗號: (b7: 1=內值設定完成),
};

struct TFuncArgList { // only prg_stru.pas used
    unsigned int func_token;	// 函數或程序
    unsigned short nArgs, pad;	
    void *pArgs[32];			// 上限最多 32 個參數指標 (各自指到一串整數 Tokens)
};

struct TFuncProc {	// !! 缺乏來源程式編號, 萬一遇到同名函數, 將會錯亂 !!
    char *name;		// 函數或程序名稱
    int lineNo;		// lineNo = Prg_ndx: Hi-Byte --> 供 pss[] 使用之程式架構索引, low-3-byte: 行號 !
    short ret_Type, nArgs;		// 簡短表示其傳回值代碼跟需要幾個參數, -1: 尚未分析 !
    struct TArgs *pArgs;		// nil = 尚未被分析 !
    // [pArgs] = { ret_Type: Smallint;  // ret_Type = 0 = 無返回值需要 ! (-1: 尚未分析 !)
    //             nArgs: Smallint;     // 指出後面有幾個參數
    //             arg1, arg2, arg3...: Smallint; }  // 每一個參數的型態碼
};

struct TFuncStru {      // 此架構缺乏 pArgs[], 希望漸勿使用
	char *pName;		// 函數名稱
	unsigned short bgnLine, finLine;	// 函數在程式的開始行號與終止行號
	unsigned char Prg_ndx;		// 供 pss[] 使用之程式架構索引
	unsigned char fncType;		// 是 procedure = 0, function = 1, external = $40, 尚不明 = $80
	unsigned char retType, argCount;	// 傳回值型態, procedure = 0 (不須傳回值),
//  pad: array [0..1] of Byte;
	unsigned int ref_count;		// 已經被呼叫過的次數
	void *prog_addr;			// 真正程式區塊的擺放位置 (進入點)
    unsigned int prog_len;		// 真正程式區塊的長度
};

struct TLocalStru {	// 區域變數宣告
	struct TNameStru *pVar;	// 來源之變數或宣告架構
	void *func_prc;	// 指到 TMyFunc or TMyProc [舊: 指到 pPRC (TFuncProc) (<-- pProcFuncs 在 calx_ana.pas)]
	unsigned short name_ndx;	// 指到 all_var[] (TNameStru) 的 index
	unsigned short lineNo;	// 此變數宣告來自哪行
	unsigned short initNdx;	// 初值索引表, 若是每次進入程序/函數時, 須設定初值, 則由此取 index 查 constant 表
	unsigned short vType;	// 變數或宣告型態代碼 (TypeCode[], 例如 array = $80)
	double d_Val;	// 64-bit double 值, 或是 longInt (64 bits)
	int i_Val;		// integer 或 single, 反正 32-bit 能放得下的就放這邊 (陣列則放其 array index)
	unsigned char prgID;	// 來自哪個程式 (now_prog)
	unsigned char flags;	// 旗號: (b7: 1=已有初值, b6: 1=是常數 0=是變數, b5: 1=有多個函數之不同定義 0=是全域_大家可共用此值),
	unsigned char arg_no;	// 此變數是第幾個參數 (0 = 區變, 參數應該由 1 開始)
	unsigned char pad_00;	// 剛好補足 32 bytes
};
  
struct TMyPrograms {
	char whole_filename[256];		// 來源原始程式完整檔名
    char short_filename[64];		// .MIA 內的單純短檔名 (來源原始程式名稱)
    unsigned int prog_id;			// 程式代號, 通常由 1 算起 (不從 0 起, 以免失誤)
    unsigned int line_count;		// 總行數
    unsigned int var_count;         // 總變數個數
};

struct TMyConst {	// 常數
    char name[128];
    char initVal[256];
    double dVal;
	int iVal;
	unsigned int initNdx, line_no, prog_id, var_index;
    unsigned char flag, vType;
};

struct  TMyFixVal {	// 固定值
    char initVal[256];
    char *pName;
    int iVal;
	unsigned int prog_id;
    unsigned char cType, flag;
};

struct TMyExcBlk {	// Excised Token Blocks (Undone Evals, 切出的 Undone Evals 程式區塊)
    char src_func[128];		// <in proc/func name>
    void *tokenAddr;
    unsigned int excised_ndx, token_len, line_no, prog_id;
};

struct TMyForInfo {  	// for.. 執行時使用的相關資訊
        int line_no, prog_id;
        void *tokenAddr;    // 此位址是 for 該行的開頭, 供快速返回使用

};

struct TMyFunc {	// 函數
    char name[128];
    void *pFuncStru, *tokenAddr, *pArgs;
    unsigned int bgn_ln, fin_ln, token_len, line_no, prog_id, fn_index;
    unsigned char arg_cnt, fncType, retType;
};

struct TMyLocal {	// 區域變數
    char name[128];
    char src_func[128];
    double dVal;
	int iVal;
	unsigned int initNdx, line_no, prog_id, var_index;
	unsigned char arg_ndx, flag, vType;	// arg_ndx: 0=not arg, 參數編號從 1 算起.
};

struct TMyProc {	// 程序
    char name[128];
    void *pFuncStru, *tokenAddr, *pArgs;
    unsigned int bgn_ln, fin_ln, token_len, line_no, prog_id, fn_index;
    unsigned char arg_cnt, fncType, retType;
};


struct TMyRepeatInfo {  // repeat..until 執行時使用的相關資訊
        int level, prog_id;
        void *tokenAddr;    // 此位址是 repeat 該行的開頭, 供 until 快速返回使用

};

struct TMySmallBlk { // Excised Token Blocks (切出的程序小區塊)
    void *tokenAddr;    // 此位址是舊的程式解析位址, 請自填新位址
    unsigned int block_ndx, token_len;
};
  
struct TMyUndone {
    char name[128];
    unsigned int line_no, prog_id, solved;	// pSolved 紀錄 func_stru[] 的 index
};

struct TMyVar {		// 全域變數
    char name[128];
    double dVal;
	int iVal;
	unsigned int initNdx, line_no, prog_id, var_index;
    unsigned char flag, vType;
};

struct TNameStru {	// 變數或宣告架構 (佔 7 or 8 個 32-bits)
    char *pName;	// 變數或宣告名稱字串指標
    unsigned int lineNo;	// 來自哪行
    unsigned short prgID;	// 來自哪個程式
    unsigned short flags;	// 旗號: (b7: 1=已有初值, b6: 1=是常數 0=是變數,
		// 旗號:  b6: 1=參數 or 全域變數 或 區域變數, 0=是常數
		// 旗號:  b5: 1=有多個函數之不同定義 0=是全域_大家可共用此值),
		// 旗號:  b4: 1=有多個函數之不同定義 0=是唯一名稱區域變數), 執行時減少搜尋次數, 避免不必要之 TLocalStru 搜尋
    unsigned short vType;	// 變數或宣告型態代碼 (TypeCode[], 例如 array = $80)
    unsigned short initNdx;	// 初值索引表, 若是每次進入程序/函數時, 須設定初值, 則由此取 index 查 constant 表
    short name_first;       // 第一個同名變數的位置
    short name_next;        // 指到下一個同名變數的索引, 0=未檢查,-1=此為終點
    int i_Val;	// integer 或 single, 反正 32-bit 能放得下的就放這邊 (陣列則放其 array index)
    double d_Val;	// 64-bit double 值, 或是 longInt (64 bits)
    char *str;
};

struct TProgStru {	// 程式架構
  char *pName;		// 程式名稱
  int  nLines;		// 程式總行數
};

struct TUndefs {
  char *pName;          // 函數名稱  (當程式剖析完成時, 最後 4 bytes 再補放真正的 func_index !)
  unsigned short lineNo;        // 第一次被引用時的程式的行號 (最多放到 3 bytes, 包含下一欄的 qq)
  unsigned char qq, Prg_ndx;    // 供 pss[] 使用之程式架構索引
};

struct TUndoneEval {
  int LineNo;           // 這行指令位在哪個行號
  void *pFunc;          // = PFuncStru (這行指令位在哪個函式裏)
  char *pTokens;        // 指到擺放這個未處理的運算式
  unsigned short  nTokens;      // 這個未處理的運算式總共含有多少個 tokens
  unsigned char zero, progID;   // zero = 0
  // progID = index to pss[] (程式架構)(這行指令位在哪個程式裏)
};

struct TVarNames {	// 給 PNameStru 變數或常數名稱排序用
    int index;	// bit 0: 1=是常數 0=是變數. (bit 31-1: 做為 PNameStru 的索引值)
    char *pName;
};

struct ProgDataA {
    struct TMyPrograms	*all_prog, *now_prog;	// 從 1 算起
    struct TMyVar 		*all_glob, *now_glob;	// 全域變數
    struct TMyLocal		*all_local, *now_local;	// 區域變數
    struct TMyConst		*all_const, *now_const;	// 常數
    struct TMyFixVal	*all_fixval, *now_fixval;	// 固定值
    struct TMyFunc		*all_func, *now_func;	// 函數
    struct TMyProc		*all_proc, *now_proc;	// 程序
    struct TArgs		*all_args, *now_args;	// 參數表
    struct TMyUndone	*all_undn, *now_undn;	// 未完成之函數/程序
    struct TMyExcBlk	*all_excblk, *now_excblk;	// 切出的程式區塊
	struct TMySmallBlk  *all_smblk, *now_smblk;	// 切出的程序小區塊
    unsigned char *pTokBuf, *pTokNow;	// 轉換進來的 Token Byte Data
    unsigned int cnt_prog;		// all_prog[]   已經使用的數量 (來源原始程式)
    unsigned int cnt_glob;		// all_glob[]   已經使用的數量 (全域變數)
    unsigned int cnt_local;		// all_local[]  已經使用的數量 (區域變數)
    unsigned int cnt_const;		// all_const[]  已經使用的數量 (常數)
    unsigned int cnt_fixval;	// all_fixval[] 已經使用的數量 (固定值)
    unsigned int cnt_func;		// all_func[]   已經使用的數量 (函數)
    unsigned int cnt_proc;		// all_proc[]   已經使用的數量 (程序)
    unsigned int cnt_undn;		// all_undn[]   已經使用的數量 (未完成之函數/程序)
    unsigned int cnt_excblk;	// all_excblk[] 已經使用的數量 (切出的程式區塊)
	unsigned int cnt_smblk;		// all_smblk[]  已經使用的數量 (切出的程序小區塊)
    unsigned int TokRemain;		// 尚須轉換的 Token Byte Data 資料 byte 數
    unsigned int err_id;		// 處理過程發生錯誤 (代碼 0 = No Error)
};

#define PACKAGE              __declspec(package)                 // Implemented in a package
/*namespace QSort
{
extern PACKAGE void _fastcall QSortBy(void *pDta, int ordOfs, int recSize, int nItems);		// 排序
extern PACKAGE void _fastcall SetSortFCMP(int _fastcall func(void *pA, void *pB));		// 設定排序比較函數
}

namespace iDebug {
extern PACKAGE void _fastcall cSay(char *pStr);
};

namespace C_wrap {
extern PACKAGE void _fastcall xfree(void *p);
};  */
//extern void _fastcall xHexDump(void *p, int len);
//extern void _fastcall xSayStrHex(char *pStr, int val, int len);		// iDebug.pas
