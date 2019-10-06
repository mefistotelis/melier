//---------------------------------------------------------------------------

#ifndef BorrowDynamicsH
#define BorrowDynamicsH
//---------------------------------------------------------------------------
struct TBorrowItem
{
	bool Back;
	String Name;
	unsigned char Condition;
	System::TDateTime Date;
	System::TDateTime BackDate;
	String Items;
	int ReturnEntry;
};
//---------------------------------------------------------------------------
class TBorrowList : public TList
{
protected:
	TBorrowItem * __fastcall GetBor(int Index);
	void __fastcall PutBor(int Index, TBorrowItem *Item);
public:
          //Konstrukcja i destrukcja
	__fastcall TBorrowList();
	__fastcall virtual ~TBorrowList();
          //Propertiesy
	__property TBorrowItem * Items[int Index] = {read=GetBor, write=PutBor/*, default*/};
};
//---------------------------------------------------------------------------
#endif
 