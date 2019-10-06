//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SearcherCatBuffer.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TSearcherCatBuffer::TSearcherCatBuffer()
  : TList()
{
}
//---------------------------------------------------------------------------
__fastcall TSearcherCatBuffer::~TSearcherCatBuffer()
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
void __fastcall TSearcherCatBuffer::Delete(int Index)
{
  if (Index>=Count) return;
  FreeItem(Index);
  TList::Delete(Index);
}
//---------------------------------------------------------------------------
void __fastcall TSearcherCatBuffer::FreeItem(int Index)
{
  if (Index>=Count) return;
  TSrchCatBufItem *DelItem=Items[Index];
  if (DelItem!=NULL)
    {
    if (DelItem->FileBody!=NULL)
      {
      DelItem->FileBody->Free();
      }
    delete DelItem;
    Items[Index]=NULL;
    }
}
//---------------------------------------------------------------------------
TSrchCatBufItem* __fastcall TSearcherCatBuffer::GetItm(int Index)
{
  if ((Index<Count)&&(Index>=0))
    return (TSrchCatBufItem*)Get(Index);
   else
    return NULL;
}
//---------------------------------------------------------------------------
void __fastcall TSearcherCatBuffer::PutItm(int Index, TSrchCatBufItem *Item)
{
  if (Index>=Count) ChangeItemsCount(Index+2);
  Put(Index,Item);
}
//---------------------------------------------------------------------------
void __fastcall TSearcherCatBuffer::ChangeItemsCount(int nCount)
//  Wywo³ywana by zmieniæ liczbê wskaŸników pamiêtanych
//  w buforze
{
  while (Count>nCount)
    Delete(Count-1);
  if (Count<nCount)
    Count=nCount; //To spowoduje dostawienie pustych wskaŸników
}
//---------------------------------------------------------------------------
