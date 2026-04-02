unit Q_SORT;

interface

uses
  SysUtils, iDebug;

type
  TIntegerFunction = function(pA, pB: Pointer): Integer;
  PInteger = ^Integer;

function  binFind(pDta, pToFind: Pointer; ordOfs, recSize, nItems: Integer): Integer;
        { pDta 為一個已排序好的陣列, pToFind 指到要找的數值或字串, 做二元搜尋 }
function  btGoFirst(pDta, pToFind: Pointer; ofs, nSize, loc: Integer): Integer;
        { 倒退到第一筆符合處, bug: 若 loc >= 總項數 則會當機 ! }
function  btGoLast(pDta, pToFind: Pointer; ofs, nSize, nItems, loc: Integer): Integer;
	{ 前進到最後一筆符合處 }
function  fcmppi(pA, pB: Pointer): Integer;     { 整數指標內容比較 }
function  FindLoc(head, tail: Integer): Integer;
procedure SetSortFCMP(func: TIntegerFunction);
function  table_check(pDta: Pointer; ordOfs, recSize, nItems: Integer): Integer;	// 表格若是良好則傳回 -1, 否則傳回第一個錯誤位置
procedure QSortBy(pDta: Pointer; ordOfs, recSize, nItems: Integer);
        { pDta 為一個未排序的陣列, ordOfs 是要排序的主鍵偏移位置,
            recSize 是一筆資料的長度, nItems 是整個陣列包含的資料筆數 }
procedure Sorting;
        // 找出 Array[head] 此元素的適當位置, 並分割左右

var
  fcmp: function(pA, pB: Pointer): Integer;     { 比較函式 }
  maxStk: Integer;              // 計算實際使用的堆疊空間
  MaxN: Integer;                // 排序索引, 指到 pData + ordOfs 內
  Rounds: Integer;              // 計算共呼叫幾次分割
  totalCmps: Integer;           // 計算共比較多少次數值
  totalSwaps: Integer;          // 計算共互換多少次數值
  pData: PChar = nil;           // 放待排序的整數

implementation

function  binFind(pDta, pToFind: Pointer; ordOfs, recSize, nItems: Integer): Integer;
var
   l, m, r, v: Integer;
   pMid: Pointer;
begin   { pDta 為一個已排序好的陣列, pToFind 指到要找的數值或字串, 做二元搜尋 }
     if nItems < 1 then begin  Result := 0;  Exit;  end;        { 無資料 }
     l := 0;    r := nItems - 1;        m := 0;
     while (l <= r) do begin
          m := (l + r) shr 1;   { 取中間值 }
          pMid := Pointer(Integer(pDta) + (m * recSize) + ordOfs);
          v := fcmp(pToFind, pMid);
          if v = 0 then Break;  { 找到了 }
          if v < 0 then begin   { 目標應在小側 }
                if m = r then Break
                else r := m;
          end
          else begin    { 目標應在大側 }
                if m = l then begin
                    if v > 0 then Inc (m);
                    Break;      end
                else l := m;
          end;
     end;
     Result := m;
end;

function  btGoFirst(pDta, pToFind: Pointer; ofs, nSize, loc: Integer): Integer;
var
   lastmatch, v: Integer;
begin   { 倒退到第一筆符合處 }
     Inc(PChar(pDta), ofs + (nSize * loc));	lastmatch := -1;
//     v := fcmp(pToFind, pDta);		if v = 0 then lastmatch := loc;		// 第一筆即是相符
     while loc > 0 do begin
        v := fcmp(pToFind, pDta);
        if v > 0 then begin    { 倒退過頭了, 下一筆才是 }
            Result := loc + 1;
            Exit;
        end;
	// 若是小於或等於表格值, 則須再看前一個數值
	if lastmatch <> loc then lastmatch := loc;	// 記住最新的相符位置
        Dec(PChar(pDta), nSize);        Dec(loc);
     end;
     Result := 0;	if lastmatch > 0 then Result := lastmatch;
end;

function  btGoLast(pDta, pToFind: Pointer; ofs, nSize, nItems, loc: Integer): Integer;
var
   v: Integer;
begin   { 前進到最後一筆符合處 }
     Inc(PChar(pDta), ofs + (nSize * loc));
     while loc < nItems do begin
        v := fcmp(pToFind, pDta);
        if v < 0 then begin    { 前進過頭了, 前一筆才是 }
// xSay('前進過頭 ' + IntToStr(loc));
            Result := loc - 1;
            Exit;
        end;
	// 若是大於或等於表格值, 則須再看前一個數值
        Inc(PChar(pDta), nSize);        Inc(loc);
     end;
     Result := nItems - 1;
end;

function  fcmppi(pA, pB: Pointer): Integer;
var
   a, b, r: Integer;
begin   { 整數指標內容比較 }
     a := PInteger(pA)^;        b := PInteger(pB)^;     r := 1;
     if a = b then r := 0;
     if a < b then r := -1;
     Result := r;
end;

function FindLoc(head, tail: Integer): Integer;
var             // 找出 Array[head] 此元素的適當位置, 並分割左右
   loc, left, right: Integer;
   nVal, pL, pR, pN: ^Pointer;
   good: Boolean;
label chk_loop;
begin
     loc := head;       left := head;   right := tail;
     pL := nil;         pR := nil;
chk_loop:
     good := False;
     pN := Pointer(pData + (loc shl 2));        // 指到現在檢查值的位址
     nVal := pN^;       { 現在檢查值的位址 }
     // 找出右方比現在檢查值小者
     while loc < right do
          begin
               pR := Pointer(pData + (right shl 2));
               Inc(totalCmps);
               if fcmp(pN^, pR^) <= 0 then Dec(right)
               else Break;      // 找到右方的較小值
          end;
     Result := right;
     if loc = right then Exit;  // 現在檢查值的右方都比較大
     if fcmp(pN^, pR^) > 0 then
        begin
             pN^ := pR^;        // swap (*loc, *right)
             pR^ := nVal;
             loc := right;
             Inc(totalSwaps);
        end;

     // 找出左方比現在檢查值大者
     while loc > left do
          begin
               pL := Pointer(pData + (left shl 2));
               Inc(totalCmps);
               if fcmp(pN^, pL^) >= 0 then Inc(left)
               else Break;      // 找到左方的較大值
          end;
     Result := left;
     if loc = left then Exit;   // 現在檢查值的左方都比較小
     if fcmp(pN^, pL^) < 0 then
        begin
             pN := Pointer(pData + (loc shl 2));
             pN^ := pL^;        // swap (*loc, *left)
             pL^ := nVal;
             loc := left;
             Inc(totalSwaps);
             good := True;
        end;
     if good then goto chk_loop;
     Result := loc;
end;

procedure SetSortFCMP(func: TIntegerFunction);
begin
     fcmp := func;
end;

procedure QSortBy(pDta: Pointer; ordOfs, recSize, nItems: Integer);
var             // 準備投入 Quick Sort
   i, j: Integer;
   pI: ^Integer;
   pc, pt: PChar;
begin
     if nItems < 1 then Exit;           { 無資料 }
     if not Assigned(fcmp) then Exit;	{ nil PTR }
     MaxN := nItems;
     pc := AllocMem(MaxN shl 2);        // 借恰好空間, 放排序索引表
     for i := 0 to MaxN-1 do
         begin
              pI := Pointer(pc + (i shl 2));
              pI^ := Integer(pDta) + (recSize * i) + ordOfs;
         end;
     pData := pc;       { pData[] 為要被排序的目標位址陣列 }
     Sorting;   // 建立堆疊並開始排序
     { 依序寫回資料 }
     pt := AllocMem(recSize * nItems);
     for i := 0 to MaxN-1 do
         begin
              pI := Pointer(pc + (i shl 2));    j := pI^;   { 取出排序索引 }
              pI := Pointer(j - ordOfs);
              Move(pI^, pt[i * recSize], recSize);      { 搬到新位置 }
         end;
     Move(pt^, pDta^, recSize * nItems);        { 拷貝回到來源 }
     FreeMem(pt);       FreeMem(pc);
end;

procedure Sorting;
var
   bgn, fin, loc, stktop: Integer;
   stk: Array of Integer;
begin
     SetLength(stk, 200);       { 4 百萬個數值排序約須 32-36 個堆疊空間 }
     totalCmps :=0;     totalSwaps := 0;
     maxStk := 0;       Rounds := 0;
     stktop := 0;       // 空堆疊
     stk[stktop] := 0;
     stk[stktop+1] := MaxN - 1;
     stktop := stktop + 2;      // 設立第一個初值
     while stktop > 0 do
           begin
                if maxStk < stktop then maxStk := stktop;
                // 取出堆疊頂值
                Dec(stktop);    fin := stk[stktop];
                Dec(stktop);    bgn := stk[stktop];
                loc := FindLoc(bgn, fin);
                // 如果 loc 左方有 2 個以上的數值, 則送入堆疊
                if bgn < (loc - 1) then
                   begin
                        stk[stktop] := bgn;
                        stk[stktop+1] := loc - 1;
                        stktop := stktop + 2;
                   end;
                // 如果 loc 右方有 2 個以上的數值, 則送入堆疊
                if fin > (loc + 1) then
                   begin
                        stk[stktop] := loc + 1;
                        stk[stktop+1] := fin;
                        stktop := stktop + 2;
                   end;
                Inc(Rounds);    // 計算共呼叫幾次分割
           end;

{     xSay(Format('共呼叫了 %d 次分割函式', [Rounds]));
     xSay(Format('共比較了 %d 個表格數值', [totalCmps]));
     xSay(Format('共交換了 %d 對表格數值', [totalSwaps]));
     xSay(Format('此次用了 %d 個堆疊空間', [maxStk shr 1]));	}
end;

function  table_check(pDta: Pointer; ordOfs, recSize, nItems: Integer): Integer;
var
   pI: ^Integer;
   i, thisV: Integer;
begin	// 檢查表格的 第 ordOfs 處的 Integer 是否由小到大正確排列, 若是則傳回 -1, 否則傳回第一個錯誤位置
     Result := -1;	if (pDta = nil) or (nItems < 2) then Exit;	// 無表格可比較 !
     pI := Pointer(PChar(pDta) + ordOfs);	// 指到第一值.
     thisV := pI^;	i := 0;		Dec(nItems);
     while i < nItems do begin
	pI := Pointer(PChar(pI) + recSize);
	if thisV > pI^ then begin  Result := i;  Exit;  end;	// 找到錯誤了 !
	Inc(i);
     end;
end;

end.
