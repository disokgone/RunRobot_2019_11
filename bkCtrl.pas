unit bkCtrl;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  ComCtrls, ExtCtrls, StdCtrls, Buttons, Grids, Menus,
  MapWork, CommDef, C_Runner, C_wrap, Q_Sort;
  
type
  PMyWatch = ^TMyWatch;
  TMyWatch = record
    pVar: Pointer;
    iType: Integer;     // 0=Global, 1=Local (包括參數), 2=其他自訂位址或結構指標
    dispMode: Pointer;  // Nil=預設顯示方法, NZ=特殊顯示函式
  end;
  PFnLino = ^TFnLino;
  TFnLino = record      // 每個函數的行號陣列資訊,供設立除錯中斷點使用
    pMyFn: Pointer;     // 函數或程序 (PMyFunc or PMyProc)
    linos: ^Integer;    // 行號陣列
    cnt_linos: Integer; // linos[] 總共有幾項
    prog_id: Word;      // 這是哪個程式 (從 1 算起)
    Remains: Word;      // linos[] 還剩幾個備用空間 (多借的)
  end;
  TBkgCtrl = class(TForm)
    BitBtn1: TBitBtn;
    BitBtn2: TBitBtn;
    BitBtn3: TBitBtn;
    BitBtn4: TBitBtn;
    BitBtn5: TBitBtn;
    BitBtn6: TBitBtn;
    BitBtn7: TBitBtn;
    BitBtn8: TBitBtn;
    BitBtn9: TBitBtn;
    BitBtn10: TBitBtn;
    BitBtn11: TBitBtn;
    BitBtn12: TBitBtn;
    BitBtn13: TBitBtn;
    BitBtn14: TBitBtn;
    BitBtn15: TBitBtn;
    BitBtn16: TBitBtn;
    BitBtn17: TBitBtn;
    BitBtn18: TBitBtn;
    FastKeyPanel: TPanel;
    GNameCmbBox: TComboBox;
    IExit1: TMenuItem;
    IFile1: TMenuItem;
    IOpen1: TMenuItem;
    IRun1: TMenuItem;
    ISave1: TMenuItem;
    ISaveAs1: TMenuItem;
    IStep1: TMenuItem;
    IStop1: TMenuItem;
    ITrace1: TMenuItem;
    IWork1: TMenuItem;
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    MainMenu1: TMainMenu;
    Memo1: TMemo;
    Memo2: TMemo;
    OpenDlg: TOpenDialog;
    PageCtrl: TPageControl;
    PageCtrlM: TPageControl;
    Panel1: TPanel;
    Panel2: TPanel;
    Panel3: TPanel;
    Panel4: TPanel;
    Panel5: TPanel;
    Panel6: TPanel;
    Panel7: TPanel;
    SaveDlg: TSaveDialog;
    SpdBtn1: TSpeedButton;
    SpdBtn2: TSpeedButton;
    SpdBtn3: TSpeedButton;
    SpdBtn4: TSpeedButton;
    SpdBtn5: TSpeedButton;
    SpdBtn6: TSpeedButton;
    SpdBtn7: TSpeedButton;
    SpdBtn8: TSpeedButton;
    SpdBtn9: TSpeedButton;
    Splitter1: TSplitter;
    Splitter2: TSplitter;
    Splitter3: TSplitter;
    Splitter4: TSplitter;
    Splitter5: TSplitter;
    StatBar: TStatusBar;
    StDateCmbBox: TComboBox;
    StrGrid1: TStringGrid;      // Global
    StrGrid2: TStringGrid;      // Local
    StrGrid3: TStringGrid;      // Constant
    StrGrid4: TStringGrid;      // Var Watch
    TabSheet1: TTabSheet;
    TabSheet2: TTabSheet;
    TabSheet3: TTabSheet;
    TabSheet4: TTabSheet;
    UsrDateEdit: TEdit;
    procedure at_activate(Sender: TObject);
    procedure ChangeMainFormSize(Sender: TObject);
    procedure doMs1Up(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure doMs2Up(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure doMs4Up(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure do_Exit(Sender: TObject);
    procedure do_moved_splt1(Sender: TObject);
    procedure do_moved_splt4(Sender: TObject);
    procedure do_moved_splt5(Sender: TObject);
    procedure do_ms1_dbclk(Sender: TObject);
    procedure do_resize_splt2(Sender: TObject);
    procedure do_resize_splt3(Sender: TObject);
    procedure do_run(Sender: TObject);
    procedure do_trace(Sender: TObject);
    procedure do_StepOver(Sender: TObject);
    procedure do_Stop(Sender: TObject);
    procedure load_token_file(Sender: TObject);
    procedure do_ms2_dbclk(Sender: TObject);
  private
    procedure AddThisWatch(wt: PMyWatch);
    procedure adj_RowCounts;
    procedure collect_lino_info;
    procedure Show_Infos;
  public
    procedure AnsMsg(s: String);
    procedure Msg(s: String);
    procedure StatSay(col: Integer; str: String);
  end;

  procedure Add_Size_FL;        // pAllFL 表格已滿, 再擴增 !
  procedure AppendLineNoToArray(pF: PMyFunc; lineNo: Integer); // 搜尋 pAllFL[], 找到具有 pF 的那函數項, 加註 lineNo 到行號陣列 !
  procedure DeleteWatch(ndx: Integer);  // 刪除此項 Watch
  function  IntDump(p: Pointer; cnt: Integer): String;
  procedure FindCurrentFunc(lino: Integer);     // 查詢現在在哪個函數 ! 結果保存於 nowInFunc
  function  FindFirstStopLineNo(pName: PChar): Integer;     // 由名稱字串來找其函數第一個有效的可被中斷的行號
  function  FindFunc(pName: PChar): Pointer;    // 由函數名稱字串來找其結構定義
  function  GetByOfs(p: Pointer; ofs: Integer): Integer;
  function  GetFirstStopLineNo(pMF: PMyFunc): Integer;  // 由函數指標來找其第一個有效的可被中斷的行號
  function  GetNextStopLineNo(lino: Integer): Integer;  // 取得下一個可以被中斷的行號
  procedure MakeRunThread(funcPtr, paramPtr: Pointer);
  procedure prepare_prog;
  function  RemoveDuplicate(p: Pointer; cnt: Integer): Integer;

{$I Hex}

const
  width_1: Array [0..5] of Integer = (32, 160, 128, 48, 48, 128);
  caption_1: Array [0..5] of String = ('No.', 'Name', 'Type', 'flag', 'line', 'file');
  width_1a: Array [0..2] of Integer = (48, 100, 60);
  caption_1a: Array [0..2] of String = ('型態', '名稱', '現值'); // Var Watch
  WaitObjName = 'SuDebugEvent';
  vTypeStr: Array [0..19] of String = ('array', 'char', 'byte', 'word',
		'dword', 'sint8', 'sint16', 'int', 'dblchar', 'unicode', 'string', 'single',
		'double', 'pchar', 'ptr', 'integer', 'sint64', 'qword', 'void', 'ansistring');
  MaxWatchCount = 128;

var
  BkgCtrl: TBkgCtrl;
  pProg: Pointer;       // 目前的程式總資料
  nowInFunc: PMyFunc;   // 現在執行到哪個函數 (PMyFunc or PMyProc)
  nowInPFno: PFnLino;   // 現在執行到哪個函數 (PFnLino)
  pAllFL: PFnLino;      // 函數程序的行號表
  pAllWatch: PMyWatch;  // 所有的變數 Watch 表
  lpThrID: LongWord;
  hThr, hWait: HWND;
  line_to_stop: Integer;        // 下一行斷點
  nAllFL: Integer;      // 函數程序的行號表 (現有項目數)
  nMiaFiles: Integer = 0;
  nSrcFiles: Integer = 0;
  nowExecLine: Integer = 0;
  ms1X, ms1Y: Integer;  // msY < 0: 出界, 0=標題欄
  ms2X, ms2Y: Integer;  // msY < 0: 出界, 0=標題欄
  myHandle, nWatch: Integer;
  MaxFL_Count: Integer;    // 函數程序的行號表 (最大項目數)
  miaFiles: Array [0..31] of String;          // Token  filenames
  srcFiles: Array [0..31] of String;          // Source filenames
  nowSrc: String;       // 目前 Memo1 在看的程式
  bInit, bUserStop, fOnDebug: Boolean;


implementation

{$R *.DFM}
{$L Hex.obj}

uses iDebug, VarsMngr;

procedure Add_Size_FL;
var     // 表格已滿, 再擴增 !
   pFL: PFnLino;
   n: Integer;
begin   // 表格已滿, 再擴增 !
     n := SizeOf(TFnLino) * (MaxFL_Count + 64);
     pFL := AllocMem(n);
     Move(pAllFL^, pFL^, SizeOf(TFnLino) * MaxFL_Count);
     FreeMem(pAllFL);   pAllFL := pFL;          Inc(MaxFL_Count, 64);
end;

procedure AppendLineNoToArray(pF: PMyFunc; lineNo: Integer);
var     // 搜尋 pAllFL[], 找到具有 pF 的那函數項, 加註 lineNo 到行號陣列 !
   pFL: PFnLino;
   pI: ^Integer;
   i, n: Integer;
begin
     pFL := pAllFL;
     for i := 0 to nAllFL - 1 do begin
        if pFL^.pMyFn = pF then begin   // 找到了 !
            n := pFL^.cnt_linos;
            if pFL^.Remains < 1 then begin      // 預備空間沒了, 再借 8 個 !
                pFL^.Remains := 8;
                pI := AllocMem((n + 8) shl 2);
                Move(pFL^.linos^, pI^, n shl 2);
                FreeMem(pFL^.linos);    pFL^.linos := pI;
            end;
            // 新增此行號到尾端, 最後再排序
            pI := pFL^.linos;           Inc(pI, n);
            pI^ := lineNo;              Inc(pFL^.cnt_linos);
            Dec(pFL^.Remains);          Exit;
        end;
     end;
end;

procedure Append_SB_LineNoToArray(pSB: PMySmallBlk);
var
   pFL: PFnLino;
   pMF: PMyFunc;
   pI: ^Integer;
   pB: ^Byte;
   i, j, n, nTk, ofs: Integer;
begin
     nTk := pSB^.token_len;     pB := pSB^.tokenAddr;   ofs := 0;
     if pB^ <> $9E then Exit;   // 不正確的行號開頭
     j := (GetByOfs(pB, 1) + 1) shl 2;
     Inc(ofs, j);       Inc(pB, j);     // 忽略指令群內涵
     if pB^ <> $9F then Exit;   // 不正確的行號結尾
     j := GetByOfs(pB, 1);      // 取得此行號
     // 查詢此行號位於哪個函數段
     pFL := pAllFL;
     for i := 0 to nAllFL - 1 do begin
        pMF := pFL^.pMyFn;             
        if (pMF^.bgn_ln <= j) and (j <= pMF^.fin_ln) then begin // 找到了 !
            // 把剩餘的行號都附加上來 !
            repeat
                n := pFL^.cnt_linos;
                if pFL^.Remains < 1 then begin      // 預備空間沒了, 再借 8 個 !
                   pFL^.Remains := 8;
                   pI := AllocMem((n + 8) shl 2);
                   Move(pFL^.linos^, pI^, n shl 2);
                   FreeMem(pFL^.linos);    pFL^.linos := pI;
                end;
                // 新增此行號到尾端, 最後再排序
                pI := pFL^.linos;           Inc(pI, n);
                pI^ := j;                   Inc(pFL^.cnt_linos);
                Dec(pFL^.Remains);
                Inc(ofs, 4);    Inc(pB, 4);     // 越過此行號結尾
                if ofs >= nTk then Break;       // 此區段程式完成
                // 此處應該是下一行的開頭
                if pB^ <> $9E then Exit;        // 不正確的行號開頭
                j := (GetByOfs(pB, 1) + 1) shl 2;
                Inc(ofs, j);    Inc(pB, j);     // 忽略指令群內涵
                if pB^ <> $9F then Exit;   // 不正確的行號結尾
                j := GetByOfs(pB, 1);      // 取得此行號
                if (pMF^.bgn_ln > j) or (j > pMF^.fin_ln) then Exit; // 不正確的行號 !
            until ofs >= nTk;                   // 此區段程式完成
            Exit;
        end;
     end;
end;

procedure DeleteWatch(ndx: Integer);
var
   pNow, pNxt: PMyWatch;
begin   // 刪除此項 Watch
     pNow := pAllWatch;         Inc(pNow, ndx);
     pNxt := pNow;              Inc(pNxt);
     Move(pNxt^, pNow^, (nWatch - 1 - ndx) * SizeOf(TMyWatch));
end;

procedure FindCurrentFunc(lino: Integer);
var
   pFL: PFnLino;
   pMF: PMyFunc;
   i: Integer;
begin   // 查詢現在在哪個函數 ! 結果保存於 nowInFunc  BkgCtrl.
     if pProg = Nil then prepare_prog;
     pFL := pAllFL;     nowInFunc := Nil;
     for i := 0 to nAllFL - 1 do begin
        pMF := pFL^.pMyFn;
        if (pMF^.bgn_ln <= lino) and (lino <= pMF^.fin_ln) then begin // 找到了 !
            nowInPFno := pAllFL;        nowInFunc := pMF;       Exit;
        end;
        Inc(pFL);
     end;
end;

function  GetFirstStopLineNo(pMF: PMyFunc): Integer;
var     // 由函數指標來找其第一個有效的可被中斷的行號
   pFL: PFnLino;
   i: Integer;
begin
     if pMF = Nil then begin  Result := -1;  Exit;  end;    // 無此函數 !
     pFL := pAllFL;
     for i := 0 to nAllFL - 1 do begin
        if pMF = pFL^.pMyFn then begin  // 找到了 !
            Result := pFL^.linos^;      // 傳回第一個有效的可被中斷的行號
            Exit;
        end;
        Inc(pFL);
     end;
     Result := pMF^.bgn_ln;     // 至少傳回該函數的第一行
end;

function  FindFirstStopLineNo(pName: PChar): Integer;
var     // 由名稱字串來找其函數第一個有效的可被中斷的行號
   pMF: PMyFunc;
begin
     Result := -1;
     if pName = Nil then Exit;
     pMF := FindFunc(pName);
     if pMF = Nil then Exit;    // 無此函數 !
     Result := GetFirstStopLineNo(pMF);
end;

function  FindFunc(pName: PChar): Pointer;
var     // 由函數名稱字串來找其結構定義
   pMF: PMyFunc;
   pMP: PMyProc;
   i, n: Integer;
begin
     Result := Nil;
     if pName = Nil then Exit;
     if StrLen(pName) < 1 then Exit;
     n := VarsMgr.GetFuncCount; // 取得函數個數
     for i := 0 to n - 1 do begin
        pMF := VarsMgr.GetFunc(i);
        if StrIComp(pName, pMF^.name) = 0 then begin
            Result := pMF;              Exit;
        end;
     end;

     n := VarsMgr.GetProcCount; // 取得程序個數
     for i := 0 to n - 1 do begin
        pMP := VarsMgr.GetFunc(i);
        if StrIComp(pName, pMP^.name) = 0 then begin
            Result := pMP;              Exit;
        end;
     end;
end;

function  GetByOfs(p: Pointer; ofs: Integer): Integer;
var
   pI: ^Integer;
begin
     pI := p;           Result := 0;
     case ofs of
        0: Result := pI^;
        1: Result := pI^ shr 8;
        2: Result := pI^ shr 16;
        3: Result := pI^ shr 24;
     end;
end;

function  GetNextStopLineNo(lino: Integer): Integer;
var
   pFL: PFnLino;
   pMF: PMyFunc;
   pI: ^Integer;
   i: Integer;
begin   // 取得下一個可以被中斷的行號
     if nowInPFno = Nil then FindCurrentFunc(lino);     // 查詢現在在哪個函數 !
     if nowInPFno = Nil then begin Result := -1;  Exit;  end;   // 失敗 ! 可能沒開啟程式
     pFL := nowInPFno;          pMF := pFL^.pMyFn;
     if lino < pMF^.bgn_ln then begin  Result := pMF^.bgn_ln;  Exit;  end; // 第一行就須停止 !
     pI := pFL^.linos;          Result := pMF^.fin_ln;
     for i := 0 to pFL^.cnt_linos - 1 do begin
        if pI^ > lino then begin // 找到第一個比此行號大的值
            Result := pI^;      Exit;
        end;
        Inc(pI);
     end;
end;

function  IntDump(p: Pointer; cnt: Integer): String;
var
   pI: ^Integer;
   i: Integer;
   s: String;
begin
     s := '';   pI := p;
     for i := 1 to cnt do begin
        s := s + ' ' + IntToStr(pI^);           Inc(pI);
        if Length(s) > 253 then Break;          // string is full !!
     end;
     Result := s;
end;

procedure MakeRunThread(funcPtr, paramPtr: Pointer);
var
   sa: SECURITY_ATTRIBUTES;
begin           { 若無執行緒, 則創造之 }
     if hWait = 0 then begin
        sa.nLength := sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor := Nil;
	sa.bInheritHandle := False;
	hWait := CreateEvent(@sa, False, False, WaitObjName);
     end;
     hThr := CreateThread(Nil, 0, funcPtr, Pointer(hWait), 0, lpThrID);  // paramPtr
     // xSetHandle(myHandle, hThr);  (Why ? 產生執行緒後,送不出去 !!)
end;

procedure prepare_prog;
begin
     init_All_Before_Run;
     pProg := VarsMgr.Collect_Prog_data_A;
     // xHexDump(pProg, 128);      // 124 bytes in 32-bit mode
     load_prog_info(pProg);        // 反覆載入數個程式資料.. 或是 run-time 載入程式資料
     BkgCtrl.collect_lino_info;    // 收集每個函數的行號陣列資訊,供設立除錯中斷點使用
end;

function  RemoveDuplicate(p: Pointer; cnt: Integer): Integer;
var
   pI, pJ: ^Integer;
   nRem: Integer;
   bFound: Boolean;
begin   // 去除重複的 Integer 項目, p 指到整數陣列
     Result := cnt;
     if (p = Nil) or (cnt < 2) then Exit;       // 無資料
     pI := p;       Inc(pI, cnt - 1);
     nRem := 0;      // = pJ 後面剩餘需搬移的資料項數 (pJ^ 本身不用搬)
     repeat
        pJ := pI;   Dec(pI);    bFound := False;        Inc(nRem);
        while pI^ = pJ^ do begin        // 發現重複 !
            bFound := True;     Dec(cnt);
            Dec(pI);    // pJ 不動, nRem 維持不動 !
        end;
        if bFound then begin
           Inc(pI);     Move(pJ^, pI^, nRem shl 2);     Dec(pI);
        end;
     until Cardinal(pI) <= Cardinal(p);
     Result := cnt;
end;

procedure TBkgCtrl.AddThisWatch(wt: PMyWatch);
var
   pW: PMyWatch;
   pG: PMyVar;
   pL: PMyLocal;
   save_nowInFunc: PMyFunc;
   save_nowInPFno: PFnLino;
   j: Integer;
   s0, s1, s2: String;
begin //型態, 名稱, 現值
     if nWatch > (MaxWatchCount - 2) then Exit; // Watch 表格已滿
     pW := pAllWatch;           Inc(pW, nWatch);
     Move(wt^, pW^, SizeOf(TMyWatch));  Inc(nWatch);
     j := nWatch;   // StrGrid4.RowCount;
     if wt.iType = 0 then begin
        pG := wt.pVar;
        save_nowInFunc := nowInFunc;    save_nowInPFno := nowInPFno;
        FindCurrentFunc(pG^.line_no);   // 查詢位於哪個函數, 會破壞 nowInFunc & nowInPFno 的值
        if nowInFunc = Nil then s2 := '全域變數'
        else s2 := nowInFunc^.name;
        s1 := Format(' %s <%s> 第 %d 行', [pG^.name, s2, pG^.line_no]);
        nowInFunc := save_nowInFunc;    nowInPFno := save_nowInPFno;
        s0 := Format('%s ($%x)', [vTypeStr[pG^.vType and 31], pG^.vType]);
        s2 := '..';
     end;
     if wt.iType = 1 then begin
        pL := wt.pVar;
        save_nowInFunc := nowInFunc;    save_nowInPFno := nowInPFno;
        FindCurrentFunc(pL^.line_no);   // 查詢位於哪個函數, 會破壞 nowInFunc & nowInPFno 的值
        if nowInFunc = Nil then s2 := '全域變數'
        else s2 := nowInFunc^.name;
        s1 := Format(' %s <%s> 第 %d 行', [pL^.name, s2, pL^.line_no]);
        nowInFunc := save_nowInFunc;    nowInPFno := save_nowInPFno;
        s0 := Format('%s ($%x)', [vTypeStr[pL^.vType and 31], pL^.vType]);
        s2 := '..';
     end;
     StrGrid4.Cells[0, j] := s0;
     StrGrid4.Cells[1, j] := s1;
     StrGrid4.Cells[2, j] := s2;
end;

procedure TBkgCtrl.adj_RowCounts;
begin
     StrGrid1.RowCount := StrGrid1.Height div 20;
     StrGrid2.RowCount := StrGrid2.Height div 20;
     StrGrid3.RowCount := StrGrid3.Height div 20;
     StrGrid4.RowCount := StrGrid4.Height div 20;
end;

procedure TBkgCtrl.AnsMsg(s: String);
begin
     Memo2.Lines.Add(s);
end;

procedure TBkgCtrl.at_activate(Sender: TObject);
var
   i: Integer;
begin
     if bInit then Exit;
     bInit := True;
     for i := 0 to 5 do begin
         StrGrid1.ColWidths[i] := width_1[i];
         StrGrid1.Cells[i, 0] := caption_1[i];
         StrGrid1.Height := Splitter4.Top - 20;
         StrGrid2.ColWidths[i] := width_1[i];
         StrGrid2.Cells[i, 0] := caption_1[i];
         StrGrid2.Height := Splitter5.Top - Splitter4.Top - 20;
         StrGrid3.ColWidths[i] := width_1[i];
         StrGrid3.Cells[i, 0] := caption_1[i];
         adj_RowCounts;
     end;
     for i := 0 to 2 do begin
         StrGrid4.ColWidths[i] := width_1a[i];
         StrGrid4.Cells[i, 0] := caption_1a[i];
     end;

     ClearAllMemory;		// 清除所有記憶體 (MapWork)
     ParseMapFile('RunRobot.map');
     MaxFL_Count := 64;         nAllFL := 0;    nWatch := 0;
     pAllFL := AllocMem(SizeOf(TFnLino) * MaxFL_Count);
     pAllWatch := AllocMem(SizeOf(TMyWatch) * MaxWatchCount);  // 所有的變數 Watch 表
     VarsMgr.init;
     xSetHandle(Handle, hThr);  myHandle := Handle;
end;

procedure TBkgCtrl.ChangeMainFormSize(Sender: TObject);
begin
     PageCtrl.Height := Splitter1.Top - 4;
     PageCtrlM.Height := PageCtrl.Height - 70;
end;

procedure GetLineNoArray(pFNL: PFnLino; pFn: PMyFunc);
var
   cnt, highln, i, j, lowln, nLines, nMax: Integer;
   pTk: ^Byte;
   lines, pLNs: ^Integer;
begin
     if (pFNL = Nil) or (pFn = Nil) then Exit;
     lowln := pFn^.bgn_ln;      highln := pFn^.fin_ln;
     pTk := pFn^.tokenAddr;     cnt := pFn^.token_len shr 2;
     nLines := 0;               nMax := 512;
     lines := AllocMem(nMax shl 2);     // 初步 max 512 行, 不足再擴增
     pLNs := lines;             i := 0;
     while i < cnt do begin
        if pTk^ <> $9E then Break;      // 不是行開頭, 終止 !
        // 是行開頭, OK !
        j := GetByOfs(pTk, 1) + 1;      Inc(pTk, j shl 2);      // 忽略指令群內涵
        Inc(i, j);
        if pTk^ <> $9F then Break;      // 不是行結尾, 終止 !
        // 是行結尾, OK !
        j := GetByOfs(pTk, 1);          // 取得此行的行號 !
        if nLines > (nMax - 2) then begin   // 表格將滿, 再擴增 512 行
            Inc(nMax, 512);             pLNs := AllocMem(nMax shl 2);
            Move(lines^, pLNs^, nLines shl 2);      FreeMem(lines);
            lines := pLNs;              Inc(pLNs, nLines);
        end;
        if (lowln > j) or (highln < j) then Break;  // 行號的值不正確 !
        pLNs^ := j;     Inc(pLNs);      Inc(nLines);
        Inc(pTk, 4);    Inc(i);         // xSay('Get #' + IntToStr(j));
     end;
     if nLines > 0 then begin
        pFNL^.Remains := 8;     // 多借 8 個 Integer 當預備空間
        pLNs := AllocMem((nLines + 8) shl 2);   // 多借 8 個 Integer 當預備空間
        Move(lines^, pLNs^, nLines shl 2);
     end;
     pFNL^.linos := pLNs;       pFNL^.cnt_linos := nLines;
     if lines <> Nil then FreeMem(lines);
end;

procedure TBkgCtrl.collect_lino_info;
var
   pFLs: PFnLino;
   pMF: PMyFunc;
   pMP: PMyProc;
   pUD: PMyExcBlk;
   pSB: PMySmallBlk;
   i, n: Integer;
   FnLn: TFnLino;
begin   // 收集每個函數的行號陣列資訊,供設立除錯中斷點使用
     if pProg = Nil then prepare_prog;
     if pProg = Nil then Exit;  // No data !
     pFLs := pAllFL;    Inc(pFLs, nAllFL);      // 從現有的項目後方新增
     // 取得 [函數] 行號陣列..
     n := VarsMgr.GetFuncCount; // 取得函數個數
     for i := 0 to n - 1 do begin
        pMF := VarsMgr.GetFunc(i);      FnLn.Remains := 0;
        FnLn.pMyFn := pMF;      FnLn.prog_id := pMF^.prog_id;
        GetLineNoArray(@FnLn, pMF);
        if nAllFL > (MaxFL_Count - 2) then Add_Size_FL;    // 表格已滿, 再擴增 !
        Move(FnLn, pFLs^, SizeOf(TFnLino));
        Inc(nAllFL);            Inc(pFLs);
     end;
     // 取得 [程序] 行號陣列..
     n := VarsMgr.GetProcCount; // 取得程序個數
     for i := 0 to n - 1 do begin
        pMP := VarsMgr.GetProc(i);      FnLn.Remains := 0;
        FnLn.pMyFn := pMP;      FnLn.prog_id := pMP^.prog_id;
        GetLineNoArray(@FnLn, PMyFunc(pMP));
        if nAllFL > (MaxFL_Count - 2) then Add_Size_FL;    // 表格已滿, 再擴增 !
        Move(FnLn, pFLs^, SizeOf(TFnLino));
        Inc(nAllFL);            Inc(pFLs);
     end;
     // 查詢 [UndoneEval] 行號陣列..
     n := VarsMgr.GetUndnCount;
     for i := 0 to n - 1 do begin
        pUD := VarsMgr.GetUndn(i);
        FnLn.pMyFn := FindFunc(pUD^.src_func);
        FnLn.prog_id := pUD^.prog_id;
        AppendLineNoToArray(PMyFunc(FnLn.pMyFn), pUD^.line_no);
     end;
     // 查詢 [切出的程序小區塊] 行號陣列..
     n := VarsMgr.GetSmaBlkCount;
     for i := 0 to n - 1 do begin
        pSB := VarsMgr.GetSmaBlk(i);
        Append_SB_LineNoToArray(pSB);
     end;
     // 將行號陣列排序
     SetSortFCMP(fcmppi);
     pFLs := pAllFL;
     for i := 0 to nAllFL - 1 do begin
        QSortBy(pFLs^.linos, 0, 4, pFLs^.cnt_linos);    // 由小到大排序 !
        pFLs^.cnt_linos := RemoveDuplicate(pFLs^.linos, pFLs^.cnt_linos);  // 去除重複
        pMF := pFLs^.pMyFn;
        xSay(Format('<%s> 有 %d 個行號: %s', [pMF^.name, pFLs^.cnt_linos, IntDump(pFLs^.linos, pFLs^.cnt_linos)]));
        Inc(pFLs);
     end;
end;

procedure TBkgCtrl.doMs1Up(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
     StrGrid1.MouseToCell(X, Y, ms1X, ms1Y);
end;

procedure TBkgCtrl.doMs2Up(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
     StrGrid2.MouseToCell(X, Y, ms2X, ms2Y);
end;

procedure TBkgCtrl.doMs4Up(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
var
   msX, msY: Integer;   // msY < 0: 出界, 0=標題欄
begin
     StrGrid4.MouseToCell(X, Y, msX, msY);
     if msY < 1 then Exit;
//   xSay(format('xy= %d %d', [msX, msY]));
//   if Button = mbLeft then xSay('Lt');
     if Button = mbRight then begin     // 刪除一項 Watch
        if msY > nWatch then Exit;      // 超過現有 Watch 項數
        for Y := msY to StrGrid4.RowCount - 2 do
            for X := 0 to StrGrid4.ColCount - 1 do
                StrGrid4.Cells[X, Y] := StrGrid4.Cells[X, Y + 1];
        Y := StrGrid4.RowCount - 1;
        for X := 0 to StrGrid4.ColCount - 1 do StrGrid4.Cells[X, Y] := '';
        Dec(msY);
        DeleteWatch(msY);       // 刪除此項 Watch
     end;
end;

procedure TBkgCtrl.do_moved_splt1(Sender: TObject);
begin
     PageCtrl.Height := Splitter1.Top - 4;
     PageCtrlM.Height := PageCtrl.Height - 70;
     adj_RowCounts;
end;

procedure TBkgCtrl.do_ms1_dbclk(Sender: TObject);
var
   watch: TMyWatch;
begin   // 把 ms1Y 所指的這項全域變數加到 Watch 表格中
     if ms1Y < 1 then Exit;
     if ms1Y > VarsMgr.GetGlobalVarCount then Exit;     // 超過上限
     watch.iType := 0;  // 0=Global, 1=Local (包括參數), 2=其他自訂位址或結構指標
     watch.dispMode := Nil;     // Nil=預設顯示方法, NZ=特殊顯示函式
     watch.pVar := VarsMgr.GetGlobalVar(ms1Y - 1);
     AddThisWatch(@watch);
end;

procedure TBkgCtrl.do_ms2_dbclk(Sender: TObject);
var
   watch: TMyWatch;
begin   // 把 ms1Y 所指的這項全域變數加到 Watch 表格中
     if ms2Y < 1 then Exit;
     if ms2Y > VarsMgr.GetLocalVarCount then Exit;      // 超過上限
     watch.iType := 1;  // 0=Global, 1=Local (包括參數), 2=其他自訂位址或結構指標
     watch.dispMode := Nil;     // Nil=預設顯示方法, NZ=特殊顯示函式
     watch.pVar := VarsMgr.GetLocalVar(ms2Y - 1);
     AddThisWatch(@watch);
end;

procedure TBkgCtrl.do_resize_splt2(Sender: TObject);
var
   i: Integer;
begin   { 改變上方兩個 panel 的大小 }
//     xSay(Format('sp.L %d, P1.W %d, p3.W %d', [Splitter2.Left, Panel1.Width, Panel3.Width]));
//     Panel1.Width := Splitter2.Left; 不要改變 panel 1 的寬度, 而是去改裏面元件的寬度
     // 防止被拉到最右邊
     if Splitter2.Left > (BkgCtrl.Width - 30) then Panel3.width := 26;
     i := StrGrid4.Width - StrGrid4.ColWidths[0];
     StrGrid4.ColWidths[1] := (i shr 4) * 10;
     StrGrid4.ColWidths[2] := i - StrGrid4.ColWidths[1] - 8;
     adj_RowCounts;
end;

procedure TBkgCtrl.do_resize_splt3(Sender: TObject);
begin
     // 防止被拉到最右邊
     if Splitter3.Left > (BkgCtrl.Width - 30) then Panel4.width := 26;
end;

procedure TBkgCtrl.do_moved_splt4(Sender: TObject);
begin
     // if Splitter4.Top < 40 then Splitter4.Top := 40;
     StrGrid1.Height := Splitter4.Top - 20;
     adj_RowCounts;
end;

procedure TBkgCtrl.do_moved_splt5(Sender: TObject);
begin
     // if Splitter5.Top < 80 then Splitter5.Top := 80;
     StrGrid2.Height := Splitter5.Top - Splitter4.Top - 20;
     adj_RowCounts;
end;

procedure TBkgCtrl.load_token_file(Sender: TObject);
var
   TabSht: TTabSheet;
   Mo: TMemo;
   s: String;
   bScp, bMia: Boolean;
begin   // 載入代碼 (read *.MIA file)
     bMia := False;     bScp := False;
     OpenDlg.DefaultExt := '*.MIA';     OpenDlg.FilterIndex := 2;
     OpenDlg.Filter := '原始程式|*.SCP|代碼 MIA files|*.MIA|all files|*.*';
     repeat
        if not OpenDlg.Execute then Break;
        s := ExtractFileName(OpenDlg.FileName);
        if Pos('scp', LowerCase(s)) > 1 then begin  // 是原始程式
            if nSrcFiles > 31 then begin
                MessageBox(Application.Handle, '太多原始檔案 !', '最多允許 32 檔案', MB_OK);
                Exit;
            end;
            nowSrc := s;        StatSay(0, '原始程式: ' + nowSrc);
            TabSht := TTabSheet.Create(PageCtrlM);
            TabSht.PageControl := PageCtrlM;
            TabSht.Caption := s;
            Mo := TMemo.Create(TabSht);     Mo.Parent := TabSht;
            Mo.Align := alClient;           Mo.Show;        Mo.ScrollBars := ssBoth;
            Mo.Color := $4F0000;            Mo.Font.Assign(Font);
            Mo.Font.Color := $40F0F0;       Mo.Font.Size := 14;
            srcFiles[nSrcFiles] := OpenDlg.FileName;
            Inc(nSrcFiles);
            Mo.Lines.LoadFromFile(OpenDlg.FileName);
            bScp := True;                       OpenDlg.FilterIndex := 2;
            OpenDlg.DefaultExt := '*.MIA';      OpenDlg.FileName := '';
        end;
        if Pos('mia', LowerCase(s)) > 1 then begin  // 是代碼程式
            if nMiaFiles > 31 then begin
                MessageBox(Application.Handle, '太多代碼檔案 !', '最多允許 32 代碼', MB_OK);
                Exit;
            end;
            StatSay(0, '代碼程式: ' + s);
            miaFiles[nMiaFiles] := OpenDlg.FileName;
            Inc(nMiaFiles);
            VarsMgr.LoadMiaFile(OpenDlg.FileName);
            Show_Infos;
            bMia := True;                       OpenDlg.FilterIndex := 1;
            OpenDlg.DefaultExt := '*.SCP';      OpenDlg.FileName := '';
        end;
     until bScp and bMia;
end;

procedure TBkgCtrl.Msg(s: String);
begin
     Memo1.Lines.Add(s);
end;

procedure TBkgCtrl.Show_Infos;
var
   i, n: Integer;
   pL: PMyLocal;
   pN: PMyConst;
   pV: PMyVar;
begin
     n := VarsMgr.GetGlobalVarCount;
     for i := 0 to n - 1 do begin
        pV := VarsMgr.GetGlobalVar(i);
        StrGrid1.Cells[0, i + 1] := IntToStr(i + 1);    // No.
        StrGrid1.Cells[1, i + 1] := StrPas(pV.name);    // Name
        StrGrid1.Cells[2, i + 1] := Format('$%02x (%s)', [pV.vType, vTypeStr[pV.vType and 31]]);    // Type
        StrGrid1.Cells[3, i + 1] := '$' + IntToHex(pV.flag, 2);     // flag
        StrGrid1.Cells[4, i + 1] := IntToStr(pV.line_no);           // line
        StrGrid1.Cells[5, i + 1] := VarsMgr.GetProgName(pV.prog_id);    // file
     end;

     StrGrid2.Cells[3, 0] := '參數編號';
     n := VarsMgr.GetLocalVarCount;
     for i := 0 to n - 1 do begin
        pL := VarsMgr.GetLocalVar(i);
        StrGrid2.Cells[0, i + 1] := IntToStr(i + 1);    // No.
        StrGrid2.Cells[1, i + 1] := StrPas(pL.name);    // Name
        StrGrid2.Cells[2, i + 1] := Format('$%02x (%s)', [pL.vType, vTypeStr[pL.vType and 31]]);    // var type
        StrGrid2.Cells[3, i + 1] := IntToStr(pL.arg_ndx);           // arg ndx (0 = 區域變數, 參數應該由 1 開始)
        StrGrid2.Cells[4, i + 1] := IntToStr(pL.line_no);           // line
        StrGrid2.Cells[5, i + 1] := VarsMgr.GetProgName(pL.prog_id);    // file
     end;
     n := VarsMgr.GetConstCount;
     for i := 0 to n - 1 do begin
        pN := VarsMgr.GetConst(i);
        StrGrid3.Cells[0, i + 1] := IntToStr(i + 1);    // No.
        StrGrid3.Cells[1, i + 1] := StrPas(pN.name);    // Name
        StrGrid3.Cells[2, i + 1] := '$' + IntToHex(pN.vType, 2);    // var type
        StrGrid3.Cells[3, i + 1] := '$' + IntToHex(pN.flag, 2);     // flag
        StrGrid3.Cells[4, i + 1] := IntToStr(pN.line_no);           // line
        StrGrid3.Cells[5, i + 1] := VarsMgr.GetProgName(pN.prog_id);    // file
     end;
     VarsMgr.Show_Fix_Values;
end;

procedure TBkgCtrl.StatSay(col: Integer; str: String);
begin
     StatBar.Panels.Items[col].Text := str;
     Application.ProcessMessages;
end;

procedure TBkgCtrl.do_run(Sender: TObject);
begin
     bUserStop := False;
     if Sender <> Nil then set_step_trace_flag(0);
     if hThr <> 0 then begin
	if hWait <> 0 then PulseEvent(hWait);
	fOnDebug := False;
	Exit;	// 已經在執行了 !
     end;
     if pProg = Nil then prepare_prog;
     set_break_line(1, 51);     // 暫停於第 51 行               
     MakeRunThread(@Run_Program, Nil);
     Run_was_done;              // Free All Ptrs !
end;

procedure TBkgCtrl.do_trace(Sender: TObject);
begin   // 逐行執行追蹤程式 (會跳進去 call 之類的函數)
     if pProg = Nil then prepare_prog;
     if nowExecLine > 0 then line_to_stop := GetNextStopLineNo(nowExecLine)
     else line_to_stop := FindFirstStopLineNo('start');
     set_step_trace_flag(2);         
     Msg('設定下一行斷點: ' + IntToStr(line_to_stop));
     if line_to_stop > 0 then set_break_line(1, line_to_stop);  // prog 1, line 19
     if fOnDebug then begin
	// SetDebug_at_NextLine;	// 在現在的中斷點的下一行設置中斷點 (在 calx_ana)
	fOnDebug := False;
	PulseEvent(hWait);		// 繼續執行 !
	Exit;
     end;
     do_run(nil);
end;

procedure TBkgCtrl.do_StepOver(Sender: TObject);
begin	// Step over 單步執行 (不跳進去 call 之類的函數)
     if pProg = Nil then prepare_prog;
     if nowExecLine > 0 then line_to_stop := GetNextStopLineNo(nowExecLine)
     else line_to_stop := FindFirstStopLineNo('start');
     set_step_trace_flag(1);
     Msg('設定下一行斷點: ' + IntToStr(line_to_stop));
     if line_to_stop > 0 then set_break_line(1, line_to_stop);  // prog 1, line 19
     if fOnDebug then begin
	// SetDebug_at_NextLine;	// 在現在的中斷點的下一行設置中斷點 (在 calx_ana)
	fOnDebug := False;
	PulseEvent(hWait);		// 繼續執行 !
	Exit;
     end;
     do_run(nil);
end;

procedure TBkgCtrl.do_Stop(Sender: TObject);
// var
//   p: PChar;
begin
     bUserStop := True;         StatSay(0, '嘗試停止執行..');
     xSetHandle(Handle, hThr);
     myHandle := Handle;
end;

procedure TBkgCtrl.do_Exit(Sender: TObject);
begin
     Application.Terminate;
end;

end.
// !!@@ 注意 @@!!
// do_sys_error() // #line 1384  bSysErr = 1;	// true




