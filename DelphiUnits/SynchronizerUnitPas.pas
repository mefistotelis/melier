unit SynchronizerUnitPas;

interface
uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  ComCtrls, Tabnotbk, StdCtrls, ExtCtrls, FileCtrl, Registry, Buttons,
  Grids, Spin,
  FXProcsPas, ExportDlg, AnalyzerDynamics;

{$ObjExportAll On}

const ProgramRegistryPath='Software\FX Corp\Spis CD';
      NameDigits=4;
      MaxCategories=1024;
      BorrowFileName='\Borrow.txt';
      DescriptFile='\Readme.txt';
      VersionFile='\History.txt';
      SpisFolder='\Spis';
      MusicStr='MUSIC';
      ProgramStr='PROGRAM';
      PictureStr='PICTURE';
      AnimationStr='ANIMATION';
      DataStr='DATA';
      VariousStr='VARIOUS';
      ArchiveStr='ARCHIVE';
      PozyczDefaultText='<wybierz osobÍ>';
      CRLF=#13#10;

type TBorrVisMode=(gmHistory,gmElement,gmPerson,gmUnreturned,gmLost);
     TNeededUpdate=(nuFull,nuLastLine,nuNone);

type
  TSynchronizerPas = class(TThread)
  private
    { Private declarations }
  public
    { Public declarations }
           {elementy ogÛlne}
    SpisPath:String; {úcieøka do plikÛw programu}
    SpisSubPath:String;
    StatusBar:TStatusPanel;{TPanel}
    ProgressBar: TProgressBar;
    SmallFont: TFontDialog;
    NormFont: TFontDialog;
    CPWinButton: TRadioButton;
    CpDosButton: TRadioButton;
    ScaleComponentsCheckBox: TCheckBox;
    ScaleFactor: TEdit;

    KatBox: TGroupBox;
    MainForm:TForm;
         { Ciπgi nazw bliskoznacznych (do analizy katalogÛw na podstawie nazwy) }
    NameOfMusic:TStringList;
    NameOfPicture:TStringList;
    NameOfProgram:TStringList;
    NameOfAnimation:TStringList;
    NameOfData:TStringList;
    NameOfVarious:TStringList;
    NameOfArchive:TStringList;
         { Ciπgi rozszerzeÒ bliskoznacznych (do analizy plikÛw na podstawie nazwy) }
    ExtOfMusic:TStringList;
    ExtOfPicture:TStringList;
    ExtOfProgram:TStringList;
    ExtOfAnimation:TStringList;
    ExtOfData:TStringList;
    ExtOfVarious:TStringList;
    ExtOfArchive:TStringList;
           {elementy zakladki ZNAJDè}
    SpisCD:TListBox;
    SearchViewer:TRichEdit;
    ItemViewer:TRichEdit;
    ShowSearchResultsBtn:TSpeedButton;
    LimitLow:TEdit;
    LimitHigh:TEdit;
    Typer: TEdit;
    TyperAllRadioBtn: TRadioButton;
    TyperCommentsCheckBox: TCheckBox;
           {opcje ZNAJDè}
    ViewerShowNum: TCheckBox;
    ViewerOpRemove: TCheckBox;
    ViewerNumAtEnd: TCheckBox;
    ViewerWordWraper: TCheckBox;
    ViewerBufSize: TSpinEdit;
           {elementy ZMIE—}
    Editor: TRichEdit;
    //Editor: TMemo;
    EditorWordWraper: TCheckBox;
    DrvSelector: TComboBox;
           {opcje ANALIZUJ}
    AnalyzerEnBody: TCheckBox;
           {elementy POØYCZ}
    SpisStringGrid: TStringGrid;
    PPoComboBox: TComboBox;
    PVEEdit: TComboBox;
           { pomocnicze do formatowania tekstu }
    UpdStart: LongInt;
    UpdLength: LongInt;
    UpdEditor: TRichEdit;
    UpdFont: TFontDialog;

           { tworzenie kategorii }
    procedure KatCreationAnalyseKategorie(var KatName:String;var ContentType:String;lnstart:longint;lnend:longint);
    procedure NewKat(num:word;capt:string);
         { Funkcje do obslugi ciπgÛw nazw i rozszerzeÒ }
    function AddKatToContentList(ContentType:String;KatName:String):boolean;
    function AddSingleExtToExtensionList(ExtStr:String;ContentType:String):boolean;
           {konwersja matryc znakow}
//    Function WinCPConvertLine(s:String):String;
//    Function DosCPConvertLine(s:String):String;
//    procedure ConvertStrList2W(List:TStrings);
//    procedure ConvertStrList2D(List:TStrings);
           {funkcje ladowania opcji}
    procedure LoadDefaultOpt;
    procedure LoadOptions;
    procedure SaveOptions;
    procedure RemoveSavedOptions;
           {dodatkowe funkcje}
    procedure ScaleComponents(ScaleFactor:integer);
    Function ContainsText(Text,CmpWith:String):BooLean;
    procedure UpdateREditFormat;
           {konstruktory i de-}
    constructor Create(nMainForm:TForm);
    destructor Destroy;override;
    procedure Execute;override;
  end;

 function GetContentTypeFromString(ContentType:String):SearchEntryType;

implementation

function GetContentTypeFromString(ContentType:String):SearchEntryType;
var ConTypeFixed:String;
begin
  ConTypeFixed:=AnsiUpperCase(GetParam(1,ContentType));
  if (ConTypeFixed=MusicStr) then Result:=setMusic else
  if (ConTypeFixed=ProgramStr) then Result:=setProgram else
  if (ConTypeFixed=PictureStr) then Result:=setPicture else
  if (ConTypeFixed=AnimationStr) then Result:=setAnimation else
  if (ConTypeFixed=DataStr) then Result:=setData else
  if (ConTypeFixed=VariousStr) then Result:=setVarious else
  if (ConTypeFixed=ArchiveStr) then Result:=setArchive else
  Result:=setUnknown;
end;

constructor TSynchronizerPas.Create(nMainForm:TForm);
var
  StatusBarFull:TStatusBar;
Begin
  inherited Create(true);
  GetDir(0,SpisPath); { 0 = Current drive }
  SpisSubPath:=SpisPath+SpisFolder+'\';
 {elementy ogÛlne}
  MainForm:=nMainForm;
  StatusBarFull:=(MainForm.FindComponent('StatusBarFull')) as TStatusBar;
  if (StatusBarFull<>nil) then
      StatusBar:=(StatusBarFull.Panels[0]);
{  StatusBar:=(MainForm.FindComponent('StatusBar')) as TPanel;}
  ProgressBar:=(MainForm.FindComponent('ProgressBar')) as TProgressBar;

  SmallFont:=(MainForm.FindComponent('SmallFont')) as TFontDialog;
  NormFont:=(MainForm.FindComponent('NormFont')) as TFontDialog;
  CpDosButton:=(MainForm.FindComponent('CpDosButton')) as TRadioButton;
  CPWinButton:=(MainForm.FindComponent('CPWinButton')) as TRadioButton;
  ScaleComponentsCheckBox:=(MainForm.FindComponent('ScaleComponentsCheckBox')) as TCheckBox;
  ScaleFactor:=(MainForm.FindComponent('ScaleFactor')) as TEdit;
  {Searcher}
  SpisCD:=TListBox(MainForm.FindComponent('SpisCD'));
  SearchViewer:=TRichEdit(MainForm.FindComponent('SearchViewer'));
  ItemViewer:=TRichEdit(MainForm.FindComponent('ItemViewer'));
  ShowSearchResultsBtn:=(MainForm.FindComponent('ShowSearchResultsBtn')) as TSpeedButton;
  LimitLow:=TEdit(MainForm.FindComponent('LimitLow'));
  LimitHigh:=TEdit(MainForm.FindComponent('LimitHigh'));
  KatBox:=(MainForm.FindComponent('KatBox')) as TGroupBox;
  Typer:=TEdit(MainForm.FindComponent('Typer'));
  TyperAllRadioBtn:=TRadioButton(MainForm.FindComponent('TyperAllRadioBtn'));
  TyperCommentsCheckBox:=(MainForm.FindComponent('TyperCommentsCheckBox')) as TCheckBox;
  {searcher opcje}
  ViewerWordWraper:=(MainForm.FindComponent('ViewerWordWraper')) as TCheckBox;
  ViewerShowNum:=TCheckBox(MainForm.FindComponent('ViewerShowNum'));
  ViewerOpRemove:=TCheckBox(MainForm.FindComponent('ViewerOpRemove'));
  ViewerNumAtEnd:=(MainForm.FindComponent('ViewerNumAtEnd')) as TCheckBox;
  ViewerBufSize:=(MainForm.FindComponent('ViewerBufSize')) as TSpinEdit;
  {analizuj}
  AnalyzerEnBody:=(MainForm.FindComponent('AnalyzerEnBody')) as TCheckBox;
  {zmieÒ}
  Editor:=MainForm.FindComponent('Editor') as TRichEdit;
  EditorWordWraper:=TCheckBox(MainForm.FindComponent('EditorWordWraper'));
  DrvSelector:=TComboBox(MainForm.FindComponent('DrvSelector'));
  {poøycz}
  SpisStringGrid:=TStringGrid(MainForm.FindComponent('SpisStringGrid'));
  PPoComboBox:=(MainForm.FindComponent('PPoComboBox')) as TComboBox;
  PVEEdit:=(MainForm.FindComponent('PVEEdit')) as TComboBox;
  {zmiana czcionki Richedit}
  UpdEditor:=nil;
  UpdFont:=TFontDialog.Create(MainForm);
End;


destructor TSynchronizerPas.Destroy;
Begin
  inherited Destroy;
End;

Function TSynchronizerPas.ContainsText(Text,CmpWith:String):BooLean;
var Counter:Word;
Begin
 If (length(text)>0)and(length(CmpWith)<=length(text)) then Begin
  For counter:=0 to(length(text)-length(CmpWith)) do
   if copy(text,counter,length(CmpWith))=CmpWith then Begin
    Result:=true;
    exit;
   End;
 End;
  Result:=false;
End;

procedure TSynchronizerPas.UpdateREditFormat;
Begin
 With UpdEditor do
  Begin
  SelStart:=UpdStart;
  SelLength:=UpdLength;
  SelAttributes.Name:=UpdFont.Font.Name;
  SelAttributes.Size:=UpdFont.Font.Size;
  SelAttributes.Color:=UpdFont.Font.Color;
  End;
End;

function TSynchronizerPas.AddSingleExtToExtensionList(ExtStr:String;ContentType:String):boolean;
{
  Dodaje rozszerzenie pliku do list znanych rozszerzeÒ
}
var CType:SearchEntryType;
begin
  CType:=GetContentTypeFromString(ContentType);
  case (CType) of
    setMusic:    ExtOfMusic.Add(ExtStr);
    setProgram:  ExtOfProgram.Add(ExtStr);
    setPicture:  ExtOfPicture.Add(ExtStr);
    setAnimation:ExtOfAnimation.Add(ExtStr);
    setData:     ExtOfData.Add(ExtStr);
    setVarious:  ExtOfVarious.Add(ExtStr);
    setArchive:  ExtOfArchive.Add(ExtStr);
    else         ExtOfVarious.Add(ExtStr);
  end;
  Result:=true;
end;

function TSynchronizerPas.AddKatToContentList(ContentType:String;KatName:String):boolean;
{
  Dodaje nazwe kategorii do odpowiedniej grupy
}
var CType:SearchEntryType;
begin
  CType:=GetContentTypeFromString(ContentType);
  case (CType) of
    setMusic:    NameOfMusic.Add(KatName);
    setProgram:  NameOfProgram.Add(KatName);
    setPicture:  NameOfPicture.Add(KatName);
    setAnimation:NameOfAnimation.Add(KatName);
    setData:     NameOfData.Add(KatName);
    setVarious:  NameOfVarious.Add(KatName);
    setArchive:  NameOfArchive.Add(KatName);
    else         NameOfVarious.Add(KatName);
  end;
  Result:=true;
end;

procedure TSynchronizerPas.NewKat(num:word;capt:string);
var CBox:TSpeedButton;
Begin
  CBox:=TSpeedButton.Create(KatBox);
  CBox.Left:=4;
  CBox.Width:=KatBox.Width-2*CBox.Left;
  CBox.GroupIndex:=500+num;
  CBox.AllowAllUp:=True;
  CBox.Flat:=True;
  CBox.Visible:=True;
  CBox.Down:=False;
  Cbox.Name:='KatCB'+IntToStr(num);
  CBox.Caption:=capt;
  KatBox.InsertControl(CBox);
End;

 procedure TSynchronizerPas.KatCreationAnalyseKategorie(var KatName:String;var ContentType:String;lnstart:longint;lnend:longint);
 var KatNum:Word;
 Begin
  with ItemViewer.Lines do
   for KatNum:=1 to (lnend-lnstart) do
    if (strings[lnstart+KatNum]<>'') then
       begin
       if (pos('=',strings[lnstart+KatNum])>0) then
         begin
         KatName:=GetBefore('=',strings[lnstart+KatNum]);
         ContentType:=GetAfter('=',strings[lnstart+KatNum]);
         end
        else
         begin
         KatName:=strings[lnstart+KatNum];
         ContentType:=VariousStr;
         end;
       NewKat(KatNum,KatName);
       AddKatToContentList(ContentType,KatName);
       end;
 End;

procedure TSynchronizerPas.LoadDefaultOpt;
Begin
 CPDosButton.Checked:=True;
 SmallFont.Font.Name:='MS Serif';
 SmallFont.Font.Size:=6;
 NormFont.Font.Name:='MS Sans Serif';
 NormFont.Font.Size:=8;
End;

procedure TSynchronizerPas.LoadOptions;
var Reg:TRegistry;
Begin
 Try
   Reg:=TRegistry.Create;
   If Reg.OpenKey(ProgramRegistryPath,False) then
    with Reg do Begin
     LoadDefaultOpt;
     If ValueExists('SmallFontName')  then SmallFont.Font.Name:=ReadString('SmallFontName');
     If ValueExists('SmallFontSize')  then SmallFont.Font.Size:=ReadInteger('SmallFontSize');
     If ValueExists('SmallFontColor') then SmallFont.Font.Color:=ReadInteger('SmallFontColor');
     If ValueExists('NormFontName')   then NormFont.Font.Name:=ReadString('NormFontName');
     If ValueExists('NormFontSize')   then NormFont.Font.Size:=ReadInteger('NormFontSize');
     If ValueExists('NormFontColor')  then NormFont.Font.Color:=ReadInteger('NormFontColor');
     If ValueExists('ViewerWordWrap') then SearchViewer.WordWrap:=ReadBool('ViewerWordWrap');
     If ValueExists('ViewerShowNum')  then ViewerShowNum.Checked:=ReadBool('ViewerShowNum');
     If ValueExists('ViewerNumAtEnd') then ViewerNumAtEnd.Checked:=ReadBool('ViewerNumAtEnd');
     If ValueExists('AnalyzerEnBody') then AnalyzerEnBody.Checked:=ReadBool('AnalyzerEnBody');
     If ValueExists('ViewerOpRemove') then ViewerOpRemove.Checked:=ReadBool('ViewerOpRemove');
     If ValueExists('ViewerBufSize') then ViewerBufSize.Value:=ReadInteger('ViewerBufSize');
     If ValueExists('EditorWordWrap') then Editor.WordWrap:=ReadBool('EditorWordWrap');
     If ValueExists('ScaleComponents') then ScaleComponentsCheckBox.Checked:=ReadBool('ScaleComponents');
     If ValueExists('ScaleFactor')    then ScaleFactor.Text:=IntToStr(ReadInteger('ScaleFactor'));
     If ValueExists('MainWidth')      then MainForm.Width:=ReadInteger('MainWidth');
     If ValueExists('MainHeight')     then MainForm.Height:=ReadInteger('MainHeight');
     If ValueExists('MainTop')        then MainForm.Top:=ReadInteger('MainTop');
     If ValueExists('MainLeft')       then MainForm.Left:=ReadInteger('MainLeft');
     If ValueExists('UseDOSCodePage') then Begin
      If ReadBool('UseDOSCodePage') then CPDosButton.Checked:=True else CPWinButton.Checked:=True;
      End;
    End else LoadDefaultOpt;
   ViewerWordWraper.Checked:=SearchViewer.WordWrap;
   ItemViewer.WordWrap:=SearchViewer.WordWrap;
   EditorWordWraper.Checked:=Editor.WordWrap;
   Reg.Free;
 Except
  LoadDefaultOpt;
  Messagebox(MainForm.Handle,'Wystπpi≥ powaøny b≥πd podczas prÛby ≥adowania ustawieÒ z rejestru. Coú nie pracuje poprawnie.','Jezus Maria !!!',MB_OK or MB_ICONERROR);
 End;
End;

procedure TSynchronizerPas.SaveOptions;
var Reg:TRegistry;
Begin
 Try
   Reg:=TRegistry.Create;
   If Reg.OpenKey(ProgramRegistryPath,True) then
    with Reg do Begin
     WriteString('SmallFontName',SmallFont.Font.Name);
     WriteInteger('SmallFontSize',SmallFont.Font.Size);
     WriteInteger('SmallFontColor',SmallFont.Font.Color);
     WriteString('NormFontName',NormFont.Font.Name);
     WriteInteger('NormFontSize',NormFont.Font.Size);
     WriteInteger('NormFontColor',NormFont.Font.Color);
     WriteBool('ViewerWordWrap',SearchViewer.WordWrap);
     WriteBool('ViewerShowNum',ViewerShowNum.Checked);
     WriteBool('ViewerNumAtEnd',ViewerNumAtEnd.Checked);
     WriteBool('AnalyzerEnBody',AnalyzerEnBody.Checked);
     WriteBool('ViewerOpRemove',ViewerOpRemove.Checked);
     WriteInteger('ViewerBufSize',ViewerBufSize.Value);
     WriteBool('EditorWordWrap',Editor.WordWrap);
     WriteBool('ScaleComponents',ScaleComponentsCheckBox.Checked);
     WriteInteger('ScaleFactor',StrToIntDef(ScaleFactor.Text,100));
     WriteInteger('MainWidth',MainForm.Width);
     WriteInteger('MainHeight',MainForm.Height);
     WriteInteger('MainTop',MainForm.Top);
     WriteInteger('MainLeft',MainForm.Left);
     WriteBool('UseDOSCodePage',CPDosButton.Checked);
    End else Messagebox(MainForm.Handle,'Nie mogÍ uzyskaÊ dostÍpu do rejestru. Coú namiesza≥eú.','Pindol jesteú...',MB_OK or MB_ICONERROR);
   Reg.Free;
 Except
  Messagebox(MainForm.Handle,'Wystπpi≥ powaøny b≥πd podczas prÛby ≥adowania ustawieÒ z rejestru. Coú nie pracuje poprawnie.','O Kurwa MaÊ !!!',MB_OK or MB_ICONERROR);
 End;
End;

procedure TSynchronizerPas.RemoveSavedOptions;
var Reg:TRegistry;
Begin
 Try
   Reg:=TRegistry.Create;
   If not Reg.DeleteKey(ProgramRegistryPath) then
    Application.Messagebox('Nie mogÍ usunπÊ klucza rejestru: HKEY_CURRENT_USER\'+ProgramRegistryPath,'Kupa. Sam kombinuj.',0);
   Reg.Free;
 Except
  Application.Messagebox('Wystπpi≥ powaøny b≥πd podczas prÛby ≥adowania ustawieÒ z rejestru. Coú nie pracuje poprawnie.','Jezus Maria !!!',0);
 End;
End;

procedure TSynchronizerPas.ScaleComponents(ScaleFactor:integer);
  procedure ResizeForm(cmpnt:TForm);
    begin
    cmpnt.Font.Size:=(cmpnt.Font.Size*ScaleFactor)div 100;
    end;
  procedure ResizeButton(cmpnt:TButton);
    begin
    cmpnt.SetBounds((cmpnt.Left*ScaleFactor)div 100,(cmpnt.Top*ScaleFactor)div 100,(cmpnt.Width*ScaleFactor)div 100,(cmpnt.Height*ScaleFactor)div 100);
    if (cmpnt.Name='FExportBtn')or(cmpnt.Name='FShowAllBtn')or
       (cmpnt.Name='EdFindButton')or(cmpnt.Name='EdReplaceButton')or
       (cmpnt.Name='EdCaseButton')or(cmpnt.Name='Button2')then
      cmpnt.Font.Size:=(cmpnt.Font.Size*ScaleFactor)div 100;
    end;
  procedure ResizePanel(cmpnt:TPanel);
    begin
    cmpnt.SetBounds(Round((cmpnt.Left*ScaleFactor)/100),(cmpnt.Top*ScaleFactor)div 100,(cmpnt.Width*ScaleFactor)div 100,(cmpnt.Height*ScaleFactor)div 100);
    end;
  procedure ResizeGroupBox(cmpnt:TGroupBox);
    begin
    cmpnt.SetBounds(Round((cmpnt.Left*ScaleFactor)/100),(cmpnt.Top*ScaleFactor)div 100,(cmpnt.Width*ScaleFactor)div 100,(cmpnt.Height*ScaleFactor)div 100);
    end;
  procedure ResizePageControl(cmpnt:TPageControl);
    begin
    cmpnt.SetBounds((cmpnt.Left*ScaleFactor)div 100,(cmpnt.Top*ScaleFactor)div 100,(cmpnt.Width*ScaleFactor)div 100,(cmpnt.Height*ScaleFactor)div 100);
    end;
  procedure ResizeCheckBox(cmpnt:TCheckBox);
    begin
    cmpnt.SetBounds((cmpnt.Left*ScaleFactor)div 100,(cmpnt.Top*ScaleFactor)div 100,(cmpnt.Width*ScaleFactor)div 100,(cmpnt.Height*ScaleFactor)div 100);
    if (cmpnt.Name='TyperCommentsCheckBox')then
      cmpnt.Font.Size:=(cmpnt.Font.Size*ScaleFactor)div 100;
    end;
  procedure ResizeRadioButton(cmpnt:TRadioButton);
    begin
    cmpnt.SetBounds((cmpnt.Left*ScaleFactor)div 100,(cmpnt.Top*ScaleFactor)div 100,(cmpnt.Width*ScaleFactor)div 100,(cmpnt.Height*ScaleFactor)div 100);
    if (cmpnt.Name='TyperAllRadioBtn')or(cmpnt.Name='TyperAnyRadioBtn')then
      cmpnt.Font.Size:=(cmpnt.Font.Size*ScaleFactor)div 100;
    end;
  procedure ResizeLabel(cmpnt:TLabel);
    begin
    cmpnt.SetBounds((cmpnt.Left*ScaleFactor)div 100,(cmpnt.Top*ScaleFactor)div 100,(cmpnt.Width*ScaleFactor)div 100,(cmpnt.Height*ScaleFactor)div 100);
    if (cmpnt.Name='TyperOptionsLabel')then
      cmpnt.Font.Size:=(cmpnt.Font.Size*ScaleFactor)div 100;
    end;
  procedure ResizeEdit(cmpnt:TEdit);
    begin
    cmpnt.SetBounds((cmpnt.Left*ScaleFactor)div 100,(cmpnt.Top*ScaleFactor)div 100,(cmpnt.Width*ScaleFactor)div 100,(cmpnt.Height*ScaleFactor)div 100);
    end;
  procedure ResizeMemo(cmpnt:TMemo);
    begin
    cmpnt.SetBounds((cmpnt.Left*ScaleFactor)div 100,(cmpnt.Top*ScaleFactor)div 100,(cmpnt.Width*ScaleFactor)div 100,(cmpnt.Height*ScaleFactor)div 100);
    end;
  procedure ResizeRichEdit(cmpnt:TRichEdit);
    begin
    cmpnt.SetBounds((cmpnt.Left*ScaleFactor)div 100,(cmpnt.Top*ScaleFactor)div 100,(cmpnt.Width*ScaleFactor)div 100,(cmpnt.Height*ScaleFactor)div 100);
    end;
  procedure ResizeStringGrid(cmpnt:TStringGrid);
    begin
    cmpnt.SetBounds((cmpnt.Left*ScaleFactor)div 100,(cmpnt.Top*ScaleFactor)div 100,(cmpnt.Width*ScaleFactor)div 100,(cmpnt.Height*ScaleFactor)div 100);
    cmpnt.DefaultColWidth:=(cmpnt.DefaultColWidth*ScaleFactor)div 100;
    cmpnt.DefaultRowHeight:=(cmpnt.DefaultRowHeight*ScaleFactor)div 100;
    end;
  procedure ResizeControl(cmpnt:TControl);
    begin
    cmpnt.SetBounds((cmpnt.Left*ScaleFactor)div 100,(cmpnt.Top*ScaleFactor)div 100,(cmpnt.Width*ScaleFactor)div 100,(cmpnt.Height*ScaleFactor)div 100);
    end;
{ Mnoøy rozmiary wszystkich komponentÛw przez podany wspÛlczynnik }
var i:Integer;
Begin
  MainForm.DisableAutoRange;
  ResizeForm(MainForm);
  for i:=0 to MainForm.ComponentCount-1 do
   if (MainForm.Components[i]<>nil) then
    begin
    if (MainForm.Components[i] is TPageControl) then ResizePageControl(MainForm.Components[i] as TPageControl) else
    if (MainForm.Components[i] is TButton) then ResizeButton(MainForm.Components[i] as TButton) else
    if (MainForm.Components[i] is TPanel) then ResizePanel(MainForm.Components[i] as TPanel) else
    if (MainForm.Components[i] is TGroupBox) then ResizeGroupBox(MainForm.Components[i] as TGroupBox) else
    if (MainForm.Components[i] is TCheckBox) then ResizeCheckBox(MainForm.Components[i] as TCheckBox) else
    if (MainForm.Components[i] is TRadioButton) then ResizeRadioButton(MainForm.Components[i] as TRadioButton) else
    if (MainForm.Components[i] is TLabel) then ResizeLabel(MainForm.Components[i] as TLabel) else
    if (MainForm.Components[i] is TEdit) then ResizeEdit(MainForm.Components[i] as TEdit) else
    if (MainForm.Components[i] is TMemo) then ResizeMemo(MainForm.Components[i] as TMemo) else
    if (MainForm.Components[i] is TRichEdit) then ResizeRichEdit(MainForm.Components[i] as TRichEdit) else
    if (MainForm.Components[i] is TStringGrid) then ResizeStringGrid(MainForm.Components[i] as TStringGrid) else
    if (MainForm.Components[i] is TControl) then ResizeControl(MainForm.Components[i] as TControl);
    end;
  MainForm.EnableAutoRange;
End;

procedure TSynchronizerPas.Execute;
{ G≥Ûwna funkcja wπtku }
Begin
End;

end.
