//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SearcherItmBuffer.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TSearcherItmBuffer::TSearcherItmBuffer()
  : TList()
{
  BufferUsed=sizeof(TSearcherItmBuffer);
}
//---------------------------------------------------------------------------
__fastcall TSearcherItmBuffer::~TSearcherItmBuffer()
{
 __try
  {
  //Czyszczenie pointerów mamy w klasie nadrzêdnej,
  // ale Delete usuwa te¿ struktury z pamiêci
  while (Count>0)
      Delete(Count-1);
  }
 __except(EXCEPTION_EXECUTE_HANDLER) { };
}
//---------------------------------------------------------------------------
void __fastcall TSearcherItmBuffer::Delete(int Index)
{
  if (Index>=Count) return;
  FreeItem(Index);
  TList::Delete(Index);
}
//---------------------------------------------------------------------------
void __fastcall TSearcherItmBuffer::FreeItem(int Index)
{
  if (Index>=Count) return;
  TSrchItmBufItem *DelItem=Items[Index];
  if (DelItem!=NULL)
    {
    if (DelItem->FileBody!=NULL)
      {
      BufferUsed-=(sizeof(TStringList)+DelItem->Size);
      DelItem->FileBody->Free();
      }
    BufferUsed-=sizeof(TSrchItmBufItem);
    delete DelItem;
    Items[Index]=NULL;
    }
}
//---------------------------------------------------------------------------
TSrchItmBufItem* __fastcall TSearcherItmBuffer::GetItm(int Index)
{
  if ((Index<Count)&&(Index>=0))
    return (TSrchItmBufItem*)Get(Index);
   else
    return NULL;
}
//---------------------------------------------------------------------------
void __fastcall TSearcherItmBuffer::PutItm(int Index, TSrchItmBufItem *Item)
{
  if (Index>=Count) ChangeItemsCount(Index+2);
  Put(Index,Item);
}
//---------------------------------------------------------------------------
bool __fastcall TSearcherItmBuffer::CanLoad(int nItmNumber, AnsiString nFileName)
{
// Sprawdza czy w buforze jest odpowiedni do pobrania plik bazy
  TSearchRec F;
  TSrchItmBufItem *CurItem=Items[nItmNumber];
  //Sprawdzamy czy w ogóle mamy bufor i za³adowany plik
  if (CurItem==NULL) return false;
  if (CurItem->FileBody==NULL) return false;
  //Skoro cos jest za³adowane, sprawdzamy czy pasuje do pliku na dysku
  if (FindFirst(nFileName,faReadOnly+faHidden+faSysFile+faArchive,F)!=0) return false;
  if (F.Time!=CurItem->Time) return false;
  if (F.Size!=CurItem->Size) return false;
  return true;
}
//---------------------------------------------------------------------------
void __fastcall TSearcherItmBuffer::ChangeItemsCount(int nCount)
//  Wywo³ywana by zmieniæ liczbê wskaŸników pamiêtanych
//  w buforze
{
  while (Count>nCount)
    Delete(Count-1);
  if (Count<nCount)
    Count=nCount; //To spowoduje dostawienie pustych wskaŸników
}
//---------------------------------------------------------------------------
int __fastcall TSearcherItmBuffer::ConsiderAddingItem(int nItmNumber, AnsiString nFileName, Classes::TStrings* nFileBody)
{
//  Okreœla czy w buforze jest miejsce, jak tak to dodaje tam plik.
//  Zwraca 0 jezeli doda plik.
  TSearchRec F;
  //usuwamy to, co mog³o buc wczesniej w buforze
  FreeItem(nItmNumber);
  //szukamy pliku na dysku
  if (FindFirst(nFileName,faReadOnly+faHidden+faSysFile+faArchive,F)!=0) return 12;
  //wiec wszystko mamy. Teraz decyzja: czy dodawac
    // Potem mo¿e napiszê lepszy system decyzyjny (w buforze lepiej trzymac male pliki)
  if (BufferUsed+F.Size > MaxBuffer) return 11;
  //no i dodajemy
  return AddItm(nItmNumber,F.Time,F.Size,nFileBody);
}
//---------------------------------------------------------------------------
int __fastcall TSearcherItmBuffer::AddItm(int nItmNumber, int nDate, int nFileSize, Classes::TStrings* &nFileBody)
{
//  Dodaje element do bufora.
//  Zwraca 0 jesli doda.
 TSrchItmBufItem *Item;
 __try
  {
  FreeItem(nItmNumber);
  Item=new TSrchItmBufItem;
  __try
    {
    BufferUsed+=sizeof(TSrchItmBufItem);
    Item->Time=nDate;
    Item->Size=nFileSize;
    Item->FileBody=new TStringList();
    Item->FileBody->Assign(nFileBody);
    BufferUsed+=(sizeof(TStringList)+Item->Size);
    Items[nItmNumber]=Item;
    return 0;
    }
   __except(EXCEPTION_EXECUTE_HANDLER)
    {
    BufferUsed-=sizeof(TSrchItmBufItem);
    delete Item;
    Items[nItmNumber]=NULL;
    return 2;
    }
  }
 __except(EXCEPTION_EXECUTE_HANDLER)
  {
  Items[nItmNumber]=NULL;
  return 1;
  }

}
//---------------------------------------------------------------------------

