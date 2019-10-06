unit FXProcsPas;

interface

Uses
  Classes, SysUtils;

  Function max(x1,x2:Integer):Integer;
  Function min(x1,x2:Integer):Integer;
  Function GetParam(Number:Integer;Src:String):String;
  Function GetTabParam(Number:Integer;Src:String):String;
  Function GetSpaceParam(Number:Integer;Src:String):String;
  Function GetAfter(x:char;Src:String):String;
  Function GetBefore(x:char;Src:String):String;
  Function RemoveMultipleChars(x:char;SrcStr:String):String;
  Function FindNextBlockBorders(Lines:TStrings;var lnStart:longint;var lnEnd:longint):boolean;
  Function CheckCase(SrcStr:String):String;
  Function ChangeCaseWordsStartsBig(SrcStr:String):String;
  Function IsNearlyInRightCase(Str:String):boolean;

type pBoolean = ^Boolean;

const
  WordChars: set of Char = ['0'..'9', 'A'..'Z', '_', 'a'..'z','¹','æ','ê','³','ñ','ó','œ','Ÿ','¿','¥','Æ','Ê','£','Ñ','Ó','Œ','','¯'];
  DigitsPas=['0'..'9'];
  SymbolsPas=['-','_','.',' ','~',',',';','|','\','/','!','#','%','^','&','*','?','`'];
  LettersPas=['a'..'z','A'..'Z'];


implementation

Function max(x1,x2:Integer):Integer;
Begin
  If x1>x2 then Result:=x1 else Result:=x2;
End;

Function min(x1,x2:Integer):Integer;
Begin
  If x1<x2 then Result:=x1 else Result:=x2;
End;

Function GetParam(Number:Integer;Src:String):String;
var NewStart,NewEnd,CurNum:Integer;
  Procedure OminSpacje(var x:Integer);
  Begin
    If x<=length(src) then
      while (x<=length(src))and not(src[x] in WordChars) do
        Inc(x);
  End;
  Procedure OminWyraz(var x:Integer);
  Begin
    If x<=length(src) then
      while (x<=length(src))and(src[x] in WordChars) do
        Inc(x);
  End;
Begin
  NewEnd:=1;
  CurNum:=0;
  Repeat
    OminSpacje(NewEnd);
    NewStart:=NewEnd;
    OminWyraz(NewEnd);
    Inc(CurNum);
  Until CurNum=Number;
  Result:=Copy(src,NewStart,NewEnd-NewStart);
End;

Function GetTabParam(Number:Integer;Src:String):String;
var NewStart,NewEnd,CurNum:Integer;
  Procedure OminSpacje(var x:Integer);
  Begin
    If x<=length(src) then
      while (x<=length(src))and({(src[x]=#32)or}(src[x]=#9)) do
        Inc(x);
  End;
  Procedure OminWyraz(var x:Integer);
  Begin
    If x<=length(src) then
      while (x<=length(src))and({(src[x]<>#32)and}(src[x]<>#9)) do
        Inc(x);
  End;
Begin
  NewEnd:=1;
  CurNum:=0;
  Repeat
    OminSpacje(NewEnd);
    NewStart:=NewEnd;
    OminWyraz(NewEnd);
    Inc(CurNum);
  Until CurNum=Number;
  Result:=Copy(src,NewStart,NewEnd-NewStart);
End;

Function GetSpaceParam(Number:Integer;Src:String):String;
var NewStart,NewEnd,CurNum:Integer;
  Procedure OminSpacje(var x:Integer);
  Begin
    If x<=length(src) then
      while (x<=length(src))and((src[x]=#32){or(src[x]=#9)}) do
        Inc(x);
  End;
  Procedure OminWyraz(var x:Integer);
  Begin
    If x<=length(src) then
      while (x<=length(src))and((src[x]<>#32){and(src[x]<>#9)}) do
        Inc(x);
  End;
Begin
  NewEnd:=1;
  CurNum:=0;
  Repeat
    OminSpacje(NewEnd);
    NewStart:=NewEnd;
    OminWyraz(NewEnd);
    Inc(CurNum);
  Until CurNum=Number;
  Result:=Copy(src,NewStart,NewEnd-NewStart);
End;

Function GetBefore(x:char;Src:String):String;
var XPos:Integer;
Begin
  XPos:=pos(x,src);
  If XPos>0 then Result:=copy(src,1,xpos-1)
   else Result:=Src;
End;

Function GetAfter(x:char;Src:String):String;
var XPos:Integer;
Begin
  XPos:=pos(x,src);
  If XPos>0 then Result:=copy(src,xpos+1,length(src))
   else Result:='';
End;

Function FindNextBlockBorders(Lines:TStrings;var lnStart:longint;var lnEnd:longint):boolean;
var
  CurLine:String;
begin
  lnStart:=lnEnd;
  while (lnStart<Lines.Count) do
    begin
      CurLine:=Lines.Strings[lnStart];
      if (Length(CurLine)>0) then
        if (CurLine[Length(CurLine)]=':') then break;
      Inc(lnStart);
    end;
  lnEnd:=lnStart+1;
  while (lnEnd<Lines.Count) do
    begin
      CurLine:=Lines.Strings[lnEnd];
      if (Length(CurLine)>0) then
        if (CurLine[Length(CurLine)]=':') then
          begin
          Dec(lnEnd);
          break;
          end;
      Inc(lnEnd);
    end;
  if ((lnEnd>lnStart)and(lnEnd<=Lines.Count)) then
    Result:=true
   else
    Result:=false;
end;

Function ChangeCaseWordsStartsBig(SrcStr:String):String;
var WordStart:boolean;
    i:longint;
begin
    WordStart:=true;
    Result:=SrcStr;
    for i:=1 to Length(SrcStr) do
      begin
      if (SrcStr[i] in WordChars) then
        begin
        if (WordStart) then
          begin
          Result[i]:=AnsiUpperCase(SrcStr[i])[1];
          WordStart:=false;
          end
         else
          Result[i]:=AnsiLowerCase(SrcStr[i])[1];
        end
       else
        WordStart:=true;
      end;
end;

Function CheckCase(SrcStr:String):String;
begin
  if ((not IsNearlyInRightCase(SrcStr))or(AnsiLowerCase(SrcStr)=SrcStr)or(AnsiUpperCase(SrcStr)=SrcStr)) then
    {Normalizujeny wielkoœci liter}
    Result:=ChangeCaseWordsStartsBig(SrcStr)
   else
    Result:=SrcStr;
end;

Function IsNearlyInRightCase(Str:String):boolean;
var CaseCheck:longint;
    cntr,i:longint;
    OneWord:String;
    CheckedCharsCount:LongInt;
begin
{
  Zwraca prawde je¿eli ci¹g ma case zbli¿ony do takiego, gdzie
  pierwsze litery wyrazów s¹ du¿e, pozostale male
}
  CaseCheck:=0;
  cntr:=0;
  CheckedCharsCount:=0;
  repeat
    Inc(cntr);
    OneWord:=GetParam(cntr,Str);
    if (length(OneWord)>1) then
      begin
      inc(CheckedCharsCount,length(OneWord));
      if (OneWord[1]=AnsiUpperCase(OneWord[1])) then Inc(CaseCheck,2);
      for i:=2 to length(OneWord) do
        begin
        if (OneWord[i]=AnsiLowerCase(OneWord[i])) then Inc(CaseCheck);
        end;
      end;
  until (Length(OneWord)<1);
  {Any dzialalo poprawne musi byc ponad 50% ciagu}
  if (CaseCheck > (CheckedCharsCount*3 div 4)) then Result:=true
   else Result:=false;
end;

Function RemoveMultipleChars(x:char;SrcStr:String):String;
{
 Usuwa ze stringa wielokrotne powtórzenia wymienionego znaku
}
var
  chrPos:Integer;
Begin
  Result:='';
  While (Length(SrcStr)>0) do
    Begin
    {Szukamy nastepnego wyst¹pienia znaku}
    chrPos:=AnsiPos(x,SrcStr);
    {Kopiujemy string do tego znaku w³¹cznie}
    {i usuwamy wielokrotne wyst¹pienia znaku}
    if (chrPos<1) then
       Begin
       Result:=Result+SrcStr;
       SrcStr:='';
       End
      else
       Begin
       Result:=Result+Copy(SrcStr,1,chrPos);
       Delete(SrcStr,1,chrPos);
       End;
    {Usuwamy zbedn¹ czêœæ SrcStr}
    while (AnsiPos(x,SrcStr)=1) do
      Delete(SrcStr,1,1);
      if (Length(SrcStr)<1) then Break;
    End;
End;


end.
