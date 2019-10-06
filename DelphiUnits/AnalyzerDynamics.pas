unit AnalyzerDynamics;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ComCtrls;

type SearchEntryType=
             (
             setUnknown,{nierozpoznany}
             setLink,   {Po prostu: link lub plik PIF}
             setVarious,{mieszana zawartoœæ}
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
            rpNone,{nie przenoœ}
            rpMain,{przenieœ do g³ównego, jako kategoriê}
            rpCategory{przenieœ do kategorii, jako wpis}
            );

type TAnalistResult=Record
       Name:String;  {jak wpis powinien siê nazywaæ}
       EntryType:SearchEntryType;{czym jest}
       CategoryName:String; {jak powinien siê nazywaæ kat. macierzysty}
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
         {dwa wpisy przejœciowe, do analizy}
       NameAnalist:TAnalistResult;
       BodyAnalist:TAnalistResult;
         {no i wpis koñcowy, do pobierania w³asnoœci}
       RealProperties:TAnalistResult;
       AverageChildSize:Integer;
       Path:String;

       F: TSearchRec;
     end;

type TDynamicStruct=class(TPersistent)
  private
    { Private declarations }
    procedure CreateUnassignedSRec(var NewRec: PSearchRecList;Value: TSearchRec);{tworzy nowy element nie umieszczaj¹c go w strukturze}
    procedure CutSRecSubTree(var Rec: PSearchRecList);{wywala element z drzewa nie zwalniaj¹c go}
    procedure PasteSRecSubTreeAfter(var NewRec: PSearchRecList;RecBefore: PSearchRecList);{dodaje wczeœniej utworzony element do drzewa za podanym rekordem}
    procedure PasteSRecSubTreeBefore(var NewRec: PSearchRecList;RecAfter: PSearchRecList);{dodaje wczeœniej utworzony element do drzewa przed podanym rekordem}
  public
    { Public declarations }
    RootEntryList:PSearchRecList;{Pierwszy wpis tablicy plików}
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
{tworzy nowy element nie umieszczaj¹c go w strukturze
 UWAGA! Nie wpisuje nilów w miejscach po³¹czenia ze struktur¹.}
begin
  {utwórz nowy element}
  New(NewRec);
  {wpisz wartoœci do nowego elementu}
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
{wywala element z drzewa nie zwalniaj¹c go}
begin
  If (Rec=Nil) then Exit;
  {wywalam ze struktury tego samego poziomu}
  if (Rec.Prev<>Nil) then
    Rec.Prev.Next:=Rec.Next;
  if (Rec.Next<>Nil) then
    Rec.Next.Prev:=Rec.Prev;
  {a co jeœli to element wskazuj¹cy ten poziom
   z poziomu wy¿szego ? }
  if (Rec.Up<>Nil) then if (Rec.Up.Down=Rec) then
      Begin
      {a wiêc to element wskazuj¹cy. zmiana wskazania:}
        if (Rec.Prev<>Nil) then Rec.Up.Down:=Rec.Prev
          else Rec.Up.Down:=Rec.Next;
      End;
  {a mo¿e to element który jest wejœciem tej klasy na strukturê?}
  if (RootEntryList=Rec) then
    Begin
        {a wiêc jest. Zmiana elementu wejœciowego:}
        if (Rec.Prev<>Nil) then RootEntryList:=Rec.Prev
          else RootEntryList:=Rec.Next;
    End;
  {element w pe³ni wywalony. Jeszcze na wszelki wypadek
   mo¿na go wyczyœciæ}
  Rec.Prev:=Nil;
  Rec.Next:=Nil;
  Rec.Up:=Nil;
end;

procedure TDynamicStruct.PasteSRecSubTreeAfter(var NewRec: PSearchRecList;RecBefore: PSearchRecList);
{dodaje wczeœniej utworzony element do drzewa za podanym rekordem}
begin
  If (NewRec=Nil) then Exit; {Trudno zmieniæ pozycjê niczego}
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
    {Skoro element poprzedni jest Nil, to mo¿e wstawiamy
     pierwszy element w strukturze. Upewnimy siê i ewentualnie
     ustawimy ten element}
    if (RootEntryList=nil) then RootEntryList:=NewRec;
  End;
end;

procedure TDynamicStruct.PasteSRecSubTreeBefore(var NewRec: PSearchRecList;RecAfter: PSearchRecList);
{dodaje wczeœniej utworzony element do drzewa przed podanym rekordem}
begin
  If (NewRec=Nil) then Exit; {Trudno zmieniæ pozycjê niczego}
  If (RecAfter<>Nil) then
   Begin
    {zmieniamy w³asne wskaŸniki elementu}
    NewRec.Up:=RecAfter.Up;
    NewRec.Next:=RecAfter;
    NewRec.Prev:=RecAfter.Prev;
    {wskaŸniki elementów z lewej i prawej}
    RecAfter.Prev:=NewRec;
    if (NewRec.Prev <> Nil) then
      NewRec.Prev.Next:=NewRec;
    {je¿eli poziom wy¿ej(Up) wskazywa³ na preva, mo¿naby te¿ go zmieniæ
     ¿eby zawsze wskazanie by³o na pierwszego childa}
    if (RecAfter.Up.Down=RecAfter) then RecAfter.Up.Down:=NewRec;
    {to samo tyczy siê wejœcia klasy na strukturê}
    if (RootEntryList=RecAfter) then RootEntryList:=NewRec;
   End else Begin
    NewRec.Next:=Nil;
    NewRec.Prev:=Nil;
    NewRec.Up:=Nil;
    {Skoro element nastêpny jest Nil, to mo¿e wstawiamy
     pierwszy element w strukturze. Upewnimy siê i ewentualnie
     ustawimy ten element}
    if (RootEntryList=nil) then RootEntryList:=NewRec;
  End;
end;

procedure TDynamicStruct.NewSRecDown(var UpRec: PSearchRecList;Value: TSearchRec);
{wstawia element jako pierwszego childa, istniej¹ce childy "przesuwa" dalej}
var NewRec: PSearchRecList;
Begin
  {Ustaw zmienne lokalne}
  if (UpRec<>Nil) then NewRec:=UpRec.Down
    else NewRec:=Nil;
  {dodaj element}
  NewSRecBefore(NewRec,Value);
  {wstaw go do struktury je¿eli NewSRecBefore z tym se nie poradzi³
   (a nie poradzi³ sobie je¿eli UpRec lub UpRec.Down jest NIL)}
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
  {zapamiêtaj poprzedni element}
  LastRec:=NewRec;
  {utwórz nowy element}
  CreateUnassignedSRec(NewRec,Value);
  {Dodaj element do struktury}
  NewRec.Down:=Nil; {jest nowy, wiêc nie ma childów}
  PasteSRecSubTreeAfter(NewRec,LastRec);
End;

procedure TDynamicStruct.NewSRecBefore(var NewRec: PSearchRecList;Value: TSearchRec);
var
    NextRec: PSearchRecList;
Begin
  {Ustaw zmienne lokalne}
  NextRec:=NewRec;
  {utwórz nowy element}
  CreateUnassignedSRec(NewRec,Value);
  {Dodaj element do struktury}
  NewRec.Down:=Nil; {jest nowy, wiêc nie ma childów}
  PasteSRecSubTreeBefore(NewRec,NextRec);
End;

procedure TDynamicStruct.DelSearchRec(var Rec: PSearchRecList);
var GivenRec: PSearchRecList;
Begin
  If (Rec=Nil) then Exit;
  {przygotuj wartoœæ Rec po jego usuniêciu}
  If (Rec.Prev<>Nil) then GivenRec:=Rec.Prev
    else GivenRec:=Rec.Next;
  {wywal Rec ze struktury drzewiastej}
  CutSRecSubTree(Rec);
  {usuñ elementy podrzêdne}
  If (Rec.Down<>Nil) then DelAllSearchRec(Rec.Down);
  {usuñ element}
  Dispose(Rec);
  {ustaw wartoœæ zwrotu}
  Rec:=GivenRec;
End;

procedure TDynamicStruct.DelLastSearchRec(var Rec: PSearchRecList);
var GivenRec: PSearchRecList;
Begin
  If (Rec=Nil) then Exit;
  {przygotuj wartoœæ Rec po jego usuniêciu}
  If (Rec.Next<>Nil) then GivenRec:=Rec else GivenRec:=Rec.Prev;
  {przejdŸ do ostatniego elementu}
  MoveToLastItem(Rec);
  {usuñ element}
  DelSearchRec(Rec);
  {przywróæ wartoœæ Rec}
  Rec:=GivenRec;
End;

procedure TDynamicStruct.DelAllSearchRec(var Rec: PSearchRecList);
{Usuwa wszystkie elementy z poziomu, którego wskaŸnik dosta³o}
var CurRec: PSearchRecList;
begin
  CurRec:=Rec;
  MoveToFirstItem(CurRec);{to na wydadek gdyby nie dosta³ pierwszego elementu}
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
