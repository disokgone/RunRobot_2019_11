// CT_xxx = const type code, 用於 TConstStru.cType 欄位
#define	CT_NONE 	0		// 此常數資料尚未定義 !
#define	CT_BYTE		2		// = 無號 8-bit
#define	CT_S8		3		// = 有號 8-bit
#define	CT_WORD		4		// = 無號 16-bit
#define	CT_S16		5		// = 有號 16-bit
#define	CT_DWORD	6		// = 無號 32-bit
#define	CT_S32		7		// = 有號 32-bit
#define	CT_QWORD	8		// = 無號 64-bit
#define	CT_S64		9		// = 有號 64-bit
#define	CT_SINGLE	10		// 32-BIT 實數
#define	CT_DOUBLE	11		// 64-BIT 實數
#define	CT_CHAR	12			// 單一字元
#define	CT_DBLCHAR	13		// 單一雙字元 (16-bit 中文/日文/韓文)
#define	CT_UNICODE	14		// 單一四字元 (32-bit 可放 UniCode)
#define	CT_STRING	15		// 字串
#define	CT_MASK		15		// 最高 00..15 種常數類型
#define	CT_ARRAY	0x80	// 代表此為常數陣列

// 內部判定類別
#define	CONST_CHAR		1
#define	CONST_BYTE		2
#define	CONST_WORD		3
#define	CONST_DWORD		4
#define	CONST_QWORD		5
#define	CONST_SINGLE	6
#define	CONST_DOUBLE	7
#define	CONST_STRING	8
#define	CONST_DONE		0x80

#define	ITS_SYMBOL	1
#define	ITS_NUM		2
#define	ITS_CHAR	3

#define	FUNC_TOKEN		0x90
#define	VAR_TOKEN		0x91
#define	SYMB_TOKEN		0x92
#define	TYPE_TOKEN		0x93
#define	TEMP_STR_TOKEN	0x94		// 暫時字串 token
#define	CONST_TOKEN		0x95
#define	RESV_TOKEN		0x96
#define	BLOCK_TOKEN		0x97		// 被切出的部份程序代號
#define	RES_FUNC_TOKEN	0x98		// 內建函數
#define	LineNo_TOKEN	0x9E
#define	LineEnd_TOKEN	0x9F

#define	CALLER_SIG		8 << 24		// 代表這個令牌是使用程式陣列指標指到一個獨立的程式小段落
#define	MAKE_Zero_Priority	0xFF00FFFF
#define	TOKEN_LEFT_Bracket	(0x0F0300 | SYMB_TOKEN)
#define	TOKEN_EQUAL			(0x021400 | SYMB_TOKEN)
#define	TOKEN_RIGHT_Bracket	(0x0F0400 | SYMB_TOKEN)
#define	TOKEN_COLON			(0x031000 | SYMB_TOKEN)
#define	TOKEN_SEMICOLON		(0x036300 | SYMB_TOKEN)

#define	FIRST_TOKEN				0x81	// = 第一個字串令牌 (供行內臨時字串定義)
#define	errUnPairedBrackets		17		// 括號沒有對稱好
#define	MAX_CALLER_COUNT		4096	// 程式陣列指標指到一個獨立的程式小段落, 最多 4096 個小段落
#define	MAX_POOL_A				0x20000	// 函數 or 程序的輸出暫存空間容量 (第一回處理)
#define	MAX_UNDONE_EVALS_SPACE	0x30000	// 提供空間給未被處理的運算式
#define	MAX_UNDONE_EVALS		0x800	// max $800 = 2048 個未被處理的運算式

#define	DO_NONE			0
#define	DO_VAR_FUNC		1
#define	DO_SYMBOL		2
#define	DO_STRING		3
#define	DO_SKIP			4			// 這次的文字要忽略, 因為是註解
#define	DO_REMARK_BEGIN	5			// 遇到 '{' 或是 '/*', 但是沒有遇到結尾的記號

// ansToken 內值如下所列, $00-$2F: 數學運算子, $30-$5F: 內部系統指令
#define	IS_SYS_TYPE		0x30		// $30-$3F: 系統內建型別 (= char~ double, ptr.. 等等)
#define	IS_USR_CMD		0x40		// $40-$5F: 外部的使用者函數 (可放 MAX_USR_CMD 個 tokens)
#define	IS_STR			0xF0		// $F0 ~ $FE = 字串
#define	IS_NONE			0xFF

#define	MAX_SYS_STR		14			// 最多 15 個字串
#define	MAX_SYS_CMD		18			// 系統內建保留字/指令 18 個
#define	MAX_SYS_TYPE	14			// 系統內建型別 14 個
#define	MAX_USR_CMD		32			// 數學運算內最多可有幾個使用者函數傳回值
#define	USER_FUNC_STR_SIZE	1024	// 1024 = 32 * 32
#define	MAX_FUNCTIONS		2048	// 最多 2048 個不同的函式 (prg_stru.pas #82)
#define	MAX_TEMP_TOKEN_SIZE	4096	// 最多 1024 TOKENS (1 TOKEN = 4 BYTES)
#define MAX_FUNC_TABLECOUNT	8192
#define	MAX_FUNC_TABLESIZE	MAX_FUNC_TABLECOUNT * sizeof(struct TFuncProc)	// 1 TFuncProc = 16 bytes
#define	Max_Codes_Size		32768

enum TOKEN_ID_LIST { TOKEN_ID_NONE, TOKEN_ID_BEGIN, TOKEN_ID_END, TOKEN_ID_REPEAT, TOKEN_ID_UNTIL,
        TOKEN_ID_IF, TOKEN_ID_THEN, TOKEN_ID_ELSE, TOKEN_ID_DO, TOKEN_ID_WHILE, TOKEN_ID_FOR, TOKEN_ID_DOWNTO, TOKEN_ID_TO,
        TOKEN_ID_STEP, TOKEN_ID_CASE, TOKEN_ID_OF, TOKEN_ID_CONTINUE, TOKEN_ID_BREAK, TOKEN_ID_EXIT };
        // 如果 TOKEN_ID_EXIT 前的字碼要改變位置, 請連帶修改本程式的 sys_cmd[] string 陣列的順序 !!

