unit AnalyzerDynamics;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ComCtrls;

type SearchEntryType=
             (
             setUnknown,{nierozpoznany}
             setLink,   {Po prostu: link lub plik PIF}
             setVarious,{mieszana zawarto��}
             setCategory,{kategoria}
             setMusic,   {muzyka}
             setPicture, {obrazki}
             setProgram, {programy oraz katalogi z programami}
             setAnimation,{animacje}
             setData,    {dane, dokumenty}
             setArchive  {Cos skompresowane}
             );
     ReplacePosition=
            (
            rpNone,{nie przeno�}
            rpMain,{przenie� do g��wnego, jako kategori�}
            rpCategory{przenie� do kategorii, jako wpis}
            );

type TAnalistResult=Record
       Name:String;  {jak wpis powinien si� nazywa�}
       EntryType:SearchEntryType;{czym jest}
       CategoryName:String; {jak powinien si� nazywa� kat. macierzysty}
       DescBefore:String;
       DescAfter:String;
       Probability:Integer; {Prawdopodob. dobrego okreslenia typu}
   end;

type PSearchRecList=^TSearchRecList;
     TSearchRecList=Record
       Next:PSearchRecList;
       Prev:PSearchRecList;
       Up:PSearchRecList;
       Down:PSearchRecList;
         {dwa wpisy przej�ciowe, do analizy}
       NameAnalist:TAnalistResult;
       BodyAnalist:TAnalistResult;
         {no i wpis ko�cowy, do pobierania w�asno�ci}
       RealProperties:TAnalistResult;
       AverageChildSize:Integer;
       Path:String;

       F: TSearchRec;
     end;

type TDynamicStruct=class(TPersistent)
  private
    { Private declarations }
    procedure CreateUnassignedSRec(var NewRec: PSearchRecList;Value: TSearchRec);{tworzy nowy element nie umieszczaj�c go w strukturze}
    procedure CutSRecSubTree(var Rec: PSearchRecList);{wywala element z drzewa nie zwalniaj�c go}
    procedure PasteSRecSubTreeAfter(var NewRec: PSearchRecList;RecBefore: PSearchRecList);{dodaje wcze�niej utworzony element do drzewa za podanym rekordem}
    procedure PasteSRecSubTreeBefore(var NewRec: PSearchRecList;RecAfter: PSearchRecList);{dodaje wcze�niej utworzony element do drzewa przed podanym rekordem}
  public
    { Public declarations }
    RootEntryList:PSearchRecList;{Pierwszy wpis tablicy plik�w}
    procedure MovetoFirstItem(var Rec: PSearchRecList);
    procedure MovetoLastItem(var Rec: PSearchRecList);
    procedure NewSRecAfter(var NewRec: PSearchRecList;Value: TSearchRec);
    procedure NewSRecBefore(var NewRec: PSearchRecList;Value: TSearchRec);
    procedure NewSRecDown(var UpRec: PSearchRecList;Value: TSearchRec);
    procedure DelSearchRec(var Rec: PSearchRecList);
    procedure DelLastSearchRec(var Rec: PSearchRecList);
    procedure DelAllSearchRec(var Rec: PSearchRecList);
    procedure ChangeItemPositionToAfter(Rec: PSearchRecList;RecBefore: PSearchRecList);
    procedure ChangeItemPositionToBefore(Rec: PSearchRecList;RecAfter: PSearchRecList);
    function Clear:Boolean;
    destructor Destroy; override;
     end;

implementation

procedure TDynamicStruct.CreateUnassignedSRec(var NewRec: PSearchRecList;Value: TSearchRec);
{tworzy nowy element nie umieszczaj�c go w strukturze
 UWAGA! Nie wpisuje nil�w w miejscach po��czenia ze struktur�.}
begin
  {utw�rz nowy element}
  New(NewRec);
  {wpisz warto�ci do nowego elementu}
  NewRec.F:=Value;
  NewRec.AverageChildSize:=0;
  NewRec.Path:='';
  NewRec.RealProperties.EntryType:=setUnknown;
  NewRec.RealProperties.CategoryName:='';
  NewRec.RealProperties.Name:='';
  NewRec.RealProperties.DescBefore:='';
  NewRec.RealProperties.DescAfter:='';
  NewRec.RealProperties.Probability:=0;
End;

procedure TDynamicStruct.CutSRecSubTree(var Rec: PSearchRecList);
{wywala element z drzewa nie zwalniaj�c go}
begin
  If (Rec=Nil) then Exit;
  {wywalam ze struktury tego samego poziomu}
  if (Rec.Prev<>Nil) then
    Rec.Prev.Next:=Rec.Next;
  if (Rec.Next<>Nil) then
    Rec.Next.Prev:=Rec.Prev;
  {a co je�li to element wskazuj�cy ten poziom
   z poziomu wy�szego ? }
  if (Rec.Up<>Nil) then if (Rec.Up.Down=Rec) then
      Begin
      {a wi�c to element wskazuj�cy. zmiana wskazania:}
        if (Rec.Prev<>Nil) then Rec.Up.Down:=Rec.Prev
          else Rec.Up.Down:=Rec.Next;
      End;
  {a mo�e to element kt�ry jest wej�ciem tej klasy na struktur�?}
  if (RootEntryList=Rec) then
    Begin
        {a wi�c jest. Zmiana elementu wej�ciowego:}
        if (Rec.Prev<>Nil) then RootEntryList:=Rec.Prev
          else RootEntryList:=Rec.Next;
    End;
  {element w pe�ni wywalony. Jeszcze na wszelki wypadek
   mo�na go wyczy�ci�}
  Rec.Prev:=Nil;
  Rec.Next:=Nil;
  Rec.Up:=Nil;
end;

procedure TDynamicStruct.PasteSRecSubTreeAfter(var NewRec: PSearchRecList;RecBefore: PSearchRecList);
{dodaje wcze�niej utworzony element do drzewa za podanym rekordem}
begin
  If (NewRec=Nil) then Exit; {Trudno zmieni� pozycj� niczego}
  If (RecBefore<>Nil) then
   Begin
    NewRec.Up:=RecBefore.Up;
    NewRec.Next:=RecBefore.Next;
    NewRec.Prev:=RecBefore;
    RecBefore.Next:=NewRec;
    if (NewRec.Next <> Nil) then
      NewRec.Next.Prev:=NewRec;
   End else Begin
    NewRec.Next:=Nil;
    NewRec.Prev:=Nil;
    NewRec.Up:=Nil;
    {Skoro element poprzedni jest Nil, to mo�e wstawiamy
     pierwszy element w strukturze. Upewnimy si� i ewentualnie
     ustawimy ten element}
    if (RootEntryList=nil) then RootEntryList:=NewRec;
  End;
end;

procedure TDynamicStruct.PasteSRecSubTreeBefore(var NewRec: PSearchRecList;RecAfter: PSearchRecList);
{dodaje wcze�niej utworzony element do drzewa przed podanym rekordem}
begin
  If (NewRec=Nil) then Exit; {Trudno zmieni� pozycj� niczego}
  If (RecAfter<>Nil) then
   Begin
    {zmieniamy w�asne wska�niki elementu}
    NewRec.Up:=RecAfter.Up;
    NewRec.Next:=RecAfter;
    NewRec.Prev:=RecAfter.Prev;
    {wska�niki element�w z lewej i prawej}
    RecAfter.Prev:=NewRec;
    if (NewRec.Prev <> Nil) then
      NewRec.Prev.Next:=NewRec;
    {je�eli poziom wy�ej(Up) wskazywa� na preva, mo�naby te� go zmieni�
     �eby zawsze wskazanie by�o na pierwszego childa}
    if (RecAfter.Up.Down=RecAfter) then RecAfter.Up.Down:=NewRec;
    {to samo tyczy si� wej�cia klasy na struktur�}
    if (RootEntryList=RecAfter) then RootEntryList:=NewRec;
   End else Begin
    NewRec.Next:=Nil;
    NewRec.Prev:=Nil;
    NewRec.Up:=Nil;
    {Skoro element nast�pny jest Nil, to mo�e wstawiamy
     pierwszy element w strukturze. Upewnimy si� i ewentualnie
     ustawimy ten element}
    if (RootEntryList=nil) then RootEntryList:=NewRec;
  End;
end;

procedure TDynamicStruct.NewSRecDown(var UpRec: PSearchRecList;Value: TSearchRec);
{wstawia element jako pierwszego childa, istniej�ce childy "przesuwa" dalej}
var NewRec: PSearchRecList;
Begin
  {Ustaw zmienne lokalne}
  if (UpRec<>Nil) then NewRec:=UpRec.Down
    else NewRec:=Nil;
  {dodaj element}
  NewSRecBefore(NewRec,Value);
  {wstaw go do struktury je�eli NewSRecBefore z tym se nie poradzi�
   (a nie poradzi� sobie je�eli UpRec lub UpRec.Down jest NIL)}
  if (UpRec<>Nil) then UpRec.Down:=NewRec;
  NewRec.Up:=UpRec;{na wszelki wypadek (pierwszy element)}
  {ustal wynik procedury}
  UpRec:=NewRec;
End;

procedure TDynamicStruct.MovetoFirstItem(var Rec: PSearchRecList);
begin
 If (Rec<>Nil) then
  while (Rec.Prev<>Nil) do Rec:=Rec.Prev;
end;

procedure TDynamicStruct.MovetoLastItem(var Rec: PSearchRecList);
begin
 If (Rec<>Nil) then
  while (Rec.Next<>Nil) do Rec:=Rec.Next;
end;

procedure TDynamicStruct.NewSRecAfter(var NewRec: PSearchRecList;Value: TSearchRec);
{Dodaje element do struktury po podanym i zmienia podanego na nowego}
var
   LastRec: PSearchRecList;
Begin
  {zapami�taj poprzedni element}
  LastRec:=NewRec;
  {utw�rz nowy element}
  CreateUnassignedSRec(NewRec,Value);
  {Dodaj element do struktury}
  NewRec.Down:=Nil; {jest nowy, wi�c nie ma child�w}
  PasteSRecSubTreeAfter(NewRec,LastRec);
End;

procedure TDynamicStruct.NewSRecBefore(var NewRec: PSearchRecList;Value: TSearchRec);
var
    NextRec: PSearchRecList;
Begin
  {Ustaw zmienne lokalne}
  NextRec:=NewRec;
  {utw�rz nowy element}
  CreateUnassignedSRec(NewRec,Value);
  {Dodaj element do struktury}
  NewRec.Down:=Nil; {jest nowy, wi�c nie ma child�w}
  PasteSRecSubTreeBefore(NewRec,NextRec);
End;

procedure TDynamicStruct.DelSearchRec(var Rec: PSearchRecList);
var GivenRec: PSearchRecList;
Begin
  If (Rec=Nil) then Exit;
  {przygotuj warto�� Rec po jego usuni�ciu}
  If (Rec.Prev<>Nil) then GivenRec:=Rec.Prev
    else GivenRec:=Rec.Next;
  {wywal Rec ze struktury drzewiastej}
  CutSRecSubTree(Rec);
  {usu� elementy podrz�dne}
  If (Rec.Down<>Nil) then DelAllSearchRec(Rec.Down);
  {usu� element}
  Dispose(Rec);
  {ustaw warto�� zwrotu}
  Rec:=GivenRec;
End;

procedure TDynamicStruct.DelLastSearchRec(var Rec: PSearchRecList);
var GivenRec: PSearchRecList;
Begin
  If (Rec=Nil) then Exit;
  {przygotuj warto�� Rec po jego usuni�ciu}
  If (Rec.Next<>Nil) then GivenRec:=Rec else GivenRec:=Rec.Prev;
  {przejd� do ostatniego elementu}
  MoveToLastItem(Rec);
  {usu� element}
  DelSearchRec(Rec);
  {przywr�� warto�� Rec}
  Rec:=GivenRec;
End;

procedure TDynamicStruct.DelAllSearchRec(var Rec: PSearchRecList);
{Usuwa wszystkie elementy z poziomu, kt�rego wska�nik dosta�o}
var CurRec: PSearchRecList;
begin
  CurRec:=Rec;
  MoveToFirstItem(CurRec);{to na wydadek gdyby nie dosta� pierwszego elementu}
  while (CurRec<>Nil) do DelLastSearchRec(CurRec);
  Rec:=CurRec;
end;

procedure TDynamicStruct.ChangeItemPositionToAfter(Rec: PSearchRecList;RecBefore: PSearchRecList);
{ przenosi element struktury w nowe miejsce }
Begin
  If (Rec=Nil) then Exit;
  {"odetnij" ze starego miejsca}
  CutSRecSubTree(Rec);
  {wstaw w nowe}
  PasteSRecSubTreeAfter(Rec,RecBefore);
End;

procedure TDynamicStruct.ChangeItemPositionToBefore(Rec: PSearchRecList;RecAfter: PSearchRecList);
{ przenosi element struktury w nowe miejsce }
Begin
  If (Rec=Nil) then Exit;
  {"odetnij" ze starego miejsca}
  CutSRecSubTree(Rec);
  {wstaw w nowe}
  PasteSRecSubTreeBefore(Rec,RecAfter);
End;

function TDynamicStruct.Clear:Boolean;
Begin
 try
  DelAllSearchRec(RootEntryList);
  RootEntryList:=Nil;
  Result:=True;
 except
  Result:=False;
 end;
end;

destructor TDynamicStruct.Destroy;
Begin
 try
  Clear;
 finally
  inherited Destroy;
 end;
end;


end.
