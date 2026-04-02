unit C_wrap;

interface

uses
  Windows, Messages, SysUtils, Classes, iDebug, MapWork;

  procedure add_watch(hash, addr: Integer; name: PChar);  // hash = high_16_data_len + mid_8_ID + low_8_type_flag
  procedure del_watch(id_type: Integer);	// id_type = mid_8_ID + low_8_type_flag
  procedure close_blob(blob_id: Integer);      // 釋放記憶體並關閉 FILE *
  procedure copy_temp_str(ptr: Pointer);  stdcall; // ? 此時無法用 _fastcall
  procedure dbp(n1, n2: Integer; sInfo: PChar);    // 顯示可被中斷除錯的訊息
  // dbp -> 可在 Delphi 側, 在此設立中斷點, 方便跳入 C 程式除錯
  function  get_blob_data(blob_id: Integer; item_ndx: Integer): Pointer;    // 頭 4 bytes = data len, 用完請自行釋放該記憶體 !
  function  get_info_array(blob_id: Integer): Pointer;   // 傳回 struct blob_array * answer[], 用完請自行釋放該記憶體 !
  function  open_blob_file(fn: PChar; mode: Integer; item_cnt: Integer): Integer;   // mode: 1=唯讀, 2=讀寫, 傳回 1-31 = OK. (if > 31, 請減 32 = DOS Error code, 小於零是內部錯誤碼)
  procedure set_temp_str(ndx: Integer; str: PChar); stdcall;

  procedure xChangeFileExt(fn, ext: PChar);
  function  xDeleteFile(fname: PChar): Boolean;
  procedure xfclose(handle: Integer);
  function  xfcreate(fname: PChar): Integer;
  function  xFileExists(fname: PChar): Integer;
  function  xfopen(fname: PChar; mode: Integer): Integer;
  function  xfread(buf: Pointer; len, count, handle: Integer): Integer;
  procedure xfree(p: Pointer);
  procedure xf_say_free(adr: Cardinal);
  procedure xf_say_malloc(adr: Cardinal);
  function  xfseek(handle, ofs, orig: Integer): Integer;
  function  xfwrite(buf: Pointer; len, count, handle: Integer): Integer;
  procedure xGetRandomFileName(fname: PChar);
  function  xGetTodayDate: Integer;	// 取得今天日期 (yy: 23 bits, mm: 4 bits, dd: 5 bits)
  function  xmalloc(size: Integer): Pointer;
  procedure xmemmove(dest, src: Pointer; len: Integer);
  procedure xmemset(pDst: Pointer; chr, len: Integer);
  function  xRenameFile(oldName: PChar; newName: PChar): Boolean;
  function  xstrcat(strDest, strSrc: PChar): PChar;
  function  xstrcmp(str1, str2: PChar): Integer;
  function  xstricomp(str1, str2: PChar): Integer;
  function  xstrcopy(strDest, strSrc: PChar): PChar;	// 速度很慢哦 !
  function  xstrncpy(dest, src: PChar; max_len: Integer): PChar;
  function  xstrPLcopy(dest: PChar; src: String; max_len: Integer): PChar;
  function  xstrlen(str: PChar): Integer;
  function  xstrupr(str: PChar): PChar;
  function  xvsprintf(buf: PChar; fmt: PChar; ap: va_list): Integer;
  procedure xxErr(str: PChar);

const
  ID_xstrcopy_src = 1;
  ID_xstrcopy_dest = 2;
  ID_xstrncpy_src = 3;
  ID_xstrncpy_dest = 4;
  ID_xstrPLcopy = 5;
  func_str: array [1..5] of String = ('xStrCopy_SRC', 'xStrCopy_DEST', 'xStrNCopy_SRC', 'xStrNCopy_DEST', 'xStrPLCopy');
  str_0d: array [0..1] of Char = (#10, #0);

var
  in_addr, in_ptr: Cardinal;
//  temp_str: Array [0..127] of PChar; 

implementation

{$L su_blob.obj}
  procedure close_blob(blob_id: Integer);  external;
  procedure copy_temp_str(ptr: Pointer);  stdcall;  external;
  function  get_blob_data(blob_id: Integer; item_ndx: Integer): Pointer; external;   // 頭 4 bytes = data len, 用完請自行釋放該記憶體 !
  function  get_info_array(blob_id: Integer): Pointer; external;  // 傳回 struct blob_array * answer[], 用完請自行釋放該記憶體 !
  function  open_blob_file(fn: PChar; mode: Integer; item_cnt: Integer): Integer; external;  // mode: 1=唯讀, 2=讀寫, 傳回 1-31 = OK. (if > 31, 請減 32 = DOS Error code, 小於零是內部錯誤碼)
  procedure set_temp_str(ndx: Integer; str: PChar); stdcall; external;

procedure add_watch(hash, addr: Integer; name: PChar);
begin   // hash = high_16_data_len + mid_8_ID + low_8_type_flag
     xSendCmd(Format('WA %d,%d,%d,%s', [hash and $FFFF, addr, hash shr 16, name]));
end;

procedure del_watch(id_type: Integer);
begin   // id_type = mid_8_ID + low_8_type_flag
     xSendCmd(Format('WD %d', [id_type]));
end;

procedure dbp(n1, n2: Integer; sInfo: PChar);
var
   pX: PChar;
begin   // 顯示可被中斷除錯的訊息
// dbp -> 可在 Delphi 側, 在此設立中斷點, 方便跳入 C 程式除錯
        xSay(Format('%d (%x) %s', [n1, n2, sInfo]));
        if n2 < $480000 then Exit;
        pX := PChar(n2);
        xHexDump(pX, 16);
end;

procedure xChangeFileExt(fn, ext: PChar);
var
   p: PChar;
begin
     if (fn = nil) or (ext = nil) then Exit;     // ext 必須包含 '.' 開頭 !
     p := StrRScan(fn, '.');    // 逆向找到 '.'
     if p <> nil then p^ := #0; // 把 '.' 改成字串尾端
     StrLCat(fn, ext, 255);     // 從尾端貼上 ext 延伸檔名
end;

function  xDeleteFile(fname: PChar): Boolean;
begin
     Result := DeleteFile(StrPas(fname));
end;

procedure xfclose(handle: Integer);
begin
     FileClose(handle);
end;

function  xfcreate(fname: PChar): Integer;
begin
     Result := FileCreate(StrPas(fname));
end;

function  xFileExists(fname: PChar): Integer;
begin
     if fname = nil then begin  Result := 0;  Exit;  end;       // 空檔名: 不存在
     Result := Integer(FileExists(StrPas(fname)));
end;

function  xfopen(fname: PChar; mode: Integer): Integer;
begin
     Result := FileOpen(StrPas(fname), mode);
end;

function  xfread(buf: Pointer; len, count, handle: Integer): Integer;
begin
//     Form1.Say('fread: hnd, len, cnt = $' + IntToHex(handle, 6) + ' $' + IntToHex(len, 6) + ' $' + IntToHex(count, 4));
     Result := FileRead(handle, buf^, len * count);
end;

procedure show_err(addr, ofs, v1, v2, v3: Integer);
var
   ret_addr: Integer;
begin
     ret_addr := Addr_BeforeN(ofs + $34C) - 5;
     xSay('出錯: ' + AnalyzeCaller(addr) + ', 數值: $' + IntToHex(v1, 8) + ',$' + IntToHex(v2, 8) + ',$' + IntToHex(v3, 8));
     xSay('    呼叫者: $' + IntToHex(ret_addr, 8) + ' = ' + AnalyzeCaller(ret_addr));
end;

procedure  xf_say_free(adr: Cardinal);
var
   s: string;
begin
     Exit;
     s := AnalyzeCaller(adr);
     xSay(Format('xfree($%x): 呼叫源於 $%x, %s', [in_ptr, adr, s]));
end;

procedure xf_say_malloc(adr: Cardinal);
var
   s: string;
begin
     Exit;
     s := AnalyzeCaller(adr);
     xSay(Format('xmalloc($%x): 呼叫源於 $%x, %s', [in_ptr, adr, s]));
end;

procedure  xfree(p: Pointer);
begin
     asm
        mov   in_ptr, eax
        mov   eax, [ebp + 4]
        sub   eax, 5
        mov   in_addr, eax
        mov   eax, in_ptr
     end;
     xf_say_free(in_addr);
     if Cardinal(p) < $10000 then begin
        // xSay('Error at xfree : p = $' + IntToHex(Integer(p), 8));
        show_err(Addr_Here, 8, Integer(p), 0, 0);
        Exit;
     end
     else FreeMem(p);
end;

function  xfseek(handle, ofs, orig: Integer): Integer;
begin
//     Form1.Say('fseek: hnd, ofs, orig = $' + IntToHex(handle, 6) + ' $' + IntToHex(ofs, 6) + ' $' + IntToHex(orig, 2));
     Result := FileSeek(handle, ofs, orig);     // return new location
end;

function  xfwrite(buf: Pointer; len, count, handle: Integer): Integer;
begin
//     Form1.Say('fwrite: hnd, len, cnt = $' + IntToHex(handle, 6) + ' $' + IntToHex(len, 6) + ' $' + IntToHex(count, 4));
     Result := FileWrite(handle, buf^, len * count);
end;

procedure xGetRandomFileName(fname: PChar);
var
   i: Integer;
   hh, mm, msec, ss: Word;
   s: String;
begin	// 使用今天日期與時間產生亂數檔名, fname 須有至少 16 bytes 空間
     s := DateToStr(now);	s := Copy(s, 3, Length(s) - 2);
     DecodeTime(now, hh, mm, ss, msec);
     i := Pos('/', s);		s[i] := Char((hh and 15) + 65);
     i := Pos('/', s);		mm := mm and 31;
     if mm > 25 then Dec(mm, 24);	s[i] := Char(mm + 65);
     mm := ss and 15;		ss := ss shr 4;
     s := s + Char(65 + ss) + Char(65 + mm);	xSay(s);
     StrPLCopy(fname, s, 15);
end;

function  xGetTodayDate: Integer;
var
   yy, mm, dd: Word;
begin	// 取得今天日期 (yy: 23 bits, mm: 4 bits, dd: 5 bits)
     DecodeDate(now, yy, mm, dd);
     Result := (yy shl 9) or (mm shl 5) or (dd and 31);
end;

function  xmalloc(size: Integer): Pointer;
begin
     asm
        mov   in_ptr, eax
        mov   eax, [ebp + 4]
        sub   eax, 5
        mov   in_addr, eax
        mov   eax, in_ptr
     end;
     xf_say_malloc(in_addr);
     Result := AllocMem(size);
end;

procedure xmemmove(dest, src: Pointer; len: Integer);
begin
//     Form1.Say('Move: src->dst, len= $' + IntToHex(Integer(src), 8) + ' $' + IntToHex(Integer(dest), 8) + ' $' + IntToHex(len, 6));
     Move(src^, dest^, len);
end;

procedure xmemset(pDst: Pointer; chr, len: Integer);
begin
     FillChar(pDst^, len, chr);
end;

function  xRenameFile(oldName: PChar; newName: PChar): Boolean;
begin
     Result := RenameFile(StrPas(oldName), StrPas(newName));
end;

function  xstrcat(strDest, strSrc: PChar): PChar;
begin
     Result := StrCat(strDest, strSrc);
end;

function  xstrcmp(str1, str2: PChar): Integer;
begin
     if ((Cardinal(str1) < $10000) or (Cardinal(str2) < $10000)) then begin
        show_err(Addr_Here, 8, Integer(str1), Integer(str2), 0);
        Result := 1;    Exit;
     end;
     Result := StrComp(str1, str2);
end;

function  xstrcopy(strDest, strSrc: PChar): PChar;
begin
     Result := strDest;
//     check_ptr(strSrc, ID_xstrcopy_src);
//     check_ptr(strDest, ID_xstrcopy_dest);
     if strDest = nil then xSay('Bad xStrCopy ! null ptr');
//     if (Cardinal(strDest) < $8000) or (Cardinal(strDest) > $2000000) then xLog('xStrCopy $', Cardinal(strDest), 4, 0, nil);
     StrCopy(strDest, strSrc);
end;

function  xstricomp(str1, str2: PChar): Integer;
begin
     Result := StrIComp(str1, str2);
end;

function  xstrncpy(dest, src: PChar; max_len: Integer): PChar;
begin	// 目前僅 su_dbf.c 在用
//     check_ptr(src, ID_xstrncpy_src);
//     check_ptr(dest, ID_xstrncpy_dest);
     Result := StrLCopy(dest, src, max_len);
end;

function  xstrPLcopy(dest: PChar; src: String; max_len: Integer): PChar;
begin
     // check_ptr(dest, ID_xstrPLcopy);
     Result := StrPLCopy(dest, src, max_len);
end;

function  xstrlen(str: PChar): Integer;
begin
     Result := StrLen(str);
end;

function  xstrupr(str: PChar): PChar;
begin
     Result := StrUpper(str);
end;

function xvsprintf(buf: PChar; fmt: PChar; ap: va_list): Integer;
begin
     Result := wvsprintf(buf, fmt, ap);
end;

procedure xxErr(str: PChar);
begin
     xSay(StrPas(str));
end;

end.


