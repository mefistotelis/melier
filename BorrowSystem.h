//---------------------------------------------------------------------------

#ifndef BorrowSystemH
#define BorrowSystemH
//---------------------------------------------------------------------------
#include "SynchronizerUnit.h"
#include "FXProcs.h"
#include "FXProcsPas.hpp"
#include "BorrowDynamics.h"
//---------------------------------------------------------------------------
enum TBorrowTask
            {
            btNone,//nic do roboty
            btUpdateView,//wyswietl wyniki w komponencie wizualnym
            };
enum TBorrowSaveRequest { bsrNone, bsrAppend, bsrFull, bsrDisconnect };
//---------------------------------------------------------------------------
class TBorrowSystem : public TThread
{
protected:
        //Niskiego poziomu
	int __fastcall GetReturnEntryIndex(TBorrowItem *Item);
	TBorrowItem* __fastcall MakeItemFromText(String istr);
	String __fastcall MakeTextFromItem(const TBorrowItem *iitm);
	bool __fastcall IsBorrowed(String itm);
	String __fastcall FindWhoHas(String itm);
        //Wyœwietlanie
        String ConditionToStr(int iCondition);
        void __fastcall FillListViewItem(TListItem *Itm,const TBorrowItem *SrcItm);
        void __fastcall MakeBorrowGridTabs();
        void __fastcall AddNewItemPtr();
        void __fastcall ClearSpisListView();
        void __fastcall AddToListViewSync();
        void __fastcall AddToListView(TBorrowItem *Item);
        //Wyœwietlanie - wysokiego poziomu
        void __fastcall ShowHistoryInBorrowGrid();
//        void __fastcall ShowPersonInBorrowGrid();
//        void __fastcall ShowElementInBorrowGrid();
//        void __fastcall ShowUnreturnedInBorrowGrid();
        void __fastcall ShowHistoryInListView();
        void __fastcall ShowPersonInListView();
        void __fastcall ShowElementInListView();
        void __fastcall ShowUnreturnedInListView();
        void __fastcall ShowLostInListView();
        //Do listy itemów i osób
        void __fastcall CreateItemListsForOne(TBorrowItem *Item);
        void __fastcall CreateItemLists();
        //Zmienne do wielow¹tkowoœci
        TBorrowTask ActiveTask;
        TBorrowTask TmpTask;
        TBorrowTask NewTask;
        TBorrowItem *AddToVisualListItem;
	TBorrowItem *AddToBorrowListItem;
	TBorrowItem AddToBorrowGridItem;
        //inne zmienne
	TBorrowList* BorrowList;
	TBorrVisMode GridDisplayMode;
	AnsiString PartialViewItemName;
	TNeededUpdate NeedUpdate;
public:
        //Niskiego poziomu
        void __fastcall AddNewItemStr(String ItmStr);
        void __fastcall StartNewTask();
        //Do w¹tku
        void __fastcall ChangeViewMode(TBorrVisMode NewMode,const String &NewViewItemName);
        void __fastcall MakeTaskUpdateBorrowGrid(Grids::TStringGrid* nSpisStringGrid);
        void __fastcall MakeTaskUpdateListView(TListView* nSpisListView);
        void __fastcall UpdateItemLists();
        virtual void __fastcall Execute();
          //Wysokiego poziomu
        bool __fastcall DoPozycz(TObject *Sender,String Osoba,String Element,int BackTime);
        bool __fastcall DoZwroc(TObject *Sender,String Element,int Condition);
          //Zdarzenia do wirtualnej listy historii
        void __fastcall HistoryOnData(TObject *Sender, TListItem *Item);
        void __fastcall HistoryOnDataFind(TObject *Sender,TItemFind Find, const AnsiString FindString,
              const TPoint &FindPosition, Pointer FindData, int StartIndex,TSearchDirection Direction, bool Wrap, int &Index);
        void __fastcall HistoryOnDataHint(TObject *Sender,int StartIndex, int EndIndex);
        void __fastcall HistoryOnDataStCh(TObject *Sender,int StartIndex, int EndIndex, TItemStates OldState,TItemStates NewState);
        void __fastcall UpdateBorrowGrid();
        void __fastcall UpdateListView();
          //Konstrukcja i destrukcja
	__fastcall TBorrowSystem(TSynchronizer* nSynchronizer);
        void __fastcall Init();
	void __fastcall DeInit();
        void __fastcall LoadFromDisk();
        void __fastcall SaveToDisk();
	__fastcall virtual ~TBorrowSystem(void);
          //Zmienne
	bool CancelAction;
	bool Working;
	bool Busy;
	TSynchronizer* Synchronizer;
	TStringList* FullPersonList;
	TStringList* FullItemList;
	TStringGrid* SpisStringGrid;
	TBorrowSaveRequest SaveMode;
        TListView *SpisListView;
};
//---------------------------------------------------------------------------
#endif
