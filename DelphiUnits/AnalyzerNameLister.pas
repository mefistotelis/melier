unit AnalyzerNameLister;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ComCtrls, AnalyzerDynamics, FXProcsPas;

type NamePartType=
             (
             nptConstant,
             nptSpecial,
             nptNumber
             );
  PSimilarityItem=^TSimilarityItem;
  TSimilarityItem= Record
             Typ:NamePartType;
             Size:Integer;
             Value:String;
             End;

  TSimilarityList = class(TList)
  protected
    function GetSim(Index: Integer): PSimilarityItem;
    procedure PutSim(Index: Integer; Item: PSimilarityItem);
  public
    constructor Create;
    destructor Destroy; override;
    function AddSim(nTyp:NamePartType;nSize:Integer;nValue:String):Integer;
    property Items[Index: Integer]: PSimilarityItem read GetSim write PutSim; default;
    procedure Delete(Index: Integer);
  end;

  TSimilarNamesListItem = class(TList)
  protected
    function GetSrchRec(Index: Integer): PSearchRecList;
    procedure PutSrchRec(Index: Integer; Item: PSearchRecList);
  public
    {Listy ze schematami podobie�stw}
    StartSimList: TSimilarityList;
    EndSimList: TSimilarityList;
    {Standardowe elementy listy SearchRec�w}
    constructor Create;
    destructor Destroy; override;
    property Items[Index: Integer]: PSearchRecList read GetSrchRec write PutSrchRec; default;
    procedure Delete(Index: Integer);
    {Tworzenie pe�nych list podobie�stw}
    procedure FillStartSimList;
    procedure FillEndSimList;
    {Funkcje pomocnicze tworzenia list podobie�stw}
    procedure DetectSimilarItemAtStart(var StartChar:Integer;NameStr:String);
    procedure DetectSimilarItemAtEnd(var StartChar:Integer;NameStr:String);
    function GetCharType(chr:char):NamePartType;
  end;

  TSimilarNamesList = class(TList)
  private
  {funkcje podrz�dne do liczenia r�nic}
//    function CountSimListDifferences(firstSList:TSimilarityList;secondSList:TSimilarityList):Integer;
    function CountSimListSimilars(firstSList:TSimilarityList;secondSList:TSimilarityList):Integer;
    function CountSimilars(firstSNLItem:TSimilarNamesListItem;secondSNLItem:TSimilarNamesListItem):Integer;
//    function CountDifferences(firstSNLItem:TSimilarNamesListItem;secondSNLItem:TSimilarNamesListItem):Integer;
//    procedure FindMostSimilarItems(var MaxSimilars:Integer;var MaxSimFirst:Integer;var MaxSimSecond:Integer);
    procedure MergeItems(firstSNLIndex:Integer;secondSNLIndex:Integer);
  protected
    function GetSN(Index: Integer): TSimilarNamesListItem;
    procedure PutSN(Index: Integer; Item: TSimilarNamesListItem);
  public
    constructor Create;
    destructor Destroy; override;
    property Items[Index: Integer]: TSimilarNamesListItem read GetSN write PutSN; default;
    procedure Delete(Index: Integer);
    {Funkcje tworzenia nowych grup}
    function AddNew(nItemSearchRec:PSearchRecList):Integer;
    procedure ReduceGroups;
    procedure RenameGroupedFiles;
  end;

implementation

constructor TSimilarityList.Create;
begin
  inherited Create;
end;

destructor TSimilarityList.Destroy;
begin
  try
  while (Count>0) do
      Delete(Count-1);
  {Czyszczenie pointer�w mamy w klasie nadrz�dnej,
   ale Delete usuwa te� struktury z pami�ci}
  finally
  inherited Destroy;
  end;
end;

procedure TSimilarityList.Delete(Index: Integer);
Begin
  if (Items[Index]<>nil) then
    Dispose(PSimilarityItem(Items[Index]));
  inherited Delete(Index);
End;

function TSimilarityList.GetSim(Index: Integer): PSimilarityItem;
Begin
    Result:=Get(Index);
End;

procedure TSimilarityList.PutSim(Index: Integer; Item: PSimilarityItem);
Begin
  Put(Index,Item);
End;

function TSimilarityList.AddSim(nTyp:NamePartType;nSize:Integer;nValue:String):Integer;
var Item: PSimilarityItem;
Begin
  New(Item);
  Item.Typ:=nTyp;
  Item.Size:=nSize;
  Item.Value:=nValue;
  Result:=Add(Item);
End;
{--------------------------------------}
constructor TSimilarNamesListItem.Create;
begin
  StartSimList:=TSimilarityList.Create;
  EndSimList:=TSimilarityList.Create;
  inherited Create;
end;

destructor TSimilarNamesListItem.Destroy;
begin
 try
  StartSimList.Free;
  EndSimList.Free;
  {Czyszczenie pointer�w mamy w klasie nadrz�dnej.
    Czy�ci� czy usuwa� samych rekord�w NIE POWINNI�MY}
 finally
  inherited Destroy;
 End;
end;

procedure TSimilarNamesListItem.Delete(Index: Integer);
{Tu niszczy bez zwalniania pami�ci - tak ma by�}
Begin
  inherited Delete(Index);
End;


function TSimilarNamesListItem.GetSrchRec(Index: Integer): PSearchRecList;
Begin
    Result:=Get(Index);
End;

procedure TSimilarNamesListItem.PutSrchRec(Index: Integer; Item: PSearchRecList);
Begin
  Put(Index,Item);
End;

procedure TSimilarNamesListItem.FillStartSimList;
var
  NameStr:String;
  StartChar:Integer;
Begin
  if (Count<1) then Exit;
  if (Items[0]=nil) then Exit;
  NameStr:=Items[0].RealProperties.Name;
  StartChar:=0;
  while (StartChar <= Length(NameStr)) do
    DetectSimilarItemAtStart(StartChar,NameStr);
End;

procedure TSimilarNamesListItem.FillEndSimList;
var
  NameStr:String;
  StartChar:Integer;
Begin
  if (Count<1) then Exit;
  if (Items[0]=nil) then Exit;
  NameStr:=Items[0].RealProperties.Name;
  StartChar:=Length(NameStr);
{    !!!!!!!!!!!!
  while (StartChar > 0) do
    DetectSimilarItemAtEnd(StartChar,NameStr);
    }
End;

procedure TSimilarNamesListItem.DetectSimilarItemAtStart(var StartChar:Integer;NameStr:String);
var SimItem:PSimilarityItem;
Begin
  if (Length(NameStr)<2) Then Begin StartChar:=Length(NameStr)+1;Exit;End;
  New(SimItem);
  {Okre�lamy typ tego fragmentu nazwy}
  SimItem.Typ:=GetCharType(NameStr[StartChar]);
  {Okreslamy d�ugo�� fragmentu}
  SimItem.Size:=1;
  while (SimItem.Size+StartChar <= Length(NameStr)) do
   Begin
   if (SimItem.Typ = GetCharType(NameStr[SimItem.Size+StartChar])) then
     Break;
   Inc(SimItem.Size);
   End;
  {No i dok�adna zawarto��}
  SimItem.Value:=AnsiUpperCase(Copy(NameStr,StartChar,SimItem.Size));
  {Jeszcze tylko doda� fragment do listy}
  StartSimList.Add(SimItem);
  {i ustali� nowy StartChar}
  Inc(StartChar,SimItem.Size);
End;

procedure TSimilarNamesListItem.DetectSimilarItemAtEnd(var StartChar:Integer;NameStr:String);
Begin
{!!!!!!!!}
End;

function TSimilarNamesListItem.GetCharType(chr:char):NamePartType;
Begin
  if (chr in DigitsPas) then
    Result:=nptNumber
   else
  if (chr in SymbolsPas) then
    Result:=nptSpecial
   else
    Result:=nptConstant;
End;
{--------------------------------------}
constructor TSimilarNamesList.Create;
begin
  inherited Create;
end;

destructor TSimilarNamesList.Destroy;
{
  Rozwala struktur� i wszystkie jej elementy,
  zostawia tylko SearchRecListy z MatchingFiles.
}
begin
  try
  while (Count>0) do
      Delete(Count-1);
  {Czyszczenie pointer�w mamy w klasie nadrz�dnej,
   ale Delete usuwa jeszcze podobjekty...}
  finally
  inherited Destroy;
  end;
end;

procedure TSimilarNamesList.Delete(Index: Integer);
Begin
  if (Items[Index]<>nil) then
      Items[Index].Free;
  inherited Delete(Index);
End;

function TSimilarNamesList.GetSN(Index: Integer): TSimilarNamesListItem;
Begin
    Result:=Get(Index);
End;

procedure TSimilarNamesList.PutSN(Index: Integer; Item: TSimilarNamesListItem);
Begin
  Put(Index,Item);
End;

function TSimilarNamesList.AddNew(nItemSearchRec:PSearchRecList):Integer;
{
 Przygotowuje now� grup� podobie�stw, tworzy niezb�dne struktury
  i umieszcza w niej pierwszy objekt.
  Zwraca indeks nowej struktury w li�cie.
}
var Item: TSimilarNamesListItem;
Begin
  try
    Item:=TSimilarNamesListItem.Create; {Podobjekty utworz� si� same}
    Item.Add(nItemSearchRec);
    Item.FillStartSimList;
    Item.FillEndSimList;
    Result:=Add(Item);
  except
    Result:=-1;
  end;
End;

{
function TSimilarNamesList.CountDifferences(firstSNLItem:TSimilarNamesListItem;secondSNLItem:TSimilarNamesListItem):Integer;
var
   cntr:Integer;
Begin
  cntr:=CountSimListDifferences(firstSNLItem.StartSimList,secondSNLItem.StartSimList);
  cntr:=cntr+CountSimListDifferences(firstSNLItem.EndSimList,secondSNLItem.EndSimList);
  Result:=cntr;
End;
}
function TSimilarNamesList.CountSimilars(firstSNLItem:TSimilarNamesListItem;secondSNLItem:TSimilarNamesListItem):Integer;
var
   cntr:Integer;
Begin
  cntr:=CountSimListSimilars(firstSNLItem.StartSimList,secondSNLItem.StartSimList);
  cntr:=cntr+CountSimListSimilars(firstSNLItem.EndSimList,secondSNLItem.EndSimList);
  Result:=cntr;
End;

procedure TSimilarNamesList.MergeItems(firstSNLIndex:Integer;secondSNLIndex:Integer);
{
  Dostaje dwia Itemy. Poszerza pierwszego o elementy z drugiej,
 drug� za� usuwa.
}
var
   nSimilars:Integer;
   nItem:Integer;
Begin
  {Trzeba usun�� z list podobie�stw elementy, kt�re nie s� wsp�lne}
  nSimilars:=CountSimListSimilars(Items[firstSNLIndex].StartSimList,Items[secondSNLIndex].StartSimList);
  while (Items[firstSNLIndex].StartSimList.Count>nSimilars) do
    Items[firstSNLIndex].StartSimList.Delete(Items[firstSNLIndex].StartSimList.Count-1);
  nSimilars:=CountSimListSimilars(Items[firstSNLIndex].EndSimList,Items[secondSNLIndex].EndSimList);
  while (Items[firstSNLIndex].StartSimList.Count>nSimilars) do
    Items[firstSNLIndex].EndSimList.Delete(Items[firstSNLIndex].EndSimList.Count-1);
  {Pozostaje przenie�� elementy listy drugiej do pierwszej}
  for nItem:=0 to Items[secondSNLIndex].Count-1 do
    Items[firstSNLIndex].Add(Items[secondSNLIndex].Items[nItem]);
  Delete(secondSNLIndex);
End;

(*
function TSimilarNamesList.CountSimListDifferences(firstSList:TSimilarityList;secondSList:TSimilarityList):Integer;
var
   cntr:Integer;
Begin
  cntr:=CountSimListSimilars(firstSList,secondSList);
  {No i mamy cntr=ilo�� podobie�stw. St�d �atwo ilo�� r�nic}
  Result:=max(firstSList.Count,secondSList.Count)-cntr;
End;
*)
function TSimilarNamesList.CountSimListSimilars(firstSList:TSimilarityList;secondSList:TSimilarityList):Integer;
{
 Zwraca ilo�� pocz�tkowych podobnych wyraz�w w listach
}
var
   tmpNum:Integer;
   cntr:Integer;
Begin
  cntr:=0;
  while (cntr<min(firstSList.Count,secondSList.Count)) do
    Begin
    {Je�eli typy si� nie zgadzaj� to koniec}
    if (firstSList.Items[cntr].Typ <> secondSList.Items[cntr].Typ) then Break;
    {Typy s� takie same.}
    case (firstSList.Items[cntr].Typ) of
    {Je�eli to sta�e, musz� by� identyczne (mo�e potem to poszerz�...)}
    nptConstant:Begin
      if (firstSList.Items[cntr].Value<>secondSList.Items[cntr].Value) then Break;
      End;
    {Je�eli to znaki specjalne, musz� by� identyczne}
    nptSpecial:Begin
      if (firstSList.Items[cntr].Value<>secondSList.Items[cntr].Value) then Break;
      End;
    {Je�eli oba s� liczbami, to mog� by� dowolne i r�ni� si� d�ugo�ci� co najwy�ej o 2}
    nptNumber:Begin
      tmpNum:=firstSList.Items[cntr].Size-secondSList.Items[cntr].Size;
      if ((tmpNum>2)or(tmpNum<(-2))) then Break;
      End;
      {na wypadek b��du...}
    else
      Break;
    End;
    Inc(cntr);
    End;
  Result:=cntr;
End;
procedure TSimilarNamesList.ReduceGroups;
Begin

{!!!!!!!!!!
Jak to rozwi�za�????}
End;

procedure TSimilarNamesList.RenameGroupedFiles;
Begin
{!!!!!!!!!!}
End;

end.
