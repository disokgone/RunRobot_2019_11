unit VarsMngr;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  ExtCtrls, Grids, StdCtrls, Buttons, CommDef, C_wrap, iDebug, FuncMngr;

type
  PMyPrograms = ^TMyPrograms;
  TMyPrograms = record
    whole_filename: Array [0..255] of Char;     // 來源原始程式完整檔名
    short_filename: Array [0..63] of Char;      // .MIA 內的單純短檔名 (來源原始程式名稱)
    prog_id: Integer;           // 程式代號, 通常由 1 算起 (不從 0 起, 以免失誤)
    line_count: Integer;        // 總行數
    var_count: Integer;         // 總變數個數
  end;

  TProgDataA = record
    all_prog, now_prog: PMyPrograms;    // 從 1 算起
    all_glob, now_glob: PMyVar;         // 全域變數
    all_local, now_local: PMyLocal;     // 區域變數
    all_const, now_const: PMyConst;     // 常數
    all_fixval, now_fixval: PMyFixVal;  // 固定值
    all_func, now_func: PMyFunc;        // 函數
    all_proc, now_proc: PMyProc;        // 程序
    all_args, now_args: PArgs;          // 參數表
    all_undn, now_undn: PMyUndone;      // 未完成之函數/程序
    all_excblk, now_excblk: PMyExcBlk;  // 切出的程式區塊
    all_smblk, now_smblk: PMySmallBlk;  // 程序小區塊
    pTokBuf, pTokNow: PByte;            // 轉換進來的 Token Byte Data
    cnt_prog: Integer;    // all_prog[]   已經使用的數量 (來源原始程式)
    cnt_glob: Integer;    // all_glob[]   已經使用的數量 (全域變數)
    cnt_local: Integer;   // all_local[]  已經使用的數量 (區域變數)
    cnt_const: Integer;   // all_const[]  已經使用的數量 (常數)
    cnt_fixval: Integer;  // all_fixval[] 已經使用的數量 (固定值)
    cnt_func: Integer;    // all_func[]   已經使用的數量 (函數)
    cnt_proc: Integer;    // all_proc[]   已經使用的數量 (程序)
    cnt_undn: Integer;    // all_undn[]   已經使用的數量 (未完成之函數/程序)
    cnt_excblk: Integer;  // all_excblk[] 已經使用的數量 (切出的程式區塊)
    cnt_smblk: Integer;   // all_smblk[] 已經使用的數量 (切出的程序小區塊)
    TokRemain: Integer;   // 尚須轉換的 Token Byte Data 資料 byte 數
    err_id: Integer;      // 處理過程發生錯誤 (代碼 0 = No Error)
  end;

  TBlob_array = record
    data_len, fofs: Integer;    // 資料的長度, 檔案位置
    name: Array [0..23] of Char;        // 用於識別資料的名稱, 可自行定義使用方式, 不拘於文字
  end;

  TBlob_data  = record
    func_lens, proc_lens, undone_lens, tkblk_lens, tstr_lens: ^Integer;
    func_tks, proc_tks, undone_tks, tkblk_tks, tstr_ptr: PChar;
  end;

  TVarsMgr = class(TForm)
    Label1: TLabel;
    Panel1: TPanel;
    Panel2: TPanel;
    Panel3: TPanel;
    Panel4: TPanel;
    Panel5: TPanel;
    SpdBtn1: TSpeedButton;
    SpdBtn2: TSpeedButton;
    Splitter1: TSplitter;
    Splitter2: TSplitter;
    Splitter3: TSplitter;
    StrGrid1: TStringGrid;
    StrGrid2: TStringGrid;
    procedure Toggle_FixVal_Panel(Sender: TObject);
    procedure Toggle_TempStr_Panel(Sender: TObject);
    procedure safe_check_3(Sender: TObject);
  private
    procedure Check_SmaBlk_Info(pISB: PMySmallBlk; pOMB: PMyExcBlk); // 查詢 pISB 的資訊, 結果填入 pOMB 之內..
    procedure Show_Error(err: Integer);
  public
    function  Collect_Prog_data_A: Pointer;
    function  Find_Func_by_LineNo(lino: Integer): PChar;
    function  GetConst(ndx: Integer): Pointer;
    function  GetConstCount: Integer;
    function  GetExcBlk(ndx: Integer): Pointer;
    function  GetExcBlkCount: Integer;
    function  GetFixVal(ndx: Integer): Pointer;
    function  GetFixValCount: Integer;
    function  GetFunc(ndx: Integer): Pointer;
    function  GetFuncCount: Integer;
    function  GetGlobalVar(ndx: Integer): Pointer;
    function  GetGlobalVarCount: Integer;
    function  GetLocalVar(ndx: Integer): Pointer;
    function  GetLocalVarCount: Integer;
    function  GetProc(ndx: Integer): Pointer;
    function  GetProcCount: Integer;
    function  GetProgName(ndx: Integer): String;
    function  GetSmaBlk(ndx: Integer): Pointer;
    function  GetSmaBlkCount: Integer;
    function  GetUndn(ndx: Integer): Pointer;
    function  GetUndnCount: Integer;
    procedure init;
    procedure LoadMiaFile(fn: String);
    procedure Show_Fix_Values;
  end;

  function  Divide_Str(s: String): String;
  procedure free_blob_mem;
  function  get_blob_addr(type_id: Integer): Pointer;
  function  get_blob_len(type_id: Integer):Integer;
  function  getFuncName(lineNo_progId: Integer): String;
  procedure match_local_vars;   // 將 now_args 此參數比對符合區域變數表的項目 !
  procedure parse_argument(s: String);
  procedure Parse_content(s: String);
  procedure Parse_token(s: String);
  procedure PlaceStr(p: PChar; s: String);
  function  remove_remark(s: String): String;
  procedure Save_GCT_data;
  procedure SetFlag(flag: Integer);
  procedure set_blob_filename(s: String);       // 將來可能會有多個 blob 資料檔案
  procedure set_src_filename(fn: String);       // 這通常是 .MIA 檔的第一行, 代表此 .MIA 檔的來源原始程式名稱

const
  width_2: Array [0..4] of Integer = (32, 160, 48, 48, 128);
  caption_2: Array [0..4] of String = ('No.', '字串值', 'Type', '整數值', 'file');
  width_3: Array [0..1] of Integer = (32, 320);
  caption_3: Array [0..1] of String = ('No.', '字串值');
  MAX_MY_PROGRAMS = 16;         // 原始程式上限
  MAX_MY_GLOBAL_VARS = 1024;    // 全域變數上限
  MAX_MY_LOCAL_VARS = 3072;     // 區域變數上限
  MAX_MY_CONST = 1536;          // 常數上限
  MAX_MY_FIX_VAL = 1536;        // 固定值上限
  MAX_MY_FUNC = 512;            // 函數上限
  MAX_MY_PROC = 512;            // 程序上限
  MAX_MY_ARGS = 384;            // 參數上限
  MAX_MY_UNDN = 128;            // 未完成之函數/程序上限 (UNDONEFUNC)
  MAX_MY_EXCBLK = 1024;         // 切出的程式區塊上限 (UNDONEVAL)
  MAX_MY_SMBLK = 2048;          // 切出的程程序小區塊上限 (Token Blocks)
  MAX_StrPool = 65536;          // 參數名稱 的儲存表 空間上限

  NO_JOB = 0;           // 目前無任務
  FLAG_GLOBAL = 1;              // 宣告全域變數
  FLAG_LOCAL = 2;               // 宣告區域變數
  FLAG_CONST = 3;               // 宣告常數
  FLAG_FIXVAL = 4;              // 宣告固定值
  FLAG_FUNCTION = 5;            // 宣告函數
  FLAG_PROCEDURE = 6;           // 宣告程序
  FLAG_UNDONEFUNC = 7;          // 宣告未定義函數或程序宣告區
  FLAG_UNDONEVAL = 8;           // 宣告自動切出的無名函數或程序宣告區
  FLAG_TOKENS = 9;              // 宣告已編碼區

  ERR_BASE = 5000;
  ERR_PROGRAM_FULL = ERR_BASE;          // TMyPrograms[] 超過 16 個程式 (MAX_MY_PROGRAMS)
  UNEXISTED_SOURCE_FILE = ERR_BASE + 1; // 找不到你的短檔名來源原始程式名稱
  ERR_GLOBAL_VARS_FULL = ERR_BASE + 2;  // 全域變數空間已滿
  ERR_LOCAL_VARS_FULL = ERR_BASE +3;    // 區域變數空間已滿
  ERR_CONST_FULL = ERR_BASE + 4;        // 常數空間已滿
  ERR_FIX_VAL_FULL = ERR_BASE + 5;      // 固定值空間已滿
  ERR_FUNC_FULL = ERR_BASE + 6;         // 函數空間已滿
  ERR_PROC_FULL = ERR_BASE + 7;         // 程序空間已滿
  ERR_UNDN_FULL = ERR_BASE + 8;         // 未完成之函數/程序空間已滿
  ERR_EXCBLK_FULL = ERR_BASE + 9;       // 切出的程式區塊空間已滿
  ERR_SMBLK_FULL = ERR_BASE + 10;       // 切出的程序小區塊空間已滿

  ID_FUNC   = 1;        // Blob use
  ID_PROC   = 2;        // Blob use
  ID_UNDONE = 3;        // Blob use
  ID_SMBLK  = 4;        // Blob use

  Err_Msg: Array [0..10] of PChar = ('原始程式上限為 16, 請嘗試合併小程式', '你的短檔名原始程式名稱: ',
        '全域變數空間已滿', '區域變數空間已滿', '常數空間已滿', '固定值空間已滿',
        '函數空間已滿', '程序空間已滿', '未完成之函數/程序空間已滿', '切出的程式區塊空間已滿', '切出的程序小區塊空間已滿');
  Err_Title: Array [0..10] of PChar = ('超過 16 個原始程式', '找不到你的短檔名來源原始程式名稱',
        '全域變數空間上限為 1024', '區域變數空間上限為 3072', '常數空間上限為 1536',
        '固定值空間上限為 1536', '函數空間上限為 512', '程序空間上限為 512',
        '未完成之函數/程序上限為 128', '切出的程式區塊空間上限為 1024', '切出的程序小區塊空間上限為 2048');

var
  VarsMgr: TVarsMgr;
  GCT: Generic_container;
  lastHitFunc: PMyFunc;         // 快取查詢函數名稱使用
  all_prog, now_prog: PMyPrograms;      // 從 1 算起
  all_glob, now_glob: PMyVar;           // 全域變數
  all_local, now_local: PMyLocal;       // 區域變數
  all_const, now_const: PMyConst;       // 常數
  all_fixval, now_fixval: PMyFixVal;    // 固定值
  all_func, now_func: PMyFunc;          // 函數
  all_proc, now_proc: PMyProc;          // 程序
  all_args, now_args: PArgs;            // 參數表
  all_undn, now_undn: PMyUndone;        // 未完成之函數/程序
  all_excblk, now_excblk: PMyExcBlk;    // 切出的程式區塊
  all_smblk, now_smblk: PMySmallBlk;    // 切出的程序小區塊
  bData: ^TBlob_data;   // 從 Blob 資料檔讀出來的 data ! (每個指標的頭 4 bytes 都是 int 長度)
  pTokBuf, pTokNow: PByte;              // 轉換進來的 Token Byte Data
  sStrPool, sPoolTail: PChar;           // 參數名稱 的儲存表
  msg_part: PChar;      // 附加的錯誤訊息
  iNowFlag: Integer = NO_JOB;   // 目前將要進行的任務
  blob_id, line_no: Integer;
  cur_fn: String;       // 目前剖析的完整檔名
  src_fn: String;       // .MIA 內的單純短檔名 (來源原始程式名稱)
  s_Cut: String;        // Divide_Str 分割得到的字串
  arg_index: Integer;   // now_args 解析第幾號參數 (此值 0 起跳, 需加一再放到 PMyFunc/PMyProc 下)
  cnt_prog: Integer;    // all_prog[]   已經使用的數量 (來源原始程式)
  cnt_glob: Integer;    // all_glob[]   已經使用的數量 (全域變數)
  cnt_local: Integer;   // all_local[]  已經使用的數量 (區域變數)
  cnt_const: Integer;   // all_const[]  已經使用的數量 (常數)
  cnt_fixval: Integer;  // all_fixval[] 已經使用的數量 (固定值)
  cnt_func: Integer;    // all_func[]   已經使用的數量 (函數)
  cnt_proc: Integer;    // all_proc[]   已經使用的數量 (程序)
  cnt_undn: Integer;    // all_undn[]   已經使用的數量 (未完成之函數/程序)
  cnt_excblk: Integer;  // all_excblk[] 已經使用的數量 (切出的程式區塊)
  cnt_smblk: Integer;   // all_smblk[]  已經使用的數量 (切出的程序小區塊)
  TokRemain: Integer;   // 尚須轉換的 Token Byte Data 資料 byte 數
  err_id: Integer;      // 處理過程發生錯誤 (代碼 0 = No Error)
  last_width_4: Integer;
  c_Cut: Char;          // Divide_Str 分割得到的第一控制字 (s[1])
  bIsToken: Boolean = False;
  bInit_Busy: Boolean = False;
  bTokenRdIn: Boolean = False;

implementation

uses bkCtrl;

{$R *.DFM}

function Divide_Str(s: String): String;
var     // 根據 s[1] 來分割字串
   i: Integer;
   c_Rt: Char;
begin
     c_Cut := s[1];     s_Cut := '';
     c_Rt := #0;
     if c_Cut = '[' then c_Rt := ']';
     if c_Cut = '<' then c_Rt := '>';
     i := 0;
     if c_Rt <> #0 then i := Pos(c_Rt, s);
     if i > 0 then begin
        s_Cut := Copy(s, 2, i - 2);
        s := Trim(Copy(s, i + 1, Length(s) - i));
     end;
     Result := s;
end;

function Find_Program(sfn: String): Integer;
var     // 傳回程式代號
   prog: PMyPrograms;
   prog_id: Integer;
begin   // sfn: short file name of source program to find
     Result := cnt_prog;        // 通常剛好是目前這個程式
     if sfn = src_fn then Exit;
     prog := all_prog;
     for prog_id := 1 to cnt_prog do begin
        if StrPas(prog^.short_filename) = sfn then begin
           Result := prog_id;
           Exit;
        end;
        Inc(prog);
     end;
     err_id := UNEXISTED_SOURCE_FILE;
     PlaceStr(msg_part, sfn);
     Result := 0;       // Error !
end;

procedure free_blob_mem;
begin
     if bData = Nil then Exit;
     if (bData^.func_lens <> Nil) then FreeMem(bData^.func_lens);
     if (bData^.proc_lens <> Nil) then FreeMem(bData^.proc_lens);
     if (bData^.undone_lens <> Nil) then FreeMem(bData^.undone_lens);
     if (bData^.tkblk_lens <> Nil) then FreeMem(bData^.tkblk_lens);
     if (bData^.tstr_lens <> Nil) then FreeMem(bData^.tstr_lens);
     if (bData^.func_tks <> Nil) then FreeMem(bData^.func_tks);
     if (bData^.proc_tks <> Nil) then FreeMem(bData^.proc_tks);
     if (bData^.undone_tks <> Nil) then FreeMem(bData^.undone_tks);
     if (bData^.tkblk_tks <> Nil) then FreeMem(bData^.tkblk_tks);
     if (bData^.tstr_ptr <> Nil) then FreeMem(bData^.tstr_ptr);
     FreeMem(bData);            bData := Nil;
end;

function  get_blob_addr(type_id: Integer): Pointer;
var
   pBuf: Pointer;
   pi: ^Integer;
   pc: PChar;
   i, n, ofs: Integer;
begin // !!@@ var_index @@!! (mixed used index, error !!!)
     if (bData = Nil) then begin  Result := GCT.pToken;  Exit;  end;
     ofs := 4;
     if (type_id = ID_FUNC) then begin
        pi := bData^.func_lens;         Inc(pi);
        for i := 0 to GCT.var_index -1 do begin     // 計算真正的 offset !
                Inc(ofs, pi^ + 4);      Inc(pi);    // 每組有別多出 4 bytes
        end;
        pBuf := AllocMem(pi^);
        pc := bData^.func_tks;          Move(pc[ofs], pBuf^, pi^);
        Result := pBuf;                 Exit;
    end;
    if (type_id = ID_PROC) then begin
        pi := bData^.proc_lens;         Inc(pi);
        n := GCT.var_index - 1;         // mixed used index, error !!! (sub 1 for start func)
        for i := 0 to n -1 do begin     // 計算真正的 offset !
                Inc(ofs, pi^ + 4);      Inc(pi);    // 每組有別多出 4 bytes
        end;
        pBuf := AllocMem(pi^);
        pc := bData^.proc_tks;          Move(pc[ofs], pBuf^, pi^);
        Result := pBuf;                 Exit;
    end;
    if (type_id = ID_UNDONE) then begin
        pi := bData^.undone_lens;       Inc(pi);
        for i := 0 to GCT.initNdx - 1 do begin // 計算真正的 offset !
                Inc(ofs, pi^);      Inc(pi);   // 每組長度剛好
        end;
        pBuf := AllocMem(pi^);
        pc := bData^.undone_tks;        Move(pc[ofs], pBuf^, pi^);
        Result := pBuf;                 Exit;
    end;
    if (type_id = ID_SMBLK) then begin
        pi := bData^.tkblk_lens;        Inc(pi);
        for i := 0 to GCT.token_ndx - 1 do begin // 計算真正的 offset !
                Inc(ofs, pi^);      Inc(pi);   // 每組長度剛好
        end;
        pBuf := AllocMem(pi^);
        pc := bData^.tkblk_tks;         Move(pc[ofs], pBuf^, pi^);
        Result := pBuf;                 Exit;
    end;
    Result := Nil;      Exit;
end;

function  get_blob_len(type_id: Integer):Integer;
var
   pi: ^Integer;
begin
     if (bData = Nil) then begin  Result := GCT.token_len;  Exit;  end;
     if (type_id = ID_FUNC) then begin
        pi := bData^.func_lens;         Inc(pi);
        Inc(pi, GCT.var_index);
        Result := pi^;          Exit;
     end;
     if (type_id = ID_PROC) then begin
        pi := bData^.proc_lens;         Inc(pi);
        Inc(pi, GCT.var_index);
        Result := pi^;          Exit;
     end;
     if (type_id = ID_UNDONE) then begin
        pi := bData^.undone_lens;       Inc(pi);
        Inc(pi, GCT.initNdx);
        Result := pi^;          Exit;
     end;
     if (type_id = ID_SMBLK) then begin
        pi := bData^.tkblk_lens;       Inc(pi);
        Inc(pi, GCT.token_ndx);
        Result := pi^;          Exit;
     end;
     Result := 0;
end;

function  getFuncName(lineNo_progId: Integer): String;
var
   // pMF: PMyFunc;
   line_no, prog_id: Integer;
begin
     prog_id := lineNo_progId and 255;
     line_no := (lineNo_progId shr 8)and $FFFFFF;
     if lastHitFunc <> Nil then begin
        if lastHitFunc^.prog_id = prog_id then begin
            if (lastHitFunc^.bgn_ln <= line_no) and (line_no <= lastHitFunc^.fin_ln) then begin
                Result := StrPas(lastHitFunc^.name);    Exit;   // 找到了 !
            end;
        end;
     end;
     FindCurrentFunc(line_no);  // 新寫的簡化版 (比較快)
     if nowInFunc <> Nil then begin
        Result := StrPas(nowInFunc^.name);
        lastHitFunc := nowInFunc;       Exit;
     end;
     {
     pMF := all_func;
     for i := 0 to cnt_func - 1 do begin
         if pMF^.prog_id = prog_id then begin
            if (pMF^.bgn_ln <= line_no) and (line_no <= pMF^.fin_ln) then begin
                Result := StrPas(pMF^.name);
                lastHitFunc := pMF;             Exit;   // 找到了 !
            end;
         end;
     end;
     pMF := PMyFunc(all_proc);
     for i := 0 to cnt_proc - 1 do begin
         if pMF^.prog_id = prog_id then begin
            if (pMF^.bgn_ln <= line_no) and (line_no <= pMF^.fin_ln) then begin
                Result := StrPas(pMF^.name);
                lastHitFunc := pMF;             Exit;   // 找到了 !
            end;
         end;
     end;
     }
     lastHitFunc := Nil;        Result := Format('找不到此函數 程式號:%d, 行號:%d', [prog_id, line_no]);
end;

procedure match_local_vars;
var
   pL: PMyLocal;
   i: Integer;
begin   // 將 now_args 此參數比對符合區域變數表的項目 !
     pL := all_local;           now_args^.loc_match := 0;       // 0 = not found !
     for i := 0 to cnt_local - 1 do begin
        if (pL^.line_no = now_args^.line_no) and (pL^.vType = now_args^.vType) then begin
            if StrComp(pL^.name, now_args^.name) = 0 then begin
                now_args^.loc_match := i + 1;   // 找到了 !
                Exit;
            end;
        end;
        Inc(pL);
     end;
end;

procedure Msg1(s: String);
begin
     BkgCtrl.Memo1.Lines.Add(s);
end;

procedure parse_argument(s: String);
var
   i: Integer;
   s1: String;
begin
     i := Pos(':', s);
     if i > 0 then begin
        s1 := Trim(Copy(s, 1, i - 1));
        now_args^.name := sPoolTail;
        StrPCopy(sPoolTail, s1);                // 記下變數名稱
        sPoolTail := sPoolTail + Length(s1) + 1;
        s := Copy(s, i + 1, Length(s));         // s = 殘餘字串
     end;
     i := Pos('#', s);
     if i > 0 then begin
        s := Copy(s, i + 1, Length(s));         // s = 殘餘字串
        i := Pos(',', s);
        if i > 0 then begin
            s1 := Trim(Copy(s, 1, i - 1));
            now_args^.line_no := StrToInt(s1);  // 記下行號 (最高 byte 目前未放 prog_id)
            s := Copy(s, i + 1, Length(s));     // s = 殘餘字串
        end;
     end;
     i := Pos('$', s);
     if i > 0 then begin
        s := Copy(s, i, Length(s));             // s = 殘餘字串
        i := Pos(',', s);
        if i > 0 then begin
            s1 := Trim(Copy(s, 1, i - 1));
            now_args^.flag := StrToInt(s1);     // 記下 flag
            s := Copy(s, i + 1, Length(s));     // s = 殘餘字串
        end;
     end;
     i := Pos('$', s);
     if i > 0 then begin
        s := Copy(s, i, Length(s));             // s = 殘餘字串
        now_args^.vType := StrToInt(s);         // 記下 vType
     end;
     match_local_vars;  // 將 now_args 此參數比對符合區域變數表的項目 !
     // xHexDump(now_args, SizeOf(TArgs));
     Inc(now_args);
end;

procedure Parse_content(s: String);
begin
     s_Cut := LowerCase(s_Cut);
     if s_Cut = 'source filename' then begin  set_src_filename(s);  Exit;  end;
     if s_Cut = 'source program' then begin     // 比對來源程式名稱
        if GCT.jobflag <> 0 then Save_GCT_data; // 保存前一工作內容
        GCT.jobflag := iNowFlag;        // 記下目前處理哪類工作
        GCT.prog_id := Find_Program(s);         Exit;
     end;
     if s_Cut = 'blob file name' then begin  set_blob_filename(s);  Exit;  end;
     if s_Cut = 'total lines' then begin        // 填入來源程式的行數
        now_prog^.line_count := StrToInt(s);    Exit;
     end;
     if s_Cut = 'var count' then begin          // 填入來源程式的總變數個數
        now_prog^.var_count := StrToInt(s);     Exit;
     end;
     if s_Cut = 'var index' then begin          // 告知在總變數區的絕對位置, token 使用此值 !
        GCT.var_index := StrToInt(s);     Exit;
     end;
     if s_Cut = 'global variables' then begin   // 告知進入全域變數宣告區
        SetFlag(FLAG_GLOBAL);   Exit;
     end;
     if s_Cut = 'local variables' then begin    // 告知進入區域變數宣告區
        SetFlag(FLAG_LOCAL);    Exit;
     end;
     if s_Cut = 'constants' then begin          // 告知進入常數宣告區
        SetFlag(FLAG_CONST);    Exit;
     end;
     if s_Cut = 'fixed values' then begin       // 告知進入固定值宣告區
        SetFlag(FLAG_FIXVAL);   Exit;
     end;
     if s_Cut = 'fn index' then begin           // 告知函數/程序固定值索引
        GCT.var_index := StrToInt(s);   Exit;
     end;
     if s_Cut = 'functions' then begin          // 告知進入函數宣告區
        SetFlag(FLAG_FUNCTION);         Exit;
     end;
     if s_Cut = 'procedures' then begin          // 告知進入程序宣告區
        SetFlag(FLAG_PROCEDURE);        Exit;
     end;
     if s_Cut = 'undone functions or procedures' then begin     // 告知進入未定義函數或程序宣告區
        SetFlag(FLAG_UNDONEFUNC);       Exit;
     end;
     if s_Cut = 'undone evals' then begin       // 告知進入自動切出的無名函數或程序宣告區
        SetFlag(FLAG_UNDONEVAL);        Exit;
     end;
     if s_Cut = 'token blocks' then begin       // 告知進入已編碼區
        SetFlag(FLAG_TOKENS);           Exit;
     end;
     if s_Cut = 'line number' then begin        // 取得行號
        GCT.line_no := StrToInt(s);     Exit;
     end;
     if s_Cut = 'name' then begin               // 取得名稱
        StrPLCopy(GCT.name, s, 127);    Exit;
     end;
     if s_Cut = 'global variables end' then begin       // 工作完成
        if GCT.jobflag <> 0 then Save_GCT_data; // 保存前一工作內容
        GCT.jobflag := NO_JOB;          // 目前暫不處理工作
        Exit;
     end;
     if s_Cut = 'local variables end' then begin        // 工作完成
        if GCT.jobflag <> 0 then Save_GCT_data; // 保存前一工作內容
        GCT.jobflag := NO_JOB;          // 目前暫不處理工作
        Exit;
     end;
     if (s_Cut = 'constants end') or (s_Cut = 'fixed values end') then begin // 工作完成
        if GCT.jobflag <> 0 then Save_GCT_data; // 保存前一工作內容
        GCT.jobflag := NO_JOB;          // 目前暫不處理工作
        Exit;
     end;
     if (s_Cut = 'functions end') or (s_Cut = 'procedures end') then begin // 工作完成
        if GCT.jobflag <> 0 then Save_GCT_data; // 保存前一工作內容
        GCT.jobflag := NO_JOB;          // 目前暫不處理工作
        Exit;
     end;
     if (s_Cut = 'undone functions or procedures end') or (s_Cut = 'undone evals end') then begin // 工作完成
        if GCT.jobflag <> 0 then Save_GCT_data; // 保存前一工作內容
        GCT.jobflag := NO_JOB;          // 目前暫不處理工作
        Exit;
     end;
     if (s_Cut = 'token blocks end') or (s_Cut = 'all end') then begin // 工作完成
        if GCT.jobflag <> 0 then Save_GCT_data; // 保存前一工作內容
        GCT.jobflag := NO_JOB;          // 目前暫不處理工作
        Exit;
     end;
     if s_Cut = 'flag' then begin               // 取得變/常數屬性旗號
        GCT.flag := StrToInt(s);        Exit;
     end;
     if s_Cut = 'initndx' then begin            // 取得初始值索引 (0=無初始值, 索引從 1 算起)
        GCT.initNdx := StrToInt(s);     Exit;
     end;
     if s_Cut = 'undone eval index' then begin  // 切出的 Undone Evals 索引 (此索引從 0 算起)
        GCT.initNdx := StrToInt(s);     Exit;
     end;
     if (s_Cut = 'fnctype') or (s_Cut = 'ctype') or (s_Cut = 'vtype') or
        (s_Cut = 'var type') then begin // 取得函數/程序/固定值/變/常數數值型態
        GCT.vType := StrToInt(s);       Exit;
     end;
     if s_Cut = 'dval' then begin               // 取得倍精度浮點數
        GCT.dVal := StrToFloat(s);      Exit;
     end;
     if s_Cut = 'ival' then begin               // 取得整數
        GCT.iVal := StrToInt(s);        Exit;
     end;
     if s_Cut = 'solved' then begin             // Undone Functions or Procedures 是否已解決 (正值算已解決, 負值代表未解決)
        GCT.iVal := StrToInt(s);        Exit;
     end;
     if (s_Cut = 'argument count') or (s_Cut = 'arg ndx') then begin // 取得是參數個數/第幾個參數
        GCT.arg_ndx := StrToInt(s);     Exit;   // arg_ndx: 0=not arg, 參數編號從 1 算起.
     end;
     if s_Cut = 'argument' then begin   // 取得是參數
        GCT.pArgs := now_args;
        parse_argument(s);      Exit;   // 存到 GCT.pArgs 所指的空間
     end;
     if s_Cut = 'init value' then begin         // 取得是第幾個參數
        StrPCopy(GCT.initVal, s);       Exit;   // 可放數字字串或文字字串
     end;
     if s_Cut = 'function or procedure' then begin      // 取得是第幾個參數 (也放: 區域變數來源)
        StrPCopy(GCT.initVal, s);       Exit;   // 可放數字字串或文字字串
     end;
     if s_Cut = 'in proc/func name' then begin          // 切出的 Undone Evals 來源
        StrPCopy(GCT.initVal, s);       StrPCopy(GCT.name, s);
        Exit;   // 可放數字字串或文字字串
     end;
     if s_Cut = 'pname' then begin              // 固定值內容若是數值則為 0, 否則此處是字串指標
        GCT.prog_addr := Pointer(StrToInt(s));  Exit;
     end;
     if s_Cut = 'begin line no' then begin      // 函數/程序開頭位置行號
        GCT.begin_ln := StrToInt(s);    Exit;
     end;
     if s_Cut = 'final line no' then begin      // 函數/程序結束位置行號
        GCT.end_ln := StrToInt(s);      Exit;
     end;
     if s_Cut = 'return type' then begin        // 函數/程序返回值型態
        if (s[1] = '$') or (Byte(s[1]) < 65) then begin
            GCT.ret_type := StrToInt(s);  Exit; end             // 數值格式返回值型態
        else begin
            StrPLCopy(GCT.ret_type2, s, 31);    Exit; end       // 文字格式返回值型態
     end;
     if s_Cut = 'real prog addr' then begin     // 函數/程序記憶體位址
        GCT.prog_addr := Pointer(StrToInt(s));  Exit;   // !! 注意 32/64 位元系統 !!
     end;
     if s_Cut = 'token address' then begin      // 程式代碼記憶體位址
        GCT.prog_addr := Pointer(StrToInt(s));  Exit;   // !! 注意 32/64 位元系統 !!
     end;
     if s_Cut = 'token len' then begin          // 函數/程序使用程式代碼數 (* 4 = byte counts)
        GCT.token_len := StrToInt(s);
        bTokenRdIn := True;             // 允許從註解行裡提取 token byte data
        Exit;
     end;
     if s_Cut = 'token counts' then begin       // 程式代碼數
        GCT.token_len := StrToInt(s);
        bTokenRdIn := True;             // 允許從註解行裡提取 token byte data
        Exit;
     end;
     if s_Cut = 'token block index' then begin  // 程式代碼編號
        if GCT.jobflag <> 0 then Save_GCT_data; // 保存前一工作內容
        GCT.token_ndx := StrToInt(s);
        StrPCopy(GCT.name, Format('程序小區塊編號 %d', [GCT.token_ndx]));
        GCT.jobflag := iNowFlag;        // 記下目前處理哪類工作
        Exit;
     end;
     if s_Cut = 'temporary strings' then Exit;  // 暫時字串, 免處理 (done by blob)
     if s_Cut = 'temporary strings end' then Exit; // 暫時字串, 免處理 (done by blob)
     if s_Cut = 'temp string count' then Exit;  // 暫時字串, 免處理 (done by blob)
     if s_Cut = 'total string space' then Exit; // 暫時字串, 免處理 (done by blob)
     xSay(s_Cut + ' 未處理..' + s);
end;

procedure Parse_token(s: String);
begin
     xSay('tk: ' + s);
end;

procedure PlaceStr(p: PChar; s: String);
begin
     p := AllocMem(256);
     StrPCopy(p, s);
end;

procedure ReadTokenIn(s: String);
var
   i, j: Integer;
begin   // 讀取後面的代碼值
     if pTokBuf = nil then begin
        pTokBuf := AllocMem(GCT.token_len);     pTokNow := pTokBuf;
        TokRemain := GCT.token_len;             // 剩餘需讀取的長度
     end;
     i := Pos('$', s);
     if i > 0 then begin        // 需有至少八位數 Hex 位址
        j := Pos('-', s);       // 需有 '-'
        // 直接略過 Hex 位址值
        s := Trim(Copy(s, j + 1, Length(s) - j));
        i := 1;         j := Length(s);
        repeat  // 需固定是 兩位 Hex 數
            pTokNow^ := StrToInt('$' + Copy(s, i, 2));
            Inc(i, 3);          Inc(pTokNow);           Dec(TokRemain);
            if TokRemain < 1 then begin // 已無資料須讀取
                GCT.pToken := pTokBuf;
                pTokBuf := nil;
                bTokenRdIn := False;    // 停止從註解行裡提取 token byte data
                Msg1(Format('成功轉換 %s 的代碼資料, 長度: %d bytes.', [GCT.name, GCT.token_len]));
                Exit;
            end;
        until i >= j;
     end;
end;

function remove_remark(s: String): String;
var
   i: Integer;
begin
     s := TrimLeft(s);
     i := Pos('//', s);
     if i > 0 then begin
        if Copy(s, i, 4) = '// $' then begin
            ReadTokenIn(s);     // 讀取後面的代碼值
            Result := '';       Exit;
        end;
        if i = 1 then s := ''
        else s := Copy(s, 1, i - 1);
     end;
     Result := s;
end;

procedure Save_GCT_data;
begin
     case GCT.jobflag of
        FLAG_GLOBAL: begin      // 儲存宣告全域變數
            if cnt_glob > (MAX_MY_GLOBAL_VARS - 2) then begin
               err_id := ERR_GLOBAL_VARS_FULL;  msg_part := NIL;    Exit;  end;
            now_glob.prog_id := GCT.prog_id;    // <source program>
            now_glob.line_no := GCT.line_no;    // <line number>
            StrCopy(now_glob.name, GCT.name);   // <name>
            now_glob.flag := GCT.flag;          // <flag>
            now_glob.initNdx := GCT.initNdx;    // <initNdx>
            now_glob.vType := GCT.vType;        // <vType>
            now_glob.var_index := GCT.var_index; // <var Index>
            now_glob.iVal := GCT.iVal;          // <iVal>
            now_glob.dVal := GCT.dVal;          // <dVal>
            Inc(now_glob);      Inc(cnt_glob);
          end;
        FLAG_LOCAL: begin       // 儲存宣告區域變數
            if cnt_local > (MAX_MY_GLOBAL_VARS - 2) then begin
               err_id := ERR_GLOBAL_VARS_FULL;  msg_part := NIL;    Exit;  end;
            now_local.prog_id := GCT.prog_id;   // <source program>
            StrLCopy(now_local.src_func, GCT.initVal, 128);     // <function or procedure>
            now_local.line_no := GCT.line_no;   // <line number>
            now_local.initNdx := GCT.initNdx;   // <initNdx>
            StrCopy(now_local.name, GCT.name);  // <name>
            now_local.vType := GCT.vType;       // <var type>
            now_local.arg_ndx := GCT.arg_ndx;   // <arg ndx>
            now_local.var_index := GCT.var_index; // <var Index>
            now_local.flag := GCT.flag;         // <flag>
            Inc(now_local);     Inc(cnt_local);
          end;
        FLAG_CONST: begin       // 儲存宣告常數
            if cnt_const > (MAX_MY_CONST - 2) then begin
               err_id := ERR_CONST_FULL;  msg_part := NIL;    Exit;  end;
            now_const.prog_id := GCT.prog_id;   // <source program>
            now_const.line_no := GCT.line_no;   // <line number>
            StrCopy(now_const.name, GCT.name);  // <name>
            now_const.initNdx := GCT.initNdx;   // <initNdx>
            now_const.vType := GCT.vType;       // <var type>
            now_const.flag := GCT.flag;         // <flag>
            now_const.var_index := GCT.var_index; // <var Index>
            now_const.iVal := GCT.iVal;         // <iVal>
            now_const.dVal := GCT.dVal;         // <dVal>
            StrCopy(now_const.initVal, GCT.initVal);    // <init Value>
            Inc(now_const);     Inc(cnt_const);
          end;
        FLAG_FIXVAL: begin      // 儲存宣告固定值
            if cnt_fixval > (MAX_MY_FIX_VAL - 2) then begin
               err_id := ERR_FIX_VAL_FULL;  msg_part := NIL;    Exit;  end;
            now_fixval.prog_id := GCT.prog_id;  // <source program>
            now_fixval.cType := GCT.vType;      // <cType>
            StrCopy(now_fixval.initVal, GCT.initVal);   // <init Value>
            now_fixval.pName := GCT.prog_addr;  // <pName>
            now_fixval.iVal := GCT.iVal;        // <iVal>
			now_fixval.flag := GCT.flag;        // <flag>
            Inc(now_fixval);    Inc(cnt_fixval);
          end;
        FLAG_FUNCTION: begin    // 儲存宣告函數 (忽略 fncType)
            if cnt_func > (MAX_MY_FUNC - 2) then begin
               err_id := ERR_FUNC_FULL;  msg_part := NIL;    Exit;  end;
            now_func.prog_id := GCT.prog_id;    // <source program>
            now_func.fn_index := GCT.var_index; // <Fn index>
            StrCopy(now_func.name, GCT.name);   // <name>
            now_func.fncType := GCT.vType;      // <fncType>
            now_func.arg_cnt := GCT.arg_ndx;    // <argument count>
            now_func.bgn_ln := GCT.begin_ln;    // <begin line no>
            now_func.fin_ln := GCT.end_ln;      // <final line no>
            now_func.retType := GCT.ret_type;   // <return type>
            now_func.funcAddr := GCT.prog_addr; // <real prog addr>
            now_func.pArgs := GCT.pArgs;        // <arguments list> --> TArgs[]
            if now_func.arg_cnt > 1 then Dec(now_func.pArgs, now_func.arg_cnt - 1);
            now_func.tokenAddr := GCT.pToken;   // <token addr>
            now_func.token_len := GCT.token_len;    // <Token len>
            Inc(now_func);      Inc(cnt_func);
          end;
        FLAG_PROCEDURE: begin   // 儲存宣告程序 (忽略 fncType)
            if cnt_proc > (MAX_MY_PROC - 2) then begin
               err_id := ERR_PROC_FULL;  msg_part := NIL;    Exit;  end;
            now_proc.prog_id := GCT.prog_id;    // <source program>
            now_proc.fn_index := GCT.var_index; // <Fn index>
            StrCopy(now_proc.name, GCT.name);   // <name>
            now_proc.fncType := GCT.vType;      // <fncType>
            now_proc.arg_cnt := GCT.arg_ndx;    // <argument count>
            now_proc.bgn_ln := GCT.begin_ln;    // <begin line no>
            now_proc.fin_ln := GCT.end_ln;      // <final line no>
            now_proc.retType := GCT.ret_type;   // <return type>
            now_proc.funcAddr := GCT.prog_addr; // <real prog addr>
            now_proc.pArgs := GCT.pArgs;        // <arguments list> --> TArgs[]
            if now_proc.arg_cnt > 1 then Dec(now_proc.pArgs, now_proc.arg_cnt - 1);
            now_proc.tokenAddr := GCT.pToken;   // <token addr>
            now_proc.token_len := GCT.token_len;    // <Token len>
            Inc(now_proc);      Inc(cnt_proc);
          end;
        FLAG_UNDONEFUNC: begin  // 儲存未完成之函數/程序
            if cnt_proc > (MAX_MY_UNDN - 2) then begin
               err_id := ERR_UNDN_FULL;  msg_part := NIL;    Exit;  end;
            now_undn.prog_id := GCT.prog_id;    // <source program>
            now_undn.line_no := GCT.line_no;    // <line number>
            StrCopy(now_undn.name, GCT.name);   // <name>
            now_undn.solved := GCT.iVal;        // <solved> (正值算已解決, 負值代表未解決)
            Inc(now_undn);      Inc(cnt_undn);
          end;
        FLAG_UNDONEVAL: begin   // 儲存切出的 Undone Evals 程式區塊
            if cnt_excblk > (MAX_MY_EXCBLK - 2) then begin
               err_id := ERR_EXCBLK_FULL;  msg_part := NIL;    Exit;  end;
            now_excblk.prog_id := GCT.prog_id;    // <source program>
            now_excblk.line_no := GCT.line_no;    // <line number>
            StrCopy(now_excblk.src_func, GCT.initVal);   // <in proc/func name>
            now_excblk.excised_ndx := GCT.initNdx;       // <Undo Eval index>
            now_excblk.tokenAddr := GCT.pToken;          // <token addr>
            now_excblk.token_len := GCT.token_len;       // <Token len>
            Inc(now_excblk);    Inc(cnt_excblk);
          end;
          FLAG_TOKENS: begin   // 儲存程序小區塊
            if cnt_smblk > (MAX_MY_SMBLK - 2) then begin
               err_id := ERR_SMBLK_FULL;  msg_part := NIL;    Exit;  end;
            now_smblk.block_ndx := GCT.token_ndx;       // <Token Block index>
            now_smblk.tokenAddr := GCT.pToken;          // <Token address>
            now_smblk.token_len := GCT.token_len;       // <Token counts>
            Inc(now_smblk);    Inc(cnt_smblk);
          end;
     end;
     ZeroMemory(@GCT, SizeOf(Generic_container));
     arg_index := 0;
end;

procedure SetFlag(flag: Integer);
begin   // 必須把之前未完成的宣告做結束/保存的動作
     iNowFlag := flag;
end;

procedure set_blob_filename(s: String);       // 將來可能會有多個 blob 資料檔案
var
   blob_items, pBX: ^TBlob_array; // blob 資料檔頭表
   pi: ^Integer;
   pw: ^Word;
   fn, pc: PChar;
   blob_item_cnt, i: Integer;
begin // 將來可能會有多個 blob 資料檔案
     bData := Nil;
     fn := AllocMem(Length(s) + 3);     StrPCopy(fn, s);
     blob_id := open_blob_file(fn, 1, 0);
     if (blob_id < 1) then xSay(Format('開啟 blob (%s) 時, 發生錯誤, 錯誤碼 = %d', [s, blob_id]))
     else begin // 開啟 blob 成功
        xSay('開啟 blob (' + s + ') 成功 !');
        pi := get_info_array(blob_id);  // 1st int = 項數, 2nd int = free loc. 之後才是
        blob_item_cnt := pi^;           Inc(pi, 2);
        blob_items := Pointer(pi);  // ^TBlob_array
        bData := AllocMem(SizeOf(TBlob_data)); // (struct blob_data *)
        // 開始讀入 blob data !! (頭 4 bytes = 此段資料的長度)
        pBX := blob_items;
        for i := 0 to blob_item_cnt - 1 do begin
            pi := get_blob_data(blob_id, i);
            if (pBX^.name[0] = 'F') then begin
               if (StrComp(pBX^.name, 'FuncLenArray') = 0) then bData^.func_lens := pi;
               if (StrComp(pBX^.name, 'FuncTokens') = 0) then bData^.func_tks := PChar(pi);
               Inc(pBX);        Continue;
            end;
            if (pBX^.name[0] = 'P') then begin
               if (StrComp(pBX^.name, 'ProcLenArray') = 0) then bData^.proc_lens := pi;
               if (StrComp(pBX^.name, 'ProcTokens') = 0) then bData^.proc_tks := PChar(pi);
               Inc(pBX);        Continue;
            end;
            if (pBX^.name[0] = 'T') then begin
               if (StrComp(pBX^.name, 'TokenBlkLenArray') = 0) then bData^.tkblk_lens := pi;
               if (StrComp(pBX^.name, 'TokenBlkTokens') = 0) then bData^.tkblk_tks := PChar(pi);
               if (StrComp(pBX^.name, 'TempStrLenArray') = 0) then bData^.tstr_lens := pi;
               if (StrComp(pBX^.name, 'TempStrList') = 0) then bData^.tstr_ptr := PChar(pi);
               Inc(pBX);        Continue;
            end;
            if (pBX^.name[0] = 'U') then begin
               if (StrComp(pBX^.name, 'UndoneEvalLenArray') = 0) then bData^.undone_lens := pi;
               if (StrComp(pBX^.name, 'UndoneEvalTokens') = 0) then bData^.undone_tks := PChar(pi);
               Inc(pBX);        Continue;
            end;
            xSay(Format('!!@@ Bug: Blob Item %s 未被處理 !!', [pBX^.name]));
            Inc(pBX);
        end;
        pi := Pointer(blob_items);      Dec(pi, 2);     FreeMem(pi);    // 釋放 !
        close_blob(blob_id);    blob_id := 0;   // 釋放 !
        // 把暫時字串擺到 char *temp_str[128]; (in runner.c)
        blob_item_cnt := bData^.tstr_lens^;     // 1st int = data_len (TempStrLenArray 是 WORD[] 每 2 bytes 一組)
        blob_item_cnt := blob_item_cnt shr 1;   // = 真正的項數 (是 WORD[])
        VarsMgr.StrGrid2.RowCount := blob_item_cnt + 1;
        pw := Pointer(bData^.tstr_lens);  Inc(pw, 2);   // pw = 真正的 WORD[] !
        pc := bData^.tstr_ptr;  Inc(pc, 4);     // 跳過頭 4 bytes (= 此小段的總長度)
        for i := 0 to blob_item_cnt - 1 do begin
            // xSay(Format('%d - %s', [i, pc]));
            VarsMgr.StrGrid2.Cells[0, i + 1] := IntToStr(i);
            VarsMgr.StrGrid2.Cells[1, i + 1] := StrPas(pc);
            set_temp_str(i, pc);          Inc(pc, pw^ + 1);       Inc(pw);
        end;                   
     end;
end;

procedure set_src_filename(fn: String);
begin   // 這通常是 .MIA 檔的第一行, 代表此 .MIA 檔的來源原始程式名稱
     if cnt_prog > (MAX_MY_PROGRAMS - 1) then begin
        err_id := ERR_PROGRAM_FULL;     msg_part := NIL;
        Exit;
     end;
     src_fn := fn;      // .MIA 內的單純短檔名 (來源原始程式名稱)
     StrPCopy(now_prog^.whole_filename, cur_fn);
     StrPLCopy(now_prog^.short_filename, src_fn, 63);
     Inc(cnt_prog);
     now_prog^.prog_id := cnt_prog;     // Start count from 1..
     now_prog^.line_count := 0;
end;

function  TVarsMgr.Collect_Prog_data_A: Pointer;
var
   Dt: ^TProgDataA;
begin
     Dt := AllocMem(SizeOf(TProgDataA));
     Dt^.all_prog := all_prog;          Dt^.now_prog := now_prog;
     Dt^.all_glob := all_glob;          Dt^.now_glob := now_glob;
     Dt^.all_local := all_local;        Dt^.now_local := now_local;
     Dt^.all_const := all_const;        Dt^.now_const := now_const;
     Dt^.all_fixval := all_fixval;      Dt^.now_fixval := now_fixval;
     Dt^.all_func := all_func;          Dt^.now_func := now_func;
     Dt^.all_proc := all_proc;          Dt^.now_proc := now_proc;
     Dt^.all_args := all_args;          Dt^.now_args := now_args;
     Dt^.all_undn := all_undn;          Dt^.now_undn := now_undn;
     Dt^.all_excblk := all_excblk;      Dt^.now_excblk := now_excblk;
     Dt^.all_smblk := all_smblk;        Dt^.now_smblk := now_smblk;
     Dt^.pTokBuf := pTokBuf;            Dt^.pTokNow := pTokNow;
     Dt^.cnt_prog := cnt_prog;          Dt^.cnt_glob := cnt_glob;
     Dt^.cnt_local := cnt_local;        Dt^.cnt_const := cnt_const;
     Dt^.cnt_fixval := cnt_fixval;      Dt^.cnt_func := cnt_func;
     Dt^.cnt_proc := cnt_proc;          Dt^.cnt_undn := cnt_undn;
     Dt^.cnt_excblk := cnt_excblk;      Dt^.cnt_smblk := cnt_smblk;
     Dt^.TokRemain := TokRemain;        Dt^.err_id := err_id;       
     Result := Dt;
end;

function  TVarsMgr.Find_Func_by_LineNo(lino: Integer): PChar;
var     // 從行號來查詢是在哪個函數裡
   pF: PMyFunc;
   pP: PMyProc;
   i: Integer;
begin
     pF := all_func;
     for i := 0 to cnt_func - 1 do begin
        if (pF^.bgn_ln <= lino) and (lino <= pF^.fin_ln) then begin
            Result := pF^.name;         Exit;
        end;
        Inc(pF);
     end;
     pP := all_proc;
     for i := 0 to cnt_proc - 1 do begin
        if (pP^.bgn_ln <= lino) and (lino <= pP^.fin_ln) then begin
            Result := pP^.name;         Exit;
        end;
        Inc(pP);
     end;
     Result := '找不到';
end;

function  TVarsMgr.GetConst(ndx: Integer): Pointer;
var
   pN: PMyConst;
begin
     pN := all_const;    Inc(pN, ndx);
     Result := pN;
end;

function  TVarsMgr.GetConstCount: Integer;
begin
     Result := cnt_const;
end;

function  TVarsMgr.GetExcBlk(ndx: Integer): Pointer;
var
   pE: PMyExcBlk;
begin
     pE := all_excblk;  Inc(pE, ndx);
     Result := pE;
end;

function  TVarsMgr.GetExcBlkCount: Integer;
begin
     Result := cnt_excblk;
end;

function  TVarsMgr.GetFixVal(ndx: Integer): Pointer;
var
   pF: PMyFixVal;
begin
     pF := all_fixval;    Inc(pF, ndx);
     Result := pF;
end;

function  TVarsMgr.GetFixValCount: Integer;
begin
     Result := cnt_fixval;
end;

function  TVarsMgr.GetFunc(ndx: Integer): Pointer;
var
   pN: PMyFunc;
begin
     pN := all_func;    Inc(pN, ndx);
     Result := pN;
end;

function  TVarsMgr.GetFuncCount: Integer;
begin
     Result := cnt_func;
end;

function  TVarsMgr.GetGlobalVar(ndx: Integer): Pointer;
var
   pV: PMyVar;
begin
     pV := all_glob;    Inc(pV, ndx);
     Result := pV;
end;

function  TVarsMgr.GetGlobalVarCount: Integer;
begin
     Result := cnt_glob;
end;

function  TVarsMgr.GetLocalVar(ndx: Integer): Pointer;
var
   pL: PMyLocal;
begin
     pL := all_local;    Inc(pL, ndx);
     Result := pL;
end;

function  TVarsMgr.GetLocalVarCount: Integer;
begin
     Result := cnt_local;
end;

function  TVarsMgr.GetProc(ndx: Integer): Pointer;
var
   pP: PMyProc;
begin
     pP := all_proc;    Inc(pP, ndx);
     Result := pP;
end;

function  TVarsMgr.GetProcCount: Integer;
begin
     Result := cnt_proc;
end;

function  TVarsMgr.GetProgName(ndx: Integer): String;
var
   pMP: PMyPrograms;
begin
     if ndx < 1 then ndx := 1;          // bug: 程序小區塊 無程式代號資訊
     pMP := all_prog;   Inc(pMP, ndx - 1);
     Result := StrPas(pMP.short_filename);
end;

function  TVarsMgr.GetSmaBlk(ndx: Integer): Pointer;
var
   pE: PMySmallBlk;
begin
     pE := all_smblk;   Inc(pE, ndx);
     Result := pE;
end;

function  TVarsMgr.GetSmaBlkCount: Integer;
begin
     Result := cnt_smblk;       // all_smblk[] 已經使用的數量 (切出的程序小區塊)
end;

function  TVarsMgr.GetUndn(ndx: Integer): Pointer;
var
   pU: PMyUndone;
begin
     pU := all_undn;    Inc(pU, ndx);
     Result := pU;
end;

function  TVarsMgr.GetUndnCount: Integer;
begin
     Result := cnt_undn;
end;

procedure TVarsMgr.init;
var
   i: Integer;
begin
     if bInit_Busy then Exit;
     bInit_Busy := True;
     all_prog := AllocMem(SizeOf(TMyPrograms) * MAX_MY_PROGRAMS);
     now_prog := all_prog;      cnt_prog := 0;
     all_glob := AllocMem(SizeOf(TMyVar) * MAX_MY_GLOBAL_VARS);
     now_glob := all_glob;      cnt_prog := 0;
     all_local := AllocMem(SizeOf(TMyLocal) * MAX_MY_LOCAL_VARS);
     now_local := all_local;    cnt_local := 0;
     all_const := AllocMem(SizeOf(TMyConst) * MAX_MY_CONST);
     now_const := all_const;    cnt_const := 0;
     all_fixval := AllocMem(SizeOf(TMyFixVal) * MAX_MY_FIX_VAL);
     now_fixval := all_fixval;  cnt_fixval := 0;
     all_func := AllocMem(SizeOf(TMyFunc) * MAX_MY_FUNC);
     now_func := all_func;      cnt_func := 0;
     all_proc := AllocMem(SizeOf(TMyProc) * MAX_MY_PROC);
     now_proc := all_proc;      cnt_proc := 0;
     all_args := AllocMem(SizeOf(TArgs) * MAX_MY_ARGS);
     now_args := all_args;
     all_undn := AllocMem(SizeOf(TMyUndone) * MAX_MY_UNDN);
     now_undn := all_undn;      cnt_undn := 0;
     all_excblk := AllocMem(SizeOf(TMyExcBlk) * MAX_MY_EXCBLK);
     now_excblk := all_excblk;          cnt_excblk := 0;
     all_smblk := AllocMem(SizeOf(TMySmallBlk) * MAX_MY_SMBLK);
     now_smblk := all_smblk;            cnt_smblk := 0;
     sStrPool := AllocMem(MAX_StrPool); // 參數名稱 的儲存表
     sPoolTail := sStrPool;
     for i := 0 to 4 do begin
         StrGrid1.ColWidths[i] := width_2[i];
         StrGrid1.Cells[i, 0] := caption_2[i];
     end;
     for i := 0 to 1 do begin
         StrGrid2.ColWidths[i] := width_3[i];
         StrGrid2.Cells[i, 0] := caption_3[i];
     end;
     ZeroMemory(@GCT, SizeOf(GCT));
     bInit_Busy := False;       Show;
     FuncMgr.init;
end;

procedure TVarsMgr.LoadMiaFile(fn: String);
var
   F_in: TextFile;
   s: String;
begin
     cur_fn := fn;
     AssignFile(F_in, fn);         Reset(F_in);         line_no := 0;
     while not Eof(F_in) do begin
           readln(F_in, s);     Inc(line_no);           err_id := 0;
           if not bIsToken then s := remove_remark(s);
           s := Trim(s);
           if Length(s) < 1 then Continue;      // no string !!
           s := Divide_Str(s);       // 根據 s[1] 來分割字串
           // xSay(s_Cut + '__' + s + '__');
           if s_Cut <> '' then Parse_content(s)
           else Parse_token(s);     // err_id := 5000;
           if err_id <> 0 then begin
              Show_Error(err_id);
              Break;   // Got Error !!
           end;
     end;
     CloseFile(F_in);
end;

procedure TVarsMgr.Show_Error(err: Integer);
var
   p2: PChar;
begin
     err := err mod 1000;
     if msg_part = NIL then MessageBox(Application.Handle, Err_Msg[err], Err_Title[err], MB_OK)
     else begin
        p2 := AllocMem(512);
        StrCopy(p2, Err_Msg[err]);
        StrCat(p2, msg_part);
        MessageBox(Application.Handle, p2, Err_Title[err], MB_OK);
        FreeMem(p2);
     end;
end;

procedure TVarsMgr.Check_SmaBlk_Info(pISB: PMySmallBlk; pOMB: PMyExcBlk);
var
   cnt, i: Integer;
   pI: ^Integer;
   pB: ^Byte;
begin   // 查詢 pISB 的資訊, 結果填入 pOMB 之內..
     if pISB = Nil then Exit;
     pB := Pointer(pISB^.tokenAddr);    cnt := pISB^.token_len shr 2;
     xSay(Format('小區塊 $%x, Token 數 = %d bytes', [Integer(pB), cnt]));
     for i := 0 to cnt - 1 do begin
        if pB^ = $9F then begin
            pI := Pointer(pB);
            pOMB^.line_no := pI^ shr 8;       // get 24-bit line no !!
            Break;
        end;
        Inc(pB, 4);
     end;
     // 現在有了起始行號, 可以去查詢在哪個函數了 !
     StrCopy(pOMB^.src_func, Find_Func_by_LineNo(pOMB^.line_no));
end;

procedure TVarsMgr.Show_Fix_Values;
var
   i, n: Integer;
   pF: PMyFixVal;
   pN: PMyFunc;
   pP: PMyProc;
   pU: PMyUndone;
   pE: PMyExcBlk;
   pX: PMySmallBlk;
begin
     n := GetFixValCount;
     for i := 0 to n - 1 do begin
        pF := GetFixVal(i);
        StrGrid1.Cells[0, i + 1] := IntToStr(i + 1);    // No.
        StrGrid1.Cells[1, i + 1] := StrPas(pF.initVal); // init Value
        // xHexDump(@pF.initVal[0], 16);
        StrGrid1.Cells[2, i + 1] := '$' + IntToHex(pF.cType, 2);    // Type
        StrGrid1.Cells[3, i + 1] := IntToStr(pF.iVal);          // iVal
        StrGrid1.Cells[4, i + 1] := GetProgName(pF.prog_id);    // file
     end;

     n := GetFuncCount;
     for i := 0 to n - 1 do begin
        pN := GetFunc(i);
        FuncMgr.StrGrid1.Cells[0, i + 1] := IntToStr(i + 1);    // No.
        FuncMgr.StrGrid1.Cells[1, i + 1] := StrPas(pN.name);    // Name
        FuncMgr.StrGrid1.Cells[2, i + 1] := IntToStr(pN.arg_cnt);       // argument count
        FuncMgr.StrGrid1.Cells[3, i + 1] := '$' + IntToHex(pN.retType, 2);  // return type
        FuncMgr.StrGrid1.Cells[4, i + 1] := IntToStr(pN.bgn_ln);        // begin line no
        FuncMgr.StrGrid1.Cells[5, i + 1] := GetProgName(pN.prog_id);    // file
     end;

     n := GetProcCount;
     for i := 0 to n - 1 do begin
        pP := GetProc(i);
        FuncMgr.StrGrid2.Cells[0, i + 1] := IntToStr(i + 1);    // No.
        FuncMgr.StrGrid2.Cells[1, i + 1] := StrPas(pP.name);    // Name
        FuncMgr.StrGrid2.Cells[2, i + 1] := IntToStr(pP.arg_cnt);       // argument count
        FuncMgr.StrGrid2.Cells[3, i + 1] := '$' + IntToHex(Cardinal(pP.funcAddr), 8);  // real prog addr
        FuncMgr.StrGrid2.Cells[4, i + 1] := IntToStr(pP.bgn_ln);        // begin line no
        FuncMgr.StrGrid2.Cells[5, i + 1] := GetProgName(pP.prog_id);    // file
     end;

     n := GetUndnCount;     // 切出之呼叫 Undone Evals 函數
     for i := 0 to n - 1 do begin
        pU := GetUndn(i);
        FuncMgr.StrGrid3.Cells[0, i + 1] := IntToStr(i);        // No.
        FuncMgr.StrGrid3.Cells[1, i + 1] := StrPas(pU.name);    // Name
        FuncMgr.StrGrid3.Cells[2, i + 1] := IntToStr(pU.solved);        // Solved
        FuncMgr.StrGrid3.Cells[3, i + 1] := IntToStr(pU.line_no);       // line number
        FuncMgr.StrGrid3.Cells[4, i + 1] := GetProgName(pU.prog_id);    // file
     end;
     
     pE := Nil;
     n := GetExcBlkCount;   // 切出之呼叫 Undone Evals 代碼區塊
     for i := 0 to n - 1 do begin
        pE := GetExcBlk(i);
        FuncMgr.StrGrid4.Cells[0, i + 1] := IntToStr(i);        // No.
        FuncMgr.StrGrid4.Cells[1, i + 1] := StrPas(pE.src_func);        // in proc/func name
        FuncMgr.StrGrid4.Cells[2, i + 1] := IntToStr(pE.excised_ndx);   // Undone Eval index
        FuncMgr.StrGrid4.Cells[3, i + 1] := IntToStr(pE.line_no);       // line number
        FuncMgr.StrGrid4.Cells[4, i + 1] := GetProgName(pE.prog_id);    // file
     end;

     n := GetSmaBlkCount;   // 切出之程序小區塊
     for i := 0 to n - 1 do begin
        pX := GetSmaBlk(i);
        Check_SmaBlk_Info(pX, pE);
        FuncMgr.StrGrid5.Cells[0, i + 1] := IntToStr(i);        // No.
        FuncMgr.StrGrid5.Cells[1, i + 1] := StrPas(pE.src_func);        // in proc/func name
        FuncMgr.StrGrid5.Cells[2, i + 1] := IntToStr(pX.block_ndx);     // Undone Eval index
        FuncMgr.StrGrid5.Cells[3, i + 1] := IntToStr(pE.line_no);       // start line number
        FuncMgr.StrGrid5.Cells[4, i + 1] := GetProgName(pE.prog_id);    // file (通常是同一個檔案)
     end;
end;

procedure TVarsMgr.Toggle_FixVal_Panel(Sender: TObject);
begin
     if Panel3.Width > 50 then Panel3.Width := 10
     else Panel3.Width := 392;
end;

procedure TVarsMgr.Toggle_TempStr_Panel(Sender: TObject);
begin
     if Panel4.Width > 100 then begin
        last_width_4 := Panel4.Width;
        Panel4.Width := 60;
     end
     else Panel4.Width := 392;
end;

procedure TVarsMgr.safe_check_3(Sender: TObject);
begin
     if Splitter3.Left < 64 then Splitter3.Left := 64;
     if Panel4.Width < 60 then Panel4.Width := 60;
end;

end.






