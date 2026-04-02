unit FuncMngr;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  iDebug, ExtCtrls, Buttons, Grids, StdCtrls;   // CommDef,

type
  TFuncMgr = class(TForm)
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Panel1: TPanel;
    Panel2: TPanel;
    Panel3: TPanel;
    Panel4: TPanel;
    Panel5: TPanel;
    Panel6: TPanel;
    Panel7: TPanel;
    SpdBtn1: TSpeedButton;
    SpdBtn2: TSpeedButton;
    Splitter1: TSplitter;
    Splitter2: TSplitter;
    Splitter3: TSplitter;
    Splitter4: TSplitter;
    Splitter5: TSplitter;
    StrGrid1: TStringGrid;
    StrGrid2: TStringGrid;
    StrGrid3: TStringGrid;
    StrGrid4: TStringGrid;
    StrGrid5: TStringGrid;
    procedure do_moved_splt1(Sender: TObject);
    procedure do_moved_splt4(Sender: TObject);
    procedure do_moved_splt5(Sender: TObject);
    procedure Toggle_LeftLowerBlocksize(Sender: TObject);
    procedure ToResize(Sender: TObject);
    procedure Toggle_LeftUpperBlocksize(Sender: TObject);
  private
    procedure adj_RowCounts;
  public
    procedure init;
  end;

const
  width_3: Array [0..5] of Integer = (32, 216, 60, 80, 58, 126);
  caption_3: Array [0..5] of String = ('No.', 'Name', '參數個數', '返回值型態', '首行號', 'file');
  width_4: Array [0..4] of Integer = (32, 232, 60, 80, 126);
  caption_4: Array [0..4] of String = ('No.', 'Name', '已解決', '行號', 'file');
  width_5: Array [0..4] of Integer = (32, 232, 80, 80, 126);
  caption_5: Array [0..4] of String = ('No.', '來源 (函數或程序)', '編號(切出區塊)', '行號', 'file');

var
  FuncMgr: TFuncMgr;

implementation

{$R *.DFM}

procedure TFuncMgr.adj_RowCounts;
begin
     StrGrid1.RowCount := StrGrid1.Height div 21;
     StrGrid2.RowCount := StrGrid2.Height div 21;
     StrGrid3.RowCount := StrGrid3.Height div 21;
     StrGrid4.RowCount := StrGrid4.Height div 21;
     StrGrid5.RowCount := StrGrid5.Height div 21;
end;

procedure TFuncMgr.do_moved_splt1(Sender: TObject);
begin
     adj_RowCounts;
end;

procedure TFuncMgr.do_moved_splt4(Sender: TObject);
begin
     StrGrid1.Height := Splitter4.Top - 20;
     adj_RowCounts
end;

procedure TFuncMgr.do_moved_splt5(Sender: TObject);
begin
     StrGrid3.Height := Splitter5.Top - 20;
     adj_RowCounts
end;

procedure TFuncMgr.init;
var
   i: Integer;
begin
     for i := 0 to 5 do begin
         StrGrid1.ColWidths[i] := width_3[i];
         StrGrid1.Cells[i, 0] := caption_3[i];
         StrGrid2.ColWidths[i] := width_3[i];
         StrGrid2.Cells[i, 0] := caption_3[i];
     end;
     for i := 0 to 4 do begin
         StrGrid3.ColWidths[i] := width_4[i];
         StrGrid3.Cells[i, 0] := caption_4[i];
     end;
     for i := 0 to 4 do begin
         StrGrid4.ColWidths[i] := width_5[i];
         StrGrid4.Cells[i, 0] := caption_5[i];
     end;
     for i := 0 to 4 do begin
         StrGrid5.ColWidths[i] := width_5[i];
         StrGrid5.Cells[i, 0] := caption_5[i];
     end;
     StrGrid2.Cells[3, 0] := '程序位址';
     adj_RowCounts;     Show;
end;

procedure TFuncMgr.Toggle_LeftLowerBlocksize(Sender: TObject);
begin
     if Panel4.Width > 100 then Panel4.Width := 16
     else Panel4.Width := 544;
     if StrGrid3.Height < 20 then begin
        StrGrid3.Height := 120;
        Splitter5.Top := StrGrid3.Height + 20;
     end;
end;

procedure TFuncMgr.Toggle_LeftUpperBlocksize(Sender: TObject);
begin
     if Panel3.Width > 100 then Panel3.Width := 60
     else Panel3.Width := 640;
     if StrGrid1.Height < 20 then begin
        StrGrid1.Height := 120;
        Splitter4.Top := StrGrid1.Height + 20;
     end;
end;

procedure TFuncMgr.ToResize(Sender: TObject);
begin
     adj_RowCounts;
end;

end.
