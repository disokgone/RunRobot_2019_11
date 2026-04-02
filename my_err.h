#define	DO_WARN				0xF000	// = 警告訊息
#define	errTooManyProgs		1		// 程式數目超過 32 個
#define	errTooManyFuncs		2		// 函式數目超過 2048 個
#define	errTokenError		3		// 新增字串時, 發現不合格的代碼, 應是暫存字串區已滿
#define	errUndoneFunc		4		// 尚有函式沒有被正確的被終止
#define	errOverLocalCount	5		// 不可在一行內宣告超過 64 種不同的名稱
#define	errNot_a_name		6		// 變數或常數名稱不可以用數字起頭
#define	errNoMultiDimArray	7		// 本程式暫不支援多維度陣列
#define	errStrToNumConv		8		// 數值轉換時出錯 !
#define	errNoSuchType		9		// 本系統沒有這種數值型態 --> run_core: get_type_code()
#define	errArrayBound		10		// 陣列的範圍數值不對 (上標或下標都有可能)
#define	errNoMoreArray		11		// 本系統只允許 128 個陣列
#define	warnConstNoValue	12		// 常數需要一個有效的值
#define	errUnDoneExpr		13		// 發現未完成的表示式, 請將程式寫完整, 謝謝 !
#define	errErrorExpr		14		// 不合法的表示式
#define	errLineTooLong		15		// 表示式太長
#define	errUndeftableFull	16		// 儲存未定義程序或函數的表格已滿
#define	errUnPairedBrackets	17		// 括號沒有對稱好
#define	errBadTokenLineNo	18		// 在行號的位置出現不該有的代碼 !
#define	errFlowCtrlBgn		19		// 太多組 begin..end (本系統上限: 32 組)
#define	errFlowCtrlEnd		20		// end 的位置不對, 沒有 begin 可對應 !
#define	errFlowCtrlRep		21		// 太多組 repeat..until (本系統上限: 32 組)
#define	errFlowCtrlUnt		22		// until 的位置不對, 沒有 repeat 可對應 !
#define	errFlowCtrlIf		23		// 太多組 if..then..else (本系統上限: 64 組)
#define	errFlowCtrlThen		24		// then 的位置不對, 沒有 if 可對應 !
#define	errFlowCtrlElse		25		// else 的位置不對, 沒有 then 可對應 !
#define	errTooManyBlocks	26		// 太多程序控制小段落 (每個程序或函式不可超過 512 段落)
#define	errWhenRPN			27		// 運算式轉換過程出錯誤 (內部錯誤, 請洽本程式原始開發單位)
#define	errOPR_full			28		// 運算式內含太多的變數 (超過 95 個) 或運算子 (超過 63 個)
#define	errOPD_empty		29		// 運算式錯誤 ! 變數或數值不夠供運算式使用 !
#define	errInternalNdx		30		// 程式內部索引值不良, 請洽原始設計人員 !
#define	errBadInDefine1		31		// 函式定義錯誤: 無函式名稱
#define	errBadInDefine2		32		// 函式定義錯誤: 不該出現奇怪的符號
#define	errBadInDefine3		33		// 函式定義錯誤: 此處只應該有冒號 (:)
#define	errNoMoreLocals		34		// 本系統只允許 65536 個區域變數
#define	errOverResidentFunc	35		// 不合法的內建函數
#define	errFuncNeedRTV		36		// 函數需要傳回值
#define	errBadExecAddr		37		// 不合法的執行位址
#define	errBadArguments		38		// 參數個數不正確
#define	errChangeConst		39		// 常數不能改變其值 !
#define	errNoLocVarFound	40		// 找不到指定的區域變數
#define	errLocVarStackFull	41		// 暫放 now_fun_loc_count 的堆疊已滿
#define	errTooManyWatch		42		// 太多變數在觀察了
#define	errTooManyTempBP	43		// 太多暫時中斷點
#define	errNoLoopVar	        44		// 缺乏迴圈變數
#define	errLoopSyntax           45              // 迴圈變數後面應該要有 =
#define	errNoInitVal            46              // 此迴圈缺乏初值 !
#define	errNoEndVal             47              // 此迴圈缺乏終值 !
#define	errWhileWithoutDo       48              // while 後面需要有 do !
#define	ERR_call_undef_func     0x101           // 執行到一個未定義的函數 (do_run_func, #921)
#define	ERR_caller_stack_full   0x102           // 儲存呼叫者資訊的堆疊已滿 ! (do_run_func, #943)

