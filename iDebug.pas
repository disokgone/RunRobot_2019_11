unit iDebug;

interface

uses
  Windows, SysUtils, Messages, Forms, Classes, StdCtrls;

type
  TAProc = procedure;

  function  getErrStr(id: Integer): string;
  procedure init_Debug(x, y, w, h: Integer);
  procedure msg_proc;
  procedure padLeftSpace(var s: string; len: Integer);
  procedure padSpace(var s: string; len: Integer);
  procedure PasteDebug;
  procedure xErr(n: Integer);
  procedure xHexDump(p: Pointer; n: Integer);
  procedure xImg(cType: Char; buf: Pointer; xl, yl, len: Integer);
  procedure xLog(pS: PChar; val, len, len1: Integer; ptr: Pointer);
  procedure xSay(p: PChar);     overload;
  procedure xSay(s: string);    overload;
  procedure xSayN(n: Integer; s: string);
  procedure xSayStrHex(pS: PChar; val, len: Integer);
  procedure xSendCmd(s: String);
  procedure xSetHandle(hwnd1, hwnd2: Integer);
  procedure xUseWin(i: Integer);

const
   sCRLF: PChar = #13 + #10 + #0;
   sOneLineCmd: PChar = '1 001' + #13 + #10 + #0;

var
   hDebug, mainWinHwnd, nowSay: Integer;
   tok_ndx: Integer;
   WinMajVer, WinMinVer, SrvPack: Word;
   nWait: Short;
   logFlag, sOneLine: PChar;
   pCommonPool, pComNow: PChar;
   image_buf, pOutput: PChar;
   bAllowSay: Boolean;
   App_ProcMsg: TAProc;

implementation

function  getErrStr(id: Integer): string;
var
   pTxt: array [0..127] of Char;
begin
     FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nil, id, 0, @pTxt[0], 128, nil);
     Result := StrPas(pTxt);
end;

procedure init_Debug(x, y, w, h: Integer);
//var
//   pT: Array [0..127] of Char;
//   hnd: Integer;
begin
     if hDebug = 0 then begin
        image_buf := AllocMem(4096);
        hDebug :=  FindWindow('TDebugSu', '除錯視窗');
        if pOutput = nil then pOutput := AllocMem(1024);
        if sOneLine = nil then begin
            sOneLine := AllocMem(16);
            Move(sOneLineCmd^, sOneLine^, 8);
        end;
        if hDebug = 0 then Exit;        // 找不到除錯視窗 !
        nowSay := GetWindowLong(hDebug, GWL_USERDATA);
{        hnd := GetWindow(hDebug, GW_CHILD);
        repeat  // 反覆取得其子視窗
                GetClassName(hnd, pT, 31);
                if pT = 'TMemo' then nowSay := hnd; // 找到次序: Memo4,3,2,1
                hnd := GetWindow(hnd, GW_HWNDNEXT);
                if hnd = 0 then Break;
        until nowSay > 0;  }
        App_ProcMsg := msg_proc;
     end;
end;

procedure msg_proc;
begin
     Application.ProcessMessages;
     Inc(nWait);
     if nWait > 0 then begin
	logFlag^ := #1;		nWait := 0;
     end;
end;

procedure padLeftSpace(var s: string; len: Integer);
var
   n: Integer;
begin
     n := Length(s);
     if n >= len then Exit;
     Move(s[1], s[len - n + 1], n);
     FillChar(s[1], len - n, #32);      s[0] := Char(len);
end;

procedure padSpace(var s: string; len: Integer);
var
   n: Integer;
begin
     n := Length(s);
     if n > len then Exit;
     FillChar(s[n + 1], len-n-1, #32);          s[0] := Char(len-1);
end;

procedure PasteDebug;
begin
     SendMessage(nowSay, WM_SETTEXT, 0, Integer(pOutput));
     PostMessage(nowSay, WM_KEYDOWN, VK_END, 0);
     PostMessage(nowSay, WM_KEYUP, VK_END, 0);
     PostMessage(nowSay, WM_KEYDOWN, 32, 0);
     PostMessage(nowSay, WM_KEYUP, 32, 0);
end;

procedure xHexDump(p: Pointer; n: Integer);
var
   i, j, m: Integer;
   pB: PByte;
   pC: PChar;
   pT: PChar;
   s: string;
begin
     if hDebug = 0 then init_Debug(0, 0, 0, 0);
     if hDebug = 0 then Exit;
     pB := p;   pC := PChar(pB);
     if Integer(pC) < $100000 then Exit;
     if Integer(pC) > $6000000 then Exit;
     for j := 0 to (n - 1) shr 4 do begin
        s := '$' + IntToHex(Integer(pC), 8) + ' - ';
        m := (n - 1) and 15;    if n > 16 then m := 15;
        for i := 0 to m do s := s + IntTohex(Byte(pC[i]), 2) + ' ';
        padSpace(s, 62);
        for i := 0 to m do if Byte(pC[i]) < 32 then s := s + '.'
            else s := s + pC[i];
        s[24] := '-';   s[36] := '-';   s[48] := '-';
        pT := pOutput;          StrCopy(pT, sOneLine);
        StrPCopy(pT + 7, s);    PasteDebug;     Sleep(1);
        Dec(n, 16);             Inc(pC, 16);
     end;
end;

procedure xErr(n: Integer);
var
   buf: PChar;
begin
     // #define MAKELANGID(p, s) ((((WORD) (s)) << 10) | (WORD) (p)) --> LANG_NEUTRAL=0, SUBLANG_DEFAULT=1
     buf := nil;
     FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER or FORMAT_MESSAGE_FROM_SYSTEM or FORMAT_MESSAGE_IGNORE_INSERTS,
        nil, n, $401, buf, 64, nil);
     xSay(StrPas(buf));
     LocalFree(Cardinal(buf));
end;

procedure xImg(cType: Char; buf: Pointer; xl, yl, len: Integer);
var
//   rtv: Integer;
   pT: PChar;
   pC: array [0..255] of Char;
   s: string;
   img_flag: Boolean;
begin
     if hDebug = 0 then init_Debug(0, 0, 0, 0);
     if hDebug = 0 then Exit;
     img_flag := False;    
     if len > 1048570 then len := 1048570;
     Move(buf^, image_buf^, len);
     s := Format('i %s %d,%d,%d,%d,%d,%d', [cType, xl, yl, mainWinHwnd, Integer(image_buf), len, Integer(@img_flag)]);
     pT := pOutput;     pOutput := pC;          nWait := -3;
     StrPCopy(pC, s);   PasteDebug;     Sleep(1);       pOutput := pT;
     repeat
        App_ProcMsg;            // call to Application.ProcessMessages;
        Sleep(100);
     until img_flag or Boolean(GetKeyState(VK_SCROLL));
end;

procedure xLog(pS: PChar; val, len, len1: Integer; ptr: Pointer);
var
   done_flag: Boolean;
   pT: PChar;
   pC: array [0..255] of Char;
   s: string;
begin
     if hDebug = 0 then init_Debug(0, 0, 0, 0);
     if hDebug = 0 then Exit;
     done_flag := False;    logFlag := PChar(@done_flag);
     len := len and 7;	// 目前只允許: 1=Byte(val), 2=Word(val), 4=Dword(val), 0=Pointer(此時 val 無效, 改用 ptr 所指的記憶體)
	 if len1 > 4096 then len1 := 4096;		// 目前僅限定最多搬運 4096 bytes !
     if len = 0 then begin
	Move(ptr^, image_buf^, len1);
     	s := Format('L %s %d,%d,%d,%d,%d,%s', [Char($30 or (len)), len1, mainWinHwnd, Integer(image_buf), Integer(ptr), Integer(@done_flag), pS]);
     end
     else begin
	case len of
	    1: s := Format('L 1 %d,%d,0,0,%d,%s', [val and 255, mainWinHwnd, Integer(@done_flag), pS]);
	    2: s := Format('L 2 %d,%d,0,0,%d,%s', [val and 65535, mainWinHwnd, Integer(@done_flag), pS]);
	    4: s := Format('L 4 %d,%d,0,0,%d,%s', [val, mainWinHwnd, Integer(@done_flag), pS]);
	end;
     end;
     pT := pOutput;     pOutput := pC;          nWait := -3;
     StrPCopy(pC, s);   PasteDebug;     Sleep(1);       pOutput := pT;
     repeat
        App_ProcMsg;            // call to Application.ProcessMessages;
        Sleep(100);
     until done_flag or Boolean(GetKeyState(VK_SCROLL));
end;

procedure xSay(p: PChar);  overload;
begin
     if hDebug = 0 then init_Debug(0, 0, 0, 0);
     if hDebug = 0 then Exit;
     StrCopy(pOutput, sOneLine);
     StrCopy(pOutput + 7, p);    PasteDebug;     Sleep(1);
end;

procedure xSay(s: string);  overload;
begin
     if hDebug = 0 then init_Debug(0, 0, 0, 0);
     if hDebug = 0 then Exit;
     StrCopy(pOutput, sOneLine);
     StrPCopy(pOutput + 7, s);   PasteDebug;     Sleep(1);
end;

procedure xSayN(n: Integer; s: string);
begin
     sOneLine[0] := Char(49 + n);
     xSay(s);
     sOneLine[0] := Char(49);
end;

procedure xSayStrHex(pS: PChar; val, len: Integer);
var
   pT: PChar;
   s: string;
begin
     if hDebug = 0 then init_Debug(0, 0, 0, 0);
     if hDebug = 0 then Exit;
     pT := pOutput;             StrCopy(pT, sOneLine);
     pT := StrECopy(pT + 7, pS);
     case len of
        1: s := IntToHex(val, 2);
        2: s := IntToHex(val, 4);
        else s := IntToHex(val, 8);
     end;
     StrPCopy(pT, s);   PasteDebug;     Sleep(1);
end;

procedure xSendCmd(s: String);
var
   done_flag: Boolean;
   pT: PChar;
   pC: array [0..255] of Char;
begin
     nWait := -5;       done_flag := False;     logFlag := PChar(@done_flag);
     pT := pOutput;     pOutput := pC;
     StrPCopy(pC, s);   PasteDebug;     Sleep(1);       pOutput := pT;
     repeat
        App_ProcMsg;            // call to Application.ProcessMessages;
        Sleep(100);
     until done_flag or Boolean(GetKeyState(VK_SCROLL));
end;

procedure xSetHandle(hwnd1, hwnd2: Integer);
begin
     if hDebug = 0 then Exit;      // no debug window exists !
     xSendCmd(Format('H %d,%d', [hwnd1, hwnd2]));
end;

procedure xUseWin(i: Integer);
begin
     sOneLine[0] := Char(49 + i);
end;

end.

