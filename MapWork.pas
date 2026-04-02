unit MapWork;

interface

uses
  Windows, SysUtils, Q_Sort, iDebug;

type
  TCheckPtr = record
    addr: ^Cardinal;
    str: PChar;
  end;

  TFuncEntPt = record
    Sel: Word;
    Ofs: Cardinal;
    Name: array [0..63] of Char;
  end;

  TLineAddr = record
    Sel, Module: Byte;
    LineNo: Word;
    Ofs: Cardinal;
  end;

  TSegment = record
    Sel: Word;
    Ofs, Len: Cardinal;
    Name: array [0..24] of Char;
    Clas: array [0..7] of Char;		// 'CODE', 'DATA', 'BSS'...
  end;

  procedure AddPtrCheck(infoStr: string; ptr: Pointer);	// 新增一個值到指標檢查中 !
  function  Addr_BeforeN(n: Integer): Integer;
  function  Addr_Here: Integer;		// 傳回呼叫處的位址
  function  AnalyzeCaller(addr: Integer): string;	// 傳回所給 addr 是位於哪個程式的第幾行
  procedure ClearAllMemory;			// 清除所有記憶體
  procedure do_sort_by_ofs;             // sort by linp[xx].Ofs        
  function  fcmpi(pA, pB: Pointer): Integer;
  function  GetWordBy(var st: string; stopStr: string): string;	// 從 st 取出最前端的字串, 遇到 stopStr 會略過而停止
  function  GetSegName(n: Integer): string;
  procedure LogModuleName(s: string);
  function  ParseMapFile(fn: string): Integer;	// 解析 Detailed Map file (Borland 類)
  function  PtrCheck: Boolean;
  procedure SayErr(s: String);
  function  See_Memory_Status: Integer;         // 傳回記憶體可借的數量;

const
  MAX_FUNC_ENTP = 4095;
  MAX_PTR_CHKS = 511;
  MAXINFOSTR = MAX_PTR_CHKS shl 4;
  MAX_LINE_ENTP = 131071;
  MAX_MODU_SEGS = 767;
  MAX_MODULES = 127;
  MAX_SEGS = 15;

var
  HiLimit, LoLimit: Cardinal;
  BaseAddress: Integer = $11000;  // or $401000
  nCPtr, nCurMod, nFunc, nLine, nModSegs, nSegs, ProgEntry: Integer;
  pModules, pMTail: PChar;
  pInfo, pInfA: PChar;
  cptr: array of TCheckPtr;	// [0..MAX_PTR_CHKS]
  entp: array of TFuncEntPt;	// [0..MAX_FUNC_ENTP]
  linp: array of TLineAddr;	// [0..MAX_LINE_ENTP]
  modules: array of PChar;	// [0..MAX_MODULES]
  segm: array of TSegment;	// [0..MAX_MODU_SEGS]
  segs: array of TSegment;	// [0..MAX_SEGS]
  bPtrCheck: Boolean;

implementation

procedure AddPtrCheck(infoStr: string; ptr: Pointer);
var
   fin: Integer;
begin	// 新增一個值到指標檢查中 !
     if nCPtr >= MAX_PTR_CHKS then Exit;	// 表格已滿
     if (pInfA - pInfo) >= (MAXINFOSTR - 16) then Exit;		// 字串區已滿
     fin := Length(infoStr) + 1 + Integer(pInfA);
     if (fin - Integer(pInfo)) >= (MAXINFOSTR - 16) then Exit;	// 字串區已滿
     if Cardinal(ptr) > HiLimit then Exit;
     if Cardinal(ptr) < LoLimit then Exit;	// 指標位址不良
     cptr[nCPtr].addr := ptr;			cptr[nCPtr].str := pInfA;	Inc(nCPtr);
     StrPCopy(pInfA, infoStr);			pInfA := PChar(fin);
end;

function  Addr_BeforeN(n: Integer): Integer;
begin
     asm
     push   ebp
     mov    ebp, esp
     add    eax, ebp	// eax = n
     mov    eax, [eax]
     pop    ebp
     mov    Result, eax
     end;
end;

function  Addr_Here: Integer;
begin
     asm
     push   ebp
     mov    ebp, esp
     mov    eax, [ebp + 12]
     pop    ebp
     mov    Result, eax
     end;
end;

function  find_func(addr: Integer): string;
var   //  傳回所給 addr (已經減掉 BaseAddress) 是位於哪個函數
   i: Integer;
begin
     Result := '不明函數';
     if addr < 0 then Exit;
     for i := 0 to nFunc - 2 do begin
	if Cardinal(addr) >= entp[i].Ofs then begin
	    if Cardinal(addr) < entp[i+1].Ofs then begin
//		Result := Format('在 %s: %s 之後', [GetSegName(entp[i].Sel) ,entp[i].Name]);
		Result := Format('在 %s 之後', [entp[i].Name]);
		Exit;
	    end;
	end;
     end;

end;

function  AnalyzeCaller(addr: Integer): string;
var	// 傳回所給 addr 是位於哪個程式的第幾行
   i: Integer;
begin
     Result := '此位址查不到程式行號: $' + IntToHex(addr, 8);
     addr := addr - BaseAddress;	if addr < 0 then Exit;
     for i := 0 to nLine - 2 do begin
	if Cardinal(addr) >= linp[i].Ofs then begin
	    if Cardinal(addr) < linp[i+1].Ofs then begin
                // xSay(Format('addr = $%x', [addr]));
                // xSay(Format('i: %d, %d, $%x', [linp[i].Module, linp[i].LineNo, linp[i].Ofs]));
                // xSay(Format('i+1: %d, %d, $%x', [linp[i+1].Module, linp[i+1].LineNo, linp[i+1].Ofs]));
		Result := Format('%s 第 %d 行 (%s)', [modules[linp[i].Module], linp[i].LineNo, find_func(addr)]);
		Exit;
	    end;
	end;
     end;
end;

procedure ClearAllMemory;
begin	// 清除所有記憶體
     SetLength(cptr, MAX_PTR_CHKS  +1);
     SetLength(entp, MAX_FUNC_ENTP +1);
     SetLength(linp, MAX_LINE_ENTP +1);
     SetLength(modules, MAX_MODULES +1);
     SetLength(segm, MAX_MODU_SEGS  +1);
     SetLength(segs, MAX_SEGS       +1);
     ZeroMemory(@cptr[0], (MAX_PTR_CHKS + 1) shl 2);	nCPtr := 0;	pInfo := AllocMem(MAXINFOSTR);	pInfA := pInfo;
     ZeroMemory(@segs[0], SizeOf(TSegment) * (MAX_SEGS + 1));		nSegs := 0;
     ZeroMemory(@segm[0], SizeOf(TSegment) * (MAX_MODU_SEGS + 1));	nModSegs := 0;
     ZeroMemory(@entp[0], SizeOf(TFuncEntPt) * (MAX_FUNC_ENTP + 1));	nFunc := 0;
     ZeroMemory(@linp[0], SizeOf(TLineAddr) * (MAX_LINE_ENTP + 1));	nLine := 0;
     pModules := AllocMem(32 * (MAX_MODULES + 1));	pMTail := pModules;	nCurMod := -1;
     bPtrCheck := True;		LoLimit := $10000;	HiLimit := $20000000;
end;

procedure do_sort_by_ofs;
begin   // sort by linp[xx].Ofs
     SetSortFCMP(fcmpi);
     QSortBy(entp, 4, SizeOf(TFuncEntPt), nFunc);       // align with dw, 雖然前一項是 Word, 但佔 4 bytes !
     QSortBy(linp, 4, 8, nLine);
end;

function  fcmpi(pA, pB: Pointer): Integer;
var
   pIA, pIB: ^Integer;
   r: Integer;
begin   { 整數指標內容比較 }
     pIA := PInteger(pA);       pIB := PInteger(pB);     r := 1;
     if pIA^ = pIB^ then r := 0;
     if pIA^ < pIB^ then r := -1;
     Result := r;
end;

function  GetSegName(n: Integer): string;
var
   i: Integer;
begin
     for i := 0 to nSegs - 1 do begin
         if segs[i].Sel = n then begin
            Result := segs[i].Name;
            Exit;
         end;
     end;
     Result := '不明的段落';
end;

function  GetWordBy(var st: string; stopStr: string): string;
var
   i: Integer;
begin	// 從 st 取出最前端的字串, 遇到 stopStr 會略過而停止
     i := Pos(stopStr, st);
     if i > 0 then begin
	Result := Trim(Copy(st, 1, i-1));
	st := Copy(st, i + Length(stopStr), Length(st) - i);	Exit;
     end;
     st := Trim(st);
end;

procedure LogModuleName(s: string);
var
   i, j: Integer;
//   pM: PChar;
begin
     i := Pos('(', s);	     j := Pos(')', s);		if (i < 1) or (j < 1) then Exit;	// not good !
     Inc(nCurMod);	modules[nCurMod] := pMTail;	j := j - i - 1;		 // pM := pMTail;
     StrPCopy(pMTail, Copy(s, i + 1, j));	Inc(pMTail, j + 1);
//     xSayStrHex(pM, nCurMod, 1);
end;

function  ParseMapFile(fn: string): Integer;
var
   p: Pointer;
   F: TextFile;
   i, j: Integer;
   stage: Byte;
   s, s1: string;
begin	// 解析 Detailed Map file (Borland 類)  0001:000010EC       SysGetMem
     p := @SysGetMem;           BaseAddress := Cardinal(p) - $10EC;  // !! bug: for Delphi 4.0 only !!
     xSay(Format('p = $%x', [Cardinal(p)]));
     Result := 0;		if not FileExists(fn) then Exit;
     ClearAllMemory;	// 清除所有記憶體
     AssignFile(F, fn);		FileMode := 0;	// Set file access to read only
     Reset(F);		stage := 0;	xSay('解析檔案: ' + fn);
     repeat
	Readln(F, s);
	case stage of
	    0:	if Pos('Start', s) > 0 then Inc(stage);	// 有讀取到 'Start / Length / Name / Class'
	    1:	begin	// 記錄 big segments
		    if Pos('Detailed map', s) > 0 then Inc(stage)
		    else begin	// 其他: 讀取 segment data
			if Pos(':', s) < 1 then Continue;	// 此行無資料
			if nSegs >= MAX_SEGS then Continue;	// 表格已滿
			s1 := '$' + GetWordBy(s, ':');	segs[nSegs].Sel := StrToInt(s1);
			s1 := '$' + GetWordBy(s, ' ');	segs[nSegs].Ofs := StrToInt(s1);
			s1 := '$' + GetWordBy(s, 'H ');	segs[nSegs].Len := StrToInt(s1);
			s1 := GetWordBy(s, ' ');	StrPCopy(segs[nSegs].Name, s1);
			StrPCopy(segs[nSegs].Clas, Trim(s));
//			s := Format('class = %s, name = %s, %X:%X Len=%X', [segs[nSegs].Clas, segs[nSegs].Name, segs[nSegs].Sel, segs[nSegs].Ofs, segs[nSegs].Len]);
//			xSay(s);
			Inc(nSegs);
		    end;
		end;
	    2:	begin	// 記錄 module segments
		    if Pos('by Name', s) > 0 then Inc(stage)
		    else begin	// 其他: 讀取 segment data
			if Pos(':', s) < 1 then Continue;	// 此行無資料
			if nModSegs >= MAX_MODU_SEGS then Continue;	// 表格已滿
			s1 := '$' + GetWordBy(s, ':');	segm[nModSegs].Sel := StrToInt(s1);
			s1 := '$' + GetWordBy(s, ' ');	segm[nModSegs].Ofs := StrToInt(s1);
			s1 := '$' + GetWordBy(s, ' C=');	segm[nModSegs].Len := StrToInt(s1);
			s1 := GetWordBy(s, ' S=');	StrPCopy(segm[nModSegs].Clas, Trim(s1));
			GetWordBy(s, ' M=');		s1 := GetWordBy(s, 'ACBP=');
			StrPCopy(segm[nModSegs].Name, Trim(s1));
//			s := Format('class = %s, name = %s, %X:%X Len=%X', [segm[nModSegs].Clas, segm[nModSegs].Name, segm[nModSegs].Sel, segm[nModSegs].Ofs, segm[nModSegs].Len]);
//			xSay(s);
			Inc(nModSegs);
		    end;
		end;
	    3:	begin	// 記錄 function entry address
		    if Pos('by Value', s) > 0 then Inc(stage);
		    Continue;	// 其他: 讀取函數進入點, 此為按照名稱排序, 我不讀這個..
		end;
	    4:	begin	// 記錄 function entry address
		    if Pos('Line num', s) > 0 then begin  LogModuleName(s);	Inc(stage);  end
		    else begin	// 其他: 讀取函數進入點
			if Pos(':', s) < 1 then Continue;	// 此行無資料
			if nFunc >= MAX_FUNC_ENTP then Continue;	// 表格已滿
			s1 := '$' + GetWordBy(s, ':');	entp[nFunc].Sel := StrToInt(s1);
			s1 := '$' + GetWordBy(s, ' ');
			if StrToInt(Copy(s1, 1, 4)) > $7FF then begin	// 略過此項
			    xSay('    略過 $' + s1 + ': ' + Trim(s)); Inc(nFunc);  Continue;
			end;
			entp[nFunc].Ofs := StrToInt(s1);
			s := Trim(s);	i := Length(s);		if i > 63 then s := Copy(s, i-64, 63);
			StrPCopy(entp[nFunc].Name, s);
//			s := Format('%X:%X  name = %s', [entp[nFunc].Sel, entp[nFunc].Ofs, entp[nFunc].Name]);
//			xSay(s);
			Inc(nFunc);
		    end;
		end;
	    5:	begin	// 記錄 行號資訊 entry address
		    if Pos('Bound res', s) > 0 then Inc(stage)
		    else begin	// 其他: 讀取函數進入點
			if Pos('Line num', s) > 0 then begin  LogModuleName(s);  Continue;  end;
			if Pos(':', s) < 1 then Continue;	// 此行無資料
			if nLine >= MAX_LINE_ENTP then Continue;	// 表格已滿
			j := 4;
			s := Trim(s);
			repeat
			    s1 := Trim(GetWordBy(s, ' '));     	if Length(s1) < 1 then Break;
			    i := StrToInt(s1);			linp[nLine].LineNo := i and 65535;
			    linp[nLine].Module := nCurMod;
			    s1 := '$' + GetWordBy(s, ':');		linp[nLine].Sel := StrToInt(s1);
			    if Pos(' ', s) < 1 then linp[nLine].Ofs := StrToInt('$' + s)
			    else begin  s1 := '$' + GetWordBy(s, ' ');  linp[nLine].Ofs := StrToInt(s1);  end;
//			if (cnt > 6850) and (cnt < 6860) then xSay(Format('%s (%d) %X:%X', [modules[linp[nLine].Module], linp[nLine].LineNo,linp[nLine].Sel, linp[nLine].Ofs]));
			    s := Trim(s);	Inc(nLine);	    Dec(j);
                            if Length(s) < 15 then j :=0 ;          // too short to analyze !
			until j < 1;
		    end;
		end;
	    6:	begin
		    if Pos('entry point', s) > 0 then begin
			s1 := GetWordBy(s, ':');	ProgEntry := StrToInt('$' + Trim(s));	Inc(stage);
		    end;
		end;
	    else Break;
	end;
     until Eof(F);	//or (cnt > 3500)
     CloseFile(F);	xSayStrHex('  程式進入點 $', ProgEntry, 4);
     do_sort_by_ofs;    // sort by linp[xx].Ofs & entp[xx].Ofs
     xSay(Format('  行號總數 %d', [nLine]));
end;

function  PtrCheck: Boolean;
var
   val: Cardinal;
   i: Integer;
begin
     Result := False;	// = 安全
     if not bPtrCheck then Exit;	// 暫免檢查
     for i := 0 to nCPtr - 1 do begin
	val := cptr[i].addr^;
        if (val < LoLimit) or (val > HiLimit) then begin
		xSay(Format('不良指標 %s: $%X (內值 = $%X)', [cptr[i].str, Cardinal(cptr[i].addr), val]));
		Result := True;
	end;
     end;
end;

// $4026C4 GetMem --> $402104 SysGetMem  ( 0001:00001104  SysGetMem  -- in sutest.map)
// 找到 --> $4021E4 = (20 E4 46 00)
// 變數 $46E420 呼叫 allocated memory 的次數 (AllocMemCount)
// 變數 $46E424 total allocated memory count
// 變數 $46E484 usable local memory
//  xSay('已借記憶體次數: ' + IntToStr(AllocMemCount));    // in System.pas Line 354
//  xSay('已借記憶體: ' + IntToStr(AllocMemSize) + ' bytes.');
function  See_Memory_Status: Integer;
var
   pI: ^Integer;
begin
     pI := Pointer(Cardinal(@SysGetMem) + $E0);    // = $4021E4
     pI := Pointer(pI^);       // pI = $46E420 = 變數 $46E420 呼叫 allocated memory 的次數 (AllocMemCount)
     xSay('AllocMemCount = ' + IntToStr(pI^));
     Inc(pI);
     xSay('已經借用的記憶體數量 = ' + IntToStr(pI^));
     Inc(pI, $60 shr 2);
     xSay('尚可借的記憶體數量 = ' + IntToStr(pI^));
     Result := pI^;     // 傳回記憶體可借的數量
end;

procedure SayErr(s: String);
var
   addr: Integer;
begin
     asm
     push   ebp
     mov    ebp, esp
     mov    eax, [ebp + $314]
     pop    ebp
     mov    addr, eax
     end;
     s := s + '  於 ' + AnalyzeCaller(addr);
     xSay(s);
end;

end.
