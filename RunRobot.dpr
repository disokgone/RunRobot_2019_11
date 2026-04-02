program RunRobot;

uses
  Forms,
  bkCtrl in 'bkCtrl.pas' {BkgCtrl},
  FuncMngr in 'FuncMngr.pas' {FuncMgr},
  VarsMngr in 'VarsMngr.pas' {VarsMgr},
  CommDef in 'CommDef.pas',
  C_Runner in 'C_Runner.pas',
  C_wrap in 'C_wrap.pas',
  MapWork in 'MapWork.pas';

{$R *.RES}

begin
  Application.Initialize;
  Application.CreateForm(TBkgCtrl, BkgCtrl);
  Application.CreateForm(TFuncMgr, FuncMgr);
  Application.CreateForm(TVarsMgr, VarsMgr);
  Application.Run;
end.
