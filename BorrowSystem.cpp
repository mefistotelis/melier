//---------------------------------------------------------------------------
/*
   Nowa klasa systemu po�yczek
*/

#include <vcl.h>
#pragma hdrstop

#include "BorrowSystem.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------
const String LostNameStr="Zaginione";
const String DamagNameStr="Zniszczone";
const String CheckNameStr="Sprawdzian obecno�ci";
//---------------------------------------------------------------------------
__fastcall TBorrowSystem::TBorrowSystem(TSynchronizer* nSynchronizer)
        : TThread(true)
//Konstruktor klasy
{
  __try
    {
    // Ustawiam thread
    Priority = tpLowest;
    CancelAction=false;
    Working=false;
    ActiveTask=btNone;
    NewTask=btNone;
    Busy=true;
    SaveMode=bsrNone;
    SpisListView=NULL;
    SpisStringGrid=NULL;
    // no i pozosta�e
    Synchronizer=nSynchronizer;
    }
  __except(EXCEPTION_EXECUTE_HANDLER)
    {
    SaveMode=bsrDisconnect;
    MessageBox(Synchronizer->MainForm->Handle,"B��d tworzenia objektu systemu po�yczek.","O ty huju jebany bambusie",MB_OK|MB_ICONERROR|MB_APPLMODAL);
    }//end try
}
//---------------------------------------------------------------------------
__fastcall TBorrowSystem::~TBorrowSystem()
//Czysci wszystko i usuwa strukture
{
 __try
  {
  }
 __finally
  {
  };
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::Init()
{
  __try
    {
    BorrowList=new TBorrowList();
    FullPersonList=new TStringList();
    FullItemList=new TStringList();
    //Synchronize(MakeBorrowGridTabs);
    Busy=false;
    FullItemList->Sorted=true;
    FullPersonList->Sorted=true;
    LoadFromDisk();
    }
  __except(EXCEPTION_EXECUTE_HANDLER)
    {
    SaveMode=bsrDisconnect;
    MessageBox(Synchronizer->MainForm->Handle,"B��d inicjacji podobjekt�w systemu po�yczek.","O ty huju jebany pierdolony bambusie",MB_OK | MB_ICONERROR | MB_APPLMODAL);
    Busy=false;
    }//end try
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::DeInit()
{
  try
    {
    Busy=true;
    if (Working)
      {
      NewTask=btNone;
      if (!CancelAction) CancelAction=true;
      }
    SaveMode=bsrDisconnect;
    Priority=tpHigher;
    Terminate();
    while (Suspended) Resume();
    }
  catch (...)
    {
      SaveMode=bsrDisconnect;
      MessageBox(Synchronizer->MainForm->Handle,"Wyj�tek podczas niszczenia systemu po�yczek. Taka sytuacja nie powinna mie� miejsca, ale jest nieszkodliwa.","Taki kurwa jeste� pojebany...",MB_OK|MB_ICONERROR|MB_APPLMODAL);
    }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::Execute()
// G��wna funkcja w�tku
{
 do {
  try
    {
    // System usuwania b��d�w i szczeg�lnych sytuacji
    if ((Working)||(CancelAction))
      {
      Working=false;
      TmpTask=btNone;
      Synchronize(StartNewTask);
      CancelAction=false;
      }
    // Jak nie ma zlece� to mo�na sie uspic
    if (NewTask==btNone) Suspend();
    // No i normalne wykonywanie polece�
    if (!Busy)
      {
      TmpTask=NewTask;
      NewTask=btNone;
      Synchronize(StartNewTask);
      Priority=tpNormal;
      Working=true;
      switch (ActiveTask)
         {
         case btUpdateView:
           try
             {
             //UpdateBorrowGrid();
             Priority=tpLowest;
             UpdateListView();
             Priority=tpNormal;
             TmpTask=btNone;
             }
           catch(Exception &E)
             {
             SaveMode=bsrDisconnect;
             String tmpStr="Wyst�pi� b��d podczas uaktualniania tabeli ilustruj�cej po�yczki. Przed zamkni�ciem programu wykonaj kopi� bezpiecze�stwa bazy danych.\nOryginalny komunikat b��du:\n\n"+E.Message;
             MessageBox(Synchronizer->MainForm->Handle,tmpStr.c_str(),"Co ty kurwa znowu rozpierdoli�e�?",MB_OK | MB_ICONERROR | MB_APPLMODAL);
             }
           break;
          }
      Synchronize(StartNewTask);
      CancelAction=false;
      Working=false;
      Priority=tpIdle;
      } //if (!Busy)
    }
  catch (Exception &E)
     {
     SaveMode=bsrDisconnect;
     String tmpStr="Wyst�pi� b��d podczas pracy w�tku po�yczek. Prawdopodobnie problem zwi�zany jest z wielow�tkowo�ci� i podzia�em dost�pu.\nOryginalny komunikat b��du:\n\n"+E.Message;
     MessageBox(Synchronizer->MainForm->Handle,tmpStr.c_str(),"Co ty kurwa znowu rozpierdoli�e�?",MB_OK | MB_ICONWARNING | MB_APPLMODAL);
     }
  }
 while (!Terminated);
 //Zdarzenie zako�czenia threada jest tutaj
    if (BorrowList!=NULL)
      delete BorrowList;
    BorrowList=NULL;
    if (FullPersonList!=NULL)
      delete FullPersonList;
    FullPersonList=NULL;
    if (FullItemList!=NULL)
      delete FullItemList;
    FullItemList=NULL;
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::MakeTaskUpdateBorrowGrid(Grids::TStringGrid* nSpisStringGrid)
// Publiczna funkcja do od�wierzania listy na ekranie
{
  SpisStringGrid=nSpisStringGrid;
  if (Working)
    if (ActiveTask==btUpdateView) CancelAction=true;
  NewTask=btUpdateView;
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::MakeTaskUpdateListView(TListView* nSpisListView)
// Publiczna funkcja do od�wierzania listy na ekranie
{
  SpisListView=nSpisListView;
  if (Working)
    if (ActiveTask==btUpdateView) CancelAction=true;
  NewTask=btUpdateView;
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::UpdateBorrowGrid()
{
  if (NeedUpdate!=nuNone)
  switch (GridDisplayMode)
    {
    case gmHistory:
      ShowHistoryInBorrowGrid();
      break;
    case gmElement:
      //ShowElementInBorrowGrid();
      break;
    case gmPerson:
      //ShowPersonInBorrowGrid();
      break;
    case gmUnreturned:
      //ShowUnreturnedInBorrowGrid();
      break;
    }//switch
  //Je�eli aktywny jest control z rozwijana lista
    if ((Synchronizer->MainForm->ActiveControl==Synchronizer->PPoComboBox)||
        (Synchronizer->MainForm->ActiveControl==Synchronizer->PVEEdit))
    //sprawd� aktualnosc i jak trzeba uaktualnij liste
      Synchronize(UpdateItemLists);
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::UpdateListView()
{
  if (NeedUpdate!=nuNone)
  switch (GridDisplayMode)
    {
    case gmHistory:
      ShowHistoryInListView();break;
    case gmElement:
      ShowElementInListView();break;
    case gmPerson:
      ShowPersonInListView();break;
    case gmUnreturned:
      ShowUnreturnedInListView();break;
    case gmLost:
      ShowLostInListView();break;
    }//switch
  //Je�eli aktywny jest control z rozwijana lista
    if ((Synchronizer->MainForm->ActiveControl==Synchronizer->PPoComboBox)||
        (Synchronizer->MainForm->ActiveControl==Synchronizer->PVEEdit))
    //sprawd� aktualnosc i jak trzeba uaktualnij liste
      Synchronize(UpdateItemLists);
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::MakeBorrowGridTabs()
// Wywo�ywana przez konstruktora i funkcj� wy�wietlaj�c�
{
    if (Synchronizer->SpisStringGrid==NULL) return;
    Synchronizer->SpisStringGrid->Cells[0][0]="Imi� i nazwisko";
    Synchronizer->SpisStringGrid->Cells[1][0]="Elementy";
    Synchronizer->SpisStringGrid->Cells[2][0]="Data";
    Synchronizer->SpisStringGrid->Cells[3][0]="Zwrot/Stan";
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::LoadFromDisk()
// Wywo�ywana przez konstruktora
{
  TStringList *BorrowFile;
  Busy=true;
  BorrowFile=new TStringList();
  __try
    {
    BorrowFile->LoadFromFile(Synchronizer->SpisPath+BorrowFileName);
    for (int num=0;num<BorrowFile->Count;num++)
       AddNewItemStr(BorrowFile->Strings[num]);
    NeedUpdate=nuFull;
    }
  __finally
    {
    SaveMode=bsrNone;
    delete BorrowFile;
    Busy=false;
    }//end try
  CreateItemLists();
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::AddNewItemStr(String ItmStr)
// Dodaje element do listy po�yczek
//  Nale�y pamietac by przed jego wywolaniem
//  ustawic Busy na true
{
   AddToBorrowListItem=new TBorrowItem;
   AddToBorrowListItem=MakeItemFromText(ItmStr);
   Synchronize(AddNewItemPtr);
   AddToBorrowListItem=NULL;
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::ChangeViewMode(TBorrVisMode NewMode,const String &NewViewItemName)
{
  if ((GridDisplayMode!=NewMode)||(PartialViewItemName!=NewViewItemName))
   {
    GridDisplayMode=NewMode;
    PartialViewItemName=NewViewItemName;
    NeedUpdate=nuFull;
   }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::StartNewTask()
{
  ActiveTask=TmpTask;
}
//---------------------------------------------------------------------------
bool __fastcall TBorrowSystem::DoPozycz(TObject *Sender,String Osoba,String Element,int BackTime)
{
  //warunki odpowiedniego ustawienia zak�adki
  Busy=true;
  bool Result=false;
  TBorrowItem *Item;
  if (((Osoba.Length()>1)&&(Element.Length()>0)))
    {
    //warunki ci�g�osci po�ycz-zwr��
    if (!IsBorrowed(Element))
     __try
      {
      //tworz� element
      Item=new TBorrowItem;
      //wype�niam nowy element
      Item->ReturnEntry=-1;
      Item->Back=false;
        Item->Name=Osoba;
        Item->Items=Element;
        Item->BackDate=Date()+BackTime;
        Item->Condition=0;
        Element="";
      Item->Date=Date();
      //Dodaj� element do struktury
      AddToBorrowListItem=Item;
      Synchronize(AddNewItemPtr);
      //uzupe�nianie list os�b/przedmiot�w
      CreateItemListsForOne(Item);
      //ustawienia ko�cowe
      NeedUpdate=nuFull;
      Synchronizer->UpdateStatusBar("Operacja po�yczania zako�czona poprawnie");
      Result=true;
      }
    __except(EXCEPTION_EXECUTE_HANDLER)
      {
      MessageBox(Synchronizer->MainForm->Handle,"Nie mog� doda� elementu do listy pozycze�.","Ale kutas z ciebie...",MB_OK | MB_ICONEXCLAMATION);
      Result=false;
      }
    }
   else
    {
    Synchronizer->UpdateStatusBar("Element jest ju� po�yczony.");
    }
  Busy=false;
  return Result;
}
//---------------------------------------------------------------------------
bool __fastcall TBorrowSystem::DoZwroc(TObject *Sender,String Element,int Condition)
{
  //warunki odpowiedniego ustawienia zak�adki
  Busy=true;
  bool Result=false;
  TBorrowItem *Item;
  if ((Element.Length()>0))
    {
    //warunki ci�g�osci po�ycz-zwr��
    if (IsBorrowed(Element))
     {
     __try
      {
      //tworz� nowy element
      Item=new TBorrowItem;
      //wype�niam nowy element
      Item->ReturnEntry=-1;
      Item->Back=true;
        Item->Name=FindWhoHas(Element);
        Item->Items=Element;
        Item->BackDate=0;
        Item->Condition=Condition;
        Element="";
      Item->Date=Date();
      //Dodaj� element do struktury
      AddToBorrowListItem=Item;
      Synchronize(AddNewItemPtr);
      //uzupe�nianie list os�b/przedmiot�w
      CreateItemListsForOne(Item);
      //ustawienia ko�cowe
      NeedUpdate=nuFull;
      Synchronizer->UpdateStatusBar("Operacja zwracania zako�czona poprawnie");
      Result=true;
      }
    __except(EXCEPTION_EXECUTE_HANDLER)
      {
      MessageBox(Synchronizer->MainForm->Handle,"Nie mog� doda� elementu do listy po�ycze�/zwrot�w.","Ale kutas z ciebie...",MB_OK | MB_ICONEXCLAMATION);
      Result=false;
      }
     }
    else
     {
     Synchronizer->UpdateStatusBar("Element nie jest pozyczony, nie mog� go zwr�ci�.");
     }
    }
   else
    Synchronizer->UpdateStatusBar("Wpisz nazw� elementu, potem pr�buj go zwraca�.");
  Busy=false;
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::CreateItemListsForOne(TBorrowItem *Item)
{
    if (Item!=NULL)
      {
      //uzupe�nianie list os�b/przedmiot�w
      if (FullPersonList->IndexOf(Item->Name)<0)
        FullPersonList->Add(Item->Name);
      if (FullItemList->IndexOf(Item->Items)<0)
        FullItemList->Add(Item->Items);
      }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::CreateItemLists()
{
  for (int cntr=0;cntr<BorrowList->Count;cntr++)
    CreateItemListsForOne(BorrowList->Items[cntr]);
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::UpdateItemLists()
{
  if (GridDisplayMode==gmElement)
    {
    if (Synchronizer->PVEEdit->Items->Text!=FullItemList->Text)
      {
      Synchronizer->PVEEdit->Items=FullItemList;
      Synchronizer->PVEEdit->Text="";
      }
    }
  else
  if (GridDisplayMode==gmPerson)
    {
    if (Synchronizer->PVEEdit->Items->Text!=FullPersonList->Text)
      {
      Synchronizer->PVEEdit->Items=FullPersonList;
      Synchronizer->PVEEdit->Text="";
      }
    }
  if (Synchronizer->MainForm->ActiveControl==Synchronizer->PPoComboBox)
    if (Synchronizer->PPoComboBox->Items->Text!=FullPersonList->Text)
      {
      Synchronizer->PPoComboBox->Items=FullPersonList;
      }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::SaveToDisk()
//  Zapisuje na dysk baz� danych po�yczek
//!!! zrobi� rozr�nianie pomi�dzy bsrAppend i bsrFull
{
    if (SaveMode==bsrNone) return;
    if (SaveMode==bsrDisconnect)
      {
      MessageBox(Synchronizer->MainForm->Handle,"Ze wzgl�du na pewne b��dy, kt�re wyst�pi�y w programie, nast�pi�o odci�cie systemu po�yczek od dysku. Zmniejsza to prawdopodobie�stwo utraty danych. Modyfikacje bazy danych po�yczek NIE ZOSTAN� ZAPISANE!","Ja pierdol� tak� robot�!",MB_OK | MB_ICONWARNING | MB_APPLMODAL);
      return;
      }
    TStringList *BorrowFile=new TStringList();
    __try
      {
      //Zape�niamy StringList�
      for (int cntr=0;cntr<BorrowList->Count;cntr++)
        {
        TBorrowItem *Item=BorrowList->Items[cntr];
        if (Item!=NULL)
          {
          String str=MakeTextFromItem(Item);
          BorrowFile->Add(str);
          }
        }
      //I zapisujemy do pliku
      BorrowFile->SaveToFile(Synchronizer->SpisPath+BorrowFileName);
      }
    __finally
      {
      //Usuwamy StringLista po zapisaniu
      SaveMode=bsrNone;
      delete BorrowFile;
      }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::AddNewItemPtr()
// Dodaje element do listy po�yczek.
//  Nale�y pamietac by przed jego wywolaniem
//  ustawic BorrowSystem.Busy na true.
// Korzysta ze zmiennej TempItem aby
//  umo�liwi� wywo�ania Synchronize(..);
{
  TBorrowTask RedoTask;
  //Zapami�tujemy, co program robi�
  if (Working) RedoTask=ActiveTask;
    else RedoTask=btNone;
  if (Working)
    if (!CancelAction) CancelAction=true;
  //Dodajemy element do listy
  if (AddToBorrowListItem!=NULL)
    BorrowList->Add(AddToBorrowListItem);
  //Skoro doszed� element, trzeba b�dzie zapisa� zmiany
  if (SaveMode==bsrNone)
    SaveMode=bsrAppend;
  //i ponawiamy poprzednie zadanie
  if (RedoTask!=btNone) NewTask=RedoTask;
}
//---------------------------------------------------------------------------
String TBorrowSystem::ConditionToStr(int iCondition)
{
  switch (iCondition)
    {
    case 5:  return "B. Dobry";
    case 4:  return "Do�� Dobry";
    case 3:  return "Dostat.";
    case 2:  return "S�aby";
    default: return "Nieznany";
    }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::ClearSpisListView()
{
 __try
  {
  SpisListView->Items->BeginUpdate();
  SpisListView->Items->Clear();
  }
 __finally
  {
  SpisListView->Items->EndUpdate();
  }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::FillListViewItem(TListItem *Itm,const TBorrowItem *SrcItm)
//Wype�nie ListItem na podstawie SrcItm
//UWAGA! Nie czy�ci SubItems!
{
        Itm->Caption=SrcItm->Name;
        Itm->SubItems->Add("n/a");
        Itm->SubItems->Add(DateToStr(SrcItm->Date));
        Itm->SubItems->Add("n/a");
        if (SrcItm->Back)
          {
          Itm->SubItems->Strings[0]="Zwrot: "+SrcItm->Items;
          Itm->SubItems->Strings[2]=ConditionToStr(SrcItm->Condition);
          }
        else
          {
          Itm->SubItems->Strings[0]=SrcItm->Items;
          Itm->SubItems->Strings[2]=DateToStr(SrcItm->BackDate);
          }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::AddToListViewSync()
{
        TListItem *itm=SpisListView->Items->Add();
        FillListViewItem(itm,AddToVisualListItem);
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::AddToListView(TBorrowItem *Item)
{
  if (Item==NULL) return;
  __try
    {
      //no i dodajemy element...
      AddToVisualListItem=Item;
      Synchronize(AddToListViewSync);
    }
   __finally
    {
    }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::ShowHistoryInBorrowGrid()
{
  if (SpisStringGrid==NULL) return;
  __try
      {
      //Synchronize(ClearSpisStringList);
      for (int cntr=0;cntr<BorrowList->Count;cntr++)
        {
        //TBorrowItem *Item=BorrowList->Items[cntr];
        if ((CancelAction)||(Terminated))
          {
          return;
          }
          //AddToBorrowGrid(*Item);
        }
      }
    __except(EXCEPTION_EXECUTE_HANDLER)
      {
      SaveMode=bsrDisconnect;
      MessageBox(Synchronizer->MainForm->Handle,"B��d podczas odswierzania tabeli b�dacej wizualizacj� struktury po�ycze� (tryb historii).","Zwal se !!!",MB_OK | MB_ICONERROR);
      }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::ShowHistoryInListView()
{
  __try
      {
      Synchronize(ClearSpisListView);
      SpisListView->OwnerData=true;
      SpisListView->Items->Count=BorrowList->Count;
      SpisListView->OnData=HistoryOnData;
      SpisListView->OnDataFind=HistoryOnDataFind;
      SpisListView->OnDataHint=HistoryOnDataHint;
      SpisListView->OnDataStateChange=HistoryOnDataStCh;
/*
      for (int cntr=BorrowList->Count-1;cntr>=0;cntr--)
        {
        TBorrowItem *Item=BorrowList->Items[cntr];
        if ((CancelAction)||(Terminated)) return;
        AddToListView(Item);
        }
*/
      }
    __except(EXCEPTION_EXECUTE_HANDLER)
      {
      SaveMode=bsrDisconnect;
      MessageBox(Synchronizer->MainForm->Handle,"B��d podczas odswierzania tabeli b�dacej wizualizacj� struktury po�ycze� (tryb historii).","Zwal se !!!",MB_OK | MB_ICONERROR);
      }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::ShowPersonInListView()
{
  __try
      {
      Synchronize(ClearSpisListView);
      SpisListView->OwnerData=false;
      SpisListView->OnData=NULL;
      SpisListView->OnDataFind=NULL;
      SpisListView->OnDataHint=NULL;
      SpisListView->OnDataStateChange=NULL;
      for (int cntr=0;cntr<BorrowList->Count;cntr++)
        {
        TBorrowItem *Item=BorrowList->Items[cntr];
        if ((CancelAction)||(Terminated))
          {
          return;
          }
        if (Item->Name==PartialViewItemName)
          AddToListView(Item);
        }
      }
    __except(EXCEPTION_EXECUTE_HANDLER)
      {
      SaveMode=bsrDisconnect;
      MessageBox(Synchronizer->MainForm->Handle,"B��d podczas odswierzania tabeli b�dacej wizualizacj� struktury po�ycze�. (tryb osoby).","Zwal se !!!",MB_OK | MB_ICONERROR);
      }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::ShowElementInListView()
{
  __try
      {
      Synchronize(ClearSpisListView);
      SpisListView->OwnerData=false;
      SpisListView->OnData=NULL;
      SpisListView->OnDataFind=NULL;
      SpisListView->OnDataHint=NULL;
      SpisListView->OnDataStateChange=NULL;
      for (int cntr=0;cntr<BorrowList->Count;cntr++)
        {
        TBorrowItem *Item=BorrowList->Items[cntr];
        if ((CancelAction)||(Terminated))
          {
          return;
          }
        if (Item->Items==PartialViewItemName)
          AddToListView(Item);
        }
      }
    __except(EXCEPTION_EXECUTE_HANDLER)
      {
      SaveMode=bsrDisconnect;
      MessageBox(Synchronizer->MainForm->Handle,"B��d podczas odswierzania tabeli b�dacej wizualizacj� struktury po�ycze�. (tryb elementu).","Zwal se !!!",MB_OK | MB_ICONERROR);
      }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::ShowUnreturnedInListView()
{
  __try
    {
      Synchronize(ClearSpisListView);
      SpisListView->OwnerData=false;
      SpisListView->OnData=NULL;
      SpisListView->OnDataFind=NULL;
      SpisListView->OnDataHint=NULL;
      SpisListView->OnDataStateChange=NULL;
    for (int cntr=BorrowList->Count-1;cntr>=0;cntr--)
      {
      if ((CancelAction)||(Terminated)) return;
      //�apiemy wska�nik na element
      TBorrowItem *Item=BorrowList->Items[cntr];
      //Zobaczmy, czy ten element op�aca si� w og�le sprawdza�
      //tzn. czy nie reprezentuje zwrotu i czy nie zaznaczono �e jest oddany
      if ((!Item->Back)&&(Item->ReturnEntry<0))
        {
        //czy element na pewno jest po�yczony i nie oddany?
        if (IsBorrowed(Item->Items))
          {
          int StrGridIndex=ListItemsIndexOfSub(0,Item->Items,SpisListView->Items);
          //dodanie elementu
          if (StrGridIndex<0)
            AddToListView(Item);
          }
         //Je�eli jest ju� oddany, warto to zaznaczy�
         else
          Item->ReturnEntry=GetReturnEntryIndex(Item);
        }
      }
    NeedUpdate=nuNone;
    }
  __except(EXCEPTION_EXECUTE_HANDLER)
    {
    SaveMode=bsrDisconnect;
    MessageBox(Synchronizer->MainForm->Handle,"B��d podczas odswierzania tabeli b�dacej wizualizacj� struktury po�ycze�. (tryb nieoddanych).","Zwal se !!!",MB_OK | MB_ICONERROR);
    }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::ShowLostInListView()
{
  __try
      {
      Synchronize(ClearSpisListView);
      SpisListView->OwnerData=false;
      SpisListView->OnData=NULL;
      SpisListView->OnDataFind=NULL;
      SpisListView->OnDataHint=NULL;
      SpisListView->OnDataStateChange=NULL;
      for (int cntr=0;cntr<BorrowList->Count;cntr++)
        {
        TBorrowItem *Item=BorrowList->Items[cntr];
        if ((CancelAction)||(Terminated))
          {
          return;
          }
        if ((Item->Name.UpperCase()==LostNameStr.UpperCase())||(Item->Name.UpperCase()==DamagNameStr.UpperCase()))
          AddToListView(Item);
        }
      }
    __except(EXCEPTION_EXECUTE_HANDLER)
      {
      SaveMode=bsrDisconnect;
      MessageBox(Synchronizer->MainForm->Handle,"B��d podczas odswierzania tabeli b�dacej wizualizacj� struktury po�ycze�. (tryb osoby).","Zwal se !!!",MB_OK | MB_ICONERROR);
      }
}
//---------------------------------------------------------------------------
int __fastcall TBorrowSystem::GetReturnEntryIndex(TBorrowItem *Item)
{
int StartIndex;
  StartIndex=BorrowList->IndexOf(Item);
  if ((StartIndex < 0) || (StartIndex+1 > BorrowList->Count-1)) return -1;
  for (int cntr=StartIndex+1;cntr<BorrowList->Count;cntr++)
    if ((BorrowList->Items[cntr]->Items==Item->Items) && (BorrowList->Items[cntr]->Back==true))
      {
      return cntr;
      }
  return -1;
}
//---------------------------------------------------------------------------
TBorrowItem* __fastcall TBorrowSystem::MakeItemFromText(String istr)
// Tworzy Itema kt�rego mo�na doda� do listy
{
   TBorrowItem *CItem=new TBorrowItem;
   CItem->Date=StrToDate(GetTabParam(3,istr));
   CItem->Items=GetTabParam(5,istr);
   if (AnsiUpperCase(GetTabParam(1,istr))=="Z")
     {
     CItem->Back=true;
     CItem->Name=FindWhoHas(CItem->Items);
     CItem->Condition=StrToInt(GetTabParam(4,istr));
     CItem->BackDate=0;
     }
    else
     {
     CItem->Back=false;
     CItem->Condition=0;
     CItem->Name=GetTabParam(2,istr);
     CItem->BackDate=StrToDate(GetTabParam(4,istr));
     }
   CItem->ReturnEntry=-1;
   return CItem;
}
//---------------------------------------------------------------------------
String __fastcall TBorrowSystem::MakeTextFromItem(const TBorrowItem *iitm)
{
  String str;
  if (iitm->Back)
    str="Z\t?\t"+DateToStr(iitm->Date)+"\t"+IntToStr(iitm->Condition)+"\t"+iitm->Items;
  else
    str="P\t"+iitm->Name+"\t"+DateToStr(iitm->Date)+"\t"+DateToStr(iitm->BackDate)+"\t"+iitm->Items;
  return str;
}
//---------------------------------------------------------------------------
String __fastcall TBorrowSystem::FindWhoHas(String itm)
//Funkcja pomocnicza, szuka osoby kt�ra po�yczy�a element
{
   for (int cntr=BorrowList->Count-1;cntr>=0;cntr--)
     {
     TBorrowItem *CItem=BorrowList->Items[cntr];
      if (CItem!=NULL)
        if (CItem->Items==itm)
          {
          return CItem->Name;
          }
     }
   return "?";
}
//---------------------------------------------------------------------------
bool __fastcall TBorrowSystem::IsBorrowed(String itm)
//Sprawdza czy element jest po�yczony czy zwr�cony
{
  bool Borrowed=false;
  for (int cntr=BorrowList->Count-1;cntr>=0;cntr--)
    {
    TBorrowItem *PSItem=BorrowList->Items[cntr];
      if ((PSItem!=NULL)&&(PSItem->Items==itm))
        {
        Borrowed=!PSItem->Back;
        break;
        }
    }
  return Borrowed;
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::HistoryOnData(TObject *Sender, TListItem *Item)
{
  //OnData gets called once for each item for which the ListView needs
  //data. If the ListView is in Report View, be sure to add the subitems.
  //Item is a "dummy" item whose only valid data is it's index which
  //is used to index into the underlying data.
  if (Item->Index >= BorrowList->Count) return;
  FillListViewItem(Item,BorrowList->Items[Item->Index]);
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::HistoryOnDataFind(TObject *Sender,TItemFind Find, const AnsiString FindString,
      const TPoint &FindPosition, Pointer FindData, int StartIndex,TSearchDirection Direction, bool Wrap, int &Index)
{
  //OnDataFind gets called in response to calls to FindCaption, FindData,
  //GetNearestItem, etc. It also gets called for each keystroke sent to the
  //ListView (for incremental searching)
  int I = StartIndex;
  if ((Find==ifExactString) || (Find==ifPartialString))
    {
    bool Found=false;
    do
      {
      if (I >= BorrowList->Count-1)
        {
        if (Wrap) I=0; else break;
        }
      TBorrowItem *PSItem=BorrowList->Items[I];
      Found = (UpperCase(PSItem->Name)).Pos(UpperCase(FindString)) == 1;
      I++;
      }
     while (!Found && (I!=StartIndex));
    if (Found) Index = I-1;
    }
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::HistoryOnDataHint(TObject *Sender,int StartIndex, int EndIndex)
{
/*
  if ((StartIndex>=BorrowList->Count) || (EndIndex>=BorrowList->Count)) return;
  for (int I=StartIndex; I<=EndIndex; I++)
    {
    }
*/
}
//---------------------------------------------------------------------------
void __fastcall TBorrowSystem::HistoryOnDataStCh(TObject *Sender,int StartIndex, int EndIndex, TItemStates OldState,TItemStates NewState)
{

}
//---------------------------------------------------------------------------
