/*  C:\Borland\CBuilder5\Bin\bcc32 -6 -IC:\Borland\CBuilder5\Include -c I:\BCBS\RobotCsim\runner.c
    C:\Borland\CBuilder5\Bin\bcc32 -6 -IC:\Borland\CBuilder5\Include -c runner.c
  編譯時若出現底下這行訊息, 沒關係, 是正常的, 可以忽略它
  Warning C:\HELPDIAG\BORLAND\auto\su_dbf.c 16: Redefinition of 'errno' is not identical
  必須把 *.hpp 拷貝到 C:\Borland\CBuilder5\Include 底下哦 !!
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "run_def.h"
#include "run_def2.h"
#include "my_err.h"
#include "my_mark.h"
// #include "Q_Sort.hpp"
// #include "iDebug.hpp"
#include "C_wrap.h"
#include "sufloat.h"

#define	MAX_CALLERS_COUNT	128		// 最多高達 128 層的呼叫
#define	MAX_CALLERS_SIZE	MAX_CALLERS_COUNT * sizeof(struct TCaller)
#define	MAX_FUNC_ARG_LIST_COUNT	64		// 執行中的函數或程序的參數列表
#define	MAX_LOCAL_POOL_COUNT	1024	// loc_list[ ] 的上限
#define MAX_TYPE_MASK		31			// 最高到 31 種型態
#define	Max_Err_Count		50
#define	MAX_LOCAL_STACK		64
#define	MAX_ARR_COUNT		128			// 最多 128 個陣列
#define	MAX_FINAL_BLOCKS    2048        // 全部函數裏面, 最多允許 2048 個切割出來的程式小段落 (begin/repeat/for/if.. 等控制切割)
#define	ARRAY_STRU_SPACE_SIZE	MAX_ARR_COUNT * sizeof(struct TArrayStru)	// 128 * 20 = 2560
#define	MAX_LOC_COUNT		32768		// 最多 32768 個區域變數
#define	MAX_LOC_SIZE	MAX_LOC_COUNT * sizeof(struct TLocalStru)	// 32768 * 32 = 1048576
#define	MAX_VAR_COUNT		16384		// 最多 16384 個變數
#define	MAX_VAR_SIZE	MAX_VAR_COUNT * sizeof(struct TNameStru)		// 16384 * 24 = 393216
#define	NAME_STR_SIZE		262144		// 256 KB for var/const/procedure/function names
#define	CONST_SPACE_SIZE	131072		// 128 KB for const data structure.
#define	CONST_STR_SPACE_SIZE	65536	// 64 KB for const string data.
#define	MAX_BREAK_PTS		128			// 最多 128 個程式中斷點
#define	MAX_PROGRAMS		32			// 最多 32 個不同的程式碼
#define	MAX_TYPE_MODIFIERS	20			// 最多 20 型態識別碼
#define	MAX_UNDEF_COUNT     64
#define	MAX_VAR_COUNT		16384		// 最多 16384 個變數
#define	MAX_VAR_SIZE	MAX_VAR_COUNT * sizeof(struct TNameStru)		// 16384 * 24 = 393216
#define	Type_Code_PCHAR		13			// 型態識別碼
#define	Type_Code_PTR		14			// 型態識別碼
#define	ShowFuncBeginLineNo	4

#define	RES_FUNC_COUNT          2
#define	ENABLE_BREAK			1		// ($9E=line no token) + offset 3 --> (set 1 = enable break point)
#define	Clear_BreakPoint	0			// job ID for do_break_pts()
#define	Set_BreakPoint		1			// job ID for do_break_pts()
#define	FLAG_STOP			0x80
#define	V_EXEC				0x10000
#define	BEFORE_EXEC_LINE	1
#define	DEBUG_STOP			2
#define	SHOW_INSTR			3
#define	SHOW_VAR_NS			4			// (變數通常是 struct TNameStru *)
#define	FUNC_PRINTF         5
#define	FUNC_RETURN         6
#define	WARN_OR_ERROR		7
#define	BEFORE_EXEC		(V_EXEC + BEFORE_EXEC_LINE)		// in exec_this_line
#define	DEBUG_STOPPED	(V_EXEC + DEBUG_STOP) 		// in wait_for_debug
#define	SHOW_INSTRUCT	(V_EXEC + SHOW_INSTR) 		// in try_translate_tokens
#define	SHOW_VAR		(V_EXEC + SHOW_VAR_NS) 		// in set_watch_var
#define	PRINTF_ANS		(V_EXEC + FUNC_PRINTF) 		// in myprintf
#define	WARN_ERR		(V_EXEC + WARN_OR_ERROR) 		// in do_sys_error
#define	RETURN_ANS      (V_EXEC + FUNC_RETURN) 		// in myreturn

#define	ID_TNameStru            1
#define	ID_FuncProc             2
#define	ID_Args                 3
#define	ID_FuncStru				4

char canSay = 1;
char canMark = 1;

char *Resident_FuncName[RES_FUNC_COUNT] = { "printf", "return" }; // 1,2

char *ConstTypes[16] = { "無定義", "char", "byte", "sint8",
        "word", "sint16", "dword", "integer", "qword", "sint64",
        "single", "double", "char", "雙字元", "unicode", "string" };

char *errMsg[Max_Err_Count] = { "程式數目超過 32 個", "函式數目超過 2048 個",
	"新增字串時, 發現不合格的代碼, 應是暫存字串區已滿", "尚有函式沒有被正確的被終止",
	"不可在一行內宣告超過 64 種不同的名稱", "變數或常數名稱不可以用數字起頭",
	"暫不支援多維度陣列", "數值轉換時發現不合格的文字",
	"本系統沒有這種數值型態", "陣列的範圍數值不對",
	"本系統只允許 128 個陣列", "常數需要一個有效的值, 否則將自動以 0 或 nil 代入初值",
	"發現未完成的表示式, 請將程式寫完整, 謝謝 !", "不合法的表示式",
	"表示式太長, 請精簡程式內容 !", "儲存未定義程序或函數的表格已滿 (= 512)",
	"括號沒有對稱好", "在行號的位置出現不該有的代碼 !",
	"太多組 begin..end (本系統上限: 32 組)", "end 的位置不對, 沒有 begin 可對應 !",
	"太多組 repeat..until (本系統上限: 32 組)", "until 的位置不對, 沒有 repeat 可對應 !",
	"太多組 if..then..else (本系統上限: 64 組)", "then 的位置不對, 沒有 if 可對應 !",
	"else 的位置不對, 沒有 then 可對應 !", "太多程序控制小段落 (每個程序或函式不可超過 512 段落)",
	"運算式轉換過程出錯誤 (內部錯誤, 請洽本程式原始開發單位)", "運算式內含太多的變數 (超過 95 個) 或運算子 (超過 63 個)",
	"運算式錯誤 ! 變數或數值不夠供運算式使用 !", "程式內部索引值不良, 請洽原始設計人員 !",
	"函式定義錯誤: 無函式名稱", "函式定義錯誤: 不該出現奇怪的符號",
	"函式定義錯誤: 此處只應該有冒號 (:)", "區域變數使用過度了 ! 本系統只允許 65536 個區域變數", "無此內建函數",
	"函數需要傳回值", "不合法的執行位址", "參數個數不正確", "常數不能改變其值 !", "找不到指定的區域變數",
	"暫放 now_fun_loc_count 的堆疊已滿", "太多變數在觀察了", "太多暫時中斷點", "procedure 不需要返回值",
	"錯誤上限, 你應該看不到這個啦", "缺乏迴圈變數", "迴圈變數後面應該要有 = ", "此迴圈缺乏初值 !",
    "此迴圈缺乏終值 !", "while 後面需要有 do !" };

// 以左方運算數值型態為主要型態, 位居表格 Column, 右方運算數值型態是 Row, 表格值為 0 代表無須轉換
// 底下表格 > $80 的值要扣去 $80, 並產生 <無法預期轉換結果是否仍然有效的警告>
// 底下表格 > $40 的值要扣去 $40, 並產生 <轉換結果可能有疑慮的警告> (數值或符號損失)
char MainCvt[256] = { //  (16x16 轉換表) 左或右側是陣列 (type 碼 = 0) 者, 另外處理, 此處表格不存在轉換
           5,    4,    4,    4,    7,    7,    7,    8,    9,   10,   11,   12,    0,    0,   15,   16,
           5,    4,    4,    4,    7,    7,    7,    8,    9,   10,   11,   12,   13,    7,   15,   16,
           7,    4,    4,    4,    7,    7,    7, 0x48, 0x49, 0x4A,   11,   12,   13,    4,   15,   16,
           7,    4,    4,    0,    7,    7,    7, 0x48, 0x49, 0x4A,   11,   12,   13,    4,   15,   16,
           7,    7,    7,    7,    7,    7,    7, 0x48,    9,   10,   11,   12,   13,    7,   15,   16,
           7,    7,    7,    7,    7,    7,    7, 0x48,    9,   10,   11,   12,   13,    7,   15,   16,
           7,    7,    7,    7,    7,    7,    7, 0x48, 0x49, 0x4A,   11,   12,   13,    7,   15,   16,
        0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,    0,    9,   10, 0x8B, 0x8C,    0,    0, 0x8F, 0x90,
        0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49,    8,    9,   10, 0x8B, 0x8C,    0,    0, 0x8F, 0x90,
        0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A,    8,    0,    0, 0x8B, 0x8C,    0,    0, 0x8F, 0x90,
        0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x48, 0x89, 0x8A,    0,   12, 0x47, 0x47, 0x4F, 0x50,
        0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x48, 0x89, 0x8A,   12,    0, 0x47, 0x47, 0x4F, 0x50,
           4,    4,    4,    4,    7,    7,    7, 0x48, 0x49,   10, 0x4B, 0x8C,    0,   14, 0x4F, 0x50,
           4,    4,    4,    4,    7,    7,    7, 0x48, 0x49,   10, 0x4B, 0x8C,   13,    0, 0x4F, 0x50,
        0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x48, 0x89, 0x8A, 0x4B, 0x8C, 0x4D, 0x4E,    0,   16,
        0x44, 0x44, 0x44, 0x44, 0x47, 0x47, 0x47, 0x48, 0x89, 0x8A, 0x4B, 0x8C, 0x4D, 0x4E,   15,    0};

char symbols[22] = { '.', '[', ']', '(', ')', '+', '-', '!', '*', '/',
            '%', '&', '^', '|', '~', '?', ':', ',', '>', '<', '=', ',' };
char *symbol2[26] = {"++", "--", "+", "-", "+=", "-=",
            ">=", ">>", ">>=", ">>>", ">>>=",
            "<=", "<<", "<<=", "<<<", "<<<=",
            "==", "!=", "*=", "/=", "%=", "&=", "^=", "|=",
	    "&&", "||"
            };

char *sys_cmd[MAX_SYS_CMD] = { "begin", "end", "repeat", "until", // 前四樣允許多行敘述
            "if", "then", "else", "do", "while", "for", "downto", "to",
            "step", "case", "of", "continue", "break", "exit" };
        // 如果 exit 前的字碼要改變位置, 請連帶修改 flowCtrl.pas 的 mark_check_flow_control 的 case..of !!
        // 如果 exit 前的字碼要改變位置, 請連帶修改本程式的 TOKEN_ID_LIST 的順序 !!

// 型態識別碼
char *TypeModifier[MAX_TYPE_MODIFIERS] = { "array", "char", "byte", "word",
		"dword", "sint8", "sint16", "int", "dblchar", "unicode", "string", "single",
		"double", "pchar", "ptr", "integer", "sint64", "qword", "void", "ansistring" };

char *sLValue = "左值";
char *sRValue = "右值";
char *strNULL = "(NULL)";

/* struct TFuncProc resident_funs[3] = {
        { name:"printf";   lineNo:0; ret_Type:0; nArgs:1; pArgs:NULL},
        { name:"return";   lineNo:0; ret_Type:0; nArgs:1; pArgs:NULL},
        { name:"無此函數"; lineNo:0; ret_Type:0; nArgs:0; pArgs:NULL} }; */

struct TFuncProc resident_funs[3] = {
        { "printf",   0, 0, 4, NULL},		// !!@@ 此處參數應是可變的 @@!!
        { "return",   0, 0, 1, NULL},
        { "無此函數", 0, 0, 0, NULL} };

struct TArrayStru *all_array, *now_array;
struct TCaller *callers;		// 所有的呼叫資訊
struct TCaller *cur_func;		// 呼叫函數前, 由父程序填註的父程序相關資訊
struct TConstStru *all_const, *now_const;
struct TMyFunc *my_fun;
struct TMyProc *my_proc;
struct TFuncArgList *pFunArgs, *pFunNow;
struct TFuncProc *pPRC, *pProcFuncs, *pResiFuncs;	// 全部的程式與函數索引表 (calx_ana #184)
struct TFuncStru *func_stru, *fust, *undone_func;	// func_stru = array[2048] of PFuncStru
struct TNameStru *all_run_loc, *now_run_loc;	// 執行時期使用中的區域變數
struct TNameStru *now_use_locals;		// 記住現在該取得的區域變數群 !
struct TNameStru *all_var, *now_var;
struct TNameStru *sys_argv;				// 指到內部預設參數陣列 (上限 16 個參數)
struct TLocalStru *all_local, *now_local;
struct TProgStru *pss;          // 程式代號, 通常由 1 算起(原本從 0 算起) (不從 0 起, 以免失誤)
struct TVarNames *pGlobVars;			// 有排序過的 all_var. (原則上優先取用較完整的 all_var)
struct TUndefs *undefs;         //  array [MAX_UNDEF_COUNT]
struct TUndoneEval *pUDE, *pUnDoneEval;         // 暫時擺放一堆未被處理的運算式
struct TMyRepeatInfo *all_repeat_info;          // repeat..until 執行時使用的相關資訊
struct TMyForInfo *all_for_info;                // for 執行時使用的相關資訊

int loc_list[MAX_LOCAL_POOL_COUNT];		// 放對應的 token
unsigned int loc_roll_n[MAX_LOCAL_POOL_COUNT];  // 每次呼叫函數唯一的滾動值, 代表此變數由哪個函數使用
int nConsts, nNames, array_count, now_loc_count, now_locv, now_run_prog, now_var_count;
int CallerLen;			// 呼叫函數時的詳細 token 資料長度, 供解析參數用
int callerStack, ErrorCode, ErrorLine, exec_line_No, temp_buf_size;
int prg_cnt, proc_cnt, func_cnt, func_stru_cnt, nBrkPts, nowLineNo, nowLineTokCnt, nFunCnt, nPRC, nUndef_Func, nVars, vTok, vToken1, vToken2;
int loc_stack_now;					// 指到 loc_stack[]
int loc_stack[MAX_LOCAL_STACK];		// 暫放 now_fun_loc_count 用, 最高 MAX_LOCAL_STACK = 64 層
int max_for_info_cnt, for_info_cnt;     // all_for_info[] 的上限與目前已使用個數
int max_repeat_info_cnt, repeat_info_cnt;       // all_repeat_info[] 的上限與目前已使用個數
int now_fun_loc_count;	// 目前這個函數使用的區域變數個數
unsigned int rolling_number;    // 每次呼叫函數時, 會給予一個全新的,唯一的滾動數值,比使用 Thread ID 快且專一
int sys_argn;			// 內部預設參數陣列有效參數個數 (僅有一組, 不能重入)
int nAnsToken;			// 剖析一行命令所輸出的一堆 dwords token 個數 ! , nTempToken
int xVar[96];			// 執行運算式用的 變數 stack (max 96)
void *hWait;			// HWND 除錯中斷用
unsigned short   typ1, typ2;	// 運算時的左 & 右值型態
unsigned char final_type;		// 運算時的最終運算型態
char val1[8], val2[8];			// val2 也可視為堆疊頂端的值 (若 bVal2Valid = True)
char *pAllConstStr, *all_names, *final_addr, *pAllNames, *pConStr, *pNextExec, *pThisLine;
int *pAnsTokens, *pStackToken;		// 輸出一堆 dwords token ! , pStackToken
int *pSolved;   // 儲存 undefs[] 對映的執行函數號碼, 若仍未解決則放 -1
int nUnSetBP, *pUBPs;
int finalBlkNdx, finBlockLen[MAX_FINAL_BLOCKS];
int mark_level = 1;		// 至少是 1
struct BRK_PTS *brkps;			// 中斷點
char *finBlockMem[MAX_FINAL_BLOCKS];
unsigned int *pCallerText;		// 呼叫函數時的詳細 token 資料, 供解析參數用
struct ProgDataA *mainProg;
void *ErrorPFunc;       // 記下錯誤函數
char if_result[4]; 	// 0: false, 1: true
char *sStrBack, *temp_str[128];
char su_buf[256];		// for su_printf() use, 傳回字串用
char result_str[256]; 	// for myreturn() use only !! 傳回結果用
char ssTemp[32];
char bMayHasErr,bNoToDebug, bSuspCvt, bSysErr, bUpToGo, bUserStop, bVal2Valid, fGiveUp, fOnDebug;	// bool
char bMakeExit;	// 是否遇到 return(); ?
char bLastCondTok;	// 前一個控制代碼是 if or until ? (給比較運算元 (如 =) 判斷該做比較或是設值)
char bStepTrace;	// 0=Not Enabled Stop, 1=Step Stop Enabled, 2=Trace Stop Enabled

extern void _stdcall copy_temp_str(void *your_ptr);		// 一次拷貝整個 temp_str[128] (in su_blob.c)
extern char safe_ptr(int line_no, void *ptr, char *info);	// in c_extra.c
extern int _fastcall status_updater(int c_line_no, int func_id, int func_arg, char *func_ptr);

void calc_ana_init(void);       // 運算式剖析程式初始化, 最先被呼叫 !  (Robotman 程式剛 created 時執行)
void chain_up_this_var(struct TNameStru *pNX);  // 從頭串起此同名變數;
void check_for_info(unsigned char *pTk);        // 設置並初始變數
void check_set_repeat_info(unsigned char level);	// 查詢是否已經設定好 repeat-until 資訊
void check_while_expr(unsigned char *pToken);   // 檢查是否繼續執行
void clear(void *p, int len);
void casting_one_val(char *pVal, unsigned char typ);
void *convert_fun_prc(char *func_name);         // 用函數名稱查詢, 指到 TMyFunc or TMyProc
int  cReadDW(void *p, int ofs);
int  cReadInt(void *p, int ofs);
void cSetDW(void *p, int ofs, int val);
void debug_sys(char fun);
unsigned short decode_one_value_from_stacktop(char *val);
void do_all_prog_const_init(void);
int  do_break_pts(int line_no, int job);		// 修改 Token 內容, 以便 { 清除/設置中斷點 }
void do_only_one_statement(int *pTok, int len);
void do_opr_08_mult(void);
void do_opr_09_div(void);
void do_opr_18_greater(void);   // 大於運算
void do_opr_19_lesser(void);    // 小於運算
void do_opr_20_assign(void);
void do_opr_20_equal(void);
void do_opr_22_plusplus(void);  // 加加運算
void do_opr_23_minusminus(void);        // 減減運算
void do_opr_24_add(void);
void do_opr_25_sub(void);
void do_prepare_to_run(void);	// 為最初跳進去 start() 的過程作準備
void do_run_func(int func_id);  // 執行一個 function
void do_run_reserve_word(unsigned char *pTk);    // 處理保留字
void do_run_resi_func(int func_id);     // 執行內建函數
void do_statements(int *pAll, int len);
void do_stktopVal_type_casting(void);
void do_sys_error(int msgID, int info);
void do_type_casting(void);
void exec_here(char *pGo);      // 從這邊跳進去執行 !
int  exec_this_line(char **ppTemp, unsigned char *pGo, int len);
void exec_this_small_block(int block_ndx);
void exec_this_undone_block(int block_ndx);
void fetch_one_value(void);
void fetch_two_values(void);
void fill_in_arg(char *var_name, struct TNameStru *pLocVs);
void fill_in_pProcFuncs(void);	// 把 my_fun & my_proc 的資料填到 pProcFuncs (pPRC) 裡面 !
struct TMyForInfo *find_current_for_info(void);         // 尋找目前的 for info
struct TMyRepeatInfo *find_current_repeat_info(unsigned char level);	// 尋找目前的 repeat-until info
void *find_function_by_name(char *fname);       // 找尋某個函數 (傳回 struct TMyFunc * 或 struct TMyProc *)
int  found_in_undefs(char *name);
void free_local_vars(struct TNameStru *pBgn, struct TNameStru *pEnd);	// 釋放所借的區域變數
char *get_const_by_ndx(int x);  // 取得常數內涵, 轉成字串格式void get_result(char *pResult);
char *get_fixval_as_str(int fixVal_ndx);	// 從 struct  TMyFixVal *[] 取得固定值, 以字串表達內容
char * _stdcall get_errMsg(int msg_id);
void get_result(char *pResult); // 把 Result[] 的值剖析後存到 pResult 中 (val2[]: 8 bytes)
int  get_rolling_no(void);      // 取得函數的滾動數值 ! (快速替代 Thread ID)
struct TFuncProc *get_FuncProc_byName(char *name_str);
struct TNameStru *get_newest_var(struct TNameStru *pNX);
char *get_InitVal_str(int initNdx, int flag);
char *get_undone_calls(int x);  // 取得呼叫函數的整個字串 !
char *get_var_name_by_ndx(int x);       // 取得變數名稱
void go_back_repeat(unsigned char level);	// 需回 repeat 重複執行
void _stdcall init_All_Before_Run(void);
void _stdcall load_prog_info(void *p);
void load_prog_vars(void);      // 處理常數變數總庫
void load_undone(void);         // 處理未完成部分函數/程序
// int __fastcall myStrComp(void *pA, void *pB);   // 字串指標內容比較
void make_RPN(int *pIn, int nTok);
void mark(int func_no, int status);		// 標記程式是否正常脫離
struct TFuncStru *MyFunc2FuncStru(void *fun);   // 把 struct TMyFunc * 換成 struct TFuncStru *
void parse_arguments(struct TFuncProc *pPF, struct TNameStru *pLocs);
void parse_arguments_for_nil(struct TFuncProc *pPF, struct TNameStru *pLocs);
struct TNameStru *prepare_local_vars(struct TFuncProc *pProFun);
void process_UnSetBPs(void);	// 處理未被處理的中斷點 !void recursive_parse_args(unsigned int *pTok);
void recursive_parse_args(unsigned int *pTok);
char *ret_value_str(unsigned char typ, char *pv);
void run_a_func(void *func, struct TNameStru *pLoc, char bMyFunc);    // 由 token 或 start 或 執行內部函數, 最候都呼叫到這邊 !
void run_main_start(void);		// 執行唯一的 start() !
int  _stdcall Run_Program(void *h_Wait);
int  run_resident_func(int func_id, struct TNameStru *pLocVs);  // 從外部來執行一個內建函式
void _stdcall Run_was_done(void);
void save_toSetBP(int progID, int lineNo);		// 保存尚未設定好的斷點資訊 !
void seeStru(void *p, int ID_stru);		// 檢閱結構內涵
int  _stdcall set_break_line(int prog_id, int line_no);	// 設置中斷點
void _stdcall set_step_trace_flag(int flag);	// set flag to bStepTrace (0=Not Enabled Stop, 1=Step Stop Enabled, 2=Trace Stop Enabled)
void _stdcall set_temp_str(int ndx, char *str);
void set_watch_var(void *pNS, int var_ndx);		// 更新結果顯示
char *sIntToStr(long v);
void sort_global_vars(void);
void sPrt(char *buf, char *fmt, ...);
int  status_update(int line_no, int fun_code, int fun_data, void *fun_ptr);
void su_printf(char *fmt, ...);
int  tk_job_every_line(unsigned char *pTok, int job_id, int tk_len);
int  tk_job_one_line(unsigned char *pTok, int job_id, int line_no, int tk_len);
void *try_match_local_var(struct TNameStru *pV);
void try_translate_tokens(unsigned char *pT, int len);
int  wait_for_debug(int code_in);	// 要被除錯中斷 !  --->  PulseEvent(hWait); // 繼續執行 !
void xSay(char *fmt, ...);
void xx_caller(char *s, struct TCaller *src);
void _stdcall yHexDump(void *p, int len);	// #3670

// ---------------------
void sPrt(char *buf, char *fmt, ...)
{
va_list	ap;

//  if (ap == NULL) { xstrcopy(buf, fmt);  return; }
  va_start(ap, fmt);
  wvsprintf(buf, fmt, ap);
  va_end(ap);
}

void xSay(char *fmt, ...)
{
char	buf[256];
va_list	ap;

if (! canSay) return;
//  if (ap == NULL) { cSay(fmt);  return; }
  va_start(ap, fmt);
  wvsprintf(buf, fmt, (char *) ap);
  va_end(ap);	cSay(buf);
  // 若印出的 %s 裡面還有 format 格式字串, printf 將會有不可預期的結果 ! 可改成 puts() 但會多出一個換行 !
}    

void _stdcall allocate_local(struct TArgs *pArg)
{	// 新借一個區域變數 local_var_addr 是指到 all_var: PNameStru  // = $464BF8
struct TLocalStru *pVSrc;
int ndx;
	
	xSay("#349 準備存放區域變數的位置編號 now_locv=%d, 參數需求內容為 pArg=$%x,",now_locv, pArg);
	if (now_locv > (MAX_LOCAL_POOL_COUNT - 2)) return;
	yHexDump(pArg, 20);			// call to $46C004
	ndx = pArg->loc_match - 1;	// 第幾號區域變數
	xSay("#353 即將存放於全部的區域變數第 (only 8 bits) ndx 位置 = %d", ndx);
	pVSrc = all_local;		
	pVSrc += ndx;
	// 拷貝這個變數定義
	clear(&now_run_loc->pName, sizeof(struct TNameStru));		// 變數內容填 0
	now_run_loc->pName = pArg->name;
	now_run_loc->lineNo = pArg->line_no;
	now_run_loc->prgID = pVSrc->prgID;
	now_run_loc->flags = pArg->flags;
	now_run_loc->vType = pArg->vType;
	loc_list[now_locv] = VAR_TOKEN | (ndx << 8);	// 把 token 放到表格,方便找尋
	loc_roll_n[now_locv] = cur_func->cur_rolling_no;    // 代表此變數由哪個函數使用
	xSay("#365 區域變數名稱 %s, 型態碼: $%x, 旗號: $%x, 傳回 Token= $%x", pArg->name, pArg->vType, pArg->flags, loc_list[now_locv]);
	now_locv ++;		now_run_loc ++;	
}

void calc_ana_init(void)
{ // 運算式剖析程式初始化, 最先被呼叫 !  (Robotman 程式剛 created 時執行)
    pAnsTokens = (int *) xmalloc(MAX_TEMP_TOKEN_SIZE);  nAnsToken = 0;
    pStackToken = (int *) xmalloc(MAX_TEMP_TOKEN_SIZE >> 1);
    sStrBack = (char *) xmalloc(256);
    // MAX_UNDONE_EVALS = max $800 = 2048 個未被處理的運算式
    pUnDoneEval = (struct TUndoneEval *) xmalloc(MAX_UNDONE_EVALS * sizeof(struct TUndoneEval));
    pUDE = pUnDoneEval;
    pResiFuncs = resident_funs;         // 內建函數
/*	maxUsrFunc := 0;
     pTempFuncStr := AllocMem(USER_FUNC_STR_SIZE);
     pTokens := AllocMem(MAX_TEMP_TOKEN_SIZE);
     pTKS := pTokens;           nTempToken := 0;
     pTempCaller := AllocMem(1024);     // 暫存一段函數呼叫的 token
     pCallerList := AllocMem(MAX_CALLER_COUNT shl 2);   nCallerList := 0;
     pUnEvalSpace := AllocMem(MAX_UNDONE_EVALS_SPACE);  pUES := pUnEvalSpace;
     tokenCaller := CALLER_SIG or FUNC_TOKEN;   // 暫存函數呼叫的替代代碼
	 */

    // pProgPool = xmalloc(MAX_POOL_A);				pPrgPool = pProgPool;
	//reload_pProcFuncs();	// 從 all_func & all_proc 收集函數回來
/*     pCodes := AllocMem(Max_Codes_Size);        pCodeTail := pCodes;
     prog_structure_init;
	 */
}

void casting_one_val(char *pVal, unsigned char typ)
{  // --> run_core.pas -- line 275 --  // 無法處理 64 bits !
SuDoubl *pDoubl;
SuFloat *pSingl;
char    *pC;
int     *pI;
short   *pS16;
int     i, j;
char c;
char s[256];

    pI = (int *) pVal;          pS16 = (short *) pVal;		xSay("#407 final_type = %d, typ = %d", final_type, typ);
	xHexDump(pVal, 16);			xHexDump((char *) *pI, 16);			
//     if final_type = 11 then pSingl := Pointer(pVal);   // Single
    switch(final_type) {
        case 4: // 轉成 dword (4)
            switch(typ) {
                case 1: // 1=char, 5=sint8
                case 5: c = *pVal;      *pI = c;        break;  // 把 char 當成 signed 8-bit
                case 2: *pI = (unsigned char) *pVal;    break;  // 2=byte
                case 3: *pI = (unsigned short) *pS16;   break;  // 3=word
                case 6: *pI = *pS16;    break;  // sint16
                case 7: if (pVal[3] & 0x80) bMayHasErr = 1;     // 7=int, 正負號可能丟失, -1 變很大的正值 !
                        break;  // 7=int (int 轉成 dword, 32-bit 不動 !)
                default: if (typ != final_type) xSay("#419 未設計此轉換 $%x --> $%x", typ, final_type);
            }
            break;
        case 5: // 轉成 sint8 (5)
            c = *pVal;     *pI = 0;     *pVal = c;      // 把後面的三個 byte 清為 0
            if ((typ == 2) && (pVal[0] & 0x80)) bMayHasErr = 1; // typ 2=byte 超過 127 的短正整數會轉成負值 !
            break;
        case 6: // 轉成 sint16 (6)
            if ((typ == 2) && (pVal[0] & 0x80)) bMayHasErr = 1; // typ 2=byte 超過 127 的短正整數會轉成負值 !
            switch(typ) {
                case 1: // 1=char, 5=sint8
                case 5: *pS16 = *pVal;       break;     // 把 char 當成 signed 8-bit, 再換成 signed 16-bit
                case 2: *pS16 = (unsigned char) *pVal;  break;  // 2=byte
                case 3: if (pVal[1] & 0x80) bMayHasErr = 1;   break;  // 3=word (正負號誤判, 很大的正值變負值 !)
                case 7: if ((*pI > 32767) || (*pI < -32768)) bMayHasErr = 1;  // 7=int 高位數的位元丟失 !
                        *pS16 = *pI;    break;
                case 11: a1single_int(pSingl, pI);  *pVal &= 0xFFFF;  bMayHasErr = 1;  break; // 硬轉換, 可能出錯 !
                case 12: a1double_int(pDoubl, pI);  *pVal &= 0xFFFF;  bMayHasErr = 1;  break; // 硬轉換, 可能出錯 !
            }
            break;
        case 7: // 轉成 int32 (7)
            switch(typ) {
                case 1: // 1=char, 5=sint8
                case 5: *pI = *pVal;    break;  // 把 char 當成 signed 8-bit
                case 2: *pI = (unsigned char) *pVal;    break;  // 2=byte
                case 3: *pI = (unsigned short) *pS16;   break;  // 3=word
                case 4: if (pVal[3] & 0x80) bMayHasErr = 1;  break;  // 4=dword, 正負號可能丟失, 很大的正值變負值 !
                case 6: *pI = *pS16;    break;  // 6=sint16
                case 11: a1single_int(pSingl, pI);  bMayHasErr = 1;    break; // 硬轉換, 可能出錯 !
                case 12: a1double_int(pDoubl, pI);  bMayHasErr = 1;    break; // 硬轉換, 可能出錯 !
                case 13: break; // pChar --> int (免轉換)
                default: if (typ != final_type) xSay("#450 未設計此轉換 $%x --> $%x", typ, final_type);
            }
            break;
        case 10: // 轉成 pascal short string (10) -- 長度限制 256 bytes
            switch(typ) {  // !!@@ 32/64 addr Err @@!! 最後字串位址皆只傳回 32 bit 保存 !
                case 1: // 把 char 轉成 pascal string
                        pC = (char *) xmalloc(2);       *pC = 1;
                        pC[1] = *pVal;          *pI = (int) pC;
                        break;
                case 2: // byte 整數數值透過 IntToStr(n) 變成 pascal string !
                        sPrt(s, "%d", (unsigned char) *pVal);   i = xstrlen(s);
                        pC = (char *) xmalloc(i+1);     *pC = i & 255;
                        xmemmove(pC+1, s, i);           *pI = (int) pC;
                        break;
                case 3: // word 整數數值透過 IntToStr(n) 變成 pascal string !
                        sPrt(s, "%d", (unsigned short) *pS16);  i = xstrlen(s);
                        pC = (char *) xmalloc(i+1);     *pC = i & 255;
                        xmemmove(pC+1, s, i);           *pI = (int) pC;
                        break;
                case 4: // DWORD 整數數值透過 IntToStr(n) 變成 pascal string !
                        sPrt(s, "%d", (unsigned int) *pI);      i = xstrlen(s);
                        pC = (char *) xmalloc(i+1);     *pC = i & 255;
                        xmemmove(pC+1, s, i);           *pI = (int) pC;
                        break;
                case 5: // sint8 整數數值透過 IntToStr(n) 變成 pascal string !
                        sPrt(s, "%d", *pVal);           i = xstrlen(s);
                        pC = (char *) xmalloc(i+1);     *pC = i & 255;
                        xmemmove(pC+1, s, i);           *pI = (int) pC;
                        break;
                case 6: // sint16 整數數值透過 IntToStr(n) 變成 pascal string !
                        sPrt(s, "%d", *pS16);           i = xstrlen(s);
                        pC = (char *) xmalloc(i+1);     *pC = i & 255;
                        xmemmove(pC+1, s, i);           *pI = (int) pC;
                        break;
                case 7: // sint32 整數數值透過 IntToStr(n) 變成 pascal string !
                        sPrt(s, "%d", *pI);             i = xstrlen(s);
                        pC = (char *) xmalloc(i+1);     *pC = i & 255;
                        xmemmove(pC+1, s, i);           *pI = (int) pC;
                        break;
				case 10: // pascal short string (10) 本身免轉換, 但還是檢查一下 !
						dbp(typ, (int) pI, "<- typ, pI (#491 PasStr -> PasStr) casting_one_val");
                        if ((typ & 0x40) == 0) { // 常數字串是存為 PPChar
							pC = (char *) *pI;      // !!@@ 32/64 addr Err @@!!
							dbp(typ, (int) pC, "<- typ, pC casting_one_val");
							if (pC == NULL) break;  // NULL str !!
							if (((unsigned char) *pC) > 31) { // 明顯不是 pString 而是 PPChar
								j = xstrlen(pC);
								for (i = j;i > 0;i --) pC[i] = pC[i-1];
								*pC = j & 255;      // 把 PChar 修正為 pascal string
								xSay("已把 PChar 修正為 pascal string !");
								yHexDump(pC, 16);
							}
						else yHexDump(pI, 16);
                        }
						break;
                case 13: // !! PChar --> pascal string !! 未轉換可能有 bug
						xSay("#507 PChar -> PasStr");
						pC = (char *) *pI;      // !!@@ 32/64 addr Err @@!!
						if (((unsigned char) *pC) > 31) { // 明顯不是 pString 而是 PPChar
                            j = xstrlen(pC);
                            for (i = j;i > 0;i --) pC[i] = pC[i-1];
							*pC = j & 255;      // 把 PChar 修正為 pascal string
                        }
						break;
				case 15: // C string !! 未轉換可能有 bug
						xSay("#516 右值是 C string, 需轉成字串, 在串接到左值 (pascal string) 之後");
						i = xstrlen(pVal);		// C string 變成 pascal string
						pC = (char *) xmalloc(i+1);     *pC = i & 255;
                        xmemmove(pC+1, pVal, i);        *pI = (int) pC;
						break;
                default: if (typ != final_type) xSay("#521 未設計此轉換 $%x --> $%x", typ, final_type);
            }
            break;
        case 11: // 轉成 single (11)
            pSingl = (SuFloat *) pVal;
            switch(typ) {
                case 2: // 把 byte 轉成 single
                        a1int_single((unsigned char) *pVal & 255, pSingl);  break;
                case 3: // 把 word 轉成 single
                        a1int_single((unsigned short) *pS16, pSingl);  break;
                case 4: // 把 dword 轉成 single
                        a1int_single((unsigned int) *pI, pSingl);  break;
                case 5: // 把 sint8 轉成 single
                        a1int_single((char) *pVal & 255, pSingl);  break;
                case 6: // 把 sint16 轉成 single
                        a1int_single(*pS16, pSingl);  break;
                case 7: // 7=int, 15=integer
                case 15: // 把 int 轉成 single
                        a1int_single(*pI, pSingl);  break;
                case 12: // 把 dobule 轉成 single (pVal 只有 32-bit 此處明顯有錯誤 !)
                        a1double_single((SuDoubl *) pVal, pSingl);  break;
                default: if (typ != final_type) xSay("#541 未設計此轉換 $%x --> $%x", typ, final_type);
            }
            break;
        case 13: // 轉成 PChar (13) -- 長度限制 256 bytes
            switch(typ) {
                case 1: // 把 char 轉成 PChar
                        pC = (char *) xmalloc(2);       pC[1] = 0;
                        *pC = *pVal;            *pI = (int) pC;
                        break;
                case 2: // byte 整數數值透過 IntToStr(n) 變成 PChar !
                        sPrt(s, "%d", (unsigned char) *pVal);
                        i = xstrlen(s);         pC = (char *) xmalloc(i+1);
                        xstrcopy(pC, s);        *pI = (int) pC;
                        break;
                case 3: // word 整數數值透過 IntToStr(n) 變成 string -> PChar !
                        sPrt(s, "%d", (unsigned short) *pS16);
                        i = xstrlen(s);         pC = (char *) xmalloc(i+1);
                        xstrcopy(pC, s);        *pI = (int) pC;
                        break;
                case 4: // DWORD 整數數值透過 IntToStr(n) 變成 string -> PChar !
                        sPrt(s, "%d", (unsigned int) *pI);
                        i = xstrlen(s);         pC = (char *) xmalloc(i+1);
                        xstrcopy(pC, s);        *pI = (int) pC;
                        break;
                case 5: // sint8 整數數值透過 IntToStr(n) 變成 string -> PChar !
                        sPrt(s, "%d", *pVal);
                        i = xstrlen(s);         pC = (char *) xmalloc(i+1);
                        xstrcopy(pC, s);        *pI = (int) pC;
                        break;
                case 6: // sint16 整數數值透過 IntToStr(n) 變成 string -> PChar !
                        sPrt(s, "%d", *pS16);
                        i = xstrlen(s);         pC = (char *) xmalloc(i+1);
                        xstrcopy(pC, s);        *pI = (int) pC;
                        break;
                case 15: // integer
                case 7: // sint32 整數數值透過 IntToStr(n) 變成 string -> PChar !
                        sPrt(s, "%d", *pI);
                        i = xstrlen(s);         pC = (char *) xmalloc(i+1);
                        xstrcopy(pC, s);        *pI = (int) pC;
                        break;
                default: if (typ != final_type) xSay("#581 未設計此轉換 $%x --> $%x", typ, final_type);
            }
            break;
		case 15: // 轉成 C string (15)
			switch(typ) {
				case 6:	// sint16 整數數值變成 C string
						sPrt(s, "%d", (short) (*pI & 0xffff));
						i = xstrlen(s);         pC = (char *) xmalloc(i+1);
                        xstrcopy(pC, s);        *pI = (int) pC;
						break;
				default: if (typ != final_type) xSay("#591 未設計此轉換 $%x --> $%x", typ, final_type);
			}
			break;
        default: if (typ != final_type) xSay("#593 未設計此轉換 $%x --> $%x", typ, final_type);
    }
}

void chain_up_this_var(struct TNameStru *pNX)
{  // 從頭串起此同名變數, 變數有使用到時才做此動作, 以便節省運算
struct TNameStru *pN, *pHdr;
char    *v_name;
int     first_loc, i;  // cnt,

        pN = all_var;           v_name = pNX->pName;    pHdr = NULL;
        first_loc = 0;          // cnt = 0;
        if (xstrcmp(pN->pName, v_name) == 0) { // 第一個就吻合同名變數
                pHdr = pN;      pHdr->name_first = 0;   pHdr->name_next = -1;  // cnt ++;
                }
        for (i=1;i < now_var_count;i ++) {
                pN ++;
                if (xstrcmp(pN->pName, v_name) == 0) { // 吻合同名變數
                        if (pHdr == NULL) {  // 找到第一個同名變數
                                pHdr = pN;   pHdr->name_first = i;      first_loc = i;
                                pHdr->name_next = -1;   continue;  // cnt ++;
                                }
                        pHdr->name_first = first_loc;
                        pHdr->name_next = i;          // 記住此同名變數的位置
                        pHdr = pN;      // cnt ++;
                }
        }
        if (pHdr) pHdr->name_next = -1; // 代表結尾
        // if (cnt) xSay("此變數 %s 是唯一存在.", v_name); // 無其他同名變數
}

void check_for_info(unsigned char *pTk)
{  // 設置並初始變數
struct TMyForInfo *pFI;
unsigned char *pIT;     // 初值設定的起點
unsigned char *pTX;     // 初值的運算式
unsigned char *pIV;     // 迴圈下一值的運算式
unsigned char *pFC;     // 迴圈是否結束的運算式
int     i, loop_var, nFC_tokens, var_id;
char    bBad, bEach, bKeep, bNotKeep, bStep, bTo;

        find_current_for_info(); // pFI = 
        // pTX = NULL;	pIV = NULL;  pFC = NULL;
        // if (pFI) return;        // 已經有設定好了, 就可以直接返回 ! (可另存於 Callers 內部來加速)
        // 尚未設定 repeat .. until 資訊 !
        if (for_info_cnt > (max_for_info_cnt - 2)) { xSay("#562 for 資訊表已滿 !"); return; }
        pFI = all_for_info + for_info_cnt;
        pFI->prog_id = now_run_prog;    // 現在執行中的程式編號
        pFI->line_no = nowLineNo;       // 現在執行中的程式行號
        // set loop var
        xSay("#571 目前行號 %d ==> for: %d 個 tokens", nowLineNo, nowLineTokCnt);
        yHexDump(pTk, nowLineTokCnt << 2);
        pTk += 4;       bEach = 0;      bBad = 0;
        if (pTk[0] == RESV_TOKEN) { // 下個指令是 each 嗎 ? [RESV_TOKEN (0x96)]
            if (pTk[1] == 19) { // 是 each 沒錯 !
                bEach = 1;
                xSay("#577 ==> 有 each 指令");
            }
            pTk += 4;
        }
        // 此處應該是 <迴圈變數>
        if (pTk[0] != VAR_TOKEN) { // 此處必須是變數 (loop var)
            xSay("錯誤! for 後面必須有迴圈變數!");
            bSysErr = 1;        do_sys_error(errNoLoopVar, 0);          return;
        }
        loop_var = cReadDW(pTk, 0);     // 記住 <迴圈變數>
        pIT = pTk;      // 此處為初值設定的起點
        var_id = cReadInt(pTk, 1);      xSay("#585 Loop Var Ndx %d", var_id);
        pTk += 4;
        if (bEach) { // 此處應該是 "in"
            xSay("處理 each 後續 (此處應該是 in)");
            return;
        }
        // 此處應該是 "=" (前面是迴圈變數)
        if (pTk[0] != SYMB_TOKEN) bBad = 1;     // SYMB_TOKEN (0x92)
        else if (pTk[1] != 0x14) bBad = 1;      // 目前必須是 "="
        if (bBad) {
            xSay("迴圈變數後面應該要有 = ");
            bSysErr = 1;        do_sys_error(errLoopSyntax, 0);     return;
        }
        pTk += 4;       bKeep = 1;      i = 2;  // i = 需被分析的 Tokens 數
        // 此處應該是 <迴圈初值> 可能是運算式, 故搜尋至 "to" 之前的都算 !
//        var_id = cReadInt(pTk, 1);      xSay("#602 初值 Ndx %d", var_id);
        while (bKeep) { // 搜尋至 "downto"=10 (bTo=0), "to"=11 (bTo=1)
            if (pTk[0] == RESV_TOKEN) { // RESV_TOKEN (0x96)
                if (pTk[1] == 10) bTo = 0;      // 遇到 downto
                if (pTk[1] == 11) bTo = 1;      // 遇到 to
                bKeep = 0;
            }
            if (pTk[0] == 0) bKeep = 0;         // 已經沒有 Token 了 !
            pTk += 4;       i ++;
        };
        if (i < 3) { // 沒有初值就遇到 to/downto 或其他保留字
            xSay("此迴圈缺乏初值 !");
            bSysErr = 1;        do_sys_error(errNoInitVal, 0);  return;
        }
        // 取得初值的運算式
        xSay("#618 設定迴圈 <初值>");
        pTX = (unsigned char *) xmalloc(i << 2);        i --;
        xmemmove(pTX, pIT, i << 2);     // 取得初值的運算式
        yHexDump(pTX, i << 2);
        do_only_one_statement((int *) pTX, i << 2);     // 處理運算式
        xfree(pTX);     // pTX = NULL;	// 此時初值已經被指定到迴圈變數了 !
        // 接著處理 <終值>
        pIT = pTk;      // 記下 <終值> 的起點
        bKeep = 1;      bStep = 0;      i = 0;  // i = 需被分析的 Tokens 數
        while (bKeep) { // 搜尋至 "downto"=10 (bTo=0), "to"=11 (bTo=1)
            if (pTk[0] == RESV_TOKEN) { // RESV_TOKEN (0x96) [可能遇到 07 = do]
                if (pTk[1] == 12) bStep = 1;    // 遇到 Step
                bKeep = 0;
            }
            if (pTk[0] == 0) bKeep = 0;         // 已經沒有 Token 了 !
            pTk += 4;       i ++;
        };
        i --;
        if (i < 1) { // 沒有終值就遇到 step 或其他保留字
            xSay("此迴圈缺乏終值 !");
            bSysErr = 1;        do_sys_error(errNoEndVal, 0);   return;
        }
        xSay("#638 迴圈 <終值> tokens = %d", i);
        if (bStep) {  xSay("尚須解讀 step 值"); }
        // 產生迴圈是否結束的運算式
        nFC_tokens = i + 2;
        pFC = (unsigned char *) xmalloc(nFC_tokens << 2);
        cSetDW(pFC, 0, loop_var);               // <迴圈變數>
        if (bTo) cSetDW(pFC, 4, 0x21292);       // 大於 (to)
        else cSetDW(pFC, 4, 0x21392);           // 小於 (downto)
        xmemmove(pFC + 8, pIT, i << 2);         // <終值>
        // 產生迴圈變數下一值的運算式
        pIV = (unsigned char *) xmalloc(8);
        cSetDW(pIV, 0, loop_var);               // <迴圈變數>
        if (bTo) cSetDW(pIV, 4, 0x21692);       // ++ (to)
        else cSetDW(pIV, 4, 0x21792);           // -- (downto)
        // 紀錄 for 此行的進入位置
        pFI->tokenAddr = (void *) ((size_t) cur_func->pExec - ((nowLineTokCnt + 2) << 2));
        for_info_cnt ++;        // bStep = 0;
        if (! bSysErr) do {
                // 執行 <迴圈是否結束的運算式>
                do_only_one_statement((int *) pFC, nFC_tokens << 2);    // 處理運算式
                bNotKeep = val2[0];
                xSay("#651 迴圈 <是否結束的判斷值> ans = %d (0=留在迴圈內)", bNotKeep);
                if (! bNotKeep) {
                        if (pTk[0] == BLOCK_TOKEN) exec_this_small_block(cReadInt(pTk, 1)); // 執行 do 後面的程式 (切出區塊程式)
                        do_only_one_statement((int *) pIV, 8);          // 產生 <迴圈變數> 下一值
                        }
                } while (! bNotKeep);
        for_info_cnt --;
        if (pFC) xfree(pFC);    // 釋放所借的空間
        if (pIV) xfree(pIV);    // 釋放所借的空間
        // if (pTX) xfree(pTX);    // 釋放所借的空間
}

void check_set_repeat_info(unsigned char level)
{ // 查詢是否已經設定好 repeat-until 資訊
struct TMyRepeatInfo *pRI;
int     i;

        pRI = find_current_repeat_info(level);		// 尋找目前的 repeat-until info
        if (pRI) return;        // 已經有設定好了, 就可以直接返回 ! (可另存於 Callers 內部來加速)
        // 尚未設定 repeat .. until 資訊 !
        if (repeat_info_cnt > (max_repeat_info_cnt - 2)) { xSay("#566 repeat .. until 資訊表已滿 !"); return; }
        pRI = all_repeat_info + repeat_info_cnt;
        pRI->prog_id = now_run_prog;    // 現在執行中的程式編號
        pRI->level = level;			// 現在執行中的程式行號
        // 紀錄 repeat 此行的進入位置
        pRI->tokenAddr = (void *) ((size_t) cur_func->pExec - ((nowLineTokCnt + 2) << 2));
        repeat_info_cnt ++;
}

void check_while_expr(unsigned char *pToken)
{ // 檢查是否繼續執行
unsigned char *pFC, *pFC2;      // 迴圈是否結束的運算式
int     i, nFC_tokens;
char    bBad, bDo, bKeep;

// 目前只允許 while <expr> do <exec-block> 的格式
        pToken += 4;       bDo = 0;      bKeep = 1;     pFC = pToken;
        while (bKeep) { // 搜尋至 do 指令
            if (pToken[0] == RESV_TOKEN) { // 下個指令是 do 嗎 ? [RESV_TOKEN (0x96)]
                if (pToken[1] == 7) { // 是 do 沒錯 !
                        bDo = 1;      bKeep = 0;
                        xSay("#712 ==> 有 do 指令, Good !");
                }
            }
            if (pToken[0] == 0) bKeep = 0;      // 已經沒有 Token 了 !
            pToken += 4;
        }
        if (! bDo) {
            xSay("while 後面需要有 do !");
            bSysErr = 1;        do_sys_error(errWhileWithoutDo, 0);     return;
        }
        nFC_tokens = (pToken - pFC) >> 2;
        pFC2 = (unsigned char *) xmalloc(nFC_tokens << 2);
        xmemmove(pFC2, pFC, nFC_tokens << 2);
        // 檢查迴圈是否結束
        bLastCondTok = 1;       // 如果遇到 = 時, 需改成 do_opr_20_equal
        do_only_one_statement((int *) pFC2, nFC_tokens << 2);   // 處理運算式
        bKeep = val2[0];
        xSay("#723 迴圈 <是否結束的判斷值> ans = %d (NZ=留在迴圈內)", bKeep);
        while(bKeep) {
                if (pToken[0] == BLOCK_TOKEN) exec_this_small_block(cReadInt(pToken, 1)); // 執行 do 後面的程式 (切出區塊程式)
                bLastCondTok = 1;       // 如果遇到 = 時, 需改成 do_opr_20_equal
                do_only_one_statement((int *) pFC2, nFC_tokens << 2);   // 檢查迴圈是否結束
                bKeep = val2[0];
                }
        xfree(pFC2);
}

void clear(void *p, int len) {
	// ZeroMemory(p, len);		// for Windows
	xmemset(p, 0, len);	// Other systems.
}

void *convert_fun_prc(char *func_name)
{ // 用函數名稱查詢, 指到 TMyFunc or TMyProc
int	i;
struct TMyFunc *pFun;
struct TMyProc *pPro;

	pFun = my_fun;
	for (i = 0;i < func_cnt;i ++, pFun ++)
		if (xstrcmp(pFun->name, func_name) == 0) return(pFun);

	pPro = my_proc;
	for (i = 0;i < proc_cnt;i ++, pPro ++)
		if (xstrcmp(pPro->name, func_name) == 0) return(pPro);
        return(NULL);
}

int  cReadDW(void *p, int ofs)
{
char *pc;

pc = (char *) p;         return(*(int *)(pc + ofs));
}

int  cReadInt(void *p, int ofs)
{
short v, *pc;

pc = (short *) ((size_t) p + ofs);
return(*pc);
}

void cSetDW(void *p, int ofs, int val)
{
char *pc;

pc = (char *) p;         *((int *) (pc + ofs)) = val;
}

unsigned short decode_one_value_from_stacktop(char *val)
{	// 傳回 16-bit type code ! (ret: 0 = bad)
struct TConstStru *pC;
struct TNameStru *pN; //, *pNN;
char	*p;
int     ndx;

	if (nVars < 1) { xSay("do_sys_error(errOPD_empty, 0)");  return(0); } // 變數或數值不夠供運算式使用 !
	nVars --;		vTok = xVar[nVars];		ndx = (vTok >> 8) & 0xFFFF;    // Result := 0;
	switch (vTok & 255) {
		case VAR_TOKEN:
			// canSay = 1;
			pN = all_var;	pN += ndx;
			if (pN->name_next == 0) chain_up_this_var(pN);  // 從頭串起此同名變數
			if (pN->name_next) get_newest_var(pN);    // pNN =  從此變數位置之後, 嘗試取最新產生之變數
			if (now_fun_loc_count) {
		    // if (pN->flags & 0x20) {
				xSay("#863 run_core (688) 此變數 %s 有多重定義 ! 嘗試找最適合的值..", pN->pName);
				dbp(ndx, now_fun_loc_count, "<- ndx, now_fun_loc_count");
				// 嘗試先去區域變數庫找最適合目前程式使用的值 (自動填註 8 bytes 內容到 val^, 傳回 16-bit type code)
				if (now_use_locals) pN = (struct TNameStru *) try_match_local_var(pN);	// 看看有沒有跟 pN^ 所指的變數名稱相同的區域變數
			}
			xSay("#868 行號 %d, ndx= %d, flags= $%x, initNdx= %d", nowLineNo, ndx, pN->flags, pN->initNdx);
			// canSay = 0;
			if ((pN->initNdx > 0) && (((pN->vType & 31) == 10) || ((pN->vType & 31) == 13))) { // 字串常數且有初值
				p = get_fixval_as_str(pN->initNdx);
				xSay("#872 initNdx = %d, flags = %d, p = %x (%s)", pN->initNdx, pN->flags, p, p);
				xmemmove(&pN->i_Val, &p, 4);
				//      xfree(val);  @@bug ! p was not free @@
			}
			/* if (ndx == 2) { xSay("#864 @@bug ! Const Str ! dump iVal[8] -- ");
				xmemmove(val, &pN->i_Val, 8);	// 取得連續 8 bytes !
				yHexDump(val, 16);
				} */
			xmemmove(val, &pN->i_Val, 8);	// 取得連續 8 bytes !
 			return(pN->vType);			// 傳回 16-bit type code !

		case CONST_TOKEN:
			pC = all_const;		pC += (ndx - 1);	// !! 常數表索引由一算起 !!
			xmemmove(val, &pC->i_Val, 8);	// 取得連續 8 bytes !
			return(pC->cType);			// 傳回 16-bit type code !
		case TEMP_STR_TOKEN:
		canSay = 1;
			xSay("TEMP_STR_TOKEN, ndx = %d (%s)", ndx, temp_str[ndx]);
			cSetDW(val, 0, (int) temp_str[ndx]);	// 127 個暫存字串之一 !
			cSetDW(val, 4, 0);
			return(Type_Code_PCHAR);	// 傳回 16-bit type code !
        default: xSay("錯誤: 尚未取得數值 ! token = $%x", vTok);
	}
	return(0);
}

void do_all_prog_const_init(void)
{
int	i, n;
struct TMyFixVal *pFV;
struct TNameStru *pN, *pN2;
struct TConstStru *pC;
void *p;

	// 設定所有程式的常數變數初值
	pN = all_var;	xSay("#890 now_var_count = %d", now_var_count);
	for (i=0;i < now_var_count;i ++) {
		pN->name_next = 0;		pN->name_first= 0;
        if (pN->initNdx > 0) {
			pN2 = pN;	// 暫存 pN
			if (pN->flags & 0x20) {
				xSay("#896 do_all_prog_const_init 此變數有多重定義: %s", pN->pName);
				// 嘗試先去區域變數庫找最適合目前程式使用的值 (自動填註 8 bytes 內容到 val^, 傳回 16-bit type code)
				if (now_use_locals) pN = (struct TNameStru *) try_match_local_var(pN);	// 看看有沒有跟 pN^ 所指的變數名稱相同的區域變數
			}
			pC = all_const;
			pC += (pN->initNdx - 1);     // !! 常數表索引由一算起 !!
			// xSay("#646 %s typ=%d %d", pC->pName, pC->cType, pN->vType);
			switch(pC->cType) {
				case CT_STRING:  // 15=CT_STRING 借取 256 bytes 字串空間 or StrLen(pC^.pName) > 255 時, 改成 (256 + 16*n) bytes.
					pFV = mainProg->all_fixval;     pFV += (pN->initNdx - 1);
					n = xstrlen(pFV->initVal);
					if (n < 256) n = 256;
					else n = (n & 0xFFF0) + 16;	// 上限 65535 字元
					p = xmalloc(n);   xstrcopy((char *) p, pC->pName);
					pN->i_Val = (int) p;		// bug for 64-bit !
					break;
				case CT_CHAR: pN->i_Val = (int) pC->pName;		break;	// 1=CT_CHAR 直取 4 bytes !
				case CT_DOUBLE: // 11
                case CT_QWORD: // 8
                case CT_S64: // 9
					// 直取 8 bytes !
					pN->i_Val = pC->i_Val;
					p = &pN->d_Val;
					cSetDW(p, 0, pC->i_Val);
					cSetDW(p, 4, (int) pC->pName);
					break;
				default: pN->i_Val = pC->i_Val;
			}
			pN = pN2;	// 復原 pN
		}	// -- if
                pN ++;
	}	// -- for
    sort_global_vars();	// 針對全域變數/常數/陣列排序, 數值資料取用時, 優先由函數內局部變數取用, 之後再找相同 PrgID 之數值, 最後才找全域性
    if (!sys_argv) sys_argv = (struct TNameStru *) xmalloc(sizeof(struct TNameStru) * 16); // 上限 16 個
    clear(loc_stack, MAX_LOCAL_STACK << 2);	loc_stack_now = 0;
	copy_temp_str(temp_str);	// 一次拷貝整個 temp_str[128] (in su_blob.c)
    bUpToGo = 0;	xSay("#932 do_all_prog_const_init ok.. <%s> <%s>", temp_str[0], temp_str[1]);
}

int  do_break_pts(int line_no, int job)
{ // 修改 Token 內容, 以便 { 清除/設置中斷點 }
struct TMyFunc		*pFn;	// all_func, func_cnt Set_BreakPoint
struct TMyProc		*pPr;	// all_proc, proc_cnt 
struct TMyExcBlk	*pExcBlk;	// mainProg->all_excblk, mainProg->cnt_excblk
struct TMySmallBlk	*pMSB;	// mainProg->all_smblk, mainProg->cnt_smblk
int		i, bERR;
xSay("#731 set BP at %d, job = %d (%d , %d)", line_no, job, func_cnt, proc_cnt);
    bERR = 1;	// 0 = OK, NZ= Failed !
	// 假設都在同一個 prog_id !!
	// 檢查所有函數
	pFn = my_fun;	
	for (i = 0;i < func_cnt;i ++) {
		// xSay("#717  (%d , %d)",pFn->bgn_ln, pFn->fin_ln);
		if (line_no < 0)
			bERR = tk_job_every_line((unsigned char *) pFn->tokenAddr, job, pFn->token_len); 	// 代表每行都要處理..
		else // 只處理某一行
			if ((pFn->bgn_ln <= (unsigned int) line_no) && ((unsigned int) line_no <= pFn->fin_ln)) 	// 此行在範圍內 !
				bERR = tk_job_one_line((unsigned char *) pFn->tokenAddr, job, line_no, pFn->token_len);
		pFn ++;
	}
	// 檢查所有程序
	pPr = my_proc;	
	for (i = 0;i < proc_cnt;i ++) {
		// xSay("#727  (%d , %d)",pPr->bgn_ln, pPr->fin_ln);
		if (line_no < 0)
			bERR = tk_job_every_line((unsigned char *) pPr->tokenAddr, job, pPr->token_len); 	// 代表每行都要處理..
		else // 只處理某一行
			if ((pPr->bgn_ln <= (unsigned int) line_no) && ((unsigned int) line_no <= pPr->fin_ln)) 	// 此行在範圍內 !
				bERR = tk_job_one_line((unsigned char *) pPr->tokenAddr, job, line_no, pPr->token_len);
		pPr ++;
	}
	// 檢查切出的程式區塊 (呼叫外部函數)  個數: mainProg->cnt_excblk
	if(! mainProg) return(bERR);		// 0 = OK
	pExcBlk = mainProg->all_excblk;		
	if(pExcBlk) for (i = 0;i < (int) mainProg->cnt_excblk;i ++) {
		// xSay("#745  (line no = %d)",pExcBlk->line_no);
		if (line_no < 0)
			bERR = tk_job_one_line((unsigned char *) pExcBlk->tokenAddr, job, line_no, pExcBlk->token_len); 	// 代表每行都要處理..
		else // 只處理某一行
			if (pExcBlk->line_no == (unsigned int) line_no) 	// 此行在範圍內 !
				bERR = tk_job_one_line((unsigned char *) pExcBlk->tokenAddr, job, line_no, pExcBlk->token_len);
		pExcBlk ++;
	}
	// 檢查切出的程序小區塊資訊  個數: mainProg->cnt_smblk
	pMSB = mainProg->all_smblk;
	if(pMSB) for (i = 0;i < (int) mainProg->cnt_smblk;i ++) {
		// xSay("#757  (程序小區塊 no = %d)", i);
		if (line_no < 0)
			bERR = tk_job_every_line((unsigned char *) pMSB->tokenAddr, job, pMSB->token_len); 	// 代表每行都要處理..
		else // 只處理某一行, 此行可能在範圍內 !
			bERR = tk_job_one_line((unsigned char *) pMSB->tokenAddr, job, line_no, pMSB->token_len);
		pMSB ++;
	}
	return(bERR);		// 0 = OK
}

void do_only_one_statement(int *pTok, int len)
{
int		i, j;
int		*pI;
char	bNotOperated;	// boolean

	mark(F_do_only_one_statement, F_Enter);
	xSay("#220 pTok = ");	yHexDump(pTok, len);
	nAnsToken = 0;	// make_RPN 會改變 nAnsToken
	make_RPN(pTok, len >> 2);		xSay("nAns = %d", nAnsToken);
	if (!nAnsToken) { mark(F_do_only_one_statement, F_Bye + 1);  return; }	// 無須運算
	// bLastCondTok = 0;			// 此行未遇到保留字:控制令牌 (if, then, do, while, repeat ..)	
	// RPN 轉換並不會改變令牌數目, 若改變令牌數目, 則代表運算式轉換過程出錯誤 (內部錯誤)
	// if nAnsToken <> (len shr 2) then begin do_sys_error(errWhenRPN, 0);  Exit;  end;
	// 把變數跟運算子分別推入堆疊中
	bVal2Valid = 0;		// 剛開始堆疊頂端是沒有數值的, 亦即 val2 是無效的 !
	bNotOperated = 1;	// 萬一都沒運算, 至少把第一個參數放到 val2 (堆疊頂端)
	nVars = 0;		pI = pAnsTokens;		final_type = 255;
	for (i = 0;i < nAnsToken;i ++) {
		j = *pI;
		if (((j & 255) == SYMB_TOKEN) || ((j & 255) == TYPE_TOKEN)) {
			// 依照運算子的數學運算需求, 從變數堆疊頂端彈出一或兩個變數
			switch((j >> 8) & 255) {	// 參閱 calx_ana.pas 的 symbols[21], symbol2[25] 的擺放順序
				case 8: do_opr_08_mult();	break;		// 乘法
				case 9: do_opr_09_div();	break;		// 除法
				case 18: do_opr_18_greater();   break;		// 大於
				case 19: do_opr_19_lesser();    break;		// 小於
				case 20: if (bLastCondTok) do_opr_20_equal(); else do_opr_20_assign();	// 等號: 比值 or 設值
					bNotOperated = 0;	// 確定執行完成
					break;
				case 22: do_opr_22_plusplus();  break;          // 加加
				case 23: do_opr_23_minusminus();    break;      // 減減
				case 24: do_opr_24_add();	break;		// 加法
				case 25: do_opr_25_sub();	break;		// 減法
				case 38: do_opr_20_equal();	break;		// 是 == (相等比較)
				default: xSay("未處理的運算子 $%x", j);
			}
            if (bSysErr) break;	// 運算過程有嚴重錯誤 !
		}
        else {
            if (nVars > 94) { xSay("do_sys_error(errOPR_full, 0)");  mark(F_do_only_one_statement, F_Bye + 2);  return; }
            xVar[nVars] = j;		nVars ++;
		}
        pI ++;
	}
	// 萬一都沒運算, 至少把第一個參數放到 val2 (堆疊頂端)
	if (bNotOperated) fetch_one_value();
	mark(F_do_only_one_statement, F_Bye);
}

void do_opr_08_mult(void)
{ // 乘法運算
	// !! bug: 左值 sint16 (5) * 右值 single (11) 會變成 結果是左值 sint16 (5)
// SuDoubl	*pDoub1, *pDoub2;
// SuFloat	*pSing1, *pSing2;
int		*pI1, *pI2;
int		v1, v2;
unsigned int	d1, d2;

	fetch_two_values();	// 需要兩個數值
	do_type_casting();	// 採用型態轉換表來轉換兩個數值 !
	pI1 = (int *) val1;			pI2 = (int *) val2;
	switch(final_type) { // 計算的值請放在第 2 個位置 (= stack top)
		case 4: // dword
			d1 = *pI1;     d2 = *pI2;     d2 = d1 * d2;
			*pI2 = d2;
			break;
		case 5: *pI2 = (*pI1) * ((int) val2[0]);		break;		// signed int8
		case 6: *pI2 = (*pI1) * ((short) ((*pI2) & 0xFFFF));		break;	// signed int16
		case 7: // signed int32
			v1 = *pI1;		v2 = *pI2;		*pI2 = v1 * v2;
			break;
/*		case 11: // single
			pSing1 = (float *) val1;		pSing2 = (float *) val2;
			*pSing2 = (*pSing1) * (*pSing2);
			break;
		case 12: // double
			pDoub1 = (double *) val1;		pDoub2 = (double *) val2;
			*pDoub2 = (*pDoub1) * (*pDoub2);
			break;       */
		default: xSay("乘法 運算 -- 尚未設計此類運算: %d, %d", typ1, typ2);
	}
	typ2 = final_type;		bVal2Valid = 1;
	xSay("乘法 運算後, 右值: %s", ret_value_str(final_type, val2));
}

void do_opr_09_div(void)
{ // 除法運算
// double	*pDoub1, *pDoub2;
// float	*pSing1, *pSing2;
int		*pI1, *pI2;
int		v1, v2;
unsigned int	d1, d2;

	fetch_two_values();	// 需要兩個數值
	do_type_casting();	// 採用型態轉換表來轉換兩個數值 !
	pI1 = (int *) val1;			pI2 = (int *) val2;
	switch(final_type) { // 計算的值請放在第 2 個位置 (= stack top)
		case 4: // dword
			d1 = *pI1;     d2 = *pI2;     d2 = d1 / d2;
			*pI2 = d2;
			break;
		case 5: *pI2 = (*pI1) / ((int) val2[0]);		break;		// signed int8
		case 6: *pI2 = (*pI1) / ((short) ((*pI2) & 0xFFFF));		break;	// signed int16
		case 7: // signed int32
			v1 = *pI1;		v2 = *pI2;		*pI2 = v1 / v2;
			break;
/*		case 11: // single
			pSing1 = (float *) val1;		pSing2 = (float *) val2;
			*pSing2 = (*pSing1) / (*pSing2);
			break;
		case 12: // double
			pDoub1 = (double *) val1;		pDoub2 = (double *) val2;
			*pDoub2 = (*pDoub1) / (*pDoub2);
			break;     */
		default: xSay("除法 運算 -- 尚未設計此類運算: %d, %d", typ1, typ2);
	}
	typ2 = final_type;		bVal2Valid = 1;
	xSay("除法 運算後, 右值: %s", ret_value_str(final_type, val2));
}

void do_opr_18_greater(void)
{ // 大於運算
	// !! bug: 左值 sint16 (5) * 右值 single (11) 會變成 結果是左值 sint16 (5)
// SuDoubl	*pDoub1, *pDoub2;
// SuFloat	*pSing1, *pSing2;
int		*pI1, *pI2;
int		v1, v2;
unsigned int	d1, d2;

	fetch_two_values();	// 需要兩個數值
	do_type_casting();	// 採用型態轉換表來轉換兩個數值 !
	pI1 = (int *) val1;			pI2 = (int *) val2;
	switch(final_type) { // 計算的值請放在第 2 個位置 (= stack top)
		case 4: // dword
			d1 = *pI1;     d2 = *pI2;
			if (d1 > d2) *pI2 = 1;
                        else *pI2 = 0;
			break;
		case 5: if (val1[0] > val2[0]) *pI2 = 1;    else *pI2 = 0;      break; 	// signed int8
		case 6: if (((short) ((*pI1) & 0xFFFF)) > ((short) ((*pI2) & 0xFFFF))) *pI2 = 1;
                        else *pI2 = 0;  break;	// signed int16
		case 7: // signed int32
                        if (*pI1 > *pI2) *pI2 = 1;
                        else *pI2 = 0;
			break;
/*		case 11: // single
			pSing1 = (float *) val1;		pSing2 = (float *) val2;
			break;
		case 12: // double
			pDoub1 = (double *) val1;		pDoub2 = (double *) val2;
			break;       */
		default: xSay("大於 運算 -- 尚未設計此類運算: %d, %d", typ1, typ2);
	}
	typ2 = final_type;		bVal2Valid = 1;
	xSay("大於 運算後, 右值: %s", ret_value_str(final_type, val2));
}

void do_opr_19_lesser(void)
{ // 小於運算
	// !! bug: 左值 sint16 (5) * 右值 single (11) 會變成 結果是左值 sint16 (5)
// SuDoubl	*pDoub1, *pDoub2;
// SuFloat	*pSing1, *pSing2;
int		*pI1, *pI2;
int		v1, v2;
unsigned int	d1, d2;

	fetch_two_values();	// 需要兩個數值
	do_type_casting();	// 採用型態轉換表來轉換兩個數值 !
	pI1 = (int *) val1;			pI2 = (int *) val2;
	switch(final_type) { // 計算的值請放在第 2 個位置 (= stack top)
		case 4: // dword
			d1 = *pI1;     d2 = *pI2;
			if (d1 < d2) *pI2 = 1;
                        else *pI2 = 0;
			break;
		case 5: if (val1[0] < val2[0]) *pI2 = 1;    else *pI2 = 0;      break; 	// signed int8
		case 6: if (((short) ((*pI1) & 0xFFFF)) < ((short) ((*pI2) & 0xFFFF))) *pI2 = 1;
                        else *pI2 = 0;  break;	// signed int16
		case 7: // signed int32
                        if (*pI1 < *pI2) *pI2 = 1;
                        else *pI2 = 0;
			break;
/*		case 11: // single
			pSing1 = (float *) val1;		pSing2 = (float *) val2;
			break;
		case 12: // double
			pDoub1 = (double *) val1;		pDoub2 = (double *) val2;
			break;       */
		default: xSay("小於 運算 -- 尚未設計此類運算: %d, %d", typ1, typ2);
	}
	typ2 = final_type;		bVal2Valid = 1;
	xSay("小於 運算後, 右值: %s", ret_value_str(final_type, val2));
}

void do_opr_20_assign(void)
{ // 等號設值運算 (需把運算完的 final_type 轉型成左值的型態, 再把堆疊頂端 (val2) 的值轉換, 設定到左值變數)
struct TNameStru *pN;
int		i;
	xSay("(opr =) final_type = $%x", final_type);
	if (final_type == 255) {
		fetch_two_values();	// 指定值至少需要一個數值
		do_type_casting();	// 採用型態轉換表來轉換兩個數值 !
		xSay("型態轉換後 final_type = $%x", final_type);
	}
	else {
		decode_one_value_from_stacktop(val1);	// 得知要存到哪..
		vToken1 = vTok;
	}
	xSay("要存到這裏 token1 = $%x", vToken1);
	i = (vToken1 >> 8) & (MAX_VAR_COUNT - 1);	// 取得左值變數
	pN = all_var;		pN += i;
	if ((pN->flags & 0x40) == 0) {
        xSay("錯誤: %s 常數不能改變其值 !", pN->pName);
		do_sys_error(errChangeConst, 0);	return;
	}
	if (final_type != 255) {
		typ1 = pN->vType;
		do_stktopVal_type_casting();		// 採用型態轉換表來轉換堆疊頂端數值 !
		xSay("堆疊頂端型態轉換後 final_type = $%x", final_type);
	}
	if (pN->flags & 0x20) {
		xSay("#1231 run_core (599) 此變數 %s 有多重定義: ", pN->pName);
		if (now_use_locals) pN = (struct TNameStru *) try_match_local_var(pN);	// 看看有沒有跟 pN^ 所指的變數名稱相同的區域變數
		xSay(" = 區變目標 pN --");	yHexDump(pN->pName, 32);
	}
	xSay("#1235 vType = %d ($%x)", pN->vType, pN->vType);
	switch (pN->vType & MAX_TYPE_MASK) { // vType & 31
		// case final_type of // 計算的值請放在第 1 個位置 (= 左值)
		case 1:
        case 2:
        case 5: pN->i_Val = (unsigned char) val2[0];		break;	// char, byte, sint8
		case 3:
        case 6: pN->i_Val = *((unsigned short *) val2);		break;	// word, sint16, dblchar
		case 4:
        case 7:
        case 15: pN->i_Val = cReadDW(val2, 0);	break;	// dword, int, integer
		case 8:
        case 9:
        case 11:
        case 13:
        case 14: xmemmove((void *) pN->i_Val, val2, 4);	break;	// dblchar, unicode, string, single, pchar, ptr
		case 10: // get a pascal string (in val2) .. to .. pN->i_Val
				yHexDump(val2, 12);		yHexDump((void *) pN->i_Val, 12);
                if (safe_ptr(881, val2, sRValue)) { // val2 is OK.
					//if (! safe_ptr(881, (void *) pN->i_Val, sLValue)) { // dest location is bad..
						//if (pN->flags & 0x40) pN->i_Val = (size_t) xmalloc(256);
                                //}
                    xmemmove((void *) &pN->i_Val, val2, 4);
                }
			break;
		case 12:
        case 16:
        case 17: xmemmove((void *) &pN->d_Val, val2, 8);	break;	// double, sint64, qword
		default: xSay("= 運算 -- 尚未設計此類運算: %d, %d", typ1, typ2);
	}
	typ2 = final_type;		bVal2Valid = 1;
	xSay(" =  運算後, 左值: %s", ret_value_str(pN->vType & MAX_TYPE_MASK, (char *) &pN->i_Val));
	set_watch_var(pN, vToken1 >> 8);		// 更新結果顯示
}

void do_opr_20_equal(void)
{ // 等號數值比較運算, 需把比較完的結果設定到堆疊頂端 (val2), 再把堆疊頂端 (val2) 的值轉換, 設定到左值變數
struct TNameStru *pN;
int		i;
	xSay("(opr =) final_type = $%x", final_type);
	if (final_type == 255) {
		fetch_two_values();	// 指定值至少需要一個數值
		do_type_casting();	// 採用型態轉換表來轉換兩個數值 !
		xSay("型態轉換後 final_type = $%x", final_type);
	}
	else {
		decode_one_value_from_stacktop(val1);	// 得知要存到哪..
		vToken1 = vTok;
	}
	xSay("等號比較結果要存到堆疊頂端");	// 結果要存到 val2
	i = (vToken1 >> 8) & (MAX_VAR_COUNT - 1);	// 取得左值變數
	pN = all_var;		pN += i;
	if (pN->flags & 0x20) {
		xSay("run_core (599) 此變數 %s 有多重定義: ", pN->pName);
		if (now_use_locals) pN = (struct TNameStru *) try_match_local_var(pN);	// 看看有沒有跟 pN^ 所指的變數名稱相同的區域變數
		xSay(" = 區變目標 pN-- ");		yHexDump(pN->pName, 32);
	}
	switch(final_type) { // 計算的結果請放在第 2 個位置 (= stack top)
		case 1:
        case 2:
        case 5: if ((pN->i_Val & 255) == val2[0]) cSetDW(val2, 0, 1);  else cSetDW(val2, 0, 0);	break;	// char, byte, sint8
		case 3:
        case 6: if ((pN->i_Val & 0xFFFF) == *((unsigned short *) val2)) cSetDW(val2, 0, 1);  else cSetDW(val2, 0, 0);
			break;	// word, sint16, dblchar
		case 4:
        case 7:
        case 15: if (pN->i_Val == cReadDW(val2, 0)) cSetDW(val2, 0, 1);  else cSetDW(val2, 0, 0);	break;	// dword, int, integer
//        8, 9, 11, 13, 14: -- 字串比較     // dblchar, unicode, string, single, pchar, ptr
//		case 12, 16, 17: if (pN^.d_val = get_double_val(2)) then cSetDW(@val2[0], 0, 1) else cSetDW(@val2[0], 0, 0);	// double, sint64, qword
		default: xSay("vtype = $%x", pN->vType & MAX_TYPE_MASK);
	}
	typ2 = 7;		bVal2Valid = 1;			bLastCondTok = 0;
	xSay("= 比值運算後, 堆疊頂端值: %s", ret_value_str(7, val2));
}

void do_opr_22_plusplus(void)
{ // 加加運算
// double	*pDoub2;
// float	*pSing2;
short   s2;
int	v2, *pI2;
unsigned int	d2;

	fetch_one_value();	// 需要一個數值 (放在 val2)
	xSay("右值型態: $%x, 值: %s", typ2, ret_value_str(typ2, val2));
	pI2 = (int *) val2;     final_type = typ2 & 31;
	switch(final_type) { // 計算的值請放在第 2 個位置 (= stack top)
		case 4: d2 = *pI2;      d2 ++;	    *pI2 = d2;	break;  // dword
		case 5: val2[0] ++;	break;  // signed int8
		case 6: s2 = *pI2 & 0xFFFF;     s2 ++;	*pI2 = s2;      break;   // signed int16
		case 7: v2 = *pI2;      v2 ++;	    *pI2 = v2;	break;  // signed int32
/*		case 11: // single
			pSing1 = (float *) val1;		pSing2 = (float *) val2;
			*pSing2 = (*pSing1) + (*pSing2);
			break;
		case 12: // double
			pDoub1 = (double *) val1;		pDoub2 = (double *) val2;
			*pDoub2 = (*pDoub1) + (*pDoub2);
			break;       */
		default: xSay("加加 運算 -- 尚未設計此類運算: %d", typ2);
	}
	typ2 = final_type;		bVal2Valid = 1;
        nVars = 1;      do_opr_20_assign();
	xSay("加加 運算後, 右值: %s", ret_value_str(final_type, val2));
}

void do_opr_23_minusminus(void)
{ // 減減運算
// double	*pDoub2;
// float	*pSing2;
short   s2;
int	v2, *pI2;
unsigned int	d2;

	fetch_one_value();	// 需要一個數值 (放在 val2)
	xSay("右值型態: $%x, 值: %s", typ2, ret_value_str(typ2, val2));
	pI2 = (int *) val2;     final_type = typ2 & 31;
	switch(final_type) { // 計算的值請放在第 2 個位置 (= stack top)
		case 4: d2 = *pI2;      d2 --;	    *pI2 = d2;	break;  // dword
		case 5: val2[0] --;	break;  // signed int8
		case 6: s2 = *pI2 & 0xFFFF;     s2 --;	*pI2 = s2;      break;   // signed int16
		case 7: v2 = *pI2;      v2 --;	    *pI2 = v2;	break;  // signed int32
/*		case 11: // single
			pSing1 = (float *) val1;		pSing2 = (float *) val2;
			*pSing2 = (*pSing1) + (*pSing2);
			break;
		case 12: // double
			pDoub1 = (double *) val1;		pDoub2 = (double *) val2;
			*pDoub2 = (*pDoub1) + (*pDoub2);
			break;       */
		default: xSay("減減 運算 -- 尚未設計此類運算: %d", typ2);
	}
	typ2 = final_type;		bVal2Valid = 1;
        nVars = 1;      do_opr_20_assign();
	xSay("減減 運算後, 右值: %s", ret_value_str(final_type, val2));
}

void do_opr_24_add(void)
{ // 加法運算
// double	*pDoub1, *pDoub2;
// float	*pSing1, *pSing2;
char	*p1, *p2, *p3;
int		*pI1, *pI2;
int		v1, v2;
unsigned int	d1, d2;
char	cs[2], cc;
//   s1: string;
	fetch_two_values();	// 需要兩個數值
	do_type_casting();	// 採用型態轉換表來轉換兩個數值 !
	pI1 = (int *) val1;			pI2 = (int *) val2;
	switch(final_type) { // 計算的值請放在第 2 個位置 (= stack top)
		case 0: // 一個是字串,一個是字元 (此處怪怪的,不懂 val1 val2 放的值是 ??)
			cs[1] = 0;
			if ((typ1 == Type_Code_PCHAR) || (typ1 == Type_Code_PTR)) {
				p1 = (char *) pI1;		cs[0] = val2[0];   }
			else { p1 = (char *) pI2;  cs[0] = val1[0];	}
			p2 = (char *) xmalloc(xstrlen(p1) + 1);
			xstrcopy(p2, p1);	xstrcat(p2, cs);	xfree(pI2);
			*pI2 = (int) p2;	xSay("#368 字串加法結果: %s", p2);	// !!@@ 32/64 bit bug @@!!
			break;
		case 4: // dword
			d1 = *pI1;     d2 = *pI2;     *pI2 = d1 + d2;
			break;
		case 5: *pI2 = (*pI1) + ((int) val2[0]);		break;   // signed int8
		case 6: *pI2 = (*pI1) + ((short) ((*pI2) & 0xFFFF));		break;   // signed int16
		case 7: // signed int32
			v1 = *pI1;		v2 = *pI2;		*pI2 = v1 + v2;
			break;
		case 10: // 兩個 pascal string 相加
			p1 = (char *) (*pI1);		p2 = (char *) (*pI2);
			v1 = (unsigned char) *p1;	v2 = (unsigned char) *p2;	// = strlen
			p3 = (char *) xmalloc(v1 + v2 + 1);
			xmemmove(p3, p1+1, v1);			// get str1 -> buf
			xmemmove(p3 + v1, p2+1, v2);	// get str2 -> after buf
			p3[v1 + v2] = 0;	// add \0 to string tail !
			if ((v1 + v2) > 255) { // !!@@ 目前我限制 Pascal string max 256 字元 @@!!
				xmemmove(p2+1, p3, 255);	p2[0] = 255;	// 寫回第 2 個位置 (= stack top)
			}
			else {
				v2 = v1 + v2;
				xmemmove(p2+1, p3, v2);		p2[0] = v2;		// 寫回第 2 個位置 (= stack top)
			}
			xSay("#1405 字串加法結果: %s", p2);
			break;
/*		case 11: // single
			pSing1 = (float *) val1;		pSing2 = (float *) val2;
			*pSing2 = (*pSing1) + (*pSing2);
			break;
		case 12: // double
			pDoub1 = (double *) val1;		pDoub2 = (double *) val2;
			*pDoub2 = (*pDoub1) + (*pDoub2);
			break;       */
		case 15:
		case 13: // 兩個 PChar 相加
			p1 = (char *) (*pI1);		p2 = (char *) (*pI2);
			// su_buf 先借用一下
			xstrcopy(su_buf, p1);		xstrcat(su_buf, p2);	v1 = xstrlen(su_buf) + 7;
			if (v1 > 256) v1 = 256;		p2 = (char *) xmalloc(v1);		xstrcopy(p2, su_buf);
			cSetDW(val2, 0, (size_t) p2);	// !!@@ 32/64 @@!!
			xSay("#1421 字串加法結果: %s", p2);
			break;
		default: xSay("#1423 加法 運算 -- 尚未設計此類運算: %d, %d", typ1, typ2);
	}
	typ2 = final_type;		bVal2Valid = 1;
	xSay("加法 運算後, 右值: %s", ret_value_str(final_type, val2));
}

void do_opr_25_sub(void)
{ // 減法運算
// double	*pDoub1, *pDoub2;
// float	*pSing1, *pSing2;
int		*pI1, *pI2;
int		v1, v2;
unsigned int	d1, d2;

	fetch_two_values();	// 需要兩個數值
	do_type_casting();	// 採用型態轉換表來轉換兩個數值 !
	pI1 = (int *) val1;			pI2 = (int *) val2;
	switch(final_type) { // 計算的值請放在第 2 個位置 (= stack top)
		case 4: // dword
			d1 = *pI1;     d2 = *pI2;     *pI2 = d1 - d2;
			break;
		case 5: *pI2 = (*pI1) - ((int) val2[0]);		break;   // signed int8
		case 6: *pI2 = (*pI1) - ((short) ((*pI2) & 0xFFFF));		break;    // signed int16
		case 7: // signed int32
			v1 = *pI1;		v2 = *pI2;		*pI2 = v1 - v2;
			break;
/*		case 11: // single
			pSing1 = (float *) val1;		pSing2 = (float *) val2;
			*pSing2 = (*pSing1) - (*pSing2);
			break;
		case 12: // double
			pDoub1 = (double *) val1;		pDoub2 = (double *) val2;
			*pDoub2 = (*pDoub1) - (*pDoub2);
			break;       */
		default: xSay("減法 運算 -- 尚未設計此類運算: %d, %d", typ1, typ2);
	}
	typ2 = final_type;		bVal2Valid = 1;
	xSay("減法 運算後, 右值: %s", ret_value_str(final_type, val2));
}

void do_prepare_to_run(void)
{   // 為最初跳進去 start() 的過程作準備
     callerStack = 0;          xSay("#1050 do_prepare_to_run.");
	 if (callers) xfree(callers);
     callers = (struct TCaller *) xmalloc(MAX_CALLERS_SIZE);
     clear(loc_list, MAX_LOCAL_POOL_COUNT << 2);	// 放新借區變之 token
     if (all_run_loc) xfree(all_run_loc);       now_locv = 0;
     all_run_loc = (struct TNameStru *) xmalloc(MAX_LOCAL_POOL_COUNT * sizeof(struct TNameStru));
     now_run_loc = all_run_loc;
	 cur_func = (struct TCaller *) xmalloc(sizeof(struct TCaller));
     cur_func->cur_rolling_no = get_rolling_no();   // 取得函數滾動值
	 process_UnSetBPs();	// 處理未被處理的中斷點 !
     // 尚未設計執行區域變數設定與初始化 !!
     xSay("#1060 do_prepare_to_run: 將於 run_a_func 設計執行區域變數設定與初始化 !!");
	 add_watch(0x040104, (int) &nowLineNo, "nowLineNo");	// hash = data_len, id, type_flag
	 add_watch(0x020102, (int) &typ1, "typ1");	    // hash = data_len, id, type_flag
	 add_watch(0x080101, (int) &val1, "val1[8]");	// hash = data_len, id, type_flag
	 add_watch(0x020102, (int) &typ2, "typ2");	    // hash = data_len, id, type_flag
	 add_watch(0x080101, (int) &val2, "val2[8]");	// hash = data_len, id, type_flag
	 // add_watch(0x040104, (int) &sys_argn, "sys_argn");	// hash = data_len, id, type_flag
	 // add_watch(0x14010B, (int) &sys_argv, "sys_argv");	// hash = data_len, id, type_flag
}

void do_run_core_init(void)
{   // 初始化 (Robot 程式剛 created 時執行)
	if (! brkps) {	brkps = (struct BRK_PTS *) xmalloc(sizeof(struct BRK_PTS) * MAX_BREAK_PTS);	nBrkPts = 0; }
	pss = (struct TProgStru *) xmalloc(sizeof(struct TProgStru) * MAX_PROGRAMS);
    func_stru = (struct TFuncStru *) xmalloc(sizeof(struct TFuncStru) * MAX_FUNCTIONS);
    func_stru_cnt = 0;      // func_stru 的已使用個數
	all_array = (struct TArrayStru *) xmalloc(ARRAY_STRU_SPACE_SIZE);
	now_array = all_array;		array_count = 0;	// now_array 指到現在 free 可以被使用的 TArrayStru
	all_const = (struct TConstStru *)  xmalloc(CONST_SPACE_SIZE);			// now_const 指到現在 free 可以被使用的 TConstStru
	now_const = all_const + 1;	nConsts = 1;		// !! 常數表索引由一算起 !! (0 號索引是空的)
	all_names = (char *) xmalloc(NAME_STR_SIZE);
	pAllNames = all_names;
	pAllConstStr = (char *) xmalloc(CONST_STR_SPACE_SIZE);
	pConStr = pAllConstStr;				pConStr ++;	// 第零項常數是未定義資料 !
	all_var = (struct TNameStru *) xmalloc(MAX_VAR_SIZE);		now_var = all_var;		now_var_count = 0;
	all_local = (struct TLocalStru *) xmalloc(MAX_LOC_SIZE);		now_local = all_local;	now_loc_count = 0;
    all_run_loc = (struct TNameStru *) xmalloc(MAX_LOCAL_POOL_COUNT * sizeof(struct TNameStru));
    now_run_loc = all_run_loc;      now_locv = 0;
	nNames = 0;		now_run_prog = 0;
    bNoToDebug = 0;		xSay("#1499 do_run_core_init ok. (只借空間, 未填資料)");
	calc_ana_init();
    undefs = (struct TUndefs *) xmalloc(MAX_UNDEF_COUNT * sizeof(struct TUndefs));
    pSolved = (int *) xmalloc(MAX_UNDEF_COUNT << 2);  // nUndef_Func * 4
}

void do_run_func(int func_id)
{ // 執行一個 function
struct TCaller *src;
struct TNameStru *pLocals, *pLEnd;	// 指到此次執行所借的區域變數群
struct TFuncProc *pPF;
struct TFuncStru *fu;
char *save_final_addr;   // 記住上層的 final_addr
int     ret;

    mark(F_do_run_func, F_Enter);
    if (func_id > 0x6000) {
        ret = cReadDW(pSolved, (0x7FFF - func_id) << 2);
		xSay("#1154 do_run_func, ret = %d", ret);
        if (ret < 0) {
			xSay("#1156 do_run_func, func_id = $%x", func_id);
            if (func_id == 0x7ffd) { // !!@@ Lazy @@!!
                // 解析後面的參數 !
                do_run_resi_func(0);     // 執行內建函數
				mark(F_do_run_func, F_Bye + 1);
                return;
            }			
	    xSay("執行到一個未定義的函數 -- %s", undefs[0x7FFF - func_id].pName);
	    ErrorCode = ERR_call_undef_func;
	    ErrorLine = exec_line_No;
	    ErrorPFunc = &cur_func;
		mark(F_do_run_func, F_Bye + 2);
	    return;
        }
		// now_run_prog = ??	!!@@
        func_id = 0x7FFF - func_id;         // 此為已解決之正確函數編號
    }
    if (callerStack >= (MAX_CALLERS_COUNT - 1)) {
		xSay("儲存呼叫者資訊的堆疊已滿 ! 上限: %d", MAX_CALLERS_COUNT);
		ErrorCode = ERR_caller_stack_full;
		ErrorLine = exec_line_No;
		ErrorPFunc = &cur_func;
		mark(F_do_run_func, F_Bye + 3);
		return;
    }
    fu = func_stru;	fu += func_id;
    seeStru(fu, ID_FuncStru);
	save_final_addr = final_addr; 
    xx_caller("do_run_func #1175 入>", NULL);	
    src = callers;	src += callerStack;	callerStack ++;
    // 登錄呼叫者與返回點
	now_run_prog = ((struct TFuncStru *) cur_func->pFunc)->Prg_ndx;
	xSay("1082, 登錄呼叫者 now_run_prog = %d", now_run_prog);
    src->pFunc = cur_func->pFunc;	// 記住呼叫來源的父程式架構
    src->LineNo = exec_line_No;	        // 記住目前執行的這一行行號
    src->LocPos = cur_func->LocPos;	// 記住目前執行函數的新借區域變數掃瞄起點
    src->pExec = cur_func->pExec;	// call 完之後, 要跳回去執行的第一個指令位置.. (see run_core.pas -> exec_here(pGo))
	src->cur_rolling_no = cur_func->cur_rolling_no;
    // xSay("955 save pEx = $%x", cur_func->pExec));
    src->pFin = cur_func->pFin;	        // 要跳回去執行的程式段的最後一個指令位置..
    src->Result_Type = cur_func->Result_Type;
    xmemmove(src->iResult, cur_func->iResult, 12);	// 暫存結果

    pPF = pProcFuncs;		pPF += func_id;
    pLocals = prepare_local_vars(pPF);	// 預備此次呼叫的區域變數
    pLEnd = now_run_loc;
    // xSay("pB = $%x, pE = $%x", pLocals, pLEnd);
    parse_arguments(pPF, pLocals);	// 剖析後面的參數到適當的區域變數中
    if (ErrorCode) { mark(F_do_run_func, F_Bye + 4);  return;	}  // 有嚴重錯誤 !
    cur_func->pFunc = fu;
	if (pPF->nArgs > 0) xSay("#1185 do_run_func 需執行 make_RPN !!");  // 有 > 0 個參數時.
    run_a_func(fu, pLocals, 0);	// 從外部來執行一個函式
    // !! bug_未完成 傳回值放在堆疊頂端, typ2 = 傳回值型態, 需執行型態轉換 !!
    get_result(val2);		typ2 = final_type;	// 剖析 Result[] 的值 !
    free_local_vars(pLocals, pLEnd);	// 釋放所借的區域變數

    // 彈出呼叫者, 繼續執行
	final_addr = save_final_addr;
    callerStack --;		src = callers;	    src += callerStack;
    cur_func->pFunc = src->pFunc;			now_run_prog = ((struct TFuncStru *) cur_func->pFunc)->Prg_ndx;
	xSay("1210, 彈出呼叫者 now_run_prog = %d, 呼叫後 callerStack= %d", now_run_prog, callerStack);
    cur_func->LineNo = src->LineNo;
    cur_func->LocPos = src->LocPos;
    cur_func->pExec = src->pExec;
	cur_func->cur_rolling_no = src->cur_rolling_no;
    // xSay("978 pop  pEx = $%x", cur_func->pExec);
    cur_func->pFin = src->pFin;
    cur_func->Result_Type = src->Result_Type;
    xx_caller("do_run_func #1217 出>", src);
	final_addr = save_final_addr;
    xmemmove(cur_func->iResult, src->iResult, 12);	// 取回暫存結果
    bUpToGo = 1;
    loc_stack_now--;	now_fun_loc_count = loc_stack[loc_stack_now];	// 取回此值
	mark(F_do_run_func, F_Bye);
}

void do_run_reserve_word(unsigned char *pTk)
{ // 處理保留字
int func_id, len;

	mark(F_do_run_reserve_word, F_Enter);
    func_id = cReadDW(pTk, 0);
    xSay("#1198 保留字 $%x", func_id);
    bLastCondTok = 0;			// 此行未遇到保留字:控制令牌 (if, then, do, while, repeat ..)
    switch(pTk[1]) { // sys_cmd[x]: 0=begin.. 4 = if, 5 = then, 6 = else, 7 = do, 8 = while, 9 = for
		case 0: // begin
			len = nowLineTokCnt - 1;
			xSay(" ==> 執行 begin 區塊 .. %d 個 tokens", len);
			do_statements((int *) (pTk + 4), len << 2);
			break;
		case 1: // end  (nowLineTokCnt = 1)
			xSay(" ==> 執行 end  ..");
			break;
		case 2: // repeat
			check_set_repeat_info(pTk[3]);		// 查詢是否已經設定好 repeat-until 資訊
			len = nowLineTokCnt - 1;
			xSay(" ==> 執行 repeat 區塊 .. %d 個 tokens", len);
			do_statements((int *) (pTk + 4), len << 2);
			break;
		case 3: // until
			len = nowLineTokCnt - 1;
			xSay(" ==> 執行 until 區塊 .. %d 個 tokens", len);
			bLastCondTok = 1;       // 如果遇到 = 時, 需改成 do_opr_20_equal
			do_statements((int *) (pTk + 4), len << 2);
			if (! cReadDW(val2, 0)) go_back_repeat(pTk[3]);	// 需回 repeat 重複執行
			// 清除前面設置的 TMyRepeatInfo
			break;
		case 4: // if 把後面 (func_id shr 16) and 255 個 token 送去計算運算式的結果, 若是 true 則執行 then..
			// canSay = 1;
			bLastCondTok = 1;
	        do_statements((int *) (pTk + 4), (pTk[2] - 1) << 2);
	        if_result[pTk[3]] = val2[0];	// 記下此階層的 if 比較的結果
			if (!val2[0]) xSay(" ==> if 比較的結果為 false.");	// 直接略過 then (此處不須再執行)
	        else {
				xSay(" ==> if 比較的結果為 true !");
				len = pTk[2];       // 略過前面的 if 跟比較式 !
				do  {
					if (pTk[len << 2] == RESV_TOKEN) { // 尋找後面的 then !
						if (pTk[(len << 2) + 1] == 5) { // 找到 then !!
							len ++;
							exec_this_small_block(cReadInt(pTk + (len << 2), 1));
							len = 0;    break;          // 完成 !
							}
						}
						len ++;
                    } while (len < nowLineTokCnt);
                    if (len) xSay("錯誤 !! 沒有 then 指令 !");
                }
			break;
		case 5: // then 其實不會執行到這行 !
			xSay(" ==> 執行 then 區塊 .. (其實不會執行到這裡 !)");
			break;				
		case 6: // else
			if (if_result[pTk[3]] == 0) {
				xSay(" ==> 執行 else 區塊 .. $%x", pTk);
				// try_translate_tokens(pTk + 4, 4);  // 嘗試翻譯回原本的指令
//				xSay("#1017 exec Small Block Program");
				exec_this_small_block(cReadInt(pTk + 4, 1));
/*				len = cReadInt(pTk, 1);		// if_then 段落的 token 數
				pTk += ((len + 2) << 2);	// 把 pTk 移到 else 開頭
				len += cReadInt(pTk, 1);	        // 加上 else 段落的 token 數
				cur_func->pExec = pTk + ((len + 4) << 2);	*/ // 4 = if_then 及 else 的行號 token 數
				xSay(" ==> cur_func.pExec = $%x", cur_func->pExec);
            }
            break;
		case 7: // do
                len = nowLineTokCnt - 1;
                xSay(" ==> 執行 do 區塊 .. %d 個 tokens", len);
                do_statements((int *) (pTk + 4), len << 2);
                break;
		case 8: // while
                xSay(" ==> 執行 while 區塊 ..");
                check_while_expr(pTk);  // 檢查是否繼續執行
                break;
		case 9: // for
                xSay("#1313 ==> 執行 for 區塊 ..");
				// canSay = 0;
                check_for_info(pTk);        // 設置並初始變數
                break;
		default: xSay("#1265 未設計 ! (do_run_reserve_word)");
    }
	mark(F_do_run_reserve_word, F_Bye);
}

void do_run_resi_func(int func_id)
{ // 執行內建函數
struct TCaller *src;
struct TNameStru *pLocals, *pLEnd;	// 指到此次執行所借的區域變數群
struct TFuncProc *pPF;
// struct TFuncStru *fu;
// int     ret;

    mark(F_do_run_resi_func, F_Enter);
    if (func_id >= RES_FUNC_COUNT) {
		do_sys_error(errOverResidentFunc, 0);   // 不合法的內建函數
		mark(F_do_run_resi_func, F_Bye + 1);
		return;
    }
    if (callerStack >= (MAX_CALLERS_COUNT - 1)) {
		xSay("儲存呼叫者資訊的堆疊已滿 ! 上限: %d",MAX_CALLERS_COUNT);
		ErrorCode = ERR_caller_stack_full;
		ErrorLine = exec_line_No;
		ErrorPFunc = &cur_func;
		mark(F_do_run_resi_func, F_Bye + 2);
		return;
    }
    // fu = func_stru;	fu += func_id;		(目前不用 func_stru)
	// seeStru(fu, ID_FuncStru);
    xx_caller("do_run_resi_func #1306 入>", NULL);
	xSay("#1309, do_run_resi_func, 呼叫前 callerStack= %d", callerStack);
    src = callers;	src += callerStack;	callerStack ++;
    // 登錄呼叫者與返回點
	now_run_prog = ((struct TFuncStru *) cur_func->pFunc)->Prg_ndx;
    src->pFunc = cur_func->pFunc;	// 記住呼叫來源的父程式架構
    src->LineNo = exec_line_No;	        // 記住目前執行的這一行行號
    src->LocPos = cur_func->LocPos;	// 記住目前執行函數的新借區域變數掃瞄起點
    src->pExec = cur_func->pExec;	// call 完之後, 要跳回去執行的第一個指令位置.. (see run_core.pas -> exec_here(pGo))
	src->cur_rolling_no = cur_func->cur_rolling_no;
    // xSay("955 save pEx = $%x", cur_func->pExec));
    src->pFin = cur_func->pFin;	        // 要跳回去執行的程式段的最後一個指令位置..
    src->Result_Type = cur_func->Result_Type;
    xmemmove(src->iResult, cur_func->iResult, 12);	// 暫存結果

    pPF = pResiFuncs;		pPF += func_id;
    pLocals = prepare_local_vars(pPF);	// 預備此次呼叫的區域變數
    pLEnd = now_run_loc;
    // xSay("pB = $%x, pE = $%x", pLocals, pLEnd);
    parse_arguments(pPF, pLocals);	// 剖析後面的參數到適當的區域變數中
    if (ErrorCode) return;	// 有嚴重錯誤 !
    // cur_func->pFunc = fu;	(目前不用 func_stru)
    run_resident_func(func_id, pLocals);	// 從外部來執行一個內建函式
    // !! bug_未完成 傳回值放在堆疊頂端, typ2 = 傳回值型態, 需執行型態轉換 !!
    get_result(val2);		typ2 = final_type;	// 剖析 Result[] 的值 !
    free_local_vars(pLocals, pLEnd);	// 釋放所借的區域變數

    // 彈出呼叫者, 繼續執行
    callerStack --;		src = callers;	    src += callerStack;
	xSay("#1335, do_run_resi_func, 呼叫後 callerStack= %d", callerStack);
    cur_func->pFunc = src->pFunc;			now_run_prog = ((struct TFuncStru *) cur_func->pFunc)->Prg_ndx;
    cur_func->LineNo = src->LineNo;
    cur_func->LocPos = src->LocPos;
    cur_func->pExec = src->pExec;
	cur_func->cur_rolling_no = src->cur_rolling_no;
    // xSay("978 pop  pEx = $%x", cur_func->pExec);
    cur_func->pFin = src->pFin;
    cur_func->Result_Type = src->Result_Type;
    xx_caller("do_run_resi_func #1343 出>", src);
    xmemmove(cur_func->iResult, src->iResult, 12);	// 取回暫存結果
    bUpToGo = 1;
    loc_stack_now--;	now_fun_loc_count = loc_stack[loc_stack_now];	// 取回此值
	mark(F_do_run_resi_func, F_Bye);
}

void do_stktopVal_type_casting(void)
{ // 採用型態轉換表來轉換堆疊頂端的數值 ! ** 若有陣列, 則 typ 的值須從陣列中取出, 不可放 0 (陣列)
unsigned char dest_type;

	// if (typ1 > 16) or (typ2 > 16) then begin  do_sys_error(errInternalNdx, 0);  Exit;  end;
	dest_type = typ1 & MAX_TYPE_MASK;	// 以左方運算數值為主要型態 typ1 & 31
	if (dest_type == 0) { xSay("注意！數值未經轉換！");  return; }
	bMayHasErr = 0;		bSuspCvt = 0;
	if (dest_type > 0x80) { dest_type -= 0x80;  bMayHasErr = 1; }		// !!@@ bug: run_core #891 @@!!
	if (dest_type > 0x40) { dest_type -= 0x40;  bSuspCvt = 1; }		// !!@@ bug: run_core #892 @@!!
	xSay("右值型態: $%x, 值: %s", typ2, ret_value_str(typ2, val2));
	xSay("轉換目標 : $%x (%s)", dest_type, TypeModifier[dest_type]);
	final_type = dest_type;
	casting_one_val(val2, typ2 & MAX_TYPE_MASK);
	xSay("右值型態: $%x, 值: %s", final_type, ret_value_str(final_type, val2));
}

void do_statements(int *pAll, int len)
{  // --> run_core.pas -- line 813 --
int 	*pTok;
int		this_len;
char temp_buf[256];

	mark(F_do_statements, F_Enter);
	// 執行一行敘述, 遇到比較運算 (如等號) 時, 若 bLastCondTok = 1 則不設值到變數中, 只做評估動作 (針對 if/until/while 那行)
	xSay("#1351 do_statements (RUN_CORE)");
	if (bSysErr) {  mark(F_do_statements, F_Bye + 1);  return;	 }	// 早已有嚴重錯誤 !
	yHexDump(pAll, len);
	do {
		pTok = pAll;	this_len = len;		// 一開始預設是一行只含單一敘述
		if ((pTok[0] & 255) == LineNo_TOKEN) {	// 遇到可能有多行 !
			this_len = pTok[1] << 2;
			xmemmove(temp_buf, &pTok[1], this_len);
			xmemmove(&pTok[2], temp_buf, this_len);		// 往後移動 8 bytes, 忽略掉 LineNo_TOKEN 與 LineEnd_TOKEN
			pTok += 2;		pAll += 2;
		}
		do_only_one_statement(pTok, this_len);
		pAll += this_len;		len -= this_len;
	} while (len > 0);
	xSay("#1365 do_statements 完畢 !");
	mark(F_do_statements, F_Bye);
}

void do_sys_error(int msgID, int info)
{	// 顯示系統錯誤訊息
if (msgID < Max_Err_Count) {
	if (info == DO_WARN) {
		xSay("警告: %s, 在程式 %s 第 %d 行", errMsg[msgID], pss[now_run_prog].pName, nowLineNo);
		status_update(1398, WARN_ERR, now_run_prog | (exec_line_No << 8), (void *) (msgID | (nowLineNo << 8)));
		return;
	}
        // bSysErr = 1;	// true
        if (info <= 0x7FFF) {	// < $8000: 使用預設的程式名稱及行號來指出錯誤的地方
            xSay("錯誤: %s, 在程式 %s 第 %d 行", errMsg[msgID-1], pss[now_run_prog].pName, nowLineNo);
			status_update(1406, WARN_ERR, now_run_prog | (exec_line_No << 8), (void *) ((msgID - 1) | (info << 8)));
            switch(info) {
                case ShowFuncBeginLineNo: 
                    xSay("  該未完成函式 %s 的起始行號是 %d.", (*undone_func).pName, (*undone_func).bgnLine);
					break;
            }
            // if (info != 0) xSay(s);
        }
    }
}

void do_type_casting(void)
{ // 採用型態轉換表來轉換兩個數值 ! ** 若有陣列, 則 typ 的值須從陣列中取出, 不可放 0 (陣列)
int   ndx;
unsigned char dest_type;
	//     if (typ1 > 16) or (typ2 > 16) then begin  do_sys_error(errInternalNdx, 0);  Exit;  end;
    ndx = ((typ1 & MAX_TYPE_MASK) - 1) | (((typ2 & MAX_TYPE_MASK) - 1) << 4);
    xSay("[$%x, $%x] --> ndx = $%x", typ1, typ2, ndx);
	dest_type = MainCvt[ndx];	// 以左方運算數值為主要型態
	if (dest_type == 0) { xSay("#512 注意！數值未經轉換！");  return; }
	bMayHasErr = 0;       bSuspCvt = 0;
	if (dest_type > 0x80) { dest_type -= 0x80;  bMayHasErr = 1; }
	if (dest_type > 0x40) { dest_type -= 0x40;  bSuspCvt = 1; }
	xSay("左值型態: $%x, 值: %s", typ1, ret_value_str(typ1 & MAX_TYPE_MASK, val1));
	xSay("右值型態: $%x, 值: %s", typ2, ret_value_str(typ2 & MAX_TYPE_MASK, val2));
	xSay("轉換目標 : $%x (%s)", dest_type, TypeModifier[dest_type]);
	final_type = dest_type;					
	casting_one_val(val1, typ1 & MAX_TYPE_MASK);
	casting_one_val(val2, typ2 & MAX_TYPE_MASK);
	xSay("左值型態: $%x, 值: %s", final_type, ret_value_str(final_type, val1));
	xSay("右值型態: $%x, 值: %s", final_type, ret_value_str(final_type, val2));
}

void exec_here(char *pGo)
{ // 從這邊跳進去執行 !
char *pTemp;
int  nBytes;

    temp_buf_size = 4096;
    pTemp = (char *) xmalloc(temp_buf_size);     pThisLine = pTemp;
    do {
        // Renew_Screen;
		cur_func->pExec = pGo;
        xSay("#1420 正要執行 pGo = $%x", pGo);		
        nBytes = exec_this_line(&pTemp, pGo, 0);       // 不管是 if, for, repeat 等結構, 都必需要轉成單行型式 !
        xSay("#1422 exec_here 已執行第 %d 行", exec_line_No);
		if ((exec_line_No == 62) && (callerStack == 0)) xxErr("exec_here #1452 可以利用此功能在 Delphi 除錯 !!");
		if (pGo == pNextExec) bSysErr = 999; // dead loop !
        if ((nBytes < 0) || bSysErr) break;  // 有錯誤 !
        pGo = pNextExec;	// pNextExec = 下一行該執行的位址
		if (bUpToGo) { // 改採用另外指定的下一執行位址
            pGo = (char *) cur_func->pExec;	// Func, Resv_Func, small block 等需改變返回位置
            if ((size_t) cur_func->pFin < (size_t) final_addr) final_addr = (char *) cur_func->pFin;   // 改變函數的執行終點
            bUpToGo = 0;
        }
	else cur_func->pExec = pGo;
	if (ErrorCode) break;	// 有嚴重錯誤 !
    } while (pGo < final_addr);
    xSay("#1433 執行完的 pGo = $%x, callerStack = $%x", pGo, callerStack);
    xfree(pTemp);
}

int  exec_this_line(char **ppTemp, unsigned char *pGo, int len)
{ // var pTemp
char *pTemp;
unsigned char *pT;
int  ln, nBytes;

    // Result := 0;
    pTemp = *ppTemp;
    if (*pGo == LineNo_TOKEN) {
        len = (cReadDW(pGo, 0) >> 8) & 0xFFFF;   // = 這行真正含有的令牌數目
		nowLineTokCnt = len;			// 這行真正含有的令牌數目
        ln = cReadDW(pGo, (len + 1) << 2);       yHexDump(pGo, (len + 3) << 2);
        if ((ln & 255) != LineEnd_TOKEN) { do_sys_error(errBadTokenLineNo, 0);  return(0); }
        exec_line_No = ln >> 8;         // = 這行的真正行號 ! (佔 3 bytes)
		canSay = 1;  if (exec_line_No < 51) canSay = 0;
		nowLineNo = exec_line_No;
        nBytes = (len + 2) << 2;        // 令牌數目 * 4 = 執行掉多少個 bytes
        // 送出訊息: 通知編輯器更正光棒位置
        xSay("#1454 將執行第 %d 行, 用掉 %d bytes", exec_line_No, nBytes);
		if (status_update(1351, BEFORE_EXEC, now_run_prog | (exec_line_No << 8), NULL) & FLAG_STOP) bUserStop = 1;
		try_translate_tokens((unsigned char *) pGo + 4, len << 2);		// 嘗試翻譯回原本的指令
        // if nowLineNo = 41 then pGo[3] := #1;
		// if ((unsigned int) nowLineNo == brkps->line_number) wait_for_debug(0);	    // 要被除錯中斷 !
		if (bUserStop || pGo[3] || bStepTrace) wait_for_debug(0);	    // 要被除錯中斷 !
    }
    else if (len < 1) {
        do_sys_error(errBadTokenLineNo, 0);     return(0);
    }
    // xSay(Format('#974 現在行號: %d, 現在位址: $%x', [nowLineNo, pGo]));
    xSay("#1465 現在行號: %d, 位址: $%x", nowLineNo, pGo);
    // VWatch.display_prog(exec_line_No);
    if (len < 1) return(0);      // 這行無資料 !
    len = len << 2;             pNextExec = pGo + len + 8;	// 計算出下一行要執行的位址
    // !!@@ 需要 var pTemp, 因為 xmalloc @@!!!
    if (len > temp_buf_size) { // pTemp[] 的空間不足, 取得更大的空間 !
        temp_buf_size = len + 8;       xfree(pTemp);
        pTemp = (char *) xmalloc(temp_buf_size);        *ppTemp = pTemp;
    }
    xmemmove(pTemp, pGo+4, len);        cSetDW(pTemp, len, 0);
    yHexDump(pTemp, len);               // 印出來 debug 用
    pT = pTemp;
//     if (Byte(pT^) = FUNC_TOKEN) or (Byte(pT^) = RES_FUNC_TOKEN) then begin
//	xSay("指令 len = %d", len);
    cur_func->pExec = (void *) ((size_t) cur_func->pExec + len + 8);	// 更新返回點
    xSay("#1451 執行 pTemp 後位置: $%X, pNextExec = $%X", cur_func->pExec, pNextExec);
//     end;
    // try_translate_tokens((unsigned char *) pTemp, len);   // 嘗試翻譯回原本的指令
    pCallerText = (unsigned int *) pTemp;        CallerLen = len;
    // xSay("840 返回點 pExec = $%x", cur_func->pExec);
    switch(*pT) {
        case FUNC_TOKEN: xSay("exec Func");
            // 檢查是否為跳入切出程式小塊
            if (!pT[3]) do_run_func(cReadInt(pT, 1));    // do_run_func() 在 prg_stru.pas
            else exec_this_undone_block(cReadInt(pT, 1));
            break;      // -- FUNC_TOKEN
        case RESV_TOKEN:  xSay("exec Resv");  do_run_reserve_word(pT);  break;
        case BLOCK_TOKEN: xSay("exec 切出程式小塊");  exec_this_small_block(cReadInt(pT, 1));  break;
		case RES_FUNC_TOKEN: xSay("exec Resident_Func");  do_run_resi_func(cReadInt(pT, 1));  break;
        default: do_statements((int *) pTemp, len);
    }
    return(nBytes);
}

void exec_this_small_block(int block_ndx)
{ // !!@@ bug: exec_here() 的 while (pGo < final_addr) --> final_addr 是主程式的, 永遠小於 pGo 切出塊 !!
struct TCaller *src;
char *pBuf, *save_final_addr;   // 記住上層的 final_addr

    if (block_ndx >= finalBlkNdx) {  xSay("切出區塊程式的索引值太大 this=%d final = %d !", block_ndx, finalBlkNdx);  return;  }
    if (finBlockLen[block_ndx] > 256) pBuf = (char *) xmalloc((finBlockLen[block_ndx] + 4) << 2);
    else pBuf = (char *) xmalloc(1024);
    if (callerStack >= (MAX_CALLERS_COUNT - 1)) {
		xSay("儲存呼叫者資訊的堆疊已滿 ! 上限: %d", MAX_CALLERS_COUNT);
		ErrorCode = ERR_caller_stack_full;
		ErrorLine = exec_line_No;
		ErrorPFunc = &cur_func;
		return;
    }

	xx_caller("exec_this_small_block #2010 入>", NULL);
	yHexDump(cur_func->pExec, 16);
    src = callers;	src += callerStack;		callerStack ++;
    // 登錄呼叫者與返回點
	// now_run_prog = ((struct TFuncStru *) cur_func->pFunc)->Prg_ndx; (程序小區塊本身並無記錄程式代號 !!)
    src->pFunc = cur_func->pFunc;	// 記住呼叫來源的父程式架構
    src->LineNo = exec_line_No;	        // 記住目前執行的這一行行號
    src->LocPos = cur_func->LocPos;	// 記住目前執行函數的新借區域變數掃瞄起點
    src->pExec = cur_func->pExec;	// call 完之後, 要跳回去執行的第一個指令位置.. (see run_core.pas -> exec_here(pGo))
	src->cur_rolling_no = cur_func->cur_rolling_no;
    // xSay("1187 save pEx = $%x", cur_func->pExec);
    src->pFin = cur_func->pFin;	        // 要跳回去執行的程式段的最後一個指令位置..
    src->Result_Type = cur_func->Result_Type;
    xmemmove(src->iResult, cur_func->iResult, 12);	// 暫存結果

    // cur_func.pExec = finBlockMem[block_ndx];
    xSay("#1548 執行後位置: %X", cur_func->pExec);
	save_final_addr = final_addr;       // 記住上層的 final_addr
	// 修正 final_addr !!
    final_addr = finBlockMem[block_ndx] + finBlockLen[block_ndx];
    // exec_this_line(pBuf, finBlockMem[block_ndx], finBlockLen[block_ndx]);    // -4 是因為 Move(pGo[4]..
    exec_here(finBlockMem[block_ndx]);
    // 彈出呼叫者, 繼續執行
    final_addr = save_final_addr;       // 取回上層的 final_addr
    callerStack --;		src = callers;		src += callerStack;
    xSay("#1557 切出區塊程式執行完畢 ! callerStack= %d.", callerStack);
    cur_func->pFunc = src->pFunc;			// now_run_prog = ((struct TFuncStru *) cur_func->pFunc)->Prg_ndx;
    cur_func->LineNo = src->LineNo;
    cur_func->LocPos = src->LocPos;
    cur_func->pExec = src->pExec;
	cur_func->cur_rolling_no = src->cur_rolling_no;
    // cur_func->pExec = cur_func->pExec + len + 8;	// 更新返回點
    // xSay("#1204 pop  pEx = $%x", cur_func->pExec);
    cur_func->pFin = src->pFin;
    cur_func->Result_Type = src->Result_Type;
	xx_caller("exec_this_small_block #1564 出>", src);
    xmemmove(cur_func->iResult, src->iResult, 12);	// 取回暫存結果
    bUpToGo = 1;
    xfree(pBuf);
}

// ----------------
void exec_this_undone_block(int block_ndx)
{ // undone_block 不是 undefs[] !
// undone_block 是從各個程式切出來執行單一函數呼叫的單行程式 ! (90 xx xx xx ...)
struct TUndoneEval *pUE;
char *pBuf, *save_final_addr;
struct TCaller *src;

pUE = pUnDoneEval + block_ndx;
if (pUE->nTokens > 256) pBuf = (char *) xmalloc((pUE->nTokens + 4) << 2);
else pBuf = (char *) xmalloc(1024);
if (callerStack >= (MAX_CALLERS_COUNT - 1)) {
	xSay("儲存呼叫者資訊的堆疊已滿 ! 上限: %d", MAX_CALLERS_COUNT);
	ErrorCode = ERR_caller_stack_full;
	ErrorLine = exec_line_No;
	ErrorPFunc = &cur_func;
	return;
	}

xSay("#1508 執行 undone_block 之前, callerStack= %d.", callerStack);
xx_caller("exec_this_undone_block #1590 入>", NULL);
src = callers + callerStack;		callerStack ++;
// 登錄呼叫者與返回點
src->pFunc  = cur_func->pFunc;	// 記住呼叫來源的父程式架構
src->LineNo = exec_line_No;	// 記住目前執行的這一行行號
src->LocPos = cur_func->LocPos;	// 記住目前執行函數的新借區域變數掃瞄起點
src->pExec  = cur_func->pExec;	// call 完之後, 要跳回去執行的第一個指令位置.. (see run_core.pas -> exec_here(pGo))
src->cur_rolling_no = cur_func->cur_rolling_no;
// xSay3('920 save pEx = ' + IntToHex(Integer(cur_func.pExec), 8));
src->pFin = cur_func->pFin;	// 要跳回去執行的程式段的最後一個指令位置..
src->Result_Type = cur_func->Result_Type;
xmemmove(src->iResult, cur_func->iResult, 12);	// 暫存結果

cur_func->pExec = pUE->pTokens - 4;
// 修正 final_addr !!
save_final_addr = final_addr;   // 先保存
final_addr = pUE->pTokens + (pUE->nTokens << 2);	// 更新為目前執行小段的程式末端位址
xSay("#1497 即將執行: $%x, 執行後位置 $%x", cur_func->pExec, final_addr);
// see_call_stack(1128);	// debug dump stack
exec_this_line(&pBuf, pUE->pTokens - 4, pUE->nTokens);    // -4 是因為 Move(pGo[4]..
// see_call_stack(1130);	// debug dump stack
// 彈出呼叫者, 繼續執行
callerStack --;		src = callers + callerStack;
xSay("#1529 執行 undone_block 之後, callerStack= %d.", callerStack);
// if (callerStack == 3) wait_for_debug(99999);	// !!@@ down soon @@!!
cur_func->pFunc  = src->pFunc;
cur_func->LineNo = src->LineNo;
cur_func->LocPos = src->LocPos;
cur_func->pExec  = src->pExec;
cur_func->cur_rolling_no = src->cur_rolling_no;
// xSay3('931 pop  pEx = ' + IntToHex(Integer(cur_func.pExec), 8));
cur_func->pFin = src->pFin;	
cur_func->Result_Type = src->Result_Type;
xx_caller("exec_this_undone_block #1619 出>", src);
final_addr = save_final_addr;
xSay("1646 %x <==> %x", final_addr, src->pFin);
xmemmove(cur_func->iResult, src->iResult, 12);	// 取回暫存結果
bUpToGo = 1;

xfree(pBuf);
}

void fetch_one_value(void)
{ // 需要一個數值 --> load into val2
	xSay("#2071 try to fetch_one_value to val2");
	if (! bVal2Valid) { // val2 是無效的 ! 所以 val2 要先取值 !
        typ2 = decode_one_value_from_stacktop(val2);
		vToken2 = vTok;
		xSay("#2075 fetch_one_value to val2 -- ok");
	}
}

void fetch_two_values(void)
{ // 需要兩個數值 --> load into val1 & val2
	fetch_one_value();	// 需要第一個數值 --> load into val2
	typ1 = decode_one_value_from_stacktop(val1);
	vToken1 = vTok;
	// --- debug  ** 若有陣列, 則 typ 的值須從陣列中取出, 不可放 0 (陣列)
	xSay("#1547 取二值 token2 = $%x, typ2 = $%x .. %s", vToken2, typ2, TypeModifier[typ2 & MAX_TYPE_MASK]);	// 最高到 31 種型態
	yHexDump(val2, 8);
	xSay("#1549 取二值 token1 = $%x, typ1 = $%x .. %s", vToken1, typ1, TypeModifier[typ1 & MAX_TYPE_MASK]);	// 最高到 31 種型態
	yHexDump(val1, 8);
}

void fill_in_arg(char *var_name, struct TNameStru *pLocVs)
{  // --> prg_stru.pas -- line 640 --
// 把堆疊頂端運算的結果存到新借的區域變數中, pLocVs=此函數的全部區域變數
xSay("#2127 找同名稱區域變數 (%s)", var_name);
	if (var_name) while ((size_t) pLocVs < (size_t) now_run_loc) {
		if (xstrcmp(pLocVs->pName, var_name) == 0) { // 找到同名稱變數
			xSay("#2128 找到同名稱區域變數 (%s) ! pLocVs^.vType = $%x ", var_name, pLocVs->vType);
			final_type = pLocVs->vType & MAX_TYPE_MASK;
			casting_one_val(val2, typ2 & MAX_TYPE_MASK);
			xSay("轉換後右值型態: %x, 值: %s", pLocVs->vType, ret_value_str(final_type, val2));
			xmemmove(&pLocVs->i_Val, val2, 4);	// move 4 bytes
			xmemmove(&pLocVs->d_Val, val2, 8);	// move 8 bytes
			xSay(">> 更新數值後的 *pLocVs :");
			seeStru(pLocVs, ID_TNameStru);
			return;	// 完成 !
		}
		pLocVs ++;
	}
    xSay("#1673 do_sys_error(errNoLocVarFound, 0)	找不到指定的區域變數");
}

void fill_in_pProcFuncs(void)
{	// 把 my_fun & my_proc 的資料填到 pProcFuncs (pPRC) 裡面 !
// pPRC (pProcFuncs) 歸屬於 func_stru 下, 兩者的索引值必須一致
// func_stru[0..n-1] 保留給原本內設函數, 其他函數或執行時期新增函數才放 func_stru[n..max-1] 後面.
int	i, n;
struct TUndefs *pU;
struct TMyUndone *pMU;
struct TMyFunc *pFun;
struct TMyProc *pPro;
struct TFuncStru *pFS, *pFA;

        pU = undefs;            pMU = mainProg->all_undn;
        for (i = 0;i < (int) mainProg->cnt_undn;i ++) {
                pU->pName = pMU->name;          pU->lineNo = pMU->line_no;
                pU->Prg_ndx = pMU->prog_id;     pSolved[i] = pMU->solved;
                pU->qq = pMU->solved & 255;     // -1 = not solved
                pU ++;          pMU ++;
        }

        func_stru_cnt = mainProg->cnt_undn;
	pFun = my_fun;          // pFA = func_stru + mainProg->cnt_undn;
	for (i = 0;i < func_cnt;i ++) {
                n = found_in_undefs(pFun->name);        // 查詢是否為原本內設函數
                if (n < 0) { n = func_stru_cnt;  func_stru_cnt ++; } // 是其他函數或執行時期新增函數
                else { // 是原本內設函數
                        pMU = mainProg->all_undn;		pMU += n;
                        if (pMU->solved > 128) { // 此函數需處理 !!
                                xSay("%s 此函數需處理 !! Solved = %d", pFun->name, pMU->solved);
                                n = func_stru_cnt;  func_stru_cnt ++;
                        }
                }
                pPRC = pProcFuncs + n;
		pPRC->name = pFun->name;
		pPRC->lineNo = pFun->bgn_ln;	// !! line_no = 0 !!
		pPRC->ret_Type = pFun->retType;
		pPRC->nArgs = pFun->arg_cnt;
		pPRC->pArgs = (struct TArgs *) pFun->pArgs;
		// yHexDump(pPRC, 16);
                pFS = func_stru;                pFS += n;
                pFS->pName = pFun->name;        pFS->bgnLine = pFun->bgn_ln;
                pFS->finLine = pFun->fin_ln;    pFS->Prg_ndx = pFun->prog_id;
                pFS->fncType = pFun->fncType;   pFS->retType = pFun->retType;
                pFS->argCount = pFun->arg_cnt;  pFS->ref_count = 0;
                pFS->prog_addr = pFun->tokenAddr;	pFun->pFuncStru = pFS;
                pFS->prog_len = pFun->token_len;
                pFun ++;
	}

	pPro = my_proc;
	for (i = 0;i < proc_cnt;i ++) {
                n = found_in_undefs(pPro->name);        // 查詢是否為原本內設函數
                if (n < 0) { n = func_stru_cnt;  func_stru_cnt ++; } // 是其他函數或執行時期新增函數
                else { // 是原本內設函數
                        pMU = mainProg->all_undn;		pMU += n;
                        if (pMU->solved > 128) { // 從一起跳
                                xSay("%s 此函數需處理 !! Solved = %d", pFun->name, pMU->solved);
                                n = func_stru_cnt;  func_stru_cnt ++;
                        }
                }
                pPRC = pProcFuncs + n;
		pPRC->name = pPro->name;
		pPRC->lineNo = pPro->bgn_ln;	// !! line_no = 0 !!
		pPRC->ret_Type = pPro->retType;
		pPRC->nArgs = pPro->arg_cnt;
		pPRC->pArgs = (struct TArgs *) pPro->pArgs;
		// yHexDump(pPRC, 16);
                pFS = func_stru;                pFS += n;
                pFS->pName = pPro->name;        pFS->bgnLine = pPro->bgn_ln;
                pFS->finLine = pPro->fin_ln;    pFS->Prg_ndx = pPro->prog_id;
                pFS->fncType = pPro->fncType;   pFS->retType = pPro->retType;
                pFS->argCount = pPro->arg_cnt;  pFS->ref_count = 0;
                pFS->prog_addr = pPro->tokenAddr;	pPro->pFuncStru = pFS;
                pFS->prog_len = pPro->token_len;
		pPro ++;
	}
	nPRC = func_stru_cnt;
}

struct TMyForInfo *find_current_for_info(void)
{ // 尋找目前的 for info
struct TMyForInfo *pFI;
int	i;

        if (! all_for_info) {
                max_for_info_cnt = 16;
                all_for_info = (struct TMyForInfo *) xmalloc(sizeof(struct TMyForInfo) * max_for_info_cnt);
                for_info_cnt = 0;       return(NULL);   // not found, just allocate
        }
        pFI = all_for_info;             // return(pRI);    有找到 !
        for (i = 0;i < for_info_cnt;i ++) {
                // bug !! 比對行號 -> not ok ! (line 31 & line 38 不成立 !)
                if (pFI->line_no == nowLineNo) { // !! 這樣無法在一行內有兩個以上的 repeat !!
                        if (pFI->prog_id == now_run_prog) { // 找到了 ! (now_run_prog 從 0 算起 !)
                                return(pFI);     // 有找到 !
                        }
                }
                pFI ++;
        }
        return(NULL);
}

struct TMyRepeatInfo *find_current_repeat_info(unsigned char level)
{ // 尋找目前的 repeat-until info
struct TMyRepeatInfo *pRI;
int	i;

        if (! all_repeat_info) {
                max_repeat_info_cnt = 16;
                all_repeat_info = (struct TMyRepeatInfo *) xmalloc(sizeof(struct TMyRepeatInfo) * max_repeat_info_cnt);
                repeat_info_cnt = 0;            return(NULL);   // not found, just allocate
        }
        pRI = all_repeat_info;             // return(pRI);     // 有找到 !
        for (i = 0;i < repeat_info_cnt;i ++) {
                if (pRI->level == level) {
                        if (pRI->prog_id == (now_run_prog + 1)) { // 找到了 ! (now_run_prog 從 0 算起 !)
                                return(pRI);     // 有找到 !
                        }
                }
                pRI ++;
        }
        return(NULL);
}

void *find_function_by_name(char *fname)
{	// 找尋某個函數 (傳回 struct TMyFunc * 或 struct TMyProc *)
int	i;
struct TMyFunc *pFun1;
struct TMyProc *pFun2;

	pFun1 = my_fun;
	xSay("#1631 find_function_by_name");
	for (i = 0;i < func_cnt;i ++) {
		// yHexDump(pFun1, 16);
		if (xstrcmp(pFun1->name, fname) == 0) return(pFun1);
		pFun1 ++;
	}
	pFun2 = my_proc;
	xSay("#1638 find_procedure_by_name");
	for (i = 0;i < proc_cnt;i ++) {
		// yHexDump(pFun2, 16);
		if (xstrcmp(pFun1->name, fname) == 0) return(pFun2);
		pFun2 ++;
	}
	return(NULL);	// 失敗 !
}

int found_in_undefs(char *name)
{
struct TUndefs *pU;
int   i;
        pU = undefs;
        for (i = 0;i < (int) mainProg->cnt_undn;i ++) {
                if (xstrcmp(pU->pName, name) == 0) return(i);
                pU ++;
	}
        return(-1);
}

void free_local_vars(struct TNameStru *pBgn, struct TNameStru *pEnd)
{ // 釋放所借的區域變數
int     n;

    now_run_loc = pBgn;         // 直接回到原本的位置 !
    n = ((size_t) pEnd - (size_t) pBgn) / sizeof(struct TNameStru);
    xSay("#833 釋放區域變數前, 總區域變數個數 now_locv = %d", now_locv);
    xSay("#834 釋放區域變數前, 已經使用的區域變數個數 n = %d", n);
    now_locv -= n;
}

char *get_const_by_ndx(int x)
{ // 取得常數內涵, 轉成字串格式
SuInt64 i64;
SuDoubl rd;
SuFloat rs;
struct TConstStru *pC;
va_list	ap;
char *s;


    if (x < 0) return("!常數索引值小於零!");
    pC = all_const;     pC += x;        s = su_buf;		s[0] = 0;
    switch(pC->cType) {
        case CT_STRING: return(pC->pName);
        case CT_CHAR: s[0] = (size_t) pC->pName & 255;  s[1] = 0;  break; // !!@@
        case CT_SINGLE:
            a1int_single(pC->i_Val, &rs);       // copy 4 bytes to be a float !
            xstrcopy(s, sSingleToStr(&rs));
            break;
        case CT_DOUBLE:
            a2int_double(pC->i_Val, (size_t) pC->pName, &rd);   // !!@@ pName ? 32/64 @@!!
            xstrcopy(s, sDoubleToStr(&rd));
            break;
        case CT_S64:
        case CT_QWORD:
            a2int_int64(pC->i_Val, (size_t) pC->pName, &i64);   // !!@@ pName ? 32/64 @@!!
            sPrt(s, "%ll", i64);
            break;
        default:
            sPrt(s, "%d", pC->i_Val);
            break;
    }
    return(s);
}

char * _stdcall get_errMsg(int msg_id)
{
	return(errMsg[msg_id]);
}

char *get_fixval_as_str(int fixVal_ndx)
{ // 從 struct  TMyFixVal *[] 取得固定值, 以字串表達內容
struct TMyFixVal *pFV;
char *p;
        fixVal_ndx --;
        if ((unsigned int) fixVal_ndx > mainProg->cnt_fixval) return("#1242 超過固定值上限 !");
        pFV = mainProg->all_fixval;     pFV += fixVal_ndx;
        switch(pFV->cType & CT_MASK) {
                case CT_CHAR: // 12: char  (39=單引號)
                        sPrt(ssTemp, "%c%c%c", 39, pFV->initVal[0], 39);
                        return(ssTemp);
                case CT_DBLCHAR: // 13: dblchar
                case CT_UNICODE: // 14: unicode
                case CT_STRING: // 15: PChar & string & ansistring (目前以正常字串格式保存)
                        return(pFV->initVal);
                case CT_SINGLE: // 10: 以 3.1415.. 文字字串存在於 initVal[]
                case CT_DOUBLE: // 11: 以 3.1415.. 文字字串存在於 initVal[]
                        sPrt(ssTemp, "%c%s%c", 34, pFV->initVal, 34); // (34=雙引號)
                        return(ssTemp);
                case CT_BYTE: // 2 byte
                        sPrt(ssTemp, "%d", (unsigned char) (pFV->iVal & 0xFF));
                        return(ssTemp);
                case CT_WORD: // 4 word
                        sPrt(ssTemp, "%d", (unsigned short) (pFV->iVal & 0xFFFF));
                        return(ssTemp);
                case CT_DWORD: // 6 dword
                        sPrt(ssTemp, "%d", (unsigned int) pFV->iVal);
                        return(ssTemp);
                case CT_S32: // 7 int
                        sPrt(ssTemp, "%d", pFV->iVal);
                        return(ssTemp);
                case CT_S8: // 3 sint8
                        sPrt(ssTemp, "%d", (char) (pFV->iVal & 0xFF));
                        return(ssTemp);
                case CT_S16: // 5 sint16
                        sPrt(ssTemp, "%d", (short) (pFV->iVal & 0xFFFF));
                        return(ssTemp);
                default: // 14=ptr, CT_S64=9, CT_QWORD=8, 18=void,
                        p = pFV->initVal;
                        sPrt(ssTemp, "iVal=$%x, initVal=$%x $%x", pFV->iVal, *(unsigned int *) p, *(unsigned int *) (p+4));
                        return(ssTemp);
        }
}

struct TFuncProc *get_FuncProc_byName(char *name_str)
{
struct TFuncProc *pF;
int     i;

        pF = pProcFuncs;
        for (i = 0;i < func_stru_cnt; i ++) {
                if ((pF->name) && (xstrcmp(pF->name, name_str) == 0)) return(pF);
                pF ++;
        }
        return(NULL);
}

struct TNameStru *get_newest_var(struct TNameStru *pNX)
{  // 從此變數位置之後, 嘗試取最新產生之變數
struct TNameStru *pN, *pHdr;
char *v_name;
int     first_loc, i, now_n;

        while(pNX->name_next != -1) {   // 跳到最後一個同名變數
                pNX = all_var + pNX->name_next;
        }
        now_n = pNX - all_var;  // now_n = 目前此變數的 ndx
        pN = pNX;       v_name = pN->pName;     pHdr = pN;      // 尋找的起點
        first_loc = pN->name_first;
        for (i=now_n + 1;i < now_var_count;i ++) {  // 繼續尋找此變數後的同名變數
                pN ++;
                if (xstrcmp(pN->pName, v_name) == 0) { // 吻合同名變數
                        pHdr->name_next = i;          // 記住此同名變數的位置
                        pHdr = pN;      // cnt ++;
                        pHdr->name_first = first_loc;
                        pNX = pN;       // 預備傳回值
                }

        }
//        xSay("最後一個同名變數 => %d", pNX - all_var);
        return(pNX);
}

char *get_InitVal_str(int initNdx, int flag)
{ // flag must has 0x80 on it !
/* unsigned long i64;
float   rs;
double  rd;
struct TConstStru *pC; */
char *s;

    s = (char *) xmalloc(256);          xstrcat(s,  "初值= ");
    if (flag & 0x40) { // get init val from different source
        xstrcat(s, get_fixval_as_str(initNdx));
        return(s);
    }
    else {
        xstrcat(s, get_fixval_as_str(initNdx));
        return(s);
	}
}

void get_result(char *pResult)
{    // 把 Result[] 的值剖析後存到 pResult 中 (val2[]: 8 bytes)
     final_type = cur_func->Result_Type;
//     casting_one_val(@cur_func.iResult[0], cur_func.Result_Type);
     xmemmove(pResult, cur_func->iResult, 8);
}

int  get_rolling_no(void)
{ // 取得函數的滾動數值 ! (快速替代 Thread ID)
// 每次呼叫函數時, 會給予一個全新的,唯一的滾動數值,比使用 Thread ID 快且專一
	xSay("line #%d get rolling no = %d", nowLineNo, rolling_number);
	rolling_number ++;
	return(rolling_number - 1);
}

char *get_undone_calls(int x)
{ // 取得呼叫函數的整個字串 !
struct TUndoneEval *pFUE;

    pFUE = pUnDoneEval;       pFUE += x;
    bNoToDebug = 1;
    try_translate_tokens(pFUE->pTokens, pFUE->nTokens << 2);
    bNoToDebug = 0;
    xstrcat(sStrBack, " )");    return(sStrBack);
}

char *get_var_name_by_ndx(int x)
{ // 取得變數名稱
struct TNameStru *pNS;

    pNS = all_var;      pNS += x;
    if (pNS->pName) return(pNS->pName);
    else return("變數名稱 = NULL PTR !!");
}

void go_back_repeat(unsigned char level)
{  // 需回 repeat 重複執行
struct TMyRepeatInfo *pRI;
int     i;

        pRI = find_current_repeat_info(level);		// 尋找目前的 repeat-until info
        if (pRI) {
                pNextExec = (char *) pRI->tokenAddr;    // 回到 repeat 開頭再執行
                return;        // 已經有設定好了
        }
}

void _stdcall init_All_Before_Run(void)
{	// call from  TBkgCtrl.do_run line 290
	// xSay("C_Runer_init !");
    prg_cnt = 1;    // 程式代號, 通常由 1 算起(原本從 0 算起) (不從 0 起, 以免失誤)
    // 底下逐一加入 .MIA 程式
	if (mainProg) xfree(mainProg);
	// xSayStrHex("ProgDataA size = $", sizeof(struct ProgDataA), 2);
	do_run_core_init();
}

void load_proc_func(void)
{	// 處理函數/程序
    my_fun = mainProg->all_func;	func_cnt = mainProg->cnt_func;
    my_proc = mainProg->all_proc;	proc_cnt = mainProg->cnt_proc;
    pProcFuncs = (struct TFuncProc *) xmalloc(MAX_FUNC_TABLESIZE);
    pPRC = pProcFuncs;          nPRC = 0;
    fill_in_pProcFuncs();	// 把 my_fun & my_proc 的資料填到 pProcFuncs 裡面 !
}

void _stdcall load_prog_info(void *p)
{	// call from  TBkgCtrl.do_run line 293, 參考 prg_stru.pas #1123=> run_main_start
struct TProgStru *pPS;
struct TMyPrograms *pMP;
	// xSayStrHex("prog_info at $", (int) p, 4);
	mainProg = (struct ProgDataA *) xmalloc(sizeof(struct ProgDataA));
	// xSayStrHex("save to mainProg at $", (int) mainProg, 4);
	clear(mainProg, sizeof(struct ProgDataA));
	xmemmove(mainProg, p, sizeof(struct ProgDataA));
    xSay("#2479 *mainProg:");
	xHexDump(mainProg, sizeof(struct ProgDataA));
    pPS = pss;      pPS += prg_cnt;
    pMP = mainProg->all_prog;
    pPS->pName  = pMP->short_filename;
    pPS->nLines = pMP->line_count;
    prg_cnt ++;
	now_var_count = mainProg->cnt_glob + mainProg->cnt_const;	// 較完整: all_var / 排序過的: pGlobVars
	// xSay("#1684 now_var_count = %d", now_var_count);
	// yHexDump(mainProg->all_func, 0x200);
	load_proc_func();               // 處理函數/程序
    load_prog_vars();               // 處理常數變數總庫
    load_undone();                  // 處理未完成部分函數/程序
    // debug_sys(255);
	do_all_prog_const_init();	// 設定所有程式的常數變數初值
	// run_main_start();
	// canSay = 1;		xSay("canSay 於 #2439 行後關閉 ! (load_prog_info)");
}

void load_prog_vars(void)
{ // 處理常數變數總庫
struct TMyFixVal *pFV;  // -> all_const
struct TNameStru *pNS;  // -> all_var
struct TMyVar *pGV;     // -> all_glob   (VarrMngr.pas #21)
struct TMyLocal *pLV;   // -> all_local  (VarrMngr.pas #22)
struct TNameStru *pLS;  // 使用中的區域變數
struct TConstStru *pCS;
struct TMyConst *pCV;   // -> all_const  (VarrMngr.pas #23)
unsigned int i;
    if (now_var_count < 1) return;      // 無常數變數 !
    // --- 處理 all_glob[] 全域變數 ---
    pGV = mainProg->all_glob;
    for (i = 0;i < mainProg->cnt_glob; i ++) {
        pNS = all_var;  pNS += pGV->var_index;  // 指到 all_var[all_glob[i].var_index]
        if (!pNS->pName) { // 如果還沒有資料, 就現在填入 !
            pNS->pName = pGV->name;     pNS->lineNo = pGV->line_no;
            pNS->prgID = pGV->prog_id;  pNS->flags = pGV->flag;
            pNS->vType = pGV->vType;    pNS->initNdx = pGV->initNdx;
            pNS->i_Val = pGV->iVal;     pNS->d_Val = pGV->dVal;
        }
		// xSay("1712全> %d: %s f:$%x t:$%x (%d)", pNS->lineNo, pNS->pName, pNS->flags, pNS->vType, pGV->var_index); 
        pGV ++;                         nNames ++;   // 最後與 now_var_count 相等
    }
    // --- 處理 all_local[] 區域變數 ---
    pLV = mainProg->all_local;          pLS = now_run_loc;      // 使用中的區域變數
    // 把某程式的區域變數表拷貝進當前執行的全部區域變數表
    for (i = 0;i < mainProg->cnt_local; i ++) {
        pLS = all_run_loc;      pLS += pLV->var_index;  // 指到 all_run_loc[all_local[i].var_index]
        pLS->pName = pLV->name;         pLS->lineNo = pLV->line_no;
        pLS->prgID = pLV->prog_id;      pLS->flags = pLV->flag;
        pLS->vType = pLV->vType;        pLS->initNdx = pLV->initNdx;
        pLS->d_Val = pLV->dVal;         pLS->i_Val = pLV->iVal;
		// xSay("1724區> %d: %s f:$%x t:$%x (%d)", pLS->lineNo, pLS->pName, pLS->flags, pLS->vType, pLV->var_index); 
        pLV ++;         pLS ++;         now_locv ++;
    }
    now_run_loc = pLS;
    // --- 處理 all_fixval[] 常數 ---
    // all_const 其實是從 all_fixval (struct TMyFixVal) 按次序完整建立的
    pFV = mainProg->all_fixval;         pCS = all_const;
    // 把某程式的常數表拷貝進當前執行的全部 const 表
    for (i = 0;i < mainProg->cnt_fixval - 1; i ++) {
        // pCS 指到 all_const[i]
        pCS->pName = NULL;      // pFV->pName; (此值應是零, 或是前段程式的字串位址, 不應使用 !)
        pCS->i_Val = pFV->iVal;
        pCS->cType = pFV->cType;        pCS->prgID = pFV->prog_id;
        pCS->flags = pFV->flag;         pCS->PMyConst = (void *) pFV; // 此處填 struct TMyFixVal
        // xSay("1663固> i(%d) f:$%x t:$%x 初值(%s)", pCS->i_Val, pCS->flags, pCS->cType, pFV->initVal);
        pFV ++;         pCS ++;         nConsts ++;
    }
    // --- 處理 all_const[] 常數 ---
    pCV = mainProg->all_const;
    // 把某程式的常數表拷貝進當前執行的全部 const 表
    for (i = 0;i < mainProg->cnt_const; i ++) {
        pNS = all_var;          pNS += pCV->var_index;  // 指到 all_var[all_const[i].var_index]
        if (!pNS->pName) { // 如果還沒有資料, 就現在填入 !
            pNS->pName = pCV->name;     pNS->lineNo = pCV->line_no;
            pNS->prgID = pCV->prog_id;  pNS->flags = pCV->flag;
            pNS->vType = pCV->vType;    pNS->initNdx = pCV->initNdx;
            pNS->i_Val = pCV->iVal;     pNS->d_Val = pCV->dVal;
        }
        if (pNS->initNdx) { // 有 all_fixval 資料須修正 (索引由一號起跳, 此 pCV->initNdx 需減一)
            pCS = all_const;            pCS += (pCV->initNdx - 1);
            pCS->pName = pCV->name;     pCS->PMyConst = (void *) pCV;
            // pCS->cType = pCV->vType;    pCS->prgID = pCV->prog_id;
            // pCS->flags = pCV->flag;     pCS->i_Val = pCV->iVal;
        }
        // xSay("1667常> %d: %s f:$%x t:$%x (%d)", pNS->lineNo, pNS->pName, pNS->flags, pNS->vType, pCV->var_index);
        if (pCV->var_index >= (unsigned int) nConsts) nConsts ++;
        pCV ++;         pCS ++;
    }
    now_const = pCS;
}

void load_undone(void)
{ // 處理未完成部分函數/程序
struct TMyUndone *pMU;
struct TUndefs *pUD;
struct TMyExcBlk *pExcBlk;
struct TMySmallBlk *pMSB;
void *p;
unsigned int    i, n;

/*        pMU = mainProg->all_undn;
        if (pMU) { // NULL = 無資料須處理
                pUD = undefs;
                for (i = 0;i < mainProg->cnt_undn;i ++) {
                        pUD->pName = pMU->name;
                        pUD->lineNo = pMU->line_no;
                        pUD->Prg_ndx = pMU->prog_id;
                        pUD->qq = pMU->solved;
                        pUD ++;         pMU ++;
                        nUndef_Func ++;
                }
        } */
		nUndef_Func = mainProg->cnt_undn;
        // 載入切出的程式區塊資訊    個數: mainProg->cnt_excblk
        pExcBlk = mainProg->all_excblk;
        if (pExcBlk) while (pExcBlk->tokenAddr) {
                pUDE = pUnDoneEval;     pUDE += pExcBlk->excised_ndx;   // 通常是順著 1..n 下來
                pUDE->progID = pExcBlk->prog_id;
                pUDE->LineNo = pExcBlk->line_no;
                pUDE->pFunc = (void *) pExcBlk->src_func; // 須由函數名稱回查在哪個程式裡
                pUDE->pTokens = (char *) pExcBlk->tokenAddr;
                pUDE->nTokens = pExcBlk->token_len >> 2;
                pExcBlk ++;
        }
		// 載入切出的程序小區塊資訊    個數: mainProg->cnt_smblk
		i = finalBlkNdx;		n = 0;
		pMSB = mainProg->all_smblk;
		if (pMSB) while (pMSB->tokenAddr) {
			finBlockLen[i + pMSB->block_ndx] = pMSB->token_len;
			p = xmalloc(pMSB->token_len);
			finBlockMem[i + pMSB->block_ndx] = (char *) p;
			xmemmove(p, pMSB->tokenAddr, pMSB->token_len); 
			// xSay("%d L=%d, $%x", n, pMSB->token_len, p); 
			n ++;		pMSB ++;
		}
		finalBlkNdx += n;	// xSay("#1838 fin blk = %d", finalBlkNdx);
}

void make_RPN(int *pIn, int nTok)
{ // --> calx_ana.pas -- line 553 --
int		*pOut, *pSTK;
int		i, j, k, nSTK;

    // 把中序運算式轉成後序運算式, 逆波蘭記法(Reverse Polish Notation,RPN)
	pOut = pAnsTokens;		nAnsToken = 0;
	// pIn = 來源令牌, pAnsTokens = 輸出結果的令牌堆
	pSTK = pStackToken;		nSTK = 0;		i = 0;
	clear(pSTK, MAX_TEMP_TOKEN_SIZE >> 1);
	do {
		j = *pIn;		pIn ++;		// 取一個令牌來看
		switch (j & 255) {
            // 若是 (變數) 或 (暫時字串) 或 (函式呼叫的傳回值) 則 直接輸出到 pAns[]
            case FUNC_TOKEN:
            case TEMP_STR_TOKEN:
            case CONST_TOKEN:
            case VAR_TOKEN:
				*pOut = j;		pOut ++;		nAnsToken ++;
				break;
            case SYMB_TOKEN:
            case TYPE_TOKEN:
				//: xSayStrHex('found type token = $', j, 4);
				// 若是遇到左括號, 則直接把它放到堆疊中 !
				if (j == TOKEN_LEFT_Bracket) {
					j = j & MAKE_Zero_Priority;		// 把左括號的優先度改成 0, 它在堆疊中是最低的 !
					*pSTK = j;		pSTK ++;		nSTK ++;	break;
				}
				// 若是遇到右括號, 不斷的把堆疊頂端的值彈出到 pAns[], 直到遇見左括號而互相抵消 !
				if (j == TOKEN_RIGHT_Bracket) {
					if (nSTK > 0) do {
						pSTK --;	k = *pSTK;		nSTK --;
						if ((k & 0xFF00) == (TOKEN_LEFT_Bracket & 0xFF00)) break;	// 遇見左括號而互相抵消 !
						*pOut = k;		pOut ++;	nAnsToken ++;	// 把堆疊頂端的值彈出到 pAns[]
					} while (nSTK > 0);
					break;
				}
				// 若 j 的優先度比堆疊頂端的優先度高, 則把 j 推入堆疊, 否則把堆疊頂端的值彈出到 pAns[] !
				pSTK --;
				if (nSTK > 0) k = *pSTK;  else k = 0;	// 堆疊頂端沒有令牌
				if ((j & 0xFF0000) > (k & 0xFF0000)) {
					pSTK ++;	// 底下把 j 推入堆疊 !
					*pSTK = j;		pSTK ++;	nSTK ++;
				}
				else { // if 優先度(j <= k) 則把堆疊頂端的值彈出到 pAns[] !
					*pOut = k;		pOut ++;	nAnsToken ++;
					// 再把現在的令牌 j 推入堆疊頂端
					*pSTK = j;		pSTK ++;
				}
				break;
			}
		i ++;
	} while (i < nTok);
	if (nSTK > 0) do { // 反覆把殘存於堆疊的運算令牌彈出到 pAns[]
		pSTK --;		*pOut = *pSTK;		nSTK --;
		pOut ++;		nAnsToken ++;
	} while (nSTK > 0);
	if (nAnsToken > 0) {
		xSay("轉為後序運算式為 -- ");
		yHexDump(pAnsTokens, nAnsToken << 2);
	}
}

struct TFuncStru *MyFunc2FuncStru(void *fun)
{ // 把 struct TMyFunc * 換成 struct TFuncStru *
struct TMyFunc *myfun;
struct TFuncStru *pF;   // 此架構缺乏 pArgs[], 希望漸勿使用

    myfun = (struct TMyFunc *) fun;
    // yHexDump(myfun, 256);
    if (myfun->pFuncStru) return((struct TFuncStru *) myfun->pFuncStru);     // 已經有了 !
    // --- 臨時新增函式 ---
    if (func_stru_cnt > (MAX_FUNCTIONS -2)) {
        do_sys_error(errTooManyFuncs, 0);       // 函式數目超過 2048 個
        return(NULL);
    }
    pF = func_stru;     pF += func_stru_cnt;    // 指到尾端
    pF->pName = myfun->name;            pF->bgnLine = myfun->bgn_ln;
    pF->finLine = myfun->fin_ln;        pF->Prg_ndx = myfun->prog_id;
    pF->fncType = 1;    // 是 procedure = 0, function = 1, external = $40, 尚不明 = $80
    pF->retType = myfun->retType;       pF->argCount = myfun->arg_cnt;
    pF->ref_count = 0;                  pF->prog_addr = myfun->tokenAddr;
    pF->prog_len = myfun->token_len;
    return(pF);         // 函式新增完畢
}

/* int __fastcall myStrComp(void *pA, void *pB) { // 字串指標內容比較
    return(xstrcmp((char *) pA, (char *) pB));
}  */

void parse_arguments(struct TFuncProc *pPF, struct TNameStru *pLocs)
{  // --> prg_stru.pas -- line 667 --
struct TArgs *pARs;
int		*pI;
int		i, n;
	mark(F_parse_arguments, F_Enter);
// 剖析後面的參數到適當的區域變數中
	xSay("#2742 !! 剖析後面的參數 parse_arguments");
	// yHexDump(pPF, sizeof(struct TFuncProc));	
	if (pPF->pArgs == NULL) {
		parse_arguments_for_nil(pPF, pLocs);	// 處理參數到內部預設參數陣列
		mark(F_parse_arguments, F_Bye + 1);
		return;
	}
	if (loc_stack_now > (MAX_LOCAL_STACK - 2)) {
		xSay("errLocVarStackFull: 暫放 now_fun_loc_count 的堆疊已滿");
		// do_sys_error(errLocVarStackFull, 0);	// 暫放 now_fun_loc_count 的堆疊已滿
		mark(F_parse_arguments, F_Bye + 2);
		return;
	}
	loc_stack[loc_stack_now] = now_fun_loc_count;	loc_stack_now ++;	// 記住此值
	if (pCallerText) {
		xSay("呼叫者代碼: ");
		yHexDump(pCallerText, CallerLen);
	}
	// CallerLen 是以 bytes 計算, make_RPN 是以 dword Token 數量計算
	if (pCallerText) make_RPN((int *) (((size_t) pCallerText) + 4), CallerLen >> 2);   // 轉成逆波蘭運算式       
	xSay("函數參數代碼: CallerLen = %d", CallerLen);
	seeStru(pPF->pArgs, ID_Args);
	// yHexDump(pPF->pArgs, pPF->nArgs * sizeof(struct TArgs));	// pArgs 指到 TArgs 陣列
	// 例 TArgs[2]: name="xx", flags=$60, line_no=14, vType=$47, loc_match=1
	//              name="yy", flags=$60, line_no=14, vType=$47, loc_match=2
//	xSay("#969 區域變數群 pLVS:  變數名稱 %s", pLVS->pName);
//	yHexDump(pLVS, sizeof(struct TNameStru));
	// 從這個函式當開頭, 遞迴處理 (此處不可多重進入 !)
	if (pFunArgs) xSay("!! 嚴重錯誤 !! 多重進入 !!");
	pFunArgs = (struct TFuncArgList *) xmalloc(sizeof(struct TFuncArgList) * MAX_FUNC_ARG_LIST_COUNT);
	pFunNow = pFunArgs;		nFunCnt = 0;
	if (pCallerText == NULL) { xSay("是 start 函數 ! 另外再設計 !!");  
			xfree(pFunArgs);	pFunArgs = NULL;	
			mark(F_parse_arguments, F_Bye + 3);		return; }	// 是 start 函數 ! 另外再設計 !
	else recursive_parse_args(pCallerText);		// +4 .. 跳過第一個已知 FUNC_TOKEN / RES_FUNC_TOKEN 代碼
	pFunNow --;
	if (pPF->nArgs != pFunNow->nArgs) {	// 參數個數不正確
		xSay("do_sys_error(errBadArguments, 0)");		
		mark(F_parse_arguments, F_Bye + 4);		return;
	}
	pARs = pPF->pArgs;
	for (i = 0;i < pPF->nArgs;i ++) {	// 逐一計算參數數值
		pI = (int *) pFunNow->pArgs[i];		n = *pI;	// n = 其後的 32-bit dword 個數
		pI ++;		do_statements(pI, n << 2);	// 執行這條敘述
		fill_in_arg(pARs->name, pLocs);		// 把堆疊頂端運算的結果存到新借的區域變數中
		pARs ++;	// 看下一個參數
	}
	now_fun_loc_count = pPF->nArgs;		// 記下目前這個函數的區域變數個數
	xfree(pFunArgs);		pFunArgs = NULL;
	// 函數執行完後, 要釋放各個參數列所借的記憶體
	mark(F_parse_arguments, F_Bye);
}

void parse_arguments_for_nil(struct TFuncProc *pPF, struct TNameStru *pLocs)
{  // --> prg_stru.pas -- line 715 --
struct TArgs *pARs;
struct TNameStru *pLVS;
int		*pI;
int		i, n;
	// 處理參數到內部預設參數陣列
	// xSay('呼叫者代碼: --> 請處理參數到內部預設參數陣列');
	// yHexDump(pCallerText, CallerLen);
	// 從這個函式當開頭, 遞迴處理 (此處不可多重進入 !)
	if (loc_stack_now > (MAX_LOCAL_STACK - 2)) {
		xSay("do_sys_error(errLocVarStackFull, 0) 暫放 now_fun_loc_count 的堆疊已滿");	// 暫放 now_fun_loc_count 的堆疊已滿
		return;
	}
	loc_stack[loc_stack_now] = now_fun_loc_count;		loc_stack_now ++;	// 記住此值
	if (pFunArgs) xSay("!! 嚴重錯誤 !! 多重進入 !!");
	pFunArgs = (struct TFuncArgList *) xmalloc(sizeof(struct TFuncArgList) * MAX_FUNC_ARG_LIST_COUNT);
	pFunNow = pFunArgs;			nFunCnt = 0;
	recursive_parse_args(pCallerText);	// +4 .. 跳過第一個已知 FUNC_TOKEN / RES_FUNC_TOKEN 代碼
	pFunNow --;		pLVS = sys_argv;	// 指到內部預設參數陣列
	clear(sys_argv, sizeof(struct TNameStru) * 16);	sys_argn = 0;
	now_fun_loc_count = pFunNow->nArgs;	// 記下目前這個函數的區域變數個數
	pARs = pPF->pArgs;
	for (i = 0;i < pFunNow->nArgs;i ++) {	// 逐一計算參數數值
		pI = (int *) pFunNow->pArgs[i];		n = *pI;	// n = 其後的 32-bit dword 個數
		pI ++;		do_statements(pI, n << 2);	// 執行這條敘述
		// 把堆疊頂端運算的結果存到新借的區域變數中
		xSay("#2847 堆疊頂端運算的結果 type = $%x (parse_arguments_for_nil)", typ2);
		pLVS->pName = (char *)(size_t) sys_argn;		// 當成 index
		pLVS->flags = 0x60;			pLVS->vType = typ2;
		xmemmove(&pLVS->i_Val, val2, 4);	// move 4 bytes
		xmemmove(&pLVS->d_Val, val2, 8);	// move 8 bytes
		xSay("  [無參數解析] #2854 >> 更新數值後的 sys_argv[] : (PNameStru^)");
		seeStru(pLVS, ID_TNameStru);	// !!@@ bug: 字串需另外保存 @@!!
                if (pARs) {     // printf() 是浮動參數 = NULL ! 只擺 sys_argv[] 不借新的區域變數
		        fill_in_arg(pARs->name, pLocs);		// 把堆疊頂端運算的結果存到新借的區域變數中
		        pARs ++;
                        }
                pLVS ++;        sys_argn ++;	// 看下一個參數
	}
	xfree(pFunArgs);		pFunArgs = NULL;
	xSay("解析得系統參數個數: %d", sys_argn);
}

struct TNameStru *prepare_local_vars(struct TFuncProc *pProFun)
{   // 傳回所借的 nArgs 個區域變數起點
struct TArgs *pA;
struct TNameStru *ret;
int i, len, *pI;

	// 預備此次呼叫的區域變數
	xSay("#2819 !! 預備此次呼叫的區域變數 prepare_local_vars");
	ret = now_run_loc;	// 假設不需參數
	xSay("#2821 pPro = $%x", pProFun);
	yHexDump(pProFun, 64);
	if (! pProFun) { xSay("#2823 參數 bad pProFun");  return(ret); }		// 不需借新的區域變數
	seeStru(pProFun, ID_FuncProc);
	len = pProFun->nArgs;		if (len < 1) return(ret);	// 不需參數
	pA = pProFun->pArgs;		seeStru(pA, ID_Args);
	if (! pA) {  xSay("#2827 prepare_local_vars 錯誤: 未發現 %d 個參數 !", len);  return(ret);	}	// pArgs == NULL 不需借新的區域變數
	//     Result := now_run_loc;	// 傳回所借的 nArgs 個區域變數起點
	cur_func->cur_rolling_no = get_rolling_no();
	xSay("#2830 有 %d 個參數.", len);
	for (i = 1;i <= len;i ++) {  xSay("#2831 參數 %d: $%x", i, pA);
		allocate_local(pA);		// @@ bug @@ why not pI++ ?? 借新的區域變數,按次序填註對應 token
		pA ++;
	}
	return(ret);
}

void process_UnSetBPs(void)
{ // 處理未被處理的中斷點 !
int		*pi, *po, *pFin;
int		i, j, k;
	pi = pUBPs;		k = 0;	xSay("#2091 nUnSetBP = %d", nUnSetBP);
	for (i = 0;i < nUnSetBP; i ++) {
		pi ++;		// skip prog_id !!
		if (*pi) j = do_break_pts(*pi, Set_BreakPoint);	// 行號有效 ?
		if (j == 0) {  *pi = 0;  k ++; }		// 處理完畢, 行號清零
		pi ++;
	}
	
	if (k) { // 中斷點有被成功處理了 !
		po = pUBPs;		j = nUnSetBP;		pFin = po + (j << 1);
		while (j) {
			po ++;
			if (*po) { po ++;  j --;  continue; }	// 此處仍有中斷點 !
			po ++;
		}
		// 找到空位了 !
		pi = po + 2;
		while (j) {
			if (pi >= pFin) break;		// 檢查結束
			pi ++;
			if (*pi) { // 有資料需往前搬 !
				while (po < pFin) {
					po ++;
					if (! *po) { *po = *pi;  *pi = 0;  pi ++;  j --; break; }
					po ++;
				}
			}
			pi ++;
		}
		nUnSetBP -= k;
	}
}

void recursive_parse_args(unsigned int *pTok)
{  // --> prg_stru.pas -- line 753 --
unsigned int	pArg[32];
unsigned int	pBuf[64];	// 暫放處理中的 Token 串
unsigned int	*pT;
int		nArg, nBuf, LC;	// ndx, (LC: level count)
unsigned char pTK[4];

	// if (nowLineNo == 18) yHexDump(pTok, 16);
	xSay("#2885 recursive_parse_args, pTok = $%x", pTok); 
	pFunNow->func_token = *pTok;	// 一開始就先記住這是呼叫哪個函數 !
	pFunNow->nArgs = 0;		pTok ++;
	nArg = 0;	LC = 0;		nBuf = 0;
	clear(pArg, 128);		clear(pBuf, 256);	// ndx := 0; 
	while (*pTok) {	// 最後代碼一定是 0 !
		xmemmove(pTK, pTok, 4);		// 放入 byte[4] 方便運算
		if (pTK[0] == SYMB_TOKEN) {
			if (pTK[1] == 3) {	// 是左括號
				LC ++;	if (LC > 1) {  pBuf[nBuf] = *pTok;	 nBuf ++; }
				pTok ++;	continue; 
			}
			if (pTK[1] == 4) {	// 是右括號
				if (LC > 1) {  pBuf[nBuf] = *pTok;  nBuf ++; }
				LC --;	 pTok ++;	continue;
			}
			if (pTK[1] == 17) { // 是逗號
				if (LC < 2) { // 是第一層的參數 ! 該收集它 !
					pT = (unsigned int *) xmalloc((nBuf + 1) << 2);		pArg[nArg] = (size_t) pT; // !!@@ 32/64 bug @@!!
					*pT = nBuf;			pT ++;
					xmemmove(pT, pBuf, nBuf << 2);
					// xSay('儲存第' + IntToStr(nArg+1) + '個參數..');
					// yHexDump(@pBuf[0], nBuf shl 2);
					clear(pBuf, nBuf << 2);
					nArg ++;	nBuf = 0;
					// 這個參數的值, 需要運算後再放到該函數的區域變數中, 不在這裡處理
				}
				else {  pBuf[nBuf] = *pTok;	nBuf ++; }		// 是別的函數的逗號, 直接儲存它, 之後會處理 !
				pTok ++;	continue;
			}
		}
	pBuf[nBuf] = *pTok;		nBuf ++;	pTok ++;	// 其他的 Token --> 直接儲存它, 之後會處理 !
	}
	// 完成了 ! 把這個函數的全部參數資料包裝起來
	if (nBuf > 0) {	// 至少有一個函數
		pT = (unsigned int *) xmalloc((nBuf + 1) << 2);		pArg[nArg] = (size_t) pT; // !!@@ 32/64 bug @@!!
		*pT = nBuf;		pT ++;
		xmemmove(pT, pBuf, nBuf << 2);
		// xSay('儲存第' + IntToStr(nArg+1) + '個參數..');
		// yHexDump(@pBuf[0], nBuf shl 2);
		clear(pBuf, nBuf << 2);
		nArg ++;
	}
	pFunNow->nArgs = nArg;		xmemmove(pFunNow->pArgs, pArg, 128);	// 整個拷貝過去
	xSay("解析得到函數 $%x 的參數個數: %d",pFunNow->func_token, pFunNow->nArgs);
	for (LC = 0;LC < nArg;LC ++) {
		pT = (unsigned int *) pFunNow->pArgs[LC];	nBuf = *pT;
		xSay(">> 參數 %d, 長度 %d", LC+1, nBuf);
		pT ++;		yHexDump(pT, nBuf << 2);
	}
	pFunNow ++;		nFunCnt ++;
}

char *ret_value_str(unsigned char typ, char *pv)
{  // --> run_core.pas -- line 1324 --
unsigned int  vv;
int		*pI;
char	*ps;
// float	*pSin;	// = single
// double	*pDbl;

// 傳回指標所指數值內容, 以字串呈現
	if (!pv) return("!! NULL PTR !!");		vv = *((unsigned int *) pv);
	switch (typ & MAX_TYPE_MASK) {  // typ & 31
		case 0:	su_printf("陣列位址: $%x", cReadDW(pv, 0));		return(su_buf);	// array
		case 1:	su_buf[0] = pv[0];		su_buf[1] = 0;			return(su_buf);	// char
		case 2: su_printf("$%02x", *((unsigned char *) pv));	return(su_buf);	// byte
		case 3: su_printf("$%04x", *((unsigned short *) pv));	return(su_buf);	// word
		case 4: su_printf("$%08x", vv);	return(su_buf);	// dword
		case 5: su_printf("$%02x", *pv);						return(su_buf);	// sint8
		case 6: su_printf("$%04x", *((short *) pv));			return(su_buf);	// sint16
		case 7:
//        case 15: su_printf("$%08x", *((int *) pv));			return(su_buf);	// int, integer
		case 8: su_buf[0] = pv[0];		su_buf[1] = pv[1];		su_buf[2] = 0;		return(su_buf);	// dblchar
		case 9: xmemmove(su_buf, pv, 4);	su_buf[4] = 0;		return(su_buf);	// unicode 秀出 4 字元;
		case 10: xHexDump(pv, 16);			pI = *((size_t *) pv);	xHexDump((char *) *pI, 16);
				ps = (char *) *((int *) pv);		xmemmove(su_buf, ps+1, ps[0]); 	
				su_buf[ps[0]] = 0;	return(su_buf);		// pascal short string
		case 11: su_printf("浮點 %f", *((unsigned int *) pv));			return(su_buf);	// single = float
		case 12: su_printf("倍精度 %f", *((unsigned int *) pv));		return(su_buf);	// double
		case 13: ps = (char *) *((int *) pv);   	return(ps);	// PChar
		case 14: su_printf("指標 $%08x", vv);		return(su_buf);	 // ptr (32 bit now)
		case 15: ps = (char *) *((int *) pv);		su_printf("字串 $%08x (%d, %s)", vv, ps[0], ps);	return(su_buf);	 // C string ptr
		case 16: su_printf("$%lx", *((long *) pv));			return(su_buf);	// sint64 (!!@@ 64 bit bug @@!!)
		case 17: su_printf("$%lx", *((unsigned long *) pv));	return(su_buf);	// qword (!!@@ 64 bit bug @@!!)
		default: return("無此型態 !");
	}
}

void run_a_func(void *func, struct TNameStru *pLoc, char bMyFunc)
{ // 由 token 或 start 或 執行內部函數, 最候都呼叫到這邊 !
char *fNow, *fTail, *save_final_addr;
struct TFuncStru *fu;	// 參看 prg_stru.pas 的第 16 行
    // 新版: func 是一個 struct TMyFunc *
    // 從外部來執行一個函式,
	// xSay("#2131 執行一個函式 >> ");  yHexDump(func, 0x200);
	if (bMyFunc) fu = (struct TFuncStru *) MyFunc2FuncStru(func);
    else fu = (struct TFuncStru *) func;        // 原本 func 是 PFuncStru
	// xSay("#2133 執行一個函式 fu = %x", fu);
    fNow = (char *) fu->prog_addr;
    if (ErrorCode) return;	// 有嚴重錯誤 !
	// xSay("#2136 函式 >> ");  yHexDump(fu, sizeof(struct TFuncStru));
    now_use_locals = pLoc;	// 記住現在該取得的區域變數群 !
    xSay("#2451 執行一個函式, 進入點: $%x --> %s", fNow, fu->pName);
	now_run_prog = fu->Prg_ndx;
    fTail = fNow + fu->prog_len;
	save_final_addr = final_addr;
    xSay("#2454 執行一個函式, 終止點: $%x", fTail);
    final_addr = fTail;         // 設定這個層次的執行終點 !
    cur_func->pFin = fTail;     bMakeExit = 0;
    fu->ref_count ++;           // 被參考次數加一
    // 設定此函式的常數變數初值
    if ((!fNow) || (!fTail)) do_sys_error(errBadExecAddr, 0);
    else exec_here(fNow);      // 從這邊跳進去執行 !
	final_addr = save_final_addr;
}

void run_main_start(void)
{	// 執行唯一的 start() ! << 參考 prg_stru.pas #1123 >>
int	i;
struct TMyFunc *pFN;
void *fu;
struct TNameStru *pLocals, *pLEnd;	// 指到此次執行所借的區域變數群
struct TFuncProc *pPF;

	if (bSysErr) return;      // 有嚴重錯誤, 勿執行 !
	
	mark(F_run_main_start, F_Enter);
	// 找尋 start 函式..
	fu = find_function_by_name("start");	// return TMyFunc* or TMyProc*
	xSay("#2998 start = $%x", fu);
	if (!fu) xSay("run_main_start: Bad ! No [Start] !");
	else {	// 找到 start 了 !
		// 準備執行它, 準備執行前相關動作 (break point, debug, etc..)
		fGiveUp = 0;			bUserStop = 0;		fOnDebug = 0;	    bSysErr = 0;
		do_prepare_to_run();	// 為最初跳進去 start() 的過程作準備
		xSay("#3004 cur_func = $%x", cur_func);	
		// 填入執行前的父程序資訊
		cur_func->pFunc = fu;	// = 來自於哪個函式
		now_run_prog = ((struct TFuncStru *) cur_func->pFunc)->Prg_ndx;
		cur_func->LineNo = -1;	// -1 = 從 start 之前執行 !
		cur_func->LocPos = 0;	//
		cur_func->pExec = NULL;	// 執行 start 後, 就不用再執行了 !
		cur_func->pFin = NULL;	// 執行 start 後, 就不用再執行了 !
		cur_func->Result_Type = ((struct TMyFunc *) fu)->retType;
		clear(cur_func->iResult, 12);	// 傳回值預先填零
		// yHexDump(fu, 160);	// 已經不能用 index 法了 !
		// 準備好 tokenAddr, pArgs
		pFN = (struct TMyFunc *) fu;
		pPF = get_FuncProc_byName(pFN->name);
		// xSay("#1137 pPF = $%x", pPF);
		pLocals = prepare_local_vars(pPF);	// 準備主程式需要的區域變數 (undone #992)
		pLEnd = now_run_loc;
		xSay("#3021 準備好主程式需要的區域變數了 !");
		// xSay(Format('pB = %x, pE = %x', [Integer(pLocals), Integer(pLEnd)]));
		pCallerText = NULL;
		CallerLen = 0;	// 代表這是 start !
		parse_arguments(pPF, pLocals);	// 剖析後面的參數到適當的區域變數中
		run_a_func(fu, NULL, 1);	// 從外部來執行一個函式
		get_result(val2);	typ2 = final_type;	// 剖析 Result[] 的值 !

		free_local_vars(pLocals, pLEnd);	// 釋放所借的區域變數
		canSay = 1;
		xSay("執行 start() 的傳回值 = $%x", cReadDW(val2, 0));
        xSay(" 程式結束 --> 最終傳回值為 : %s", ret_value_str(final_type, val2));
//        debug_show_current_line_text(' 程式結束 --> 最終傳回值為 : ' + ret_value_str(final_type, @val2[0]));
//		debug_sys(7);
	}
	xSay("#3036 run start 完成 !");
	mark(F_run_main_start, F_Bye);
}

void myprintf(void)
{
struct TNameStru *pLVS;
char *pFmt, *pBuf, *p01;
int		len;

if (sys_argn < 1) { xSay("#3082 exec printf(無參數).");  return; }

if (sys_argn == 1) {
        pLVS = sys_argv;	// 指到內部預設參數陣列
status_update(3086, PRINTF_ANS, now_run_prog | (exec_line_No << 8), (char *) pLVS->i_Val);
        if ((pLVS->vType & MAX_TYPE_MASK) == Type_Code_PCHAR) { // 13 = PChar
                pFmt = (char *) pLVS->i_Val;    // !!@@ 32/64 @@!!
                status_update(2526, PRINTF_ANS, now_run_prog | (exec_line_No << 8), pFmt);
                xSay("#3090 printf(%s).", pFmt);
                return;
                }
        if ((pLVS->vType & MAX_TYPE_MASK) == 10) { // 10 = pascal string
				xSay("#3094 (printf) pascal string ptr = $%x", pLVS->i_Val);
                if (pLVS->i_Val < 0x100000) { xSay("#3094 (printf) bad string ptr ! ($%x)", pLVS->i_Val);  return; }
                p01 = (char *) pLVS->i_Val;     // !!@@ 32/64 @@!! 
                len = *p01 + 3;       if ((len < 0) || (len > 255)) len = 255;
                pFmt = (char *) xmalloc(len + 3);
                xstrncpy(pFmt, p01 + 1, len);   	pFmt[len] = 0;
/*                p01 = (char *) pLVS->i_Val;     // !!@@ 32/64 @@!! 新格式: p01[0] 放字串長度, p01[1..4]=字串指標
                len = *p01;         if ((len < 0) || (len > 252)) len = 253;
                pFmt = (char *) xmalloc(len + 3);	 p01 = (char *) *((int *)(p01 + 1));	// p01[1..4]=字串指標
                xstrncpy(pFmt, p01, len);   	pFmt[len] = 0; */
                status_update(3099, PRINTF_ANS, now_run_prog | (exec_line_No << 8), pFmt);
                xSay("#3100 printf(%s).", pFmt);
                xfree(pFmt);
                return;
                }
		cSayN(3, "#3125 printf failed, (1 param)");
        }
if (sys_argn == 2) {
        pLVS = sys_argv;	// 指到內部預設參數陣列
// status_update(3010, PRINTF_ANS, now_run_prog | (exec_line_No << 8), (char *) pLVS->i_Val);
        if (pLVS->vType == Type_Code_PCHAR) { // 13 = PChar
                pBuf = (char *) xmalloc(256);
                pFmt = (char *) pLVS->i_Val;    // !!@@ 32/64 @@!!
                pLVS ++;
                sPrt(pBuf, pFmt, pLVS->i_Val);
                status_update(2548, PRINTF_ANS, now_run_prog | (exec_line_No << 8), pBuf);
                xSay("#3136 printf(%s).", pBuf);
                xfree(pBuf);
                return;
                }
        }
if (sys_argn == 3) {
        pLVS = sys_argv;	// 指到內部預設參數陣列
        if (pLVS->vType == Type_Code_PCHAR) { // 13 = PChar
                pBuf = (char *) xmalloc(256);
                pFmt = (char *) pLVS->i_Val;    // !!@@ 32/64 @@!!
                pLVS ++;        p01 = (char *) pLVS->i_Val;     pLVS ++;  // !!@@ 32/64 @@!!
                sPrt(pBuf, pFmt, p01, pLVS->i_Val);
                status_update(3148, PRINTF_ANS, now_run_prog | (exec_line_No << 8), pBuf);
                xSay("#3128 printf(%s).", pBuf);
                xfree(pBuf);
                return;
                }
        }
xSay("#3133 (printf) 不明型態 $%x , ival = $%x", pLVS->vType, pLVS->i_Val);
}

void myreturn(void)
{
struct TCaller *src;
struct TNameStru *pLVS;
char *pFmt, *pBuf, *p01;

if (sys_argn < 1) { xSay("#2575 exec return(無參數).");  return; }
if (sys_argn == 1) {
        pLVS = sys_argv;	// 指到內部預設參數陣列
        src = callers + (callerStack - 1);
        src->Result_Type = pLVS->vType;
        result_str[0] = 0;      src->iResult[0] = (size_t) result_str; // !!@@ 32/64 @@!!
        if ((pLVS->vType & MAX_TYPE_MASK) == Type_Code_PCHAR) { // 13 = PChar
                if (pLVS->i_Val < 0x100000) {
                        xSay("#2629 (return) bad string ptr ! ($%x)", pLVS->i_Val);
                        return;
                        }
                pFmt = (char *) pLVS->i_Val;    // !!@@ 32/64 @@!!
                xstrcopy(result_str, pFmt);
                status_update(2635, RETURN_ANS, now_run_prog | (exec_line_No << 8), pFmt);
                xSay("#2635 return(%s).", pFmt);
                return;
                }
        if ((pLVS->vType & MAX_TYPE_MASK) == 10) { // 10 = pascal string
                if (pLVS->i_Val < 0x100000) { xSay("#2477 (printf) bad string ptr ! ($%x)", pLVS->i_Val);  return; }
                p01 = (char *) pLVS->i_Val;     // !!@@ 32/64 @@!!
                pFmt = (char *) xmalloc(*p01 + 3);
                xstrncpy(pFmt, p01 + 1, *p01);   pFmt[*p01] = 0;
                xstrcopy(result_str, pFmt);
                status_update(2645, RETURN_ANS, now_run_prog | (exec_line_No << 8), pFmt);
                xSay("#2645 return(%s).", pFmt);
                xfree(pFmt);
                return;
                }
        }
xSay("#2659 (return) 不明型態 $%x , ival = $%x", pLVS->vType, pLVS->i_Val);
}

int _stdcall Run_Program(void *h_Wait)
{ // 執行程式 (cf RobotMan.pas, line 120)
    hWait = h_Wait;			// to stop program !
    run_main_start();     		// 執行唯一的 start() !
	// if workID <> 0 then goto quit_mlt;	// 防止重入
    // if (hThr) hThr = 0;			
	xSay("-------- 2613 Exit Run_Program--------");
    // do_BPs_work;	// 清除暫時斷點, 重新設置固定斷點
    return(0);	// errCode = 0 (no Error)
}

int  run_resident_func(int func_id, struct TNameStru *pLocVs)
{ // 從外部來執行一個內建函式
    // 解析 tokens, 把各個參數結果填到新產生的區域變數中
	xSay("#2670 run_resident_func() -> dump pThisLine, len=32: (& next dump pLocVs, len=32)");
    yHexDump(pThisLine, 32);	// 確定指令正確
    if (pLocVs) yHexDump(pLocVs, 32);
    switch(func_id) {
        case 0: myprintf(); break; // printf(format_str: string, ...);
        case 1: myreturn(); break; // return();
    }
    return(0);
}

void _stdcall Run_was_done(void)
{	// call from  TBkgCtrl.do_run line 294
	xSay("-> Run_was_done (清除 mainProg 記憶體)");
	// if (mainProg) xfree(mainProg);		mainProg = 0;
}

void save_toSetBP(int progID, int lineNo)
{ // 保存尚未設定好的斷點資訊 !
int	*pUB;
	
	if (nUnSetBP > 512) { xSay("太多尚未設定好的斷點 (此斷點未被處理) ! (表格項數 > 512)");  return; }
	if (! pUBPs) pUBPs = (int *) xmalloc(4096);	// = 512 * 8  { progId, lineNo }
	pUB = pUBPs;	pUB += (nUnSetBP << 1);
	*pUB = progID;		pUB[1] = lineNo;	nUnSetBP ++;
}

void seeStru(void *p, int ID_stru)
{ // 檢閱結構內涵
struct TArgs *pAR;
struct TFuncProc *pFP;
struct TFuncStru *pFS;
struct TNameStru *pNS;
char *pNm;

if (bad_ptr(p)) { xSay("seeStru < bad p = $%x >, ID = %d", p, ID_stru);  return; }
switch(ID_stru) {
        case ID_TNameStru:
                pNS = (struct TNameStru *) p;
                pNm = pNS->pName;       if ((size_t) pNm < 0x10000) pNm = "無名稱";
                xSay("< NameStru $%x: %s> 程式:%d, 來源行號: %d, flag $%x", p, pNm, pNS->prgID, pNS->lineNo, pNS->flags);
                xSay("  vType $%x, initNdx %d, iVal %d ($%x)", pNS->vType, pNS->initNdx, pNS->i_Val, pNS->i_Val);
                break;
        case ID_FuncProc:
                pFP = (struct TFuncProc *) p;
                pNm = pFP->name;        if ((size_t) pNm < 0x10000) pNm = "無名稱";
                xSay("< FuncProc $%x: %s> 來源行號: %d, ret_Type $%x, nArgs %d, pArgs $%x", p, pNm, pFP->lineNo, pFP->ret_Type, pFP->nArgs, pFP->pArgs);
                break;
        case ID_Args:
                pAR = (struct TArgs *) p;
                pNm = pAR->name;        if ((size_t) pNm < 0x10000) pNm = "無名稱";
                xSay("< Args $%x: %s> 來源行號: %d, flags $%x, vType $%x, loc_match %d", p, pNm, pAR->line_no, pAR->flags, pAR->vType, pAR->loc_match);
                break;
        case ID_FuncStru:
                pFS = (struct TFuncStru *) p;
                pNm = pFS->pName;       if ((size_t) pNm < 0x10000) pNm = "無名稱";
                xSay("< FuncStru $%x: %s> 參數個數: %d, retType $%x", p, pNm, pFS->argCount, pFS->retType);
                break;
        }
}

int _stdcall set_break_line(int prog_id, int line_no)
{ // 設置中斷點, 傳回尚可設置 MAX_BREAK_PTS - nBrkPts 數量 (0 = just full, -1 = other error)
struct BRK_PTS *pBP;
int		i;
	if (! brkps) { brkps = (struct BRK_PTS *) xmalloc(sizeof(struct BRK_PTS) * MAX_BREAK_PTS);	nBrkPts = 0; }	// same as init_all_before_Run()
	pBP = brkps;
	for (i = 0;i < nBrkPts;i ++) {
		if (pBP->program_id == (unsigned int) prog_id) {
			if (pBP->line_number == (unsigned int) line_no) {
				if (do_break_pts(line_no, Set_BreakPoint)) save_toSetBP(prog_id, line_no);		// 設立中斷點
				return(MAX_BREAK_PTS - nBrkPts);	// 傳回尚可設置 MAX_BREAK_PTS - nBrkPts 數量
			}
		}
		pBP ++;
	}
	// 沒找到 !
	if (nBrkPts >= MAX_BREAK_PTS) { xSay("中斷點記錄區已滿 !");  return(0); }	// 0 = table full
	pBP->program_id = prog_id;			pBP->line_number = line_no;		nBrkPts ++;
	if (do_break_pts(line_no, Set_BreakPoint)) save_toSetBP(prog_id, line_no);		// 設立中斷點
	return(MAX_BREAK_PTS - nBrkPts);	// 傳回尚可設置 MAX_BREAK_PTS - nBrkPts 數量
}

void _stdcall set_step_trace_flag(int flag)		// set flag to bStepTrace 
{ // flag:   0=Not Enabled Stop, 1=Step Stop Enabled, 2=Trace Stop Enabled)
	bStepTrace = flag & 3;		// Step 不會進入函數, Trace 會進入函數
}

void set_watch_var(void *pNS, int var_ndx)
{ // 更新結果顯示
	if (! pNS) return;		// invalid ptr (通常是 struct TNameStru *)
	status_update(nowLineNo, SHOW_VAR, var_ndx, pNS);
}

// #define AV_TRY { try {
// #define AV_CATCH } catch(EAccessViolation &av) {Application->MessageBox((("Access Violation caught: " + string(__FILE__) + "; " + string(__FUNC__) + "; " + IntToString(__LINE__) + "\n\n") + av.Message.c_str()).c_str(), ("Program Error in " + string(class_name.c_str())).c_str(), MB_OK);} }

void su_printf(char *fmt, ...)
{
va_list	ap;
// int     i;
// 若印出的 %s 裡面還有 format 格式字串, printf 將會有不可預期的結果 ! 可改成 puts() 但會多出一個換行 !
	va_start(ap, fmt);
    wvsprintf(su_buf, fmt, ap);
	va_end(ap);
}

void sort_global_vars(void)
{
int	i;
struct TNameStru *pV;
struct TVarNames *pVN;

	// 針對全域變數/常數/陣列排序, 數值資料取用時, 優先由函數內局部變數取用, 之後再找相同 PrgID 之數值, 最後才找全域性
	if (pGlobVars) xfree(pGlobVars);	// 先釋放舊記憶體
	// 統計該借多少記憶體
	pGlobVars = (struct TVarNames *) xmalloc(now_var_count * sizeof(struct TVarNames));	// 第一個 dword = (all_var 變數或常數) index, 第二個 dword = 變數或常數名稱字串位置
	pVN = pGlobVars;		pV = all_var;
	for (i = 0;i < now_var_count;i ++) {	// 準備排序, 填入資料
		pVN->index = i;		pVN->pName = pV->pName;
		pVN ++;		pV ++;
	}
//	SetSortFCMP(myStrComp);
//	QSortBy(pGlobVars, 4, 8, now_var_count);	// 執行排序
    //EndThread(0);
}

int  status_update(int line_no, int fun_code, int fun_data, void *fun_ptr)
{	// 更新到上層系統介面
    return(status_updater(line_no, fun_code, fun_data, fun_ptr));
}

int tk_job_every_line(unsigned char *pTok, int job_id, int tk_len)
{
int		len, bErr;
	tk_len = tk_len >> 2;		bErr = 1;
	// dbp(job_id, tk_len, " <- #3288 tk_job_every_line (job_id, token 數)");
	while (tk_len > 0) {
		if (*pTok == LineNo_TOKEN) { // 此為某一行的開頭
			if (job_id == Clear_BreakPoint) { pTok[3] &= (ENABLE_BREAK ^ 0xFF);  bErr = 0; }	// clear  break point !
			if (job_id == Set_BreakPoint) { pTok[3] |= ENABLE_BREAK;  bErr = 0;	}			// enable break point !
			len = (pTok[1] + 2) | (pTok[2] << 8);			tk_len -= len;
			pTok += (len << 2);
		}
		else { tk_len --;  pTok += 4; }		// 不應執行此處 (代表代碼表有誤)
	}
	return(bErr);
}

int tk_job_one_line(unsigned char *pTok, int job_id, int line_no, int tk_len)
{
int		len, lino, bErr;
char ss[64];
	tk_len = tk_len >> 2;		bErr = 1;
	// dbp(line_no, tk_len, "<- #3304 tk_job_one_line (修改 Token 內容的行號, token 數) ");
	while (tk_len > 0) {
		if (*pTok == LineNo_TOKEN) { // 此為某一行的開頭
			len = (pTok[1] + 2) | (pTok[2] << 8);		tk_len -= len;
			lino = *((int *) (pTok + (len << 2) - 4)) >> 8;			// 此為某一行的行號
			if (lino > line_no) break;
			if (lino == line_no) {
				if (job_id == Clear_BreakPoint) { pTok[3] &= (ENABLE_BREAK ^ 0xFF);  return(0); }		// clear break point !
				if (job_id == Set_BreakPoint) { pTok[3] |= ENABLE_BREAK;  return(0); }				// enable break point !
			}
			pTok += (len << 2);
		}
		else { tk_len --;  pTok += 4; }		// 不應執行此處 (代表代碼表有誤)
	}
	return(bErr);
}

void *try_match_local_var(struct TNameStru *pV)
{	// pV is TNameStru *
int		n;
struct TNameStru *pL;

	// 倒退找看看有沒有跟 pN^ 所指的變數名稱相同的區域變數
	n = now_locv;	// 取得目前這個函數的區域變數個數
	xSay("目前這個函數的區域變數個數: %d", n);
	n --;           pL = all_run_loc + n;   // now_use_locals
	while (n >= 0) {
		if (pL->pName) if (xstrcmp(pV->pName, pL->pName) == 0) { // 找到同名新增區域變數 !!@@ stricmp 未設計
			if (loc_roll_n[n] != cur_func->cur_rolling_no) { // 如果 loc_roll_n[n] 遇到 0, 需往前推
				xSay("找到同名新增區域變數, 但滾動值不對 ! (%d vs %d)", loc_roll_n[n], cur_func->cur_rolling_no);
				}
			else	{
				xSay("  [找同名新增區域變數] 找到了: $%x", (size_t) pL);
				seeStru(pL, ID_TNameStru);
				return(pL);     // 有找到更好的名新增區域變數
			}
		}
		pL --;	n --;
	}
	return(pV);	// 預設值: 沒找到 !
}

void try_translate_tokens(unsigned char *pT, int len)
{ // 嘗試翻譯回原本的指令
struct TUndoneEval *pUDE;
struct TUndefs *pUD;
int     i, j;
unsigned char b;
char s[256], st[128];
va_list	ap;

    i = 0;      s[0] = 0;       st[0]= 0;       xSay("#2804: 嘗試翻譯 $%x, len= %d", pT, len);
    while (i < len) {
        b = pT[i + 1];
        j = b + (pT[i + 2] << 8);       // use 16 bit !!
        switch(pT[i]) {
            case FUNC_TOKEN:
                if (!pT[i + 3]) { // 取得呼叫函數的整個字串 !
                    if (j < 0x7E00) xstrcat(s, get_undone_calls(b));
                    else {
                        pUD = undefs;   pUD += (0x7FFF - j);
                        xstrcat(s, pUD->pName);
                    }
                }
                else { // 這是切出的程式區塊
                    pUDE = pUnDoneEval;         pUDE += j;
                    xSay("#2819 呼叫切出的程式區塊內容如下..");
                    xSay("#2820 程式區塊數 = %d", mainProg->cnt_excblk);
                    yHexDump(pUDE->pTokens, pUDE->nTokens << 2);
                    try_translate_tokens(pUDE->pTokens, pUDE->nTokens << 2);
                }
                break;
            case VAR_TOKEN: xstrcat(s, get_var_name_by_ndx(b));  break; // 取得變數名稱
            case SYMB_TOKEN:
                if (b > 21) xstrcat(s, symbol2[b - 22]);
                else { st[0] = symbols[b];  st[1] = 0;  xstrcat(s, st); }
                break;
            case TEMP_STR_TOKEN:
                    // xSayStrHex('temp str id = ', b, 1);
                    st[0] = 39;    st[1] = 0;   xstrcat(s, st);
                    xstrcat(s, temp_str[b]);    xstrcat(s, st);
                break;
            case CONST_TOKEN: xstrcat(s, get_const_by_ndx(b-1));  break;  // 取得常數名稱或值
            case RESV_TOKEN: xstrcat(s, sys_cmd[b]);  break;
            case BLOCK_TOKEN:
                sPrt(st, "程序小區塊(%d)\n", b);
                xstrcat(s, st);
                if (i > 4) {
                    i -= 4;     // 檢查前一個代碼是 then 嗎 ?
                    if ((pT[i] == RESV_TOKEN) && (pT[i+1] == TOKEN_ID_THEN)) {
                        if (!pT[i + 2]) { st[0] = ';';  st[1] = 0;  xstrcat(s, st); }   // 加放個分號代表後面無 else !
                    }
                    i += 4;
                }
                break;
            case RES_FUNC_TOKEN:  xstrcat(s, resident_funs[j].name);  break;
        }
        st[0] = 32;  st[1] = 0;  xstrcat(s, st);
        i += 4;
    }
    // s = Trim(s);
    if (bNoToDebug) xstrcopy(sStrBack, s);
    else { xSay(s);  status_update(2856, SHOW_INSTRUCT, now_run_prog | (exec_line_No << 8), s); }
    //debug_show_current_line_text(s);   // 顯示正要被執行的命令
    xSay("#2858: 嘗試翻譯 done !");
}

char *sIntToStr(long v)
{
  sPrt(ssTemp, "%d", v);
  return(ssTemp);
}

void debug_sys(char fun)
{
unsigned long i64;
int     i, j;
SuFloat rs;
SuDoubl rd;
struct TArrayStru *pA;
struct TConstStru *pC;
struct TMyFixVal *pFV;
struct TNameStru *pN;
struct TLocalStru *pL;
struct TFuncStru *pFn;
struct TMyConst *pCN;
struct TMyFunc *pFP;
char *p;
char s[256], s1[256];

    if (fun & 1) {
        pA = all_array;         xSay("陣列一覽 --");
        for (i = 0;i < array_count;i ++) {
            // with pA^ do begin
                pN = all_var;      pN += pA->Ndx;       // 陣列名稱寄放在 all_var[pA->Ndx] 中
                xstrcopy(s, pN->pName);         j = pA->vType & MAX_TYPE_MASK;
                xSay("%d: %s: [%d..%d] of %s p=$%x", i, s, pA->LwrBound, pA->UprBound, TypeModifier[j], pA->pData);
                pA ++;
        }
    }
    if (fun & 2) {
        pN = all_var;           xSay("變數常數一覽 --");
        for (i = 0;i < now_var_count;i ++) {
			// set_watch(pN);
            // with pN^ do begin
            j = pN->vType & MAX_TYPE_MASK;              clear(s, 256);
            sPrt(s, "%d: %s: %s 現值= ", i, pN->pName, TypeModifier[j]);
            switch(j) { // 0:array
                case 1: s1[0] = pN->i_Val & 255;  s1[1] = 0;  xstrcat(s, s1);  break;   // char
                case 2: xstrcat(s, sIntToStr(pN->i_Val & 0xFF));    break;  // byte
                case 3: xstrcat(s, sIntToStr(pN->i_Val & 0xFFFF));  break;  // word
                case 4: xstrcat(s, sIntToStr((unsigned int) pN->i_Val));  break; // dword
                case 5: xstrcat(s, sIntToStr(pN->i_Val & 0xFF));    break;  // sint8  (暫未 sign extend)
                case 6: xstrcat(s, sIntToStr(pN->i_Val & 0xFFFF));  break;  // sint16 (暫未 sign extend)
                case 7: xstrcat(s, sIntToStr(pN->i_Val));  break; // int
                case 8:
                case 9:
                case 13: xstrcat(s, "32/64 Err");  break;      // xstrcat(s, (char *) pN->i_Val); dblchar, unicode, string, PChar (!!@@ 32/64 bit Err @@!!)
                case 10: xstrcat(s, " str: ");  xstrcat(s, "32/64 Err");  break;  // xstrcat(s, (char *) pN->i_Val); 暫未轉換 (!!@@ 32/64 bit Err @@!!)
                case 11: a1int_single(pN->i_Val, &rs);  xstrcat(s, sSingleToStr(&rs));  break;  // single
                case 12: xstrcat(s, sDoubleToStr((SuDoubl *) &pN->d_Val));  break; // double
//                case 14: s := s + IntToStr(Longword(cReadDW(pL^.pName, 0))); // ptr
                case 15:
                case 16:  // CT_S64, CT_QWORD
                        cSetDW(&i64, 0, pN->i_Val);
                        cSetDW(&i64, 4, (size_t) pN->pName);
                        xstrcat(s, sIntToStr(i64));
                        break;
            } // -- switch
            xstrcat(s,  ", ");
            if (!(pN->flags & 0x80)) xstrcat(s, "初值未定義之");
            if (pN->flags & 0x40) xstrcat(s, "變數");   else xstrcat(s, "常數, ");
            if (pN->initNdx > 0) {
                p = get_InitVal_str(pN->initNdx, pN->flags);
                xstrcat(s, p);          xfree(p);
				xstrcat(s, ", 來自行號 ");  xstrcat(s, sIntToStr(pN->lineNo));
            } // -- if
            xSay(s);
            pN ++;
        } // -- for
        // ----------- 局部變數一覽
        pL = all_local;		xSay("局部變數定義一覽 --");
        for (i = 0;i < now_loc_count;i ++) {
            // set_local_watch(pL);
            // with pL^ do begin
            j = pL->vType & MAX_TYPE_MASK;      clear(s, 256);
            pFP = (struct TMyFunc *) pL->func_prc;
            if (!pFP) xstrcopy(s1, "(沒填函數)");
            else xstrcopy(s1, pFP->name);
            if ((size_t) pL->pVar < 0x400000) sPrt(s, "不明變數: 來自函數 %s: %s (位於 %d 行)", s1, TypeModifier[j], pL->lineNo);
            else sPrt(s, "%s: 來自函數 %s: %s (位於 %d 行)", pL->pVar->pName, s1, TypeModifier[j], pL->lineNo);
            xstrcat(s, ": ");  xstrcat(s, TypeModifier[j]);  xstrcat(s, "初值= ");
            switch(j) { // 0:array
                case 1: s1[0] = pL->i_Val & 255;  s1[1] = 0;  xstrcat(s, s1);  break;   // char
                case 2: xstrcat(s, sIntToStr(pL->i_Val & 0xFF));    break;  // byte
                case 3: xstrcat(s, sIntToStr(pL->i_Val & 0xFFFF));  break;  // word
                case 4: xstrcat(s, sIntToStr((unsigned int) pL->i_Val));  break; // dword
                case 5: xstrcat(s, sIntToStr(pL->i_Val & 0xFF));    break;  // sint8  (暫未 sign extend)
                case 6: xstrcat(s, sIntToStr(pL->i_Val & 0xFFFF));  break;  // sint16 (暫未 sign extend)
                case 7: xstrcat(s, sIntToStr(pL->i_Val));  break; // int
                case 8:
                case 9:
                case 13: xstrcat(s, "32/64 Err");  break;      // xstrcat(s, (char *) pL->i_Val); dblchar, unicode, string, PChar (!!@@ 32/64 bit Err @@!!)
                case 10: xstrcat(s, " str: ");  xstrcat(s, "32/64 Err");   break;  // xstrcat(s, (char *) pL->i_Val); 暫未轉換 (!!@@ 32/64 bit Err @@!!)
                case 11: a1int_single(pN->i_Val, &rs);  xstrcat(s, sSingleToStr(&rs));  break;  // single
                case 12: xstrcat(s, sDoubleToStr((SuDoubl *) &pN->d_Val));  break; // double
//                case 14: s := s + IntToStr(Longword(cReadDW(pL^.pName, 0))); // ptr
                case 15:
                case 16:  // CT_S64, CT_QWORD
                        cSetDW(&i64, 0, pL->i_Val);
                        cSetDW(&i64, 4, (size_t) pL->pVar->pName);
                        xstrcat(s, sIntToStr(i64));
                        break;
            } // -- switch
            xSay(s);    pL ++;
        } // -- for
        // ----------- 運作中的局部變數一覽
		pN = all_run_loc;		xSay("運作中的局部變數一覽 --");
        for (i = 0;i < now_locv;i ++) {
			// set_watch(pN);
            // with pN^ do begin
            j = pN->vType & MAX_TYPE_MASK;
            sPrt(s, "%d: %s: %s 現值= ", i, pN->pName, TypeModifier[j]);
            switch(j) { // 0:array
                case 1: s1[0] = pN->i_Val & 255;  s1[1] = 0;  xstrcat(s, s1);  break;   // char
                case 2: xstrcat(s, sIntToStr(pN->i_Val & 0xFF));    break;  // byte
                case 3: xstrcat(s, sIntToStr(pN->i_Val & 0xFFFF));  break;  // word
                case 4: xstrcat(s, sIntToStr((unsigned int) pN->i_Val));  break; // dword
                case 5: xstrcat(s, sIntToStr(pN->i_Val & 0xFF));    break;  // sint8  (暫未 sign extend)
                case 6: xstrcat(s, sIntToStr(pN->i_Val & 0xFFFF));  break;  // sint16 (暫未 sign extend)
                case 7: xstrcat(s, sIntToStr(pN->i_Val));  break; // int
                case 8:
                case 9:
                case 13: xstrcat(s, "32/64 Err");  break;      // xstrcat(s, (char *) pN->i_Val); dblchar, unicode, string, PChar (!!@@ 32/64 bit Err @@!!)
                case 10: xstrcat(s, " str: ");  xstrcat(s, "32/64 Err");  break;  // xstrcat(s, (char *) pN->i_Val); 暫未轉換 (!!@@ 32/64 bit Err @@!!)
                case 11: a1int_single(pN->i_Val, &rs);  xstrcat(s, sSingleToStr(&rs));  break;  // single
                case 12: xstrcat(s, sDoubleToStr((SuDoubl *) &pN->d_Val));  break; // double
//                case 14: s := s + IntToStr(Longword(cReadDW(pL^.pName, 0))); // ptr
                case 15:
				case 16:  // CT_S64, CT_QWORD
                        cSetDW(&i64, 0, pN->i_Val);
                        cSetDW(&i64, 4, (size_t) pN->pName);
                        xstrcat(s, sIntToStr(i64));
                        break;
            } // -- switch
            xstrcat(s,  ", ");
            if (pN->flags & 0x80) xstrcat(s, "初值未定義之");
            if (pN->flags & 0x40) xstrcat(s, "變數");   else xstrcat(s, "常數, ");
            if (pN->initNdx > 0) {
                pC = all_const;         pC += (pN->initNdx - 1);    // !! 常數表索引由一算起 !!
                xstrcat(s,  "初值= ");
                switch(pC->cType) {
                    case CT_STRING: xstrcat(s, pC->pName);  break;
                    case CT_CHAR: s1[0] = (size_t) pC->pName & 255;  s1[1] = 0;  xstrcat(s, s1);  break;
                    case CT_SINGLE: a1int_single(pC->i_Val, &rs);  xstrcat(s, sSingleToStr(&rs));  break;
                    case CT_DOUBLE:
                        cSetDW(&rd, 0, pC->i_Val);
                        cSetDW(&rd, 4, (size_t) pC->pName);
                        xstrcat(s, sDoubleToStr((SuDoubl *) &rd));
                        break;
                    case CT_S64:
                    case CT_QWORD:
                        cSetDW(&i64, 0, pC->i_Val);
                        cSetDW(&i64, 4, (size_t) pC->pName);
                        xstrcat(s, sIntToStr(i64));
                        break;
                    default: xstrcat(s, sIntToStr(pC->i_Val));
                }
				xstrcat(s, ", 來自行號 ");  xstrcat(s, sIntToStr(pN->lineNo));
            } // -- if
            xSay(s);
            pN ++;
        } // -- for
    } // -- if (fun & 2)
    if (fun & 4) {
        pC = all_const;         xSay("常數資料一覽 --");
        for (i = 0;i < nConsts-1;i ++) {
            // with pC^ (=struct TConstStru) do begin
            j = pC->cType & CT_MASK;    // flag $80= 常數 Done OK !
            if (!j) { pC ++;  continue; }  // flag $80= 0 (bad ! 應是空格)
            if (pC->pName) {
                sPrt(s, "%d: %s: %s flag=$%x, 或是數值= ", i, TypeModifier[j], pC->pName, pC->flags);
                pCN = (struct TMyConst *) pC->PMyConst;
                if (pCN->initNdx) xstrcat(s, get_fixval_as_str(pCN->initNdx));
                else { // 舊規格
                    p = get_InitVal_str(pN->initNdx, pC->flags);
                    xstrcat(s, p);  xfree(p);
                }
            }
            else {
                pFV = (struct TMyFixVal *) pC->PMyConst;        j = pFV->cType;
                sPrt(s, "%d: %s: flag=$%x, 或是數值= %s", i, ConstTypes[j], pFV->flag, pFV->initVal);
            }
            xSay(s);
            pC ++;
        } // -- for
    } // -- if
    if (fun & 8) {
        xSay("程式一覽 --");
        for (i = 1;i < prg_cnt;i ++)
            xSay("程式名稱: %s, 總行數 %d 行", pss[i].pName, pss[i].nLines);

        pFn = func_stru;        xSay("程序/函數資料一覽 --");
        for (i = 0;i < func_stru_cnt;i ++) {
            // with pFn^ do begin
            s1[0] = 0;
            if (pFn->fncType & 0x80) xstrcat(s1, "不明的 ");
            if (pFn->fncType & 0x40) xstrcat(s1, "external ");
            if (pFn->fncType & 1) xstrcat(s1, "function ");  else xstrcat(s1, "procedure ");
            xSay("%s %s 位於程式 %s 第 %d 行至第 %d 行, 傳回值型態= %X, 被呼叫過的次數= %d", s1,
                pFn->pName, pss[pFn->Prg_ndx].pName, pFn->bgnLine, pFn->finLine, pFn->retType, pFn->ref_count);
            xSay("   程式碼位於 $%x, 程式碼長度: %d bytes", pFn->prog_addr, pFn->prog_len);
            pFn ++;
        }
        xSay("*** 未被解析的函數個數 = %d", nUndef_Func);
        xSay("未被解析的程序/函數資料一覽 --");
        for (i = 0;i < nUndef_Func;i ++) // with undefs[i] do begin
            xSay("%s 的第 %d 行: %s (id= %d)", pss[undefs[i].Prg_ndx].pName, undefs[i].lineNo, undefs[i].pName, undefs[i].qq);
    }
    // if (fun & 16) do_debug();
}

int wait_for_debug(int code_in)
{ // 要被除錯中斷 !  --->  PulseEvent(hWait); // 繼續執行 !
SECURITY_ATTRIBUTES sa;

	fGiveUp = 0;       		bUserStop = 0;
	xSay("Debug stop at line %d, code_in = %d", nowLineNo, code_in);
	ResetEvent(hWait);		fOnDebug = 1;
	status_update(3628, DEBUG_STOPPED, now_run_prog | (exec_line_No << 8), NULL);
	WaitForSingleObject(hWait, 0x7FFFFFFF);
	return(0);
}

void xx_caller(char *s, struct TCaller *src)
{
struct TFuncStru *pFS;
char *s1;

        su_buf[0] = 0;
		if (src == NULL) src = cur_func;
        if (src) {
            if (src->pFunc) {
                pFS = (struct TFuncStru *) src->pFunc;
                s1 = pFS->pName;
                if (pFS->prog_len == 0) s1 = (char *) src->pFunc;  // 是 TMyFunc *
                }
            else s1 = strNULL;
            if ((size_t) s1 < s) s1 = strNULL;
            su_printf("呼叫來源 <%s> 行號: %d, Token[返回位置:$%x, 最後位置:$%x]", s1, src->LineNo, src->pExec, src->pFin);
        }
        xSay("!! #3647 %s [$%x]: %s %s", s, src, su_buf, ssTemp);
}

void _stdcall yHexDump(void *p, int len)
{ // = 046C004
int vv;
if (!canSay) return;		if ((size_t) len > 1024) len = 1024; 
vv = ((size_t) p >> 16) & 0xffff;
if (vv < 0x14) { xSay("#3673 xHexDump (bad ptr < 0x140000)");  return; }  // size_t
if (vv > 0xC000) { xSay("#3673 xHexDump (bad ptr > 0xC0000000)");  return; }  // size_t
xHexDump(p, len);
}

void mark(int func_no, int status)
{
char ch, cc_en, st[280];
int		i, n;
	
if (! canMark) return;
cc_en = canSay;			canSay = 1;		// clear(st, 32);
n = mark_level - 1;		ch = '<';		// 是脫離
if (status & 0x80) {	ch = '>';		n ++; }		// 是進入
for (i=0;i < n;i ++) st[i] = ch;
sPrt(st + n, " %s %d", FuncName[func_no], status & 127);
cSayN(3, st);			mark_level = n;		if (status & 0x80) mark_level ++;
if ((func_no == F_parse_arguments) && (status == 4)) wait_for_debug(DEBUG_STOPPED);	// 發現問題 ! 呼叫暫停 !
canSay = cc_en;
}

// --------------------------
void _stdcall set_temp_str(int ndx, char *str)
{
	temp_str[ndx] = str;
}

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

int bad_ptr(void *ptr) {
	if ((int) ptr < 0x100000) return(1);  // size_t
	if (((size_t) ptr >> 16) > 0xE000) return(1);
	return(0);
}

char *sSingleToStr(SuFloat *v)
{
//  sPrt(ssTemp, "%7.7f", v);
  sPrt(ssTemp, "single from $%x", cReadDW(v, 4));
  return(ssTemp);
}
char safe_ptr(int line_no, void *ptr, char *info)
{
    if ((size_t) ptr < 0x300000) {
        xSay("%d: ptr $%x is bad (%s)", line_no, ptr, info);
        return(0);
    }
    return(1);
}
