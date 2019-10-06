//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "BorrowDynamics.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TBorrowList::TBorrowList()
   : TList()
{
}
//---------------------------------------------------------------------------
__fastcall TBorrowList::~TBorrowList()
{
  for (int I=0;I<Count;I++)
    {
    if (Items[I]!=NULL)
      delete ((TBorrowItem *)Items[I]);
    }
  //Czyszczenie pointerów mamy w klasie nadrzêdnej
}
//---------------------------------------------------------------------------
TBorrowItem * __fastcall TBorrowList::GetBor(int Index)
{
  return (TBorrowItem *)Get(Index);
}
//---------------------------------------------------------------------------
void __fastcall TBorrowList::PutBor(int Index, TBorrowItem *Item)
{
  Put(Index,Item);
}
//---------------------------------------------------------------------------
