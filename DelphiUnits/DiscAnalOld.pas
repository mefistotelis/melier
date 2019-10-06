unit DiscAnalOld;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Dialogs,
  ComCtrls, Tabnotbk, StdCtrls, ExtCtrls, FileCtrl, Registry, Buttons,
  Grids, FXProcsPas, SynchronizerUnitPas;

type PTreeSpis=^TTreeSpis;
     TTreeSpis=Record
      Size:LongInt;
      Num:LongInt;
      Name:String;
      Text:String;
      Items:PTreeSpis;
      Up:PTreeSpis;
      Next:PTreeSpis;
     End;

type
  TDiscAnalOld = class(TObject)
  private
    { Private declarations }
  public
    { Public declarations }
    Synchronizer:TSynchronizerPas;
    procedure CreateListOld(List:TStrings);
    constructor Create(nSynchronizer:TSynchronizerPas);
    destructor Destroy;override;
  end;


implementation

constructor TDiscAnalOld.Create(nSynchronizer:TSynchronizerPas);
begin
  inherited Create();
  Synchronizer:=nSynchronizer;
End;

destructor TDiscAnalOld.Destroy;
Begin
  inherited Destroy;
End;

procedure TDiscAnalOld.CreateListOld(List:TStrings);
var TreeSpis,CurPos:PTreeSpis;

 Function IsCategory(Nazwa:String):BooLean;
 var KatCounter:Word;
 Begin
  IsCategory:=False;
  With Synchronizer do
  For katCounter:=0 to (Katbox.ControlCount-1) do
   If AnsiUpperCase(Nazwa)=AnsiUpperCase(TSpeedButton(KatBox.Controls[KatCounter]).Caption) then
    IsCategory:=True;
 End;

 Function GetFPath:String;
 var s:String;
     c:PTreeSpis;
 Begin
  c:=CurPos;
  s:='';
  If c<>nil then
  while c.Up<>nil do begin
   s:='\'+c.Name+s;
   C:=c.Up;
  End;
  GetFPath:=s;
 End;

 procedure InsertItem(name:string;size:LongInt);
 var Added:PTreeSpis;
 Begin
  New(Added);
  Added.Name:=Name;
  Added.Text:='';
  Added.Size:=size;
  Added.Num:=-1;
  Added.Items:=Nil;
  Added.Next:=CurPos.Items;
  CurPos.Items:=Added;
  Added.Up:=CurPos;
 End;

 procedure AddItem(name:String);
 var ext:String;
     FileStream:TFileStream;
     Size:LongInt;
 Begin
  ext:=ANSIUpperCase(copy(name,max(length(name)-3,1),4));
  FileStream:=TFileStream.Create(Synchronizer.DrvSelector.Text[1]+':'+GetFPath+'\'+name,fmOpenRead+fmShareCompat);
  size:=FileStream.Size;
  FileStream.Free;
  If (ext='.EXE')or(ext='.RAR')or(ext='.ARJ')or(ext='.ZIP')or(ext='.ACE')or(ext='.MP3')or(ext='.MPG')or(ext='.AVI') then Begin
   InsertItem(Name,size);
  End else Begin
   Inc(CurPos.Size,Size);
   Inc(Curpos.Num);
  End;
 End;

 procedure AddCategory(name:String);
 var Added:PTreeSpis;
 Begin
  New(Added);
  Added.Name:=Name;
  Added.Text:='';
  Added.Size:=0;
  Added.Num:=0;
  Added.Items:=Nil;
  If CurPos=Nil then Begin
   TreeSpis:=Added;
   CurPos:=Added;
   Added.Up:=Nil;
   Added.Next:=Nil;
  End else Begin
   Added.Next:=CurPos.Items;
   CurPos.Items:=Added;
   Added.Up:=CurPos;
   Curpos:=Added;
  End;
 End;
 procedure GoOut;
 Begin
  If CurPos<>Nil then CurPos:=CurPos.Up;
 End;

 procedure AnalyzeEntry(Dir,Path:String;Depth:Integer);
 var Name,NewPath:String;
     Files:TFileListBox;
     DirNr:LongInt;
 Begin
  Name:=copy(Dir,2,length(dir)-2);
  Files:=TFileListBox.Create(Synchronizer.MainForm);
  try
   {tworzenie listy elementów danego katalogu}
   DirNr:=0;
   while Synchronizer.MainForm.FindComponent('FileList'+IntToStr(DirNr))<>nil do inc(DirNr);
   try
     Files.Name:='FileList'+IntToStr(DirNr);
     Files.Visible:=False;
     Synchronizer.MainForm.InsertControl(Files);
     Files.Drive:=Synchronizer.DrvSelector.Text[1];
     Files.FileType:=[ftDirectory];
     Files.Directory:=Path;
   except
     Messagebox(Synchronizer.MainForm.Handle,'Wyst¹pi³ b³¹d podczas wypisywania zawartoœci katalogu. B³¹d odczytu lub komponentu listujcego. Nast¹pi próba zamaskowania.','Spierdoli³o siê. Uwa¿aj.',MB_OK or MB_ICONERROR);
     If Synchronizer.MainForm.ContainsControl(Files) then Synchronizer.MainForm.RemoveControl(Files);
   End;
   {lista gotowa. Teraz tworzê listê logiczn¹.}
   AddCategory(Name);
   If Files.Items.Count>0 then
    For DirNr:=0 to (Files.Items.Count-1) do
     If Files.Items[DirNr][2]<>'.' then begin
      {jeœli trafiono na katalog - wywo³anie tej procedury dla niego}
      NewPath:=Files.Items[DirNr];
      AnalyzeEntry(NewPath,path+copy(NewPath,2,length(NewPath)-2)+'\',depth+1);
     end;
   Files.FileType:=[ftNormal];
   If Files.Items.Count>0 then
    For DirNr:=0 to (Files.Items.Count-1) do
     AddItem(Files.Items[DirNr]);
  Finally
   GoOut;
   If Synchronizer.MainForm.ContainsControl(Files) then Synchronizer.MainForm.RemoveControl(Files);
   Files.Free;
  End;
 End;

 procedure CutEntries(now:PTreeSpis);
 var Current,Nast:PTreeSpis;
 Begin
  if now<>nil then Current:=now.Items else current:=nil;
  While current<>nil do with current^ do Begin
   If Items<>nil then CutEntries(current);
   Nast:=Next;
   inc(now.Size,size);
   Inc(now.Num,num);
   If text<>'' then text:=','+Text;
   If now.Text='' then now.text:=name+text else now.Text:=name+text+','+now.text;
   now.Items:=Next;
   Dispose(current);
   Current:=Nast;
  End;
 End;
 procedure FormatMp3(now:PTreeSpis);
 var Current:PTreeSpis;
     Path:String;
 Begin
  Current:=Now.up;Path:='';
  While current<>nil do Begin
   Path:=current.Name+'\'+path;
   current:=Current.up;
  End;
  With now^ do Begin
   If (name[1] in DigitsPas)and(name[2] in DigitsPas) then Begin
    If (name[3] in symbolsPas)and(name[4] in symbolsPas) then
     delete(name,1,4);
   End;
   delete(name,max(length(name)-3,1),4);
  End;
 while pos('_',now.name)>0 do now.Name[pos('_',now.name)]:=' ';
 End;
 procedure FormatEntries(now:PTreeSpis;level:LongInt);
 var Current,Nast:PTreeSpis;
     Ext,SubStr,Rozm:String;
 Begin
  Current:=now;
  If current<>nil then If (current.name='.')and(current.Up=nil)and(current.next=nil) then Begin
   If TreeSpis=Current then TreeSpis:=Current.Items;
   Nast:=Current.Items;
   While nast<>nil do Begin
    nast.Up:=Nil;
    Nast:=nast.Next;
   End;
   Dispose(current);Current:=TreeSpis;
  End;
  while Current<>Nil do with Current^ do begin
   Nast:=Next;
   If IsCategory(Name)and(Num>=0) then Begin
     Text:=name+':';
     FormatEntries(Items,level+1);
   end else Begin
    If up<>nil then if IsCategory(up.Name) then
     If num<0 then Begin
      num:=0;
      delete(name,max(length(name)-3,1),4);
      End;
    If Num>=0 then Begin
      If level>3 then CutEntries(Current) else
        FormatEntries(Items,level+1);
      Rozm:=FloatToStr(Size/1048576);
      Rozm:=copy(Rozm,1,3);If rozm[3]=',' then delete(rozm,3,1);
      While pos(',',rozm)>0 do rozm[pos(',',rozm)]:='.';
      If num>1 then SubStr:=IntToStr(num)+','+Rozm+'MB'
        else SubStr:=Rozm+'MB';
      If text<>'' then SubStr:='('+SubStr+')';
      Text:=name+'('+Text+SubStr+')';
    End else Begin
     ext:=ANSIUpperCase(copy(name,max(length(name)-3,1),4));
     If ext='.MP3' then FormatMp3(current) else delete(name,max(length(name)-3,1),4);
     If up<>Nil then Begin
      If up.text='' then Up.Text:=name else Up.Text:=name+','+Up.text;
      Inc(up.Size,size);Inc(up.Num);
      Up.Items:=Next;
      Dispose(current);
     End;
    End;
   End;
   current:=Nast;
  End;
 End;

 procedure ShowEntries(now:PTreeSpis);
 var Prev,Current:PTreeSpis;
 Begin
   Current:=now;
   while Current<>Nil do Try
     if IsCategory(Current.Name)and(List.Count>0) then
      List.Add('');
     List.Add(Current.text);
   Finally
     ShowEntries(Current.Items);
     Prev:=Current;
     Current:=Current.Next;
     Dispose(Prev);
   End;
 End;
Begin
 CurPos:=Nil;
 Try
   AnalyzeEntry('[.]','\',0);
   FormatEntries(Treespis,1);
 Finally
   List.Clear;
   ShowEntries(Treespis);
 End;
End;

end.
