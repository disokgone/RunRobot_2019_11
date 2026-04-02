
#define F_Enter				128	// 進入某個函數時, 一律用此值
#define F_Bye				0	// 0 .. 127 = 各種不同的 bye, 0=正常結束, 1..127=距離開頭遠近不同位置的結束 (疑似不正常結束)
// ------------------- 底下是函數字串代號 --------------
#define	F_run_main_start		1
#define	F_parse_arguments		2
#define	F_do_run_func			3
#define	F_do_run_reserve_word	4
#define	F_do_run_resi_func		5
#define	F_do_only_one_statement	6
#define	F_do_statements			7
#define	F_MaxNum				8

char *FuncName[F_MaxNum + 1] = { "空", "run_main_start", "parse_arguments", "do_run_func",
	"do_run_reserve_word", "do_run_resi_func", "do_only_one_statement", "do_statements", "結束" }; 

// mark(F_, F_Enter);
// mark(F_, F_Bye);


