unit CommDef;

interface

type
  Generic_container = record
    initVal: Array [0..255] of Char;
    name: Array [0..127] of Char;
    ret_type2: Array [0..31] of Char;
    dVal: Double;
    prog_addr: Pointer;
    pToken: Pointer;
    pArgs: Pointer;     // 指到一串參數 TArgs[] 的開頭
    begin_ln, end_ln, token_len, token_ndx, var_index: Integer;
    initNdx, iVal, line_no, prog_id: Integer;
    arg_ndx, flag, jobflag, ret_type, vType: Byte;
  end;

  PArgs = ^TArgs;
  TArgs = record
    name: PChar;
    flag, line_no, vType, loc_match: Integer;
  end;

  PMyVar = ^TMyVar;
  TMyVar = record       // 全域變數
    name: Array [0..127] of Char;
    dVal: Double;
    iVal, initNdx, line_no, prog_id, var_index: Integer;
    flag, vType: Byte;  // 原本 var_index 是 Word, 但造成 c 的位置被擺到後面 +04, 故改成 int !
  end;
  PMyLocal = ^TMyLocal;
  TMyLocal = record     // 區域變數
    name: Array [0..127] of Char;
    src_func: Array [0..127] of Char;
    dVal: Double;
    iVal, initNdx, line_no, prog_id, var_index: Integer;
    // 原本 var_index 是 Word, 但造成 c 的位置被擺到後面 +04, 故改成 int !
    arg_ndx, flag, vType: Byte;         // arg_ndx: 0=not arg, 參數編號從 1 算起.
  end;
  PMyConst = ^TMyConst;
  TMyConst = record     // 常數
    name: Array [0..127] of Char;
    initVal: Array [0..255] of Char;
    dVal: Double;
    iVal, initNdx, line_no, prog_id, var_index: Integer;
    flag, vType: Byte;  // 原本 var_index 是 Word, 但造成 c 的位置被擺到後面 +04, 故改成 int !
  end;
  PMyFixVal = ^TMyFixVal;
  TMyFixVal = record     // 固定值
    initVal: Array [0..255] of Char;
    pName: PChar;
    iVal, prog_id: Integer;
    cType, flag: Byte;
  end;
  PMyFunc = ^TMyFunc;
  TMyFunc = record     // 函數
    name: Array [0..127] of Char;
    funcAddr, tokenAddr: Pointer;
    pArgs: PArgs;
    bgn_ln, fin_ln, token_len: Integer;
    line_no, prog_id, fn_index: Integer;
    arg_cnt, fncType, retType: Byte;
  end;
  PMyProc = ^TMyProc;
  TMyProc = record     // 程序
    name: Array [0..127] of Char;
    funcAddr, tokenAddr: Pointer;
    pArgs: PArgs;
    bgn_ln, fin_ln, token_len: Integer;
    line_no, prog_id, fn_index: Integer;
    arg_cnt, fncType, retType: Byte;
  end;
  PMyUndone = ^TMyUndone;
  TMyUndone = record     // Undone Functions or Procedures
    name: Array [0..127] of Char;
    line_no, prog_id, solved: Integer;
  end;
  PMyExcBlk = ^TMyExcBlk;
  TMyExcBlk = record     // Excised Token Blocks (Undone Evals, 切出的 Undone Evals 程式區塊)
    src_func: Array [0..127] of Char;   // <in proc/func name>
    tokenAddr: Pointer;
    excised_ndx, token_len, line_no, prog_id: Integer;
  end;
  PMySmallBlk = ^TMySmallBlk;
  TMySmallBlk = record     // Excised Token Blocks (切出的程序小區塊)
    tokenAddr: Pointer;    // 此位址是舊的程式解析位址, 請自填新位址
    block_ndx, token_len: Integer;
  end;
  PNameStru = ^TNameStru;
  TNameStru = record	// 變數或宣告架構 (佔 6 個 32-bits)
    pName: PChar;	// 變數或宣告名稱字串指標
    lineNo: Integer;	// 來自哪行
    prgID: Word;	// 來自哪個程式
    flags: Word;	// 旗號: (b7: 1=已有初值, b6: 1=是常數 0=是變數,
		// 旗號:  b6: 1=參數 or 全域變數 或 區域變數, 0=是常數
		// 旗號:  b5: 1=有多個函數之不同定義 0=是全域_大家可共用此值),
		// 旗號:  b4: 1=有多個函數之不同定義 0=是唯一名稱區域變數), 執行時減少搜尋次數, 避免不必要之 TLocalStru 搜尋
    vType: Word;	// 變數或宣告型態代碼 (TypeCode[], 例如 array = $80)
    initNdx: Word;	// 初值索引表, 若是每次進入程序/函數時, 須設定初值, 則由此取 index 查 constant 表
    name_first: Word;   // 第一個同名變數的位置
    name_next: Word;    // 指到下一個同名變數的索引, 0=未檢查,-1=此為終點
    i_Val: Integer;	// integer 或 single, 反正 32-bit 能放得下的就放這邊 (陣列則放其 array index)
    d_Val: double;	// 64-bit double 值, 或是 longInt (64 bits)
    str: PChar;
  end;

implementation

end.
