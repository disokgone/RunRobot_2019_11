unit C_Runner;

interface

uses
  Windows, Messages, SysUtils, Math, Classes, CommDef, C_wrap, iDebug, Q_sort;
     
const
  FLAG_STOP = $80;              // 暫停程式
  V_EXEC = $10000;              // 訊息來自程式執行中
  BEFORE_EXEC_LINE = 1;         // in exec_this_line #1334 即將執行一行指令
  BEFORE_EXEC = (V_EXEC + BEFORE_EXEC_LINE);	// in exec_this_line #1334 即將執行一行指令
  DEBUG_STOP = 2;
  DEBUG_STOPPED	= (V_EXEC + DEBUG_STOP);
  SHOW_INSTR = 3;
  SHOW_INSTRUCT	= (V_EXEC + SHOW_INSTR);
  SHOW_VAR_NS = 4;		// 變數通常是 struct TNameStru *
  SHOW_VAR = (V_EXEC + SHOW_VAR_NS);
  FUNC_PRINTF = 5;
  PRINTF_ANS = (V_EXEC + FUNC_PRINTF); 		// in myprintf
  FUNC_RETURN = 6;
  RETURN_ANS = (V_EXEC + FUNC_RETURN); 		// in myreturn
  WARN_OR_ERROR = 7;
  WARN_ERR = (V_EXEC + WARN_OR_ERROR); 		// in do_sys_error

  function  get_errMsg(msg_id: Integer): PChar; stdcall;
  procedure init_All_Before_Run; stdcall;       // 程式初始化
  procedure load_prog_info(p: Pointer); stdcall;         // 載入一個程式
  function  Run_Program(parm: Pointer): Integer; stdcall;       // 執行程式
  procedure Run_was_done; stdcall;              // Free All Ptrs !
  function  set_break_line(prog_id: Integer; line_no: Integer): Integer; stdcall; // 設置中斷點
  procedure set_step_trace_flag(flag: Integer); stdcall;  // 0=Not Enabled Stop, 1=Step Stop Enabled, 2=Trace Stop Enabled
  function  status_updater(c_line_no, func_id, func_arg: Integer; func_ptr: Pointer): Integer;
  // 底下的宣告是為了在 .map 檔裡面顯示此位址
  procedure allocate_local(pArg: Pointer); stdcall;
  // procedure yHexDump(p: Pointer; len: Integer); stdcall; 

implementation

uses bkCtrl, VarsMngr;

{$I Hex.pas}
{$L hex.obj}
{$L c_extra.obj}
{$L sufloat.obj}
{$L runner.obj}
  function  get_errMsg(msg_id: Integer): PChar; stdcall; external;
  procedure init_All_Before_Run; stdcall; external;	// 程式初始化
  procedure load_prog_info(p: Pointer); stdcall; external;       // 載入一個程式
  function  Run_Program(parm: Pointer): Integer; stdcall; external;      // 執行程式
  procedure Run_was_done; stdcall; external;            // Free All Ptrs !
  function  set_break_line(prog_id: Integer; line_no: Integer): Integer;  stdcall; external; // 設置中斷點
  procedure set_step_trace_flag(flag: Integer);  stdcall; external;
  procedure allocate_local(pArg: Pointer); stdcall; external;
  // procedure yHexDump(p: Pointer; len: Integer); stdcall;  external;
{  function  ReadDW(p: Pointer; loc: Integer): Integer; external;
  function  ReadInt(buf: PChar; index: Integer): Integer; external;
  function  SetDW(p: Pointer; loc, val: Integer): Integer; external;
  procedure SetInt(buf: PChar; index: Integer; SrcVar:Integer); external;
  function  PtrAdd(p: Pointer; disp, siz: Integer): Pointer; external; }

procedure cSay(s: PChar);
begin
     xSay(s);
end;

procedure cSayN(n: Integer; s: PChar);
begin
     xSayN(n, s);
end;

function cvtShort(c: Char): Short;
begin
     asm
        and al, al
        cbw         
        mov Result, ax
     end;
end;

function get_val_str(pNS: PNameStru): String;
var     // 把 pNS 的內涵, 傳回成字串格式
   c: Char;
   pS: PChar;
   s: String;
begin
     case (pNS^.vType and 31) of
        // 0: // array
        1: s := Char(pNS^.i_Val and 255);       // char
        2: s := Format('%d ($%x)', [Byte(pNS^.i_Val and 255), pNS^.i_Val and 255]); // byte
        3: s := Format('%d ($%x)', [Word(pNS^.i_Val and 65535), pNS^.i_Val and 65535]); // word
        4: s := Format('%d ($%x)', [Cardinal(pNS^.i_Val), pNS^.i_Val]); // dword
        5: begin
                c := Char(pNS^.i_Val and 255);
                s := Format('%d ($%x)', [cvtShort(c), cvtShort(c)]); // sint8
           end;
        6: s := Format('%d ($%x)', [Short(pNS^.i_Val and 65535), pNS^.i_Val and 65535]); // sint16
        7: s := Format('%d ($%x)', [Integer(pNS^.i_Val), pNS^.i_Val]); // int
        8: // dblchar
            s := '未設計 dblchar';
        9: // unicode
            s := '未設計 unicode';
        10: begin  // string
                pS := PChar(pNS^.i_Val);
                if Integer(pS) > $100000 then begin
                   c := pS^;    Inc(pS);
                   pS[Byte(c)] := #0;   s := Char(34) + StrPas(pS) + Char(34);
                end
                else s := '<空字串>';
            end;
        11: // single
            s := '未設計 single';
        12: // double
            s := '未設計 double';
        13: s := Format('($%x) "%s"', [pNS^.i_Val, PChar(pNS^.i_Val)]); // pchar
        14: s := Format('Ptr $%x', [pNS^.i_Val]); // ptr
        15: s := Format('%d ($%x)', [Integer(pNS^.i_Val), pNS^.i_Val]); // integer
        16: s := Format('%ld ($%lx)', [pNS^.d_Val, pNS^.d_Val]);   //sint64
        17: // qword
            s := '未設計 qword';
        18: s := Format('Void $%x', [pNS^.i_Val]); // void
        19: // ansistring
            s := '未設計 ansistring';
     end;
     Result := s;
end;

procedure Msg(s: String);
begin
     BkgCtrl.Memo1.Lines.Add(s);
end;

procedure MsgLastAdd(p: Pointer);
begin
     with BkgCtrl.Memo1 do begin
        Lines[Lines.Count - 1] := Lines[Lines.Count - 1] + '  ' + StrPas(PChar(p));  
     end;
end;

function  status_updater(c_line_no, func_id, func_arg: Integer; func_ptr: Pointer): Integer;
var
   pNS: PNameStru;
   s: String;
   pOut: Array [0..31] of Char;
begin
     if Cardinal(func_ptr) < $100000 then begin
        s := Format('不良指標值: $%x', [Cardinal(func_ptr)]);
        StrPLCopy(pOut, s, 32);         func_ptr := @pOut[0];
     end;
     case (func_id shr 16) of
        (V_EXEC shr 16): // 執行中產生的訊息
            begin
                case (func_id and $FFFF) of
                    BEFORE_EXEC_LINE: // func_arg: bit_31-8=行號, bit_7-0=prog_id
                        begin
                             s := Format('即將執行第 %d 行 (%s) <%s>', [func_arg shr 8, VarsMgr.GetProgName(func_arg and 255), getFuncName(func_arg)]);
                             nowInFunc := PMyFunc(func_arg);
                             nowExecLine := func_arg shr 8;
                             Msg(s);
                             BkgCtrl.StatSay(1, s);
                        end;
                    DEBUG_STOP: // func_arg: bit_31-8=行號, bit_7-0=prog_id
                        begin
                             s := Format('程式暫停於第 %d 行 (%s) <%s>', [func_arg shr 8, VarsMgr.GetProgName(func_arg and 255), getFuncName(func_arg)]);
                             BkgCtrl.AnsMsg(s);
                             nowInFunc := PMyFunc(func_arg);
                             nowExecLine := func_arg shr 8;
                             fOnDebug := True; // debug stop !
                        end;
                    SHOW_INSTR: // func_arg: bit_31-8=行號, bit_7-0=prog_id
                        begin
                             MsgLastAdd(func_ptr);      // 把程式貼到上一行的後面 !
                        end;
                    SHOW_VAR_NS: // func_arg: index of 變數, func_ptr 通常是 struct TNameStru *
                        begin
                             pNS := PNameStru(func_ptr);
                             BkgCtrl.AnsMsg(Format('#%d: %s = %s', [c_line_no, pNS^.pName, get_val_str(func_ptr)]));
                        end;
                    FUNC_PRINTF: // 內建函數 printf() 輸出..
                        begin
                             BkgCtrl.AnsMsg(Format('第 %d 行, printf 輸出: "%s"', [func_arg shr 8, PChar(func_ptr)]));
                        end;
                    FUNC_RETURN: // 內建函數 return() 輸出..
                        begin
                             BkgCtrl.AnsMsg(Format('第 %d 行, return 結果: %s', [func_arg shr 8, PChar(func_ptr)]));
                        end;
                    WARN_OR_ERROR: // do_sys_error() 回報..
                        begin
                             if c_line_no < 1400 then begin // Warning messages -> func_ptr = msgID | (nowLineNo << 8)
                                BkgCtrl.AnsMsg(Format('第 %d 行 (%d), 警告: %s', [func_arg shr 8, Cardinal(func_ptr) shr 8, get_errMsg(Cardinal(func_ptr) and 255)]));
                             end
                             else begin // Error messages -> func_ptr = msgID | (info << 8)
                                BkgCtrl.AnsMsg(Format('第 %d 行 (extra info = %d), 錯誤: %s', [func_arg shr 8, Cardinal(func_ptr) shr 8, get_errMsg(Cardinal(func_ptr) and 255)]));
                             end;
                        end;
                end;
            end;
     end;
     Result := 0;
     if bUserStop then Result := $80;   // Stop program !
end;

end.



