//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "KatalogCForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "Spin"
#pragma resource "*.dfm"
TKatalog *Katalog;
//---------------------------------------------------------------------------
__fastcall TKatalog::TKatalog(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::FormCreate(TObject *Sender)
{
 BorrowSystem=NULL;
 EditSystem=NULL;
 Synchronizer=new TSynchronizer(Katalog);
 Synchronizer->LoadOptions();
 if (ScaleComponentsCheckBox->Checked)
    Synchronizer->ScaleComponents(StrToIntDef(ScaleFactor->Text,100));
 Synchronizer->CreateKat();
 SearchSystem=new TKatSearcher(Synchronizer);
 SearchSystem->MakeTaskCountNum();
 SearchSystem->UpdateKatHeight();
 MainPageControlChange(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::MainPageControlChange(TObject *Sender)
{
  int Num;
  String Str;
 switch (MainPageControl->ActivePage->TabIndex)
   {
   case 0:
     //Teraz w¹tki s¹ poprawione i same robi¹ suspend
     Synchronizer->UpdateStatusBar("Opcje wyszukiwania aktywne.");
     while (SearchSystem->Suspended)
         SearchSystem->Resume();
     break;
   case 1:
     if (EditSystem==NULL)
       {
       EditSystem=new TSpisEditor(Synchronizer);
       CreateDiskDriveList();
       };
     if (EditSystem==NULL) Synchronizer->UpdateStatusBar("B³¹d inicjacji edytora.");
      else Synchronizer->UpdateStatusBar("Zak³adka modyfikacji uaktywniona.");
     Str="";
     Num=SpisCD->ItemIndex;
     if ((Num>=0)&&(Num<SpisCD->Items->Count))
         Str=SpisCD->Items->Strings[Num];
     EdNumber->Text=Str;
     break;
   case 2:
     if (SearchSystem!=NULL)
       if (!SearchSystem->Suspended) SearchSystem->Suspend();
     Synchronizer->UpdateStatusBar("Zmiana ustawieñ programu dopuszczalna.");
     break;
   case 3:
     if (BorrowSystem==NULL)
       {
       PPoComboBox->Text=PozyczDefaultText;
       if (FileExists(Synchronizer->SpisPath+BorrowFileName))
         {
         BorrowSystem=new TBorrowSystem(Synchronizer);
         BorrowSystem->Init();
         BorrowSystem->Resume();
         UpdateAnyBorrowLists();
         }
        else
         {
         Str="Plik tekstowy po¿yczek nie istnieje (.";
         Str+=BorrowFileName;Str+=")";
         MessageBox(Handle,Str.c_str(),"Odpierdol siê!",MB_OK|MB_ICONWARNING|MB_APPLMODAL);
         };
       if (BorrowSystem!=NULL)
           Synchronizer->UpdateStatusBar("System monitorowania po¿yczek uaktywniony");
         else
           Synchronizer->UpdateStatusBar("System monitorowania po¿yczek nieaktywny");
       }
      else //if (BorrowSystem==NULL)
       {
       Synchronizer->UpdateStatusBar("Aktywny system monitorowania po¿yczek.");
       if (BorrowSystem->Suspended) BorrowSystem->Resume();
       };
     PSpisPanelResize(Sender);
     break;
   case 4:
     Synchronizer->UpdateStatusBar("Zak³adka debugowania uaktywniona");
     break;
   case 5:
     Synchronizer->UpdateStatusBar("Panel informacyjny uaktywniony");
     HelpButtonClick(Sender);
     HelpProgramNamePanel->Realign();
     break;
   };
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::CreateDiskDriveList()
{
 __try {
  //Szukamy dysku
  String Drive="X:\\";
  for (Drive[1]='a';Drive[1]<='z';Drive[1]++)
    {
    unsigned int Typ=GetDriveType(Drive.c_str());
    //Jezeli jest to nosnik wymienny lub sieciowy
    if (Typ==DRIVE_REMOVABLE)
        {
        DrvSelector->Items->Add(Drive.SubString(1,2)+" [wymienny]");
        }
      else
    if (Typ==DRIVE_CDROM)
        {
        DrvSelector->Items->Add(Drive.SubString(1,2)+" [cd-rom]");
        if (DrvSelector->Text.Length()<2)
            DrvSelector->Text=DrvSelector->Items->Strings[DrvSelector->Items->Count-1];
        }
      else
    if (Typ==DRIVE_REMOTE)
        {
        DrvSelector->Items->Add(Drive.SubString(1,2)+" [sieciowy]");
        };
    };
 }__except(EXCEPTION_EXECUTE_HANDLER)
 {
  MessageBox(Handle,"Podczas przegl¹dania napêdów wyst¹pil wyj¹tek. Nie wszystkie napêdy bêd¹ widoczne na liœcie.","Wyj¹tek przy listowaniu napêdów",MB_OK|MB_APPLMODAL|MB_ICONWARNING);
 };
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::HelpButtonClick(TObject *Sender)
{
  String HlpFile;
  if (HelpDescriptionButton->Down)
      {
      HlpFile=Synchronizer->SpisPath+DescriptFile;
      }
    else
      {
      HlpFile=Synchronizer->SpisPath+VersionFile;
      };
  if (FileExists(HlpFile))
    {
    HelpRichEdit->Lines->LoadFromFile(HlpFile);
    Synchronizer->UpdateStatusBar("Plik za³adowany: "+HlpFile);
    }
   else
     {
     HelpRichEdit->Clear();
     HelpRichEdit->Lines->Add("Help file not found.");
     Synchronizer->UpdateStatusBar("Nie za³adowano: "+HlpFile);
     };
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::PSpisPanelResize(TObject *Sender)
{
  int CCntr;
  int Scale;
  if (ScaleComponentsCheckBox->Checked)
    Scale=StrToIntDef(ScaleFactor->Text,100);
   else
    Scale=100;
  Scale;//¿eby warninga nie wyœwietla³
}
//---------------------------------------------------------------------------
void TKatalog::UpdateAnyBorrowLists()
{
  if (BorrowSystem==NULL) return;
  //BorrowSystem->MakeTaskUpdateListView(PMainListView);
  BorrowSystem->SpisListView=PMainListView;
  BorrowSystem->UpdateListView();
  //if (BorrowSystem->Suspended) BorrowSystem->Resume();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::SpisCDKeyPress(TObject *Sender, char &Key)
{
 SearchSystem->MakeTaskReadFileView();
 while (SearchSystem->Suspended) SearchSystem->Resume();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::SpisCDClick(TObject *Sender)
{
 if (SpisCD->ItemIndex>=0)
   SpisItemSelector->Text=SpisCD->Items->Strings[SpisCD->ItemIndex];
 SearchSystem->MakeTaskReadFileView();
 while (SearchSystem->Suspended) SearchSystem->Resume();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::FindKeyPress(TObject *Sender, char &Key)
{
  if (Key==13)
      {
      SearchSystem->MakeTaskSearchCD();
      Key=0;
      };
  while (SearchSystem->Suspended) SearchSystem->Resume();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::EdRestoreBtnClick(TObject *Sender)
{
  String FileName;
  TStringList *StrList;
 __try
  {
  Editor->Hide();
  EditorMemoPanel->Repaint();
  StrList=new TStringList();
  __try
    {
    FileName=Synchronizer->SpisSubPath+EdNumber->Text+".txt";
     if ((EdNumber->Text.Length()==NameDigits)&&(FileExists(FileName)))
       {
       StrList->LoadFromFile(FileName);
       StrList->Text=Synchronizer->WinCPConvertLine(StrList->Text);
       //Synchronizer->ConvertStrList2W(StrList);
       Editor->Lines->Clear();
       Editor->Lines->AddStrings(StrList);
       Synchronizer->UpdateStatusBar("Plik za³adowany do edycji: "+FileName);
       }
      else
       {
       Synchronizer->UpdateStatusBar("Plik nie za³adowany: "+FileName);
       MessageBox(Handle,"Wpisa³eœ z³¹ nazwê lub plik nie istnieje.","Zrobiæ ci z dupy jesieñ œredniowiecza?",MB_OK|MB_ICONERROR|MB_APPLMODAL);
       }
    }
  __except(EXCEPTION_EXECUTE_HANDLER)
    {
    Synchronizer->UpdateStatusBar("Wyj¹tek przy ³adowaniu pliku.");
    MessageBox(Handle,"Podczas otwierania pliku wyst¹pi³ wyj¹tek. Funkcja wczytuj¹ca przerwa³a dzia³anie.","Ty stary zbiczeñcu",MB_OK|MB_ICONWARNING|MB_APPLMODAL);
    };
  delete StrList;
  }
 __finally
  {
  Editor->Show();
  };
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::EdSaveBtnClick(TObject *Sender)
{
  String FileName;
  String MessageStr;
  long FileNum;
  long MaxFileNum;
  TStringList *StrList;
// Editor->PlainText=true;
 __try
   {
   StrList=new TStringList();
   FileName=Synchronizer->SpisPath+SpisFolder+"\\"+EdNumber->Text+".txt";
   FileNum=StrToIntDef(EdNumber->Text,-1);
   if (FileExists(FileName))
     {
     MessageStr="Czy zast¹piæ aktualny plik "+FileName+" ?";
     if (MessageBox(Handle,MessageStr.c_str(),"Ej, uwa¿aj bo jak ci pierdolnê...",MB_YESNO|MB_ICONQUESTION|MB_APPLMODAL) != IDYES)
         return;
     };
   if ((EdNumber->Text.Length()!=NameDigits)||(FileNum<0))
     {
       MessageBox(Handle,"Poda³eœ b³êdn¹ nazwê. Nazw¹ musz¹ byæ cztery cyfry.","Co ty dziœ bra³eœ?",MB_OK|MB_ICONWARNING|MB_APPLMODAL);
       return;
     };
   }
  __except(EXCEPTION_EXECUTE_HANDLER)
    {
    MessageBox(Handle,"Podczas przygotowywania do zapisu pliku wyst¹pi³ wyj¹tek. Funkcja zapisuj¹ca przerwa³a dzia³anie.","Co ty dziœ bra³eœ?",MB_OK|MB_ICONERROR|MB_APPLMODAL);
    return;
    };
 __try
  {
  Editor->Hide();
  EditorMemoPanel->Repaint();
  StrList->AddStrings(Editor->Lines);
  StrList->Text=Synchronizer->DosCPConvertLine(StrList->Text);
//  Synchronizer->ConvertStrList2D(StrList);
  StrList->SaveToFile(FileName);
  MaxFileNum=StrToIntDef(LimitHigh->Text,0);
  if (MaxFileNum+1==FileNum)
      LimitHigh->Text=IntToStr(MaxFileNum+1);
  Synchronizer->UpdateStatusBar("Zapis powiód³ siê.");
  }
 __finally
  {
  delete StrList;
  Editor->Show();
  };
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::AnalyseBtnClick(TObject *Sender)
{
 if (EditSystem!=NULL)
   __try
    {
    if (EditSystem->DiscAnalyzer!=NULL)
      {
      EditSystem->DiscAnalyzer->Terminate=true;
      return;
      }
//    Editor->Hide();
//    EditorMemoPanel->Repaint();
    EdNumber->Text=IntToStr(StrToIntDef(LimitHigh->Text,0)+1);
    while (EdNumber->Text.Length()<NameDigits) EdNumber->Text="0"+EdNumber->Text;

    EditSystem->Function=etfCreateList;
    EditSystem->FuncParamList=Editor->Lines;
    EditSystem->Resume();
//    EditSystem->CreateList(Sender,Editor->Lines);
    }
   __finally
    {
//    Editor->Show();
    };
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::FShowAllBtnClick(TObject *Sender)
{
  SearchSystem->MakeTaskCountNum();
  while (SearchSystem->Suspended) SearchSystem->Resume();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::FExportBtnClick(TObject *Sender)
{
 TExportDialog *ExDialog=new TExportDialog(Katalog);
 ExDialog->DefaultExt="txt";
 __try
   {
   if (ExDialog->Execute())
     __try
      {
      if (ExDialog->ExpCurrent->Checked)
        {
         if (ExDialog->FilterIndex==1)
           {
           SearchViewer->PlainText=true;
           SearchViewer->Lines->SaveToFile(ExDialog->FileName);
           SearchViewer->PlainText=false;
           Synchronizer->UpdateStatusBar("Eksport do pliku TXT powiód³ siê.");
           }
         else
           {
           SearchViewer->PlainText=false;
           SearchViewer->Lines->SaveToFile(ExDialog->FileName);
           Synchronizer->UpdateStatusBar("Eksport do pliku RTF powiód³ siê.");
           };
        }
      else
       {
       //Tu procedury gdy jest zaznaczony zapis z wyszukiwania
       MessageBox(Handle,"NIE ZROBIONE JESZCZE!!!","@#@#%$#$%$^#$$$%^&$%$@",MB_OK|MB_ICONSTOP|MB_APPLMODAL);
       Synchronizer->UpdateStatusBar("Eksport do pliku nie uda³ siê.");
       };
      }
    __except(EXCEPTION_EXECUTE_HANDLER)
      {
      MessageBox(Handle,"Operacja nie powiod³a siê. Program nie mo¿e odczytaæ danych lub zapisaæ so pliku.","Coœ narobi³ piepszony skurwielu !?",MB_OK|MB_ICONSTOP|MB_APPLMODAL);
      Synchronizer->UpdateStatusBar("Wyj¹tek podczas eksportu.");
      };
  }
 __finally
  {
  delete ExDialog;
  };
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::ViewerWordWraperClick(TObject *Sender)
{
 SearchViewer->WordWrap=ViewerWordWraper->Checked;
 ItemViewer->WordWrap=ViewerWordWraper->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::EditorWordWraperClick(TObject *Sender)
{
 Editor->WordWrap=EditorWordWraper->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::SmallFontButtonClick(TObject *Sender)
{
  SmallFont->Execute();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::NormFontButtonClick(TObject *Sender)
{
  NormFont->Execute();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::KatPanelResize(TObject *Sender)
{
 if (SearchSystem!=NULL)
   SearchSystem->UpdateKatHeight();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::OSavePrefButtonClick(TObject *Sender)
{
 Synchronizer->SaveOptions();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::OClearPrefButtonClick(TObject *Sender)
{
  Synchronizer->RemoveSavedOptions();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::SearchSpdButtonClick(TObject *Sender)
{
  SearchSystem->MakeTaskSearchCD();
  while (SearchSystem->Suspended) SearchSystem->Resume();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::PPoEditKeyPress(TObject *Sender, char &Key)
{
  if (Key==13)
      {
      PPoButtonClick(Sender);
      if (PPoEdit->Text.Length()<1)
          Key=0;
      }
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::FormDestroy(TObject *Sender)
{
  if (BorrowSystem!=NULL)
    {
    BorrowSystem->Suspend();
    BorrowSystem->SaveToDisk();
    BorrowSystem->DeInit();
    BorrowSystem->WaitFor();
    delete BorrowSystem;
    BorrowSystem=NULL;
    };
  if (SearchSystem!=NULL)
    {
    delete SearchSystem;
    };
  if (EditSystem!=NULL)
    {
    delete EditSystem;
    };
  if (Synchronizer!=NULL)
    {
    delete Synchronizer;
    };
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::KatBoxClick(TObject *Sender)
{
  int katCounter;
  for (katCounter=0;katCounter<KatBox->ControlCount;katCounter++)
    ((TSpeedButton *)(KatBox->Controls[katCounter]))->Down=!(((TSpeedButton *)(KatBox->Controls[katCounter]))->Down);
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::KatBoxDblClick(TObject *Sender)
{
  int katCounter;
  for (katCounter=0;katCounter<KatBox->ControlCount;katCounter++)
    ((TSpeedButton *)(KatBox->Controls[katCounter]))->Down=false;
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::PPoButtonClick(TObject *Sender)
{
  if (BorrowSystem!=NULL)
   {
    if (PPoComboBox->Text!=PozyczDefaultText)
       {
      if (BorrowSystem->DoPozycz(Sender,PPoComboBox->Text,PPoEdit->Text,PBackEdit->Value))
        PPoEdit->Text="";
      UpdateAnyBorrowLists();
       }
     else
       {
       MessageBox(Handle,"Nie wybra³eœ osoby. Wybierz kogoœ w polu, które ciê o to prosi.","Jestem odpadem atomowym ...",MB_OK|MB_ICONINFORMATION|MB_APPLMODAL);
       Synchronizer->UpdateStatusBar("No wybierz osobê s³onko...");
       };
   } else
    {
    MessageBox(Handle,"System po¿yczek nie jest zainicjowany. Nie mo¿na rozpocz¹æ operacji.","Popierdoli³o ciê?",MB_OK|MB_ICONERROR|MB_APPLMODAL);
    Synchronizer->UpdateStatusBar("System po¿yczek nie zainicjowany.");
    }
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::PZwEditKeyPress(TObject *Sender, char &Key)
{
  if (Key==13)
      {
      PZwButtonClick(Sender);
      if (PZwEdit->Text.Length()<1)
          Key=0;
      }
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::PZwButtonClick(TObject *Sender)
{
  int Condition;
  if (PbdbRadioButton->Checked) Condition=5; else
  if (PdbRadioButton->Checked)  Condition=4; else
  if (PdstRadioButton->Checked) Condition=3; else
  if (PzlyRadioButton->Checked) Condition=2; else
   Condition=0;
  if (BorrowSystem!=NULL)
    {
      if (BorrowSystem->DoZwroc(Sender,PZwEdit->Text,Condition))
          PZwEdit->Text="";
      UpdateAnyBorrowLists();
    }
   else
     MessageBox(Handle,"System po¿yczek nie jest zainicjowany. Nie mo¿na rozpocz¹æ operacji.","Kurwa dasz w koñcu spokój?",MB_OK|MB_ICONERROR|MB_APPLMODAL);
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::PVSpeedButtonClick(TObject *Sender)
{
  TBorrVisMode Mode;
  PVPanel1->Visible=(PVESpeedButton->Down||PVPSpeedButton->Down);
  if (PVHSpeedButton->Down) Mode=gmHistory;    else
  if (PVESpeedButton->Down) Mode=gmElement;    else
  if (PVPSpeedButton->Down) Mode=gmPerson;     else
  if (PVZSpeedButton->Down) Mode=gmUnreturned; else
  if (PVLSpeedButton->Down) Mode=gmLost;       else
    Mode=gmHistory;
  if (BorrowSystem!=NULL)
    {
    BorrowSystem->ChangeViewMode(Mode,PVEEdit->Text);
    PVEEditEnter(Sender);
    UpdateAnyBorrowLists();
    };

}
//---------------------------------------------------------------------------
void __fastcall TKatalog::PVEEditKeyPress(TObject *Sender, char &Key)
{
  if (Key==13) PVSpeedButtonClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::AnalyseBtnOldClick(TObject *Sender)
{
 if (EditSystem!=NULL)
   __try
    {
    Editor->Hide();
    EditorMemoPanel->Repaint();
    EdNumber->Text=IntToStr(StrToIntDef(LimitHigh->Text,0)+1);
    while (EdNumber->Text.Length()<NameDigits) EdNumber->Text="0"+EdNumber->Text;
    EditSystem->CreateListOld(Editor->Lines);
    }
   __finally
    {
    Editor->Show();
    };
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::EdFindButtonClick(TObject *Sender)
{
  EdFindDialog->Execute();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::EdFindEvent(TObject *Sender)
{
  EditSystem->DoFindInRichEdit((TFindDialog *)Sender,Editor);
//  EditSystem->DoFindInMemo((TFindDialog *)Sender,Editor);
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::EdReplaceEvent(TObject *Sender)
{
  EditSystem->DoReplaceInRichEdit((TReplaceDialog *)Sender,Editor);
  //EditSystem->DoReplaceInMemo((TReplaceDialog *)Sender,Editor);
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::EdReplaceButtonClick(TObject *Sender)
{
  EdReplaceDialog->Execute();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::EdCaseButtonClick(TObject *Sender)
{
  EditSystem->ChangeSelectionCase(Sender,Editor);
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::PPoComboBoxEnter(TObject *Sender)
{
    if (BorrowSystem==NULL) return;
    BorrowSystem->UpdateItemLists();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::PVEEditEnter(TObject *Sender)
{
  if (BorrowSystem!=NULL) BorrowSystem->UpdateItemLists();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::ShowSearchResultsBtnClick(TObject *Sender)
{
  if (ShowSearchResultsBtn->Down)
    SearchSystem->MakeSearcherVisible();
   else
    SearchSystem->MakeItmViewerVisible();
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::StatusBarFullDrawPanel(TStatusBar *StatusBar,TStatusPanel *Panel, const TRect &Rect)
{
  ProgressBar->PaintTo(StatusBar->Canvas->Handle,Rect.Left,Rect.Top);
}
//---------------------------------------------------------------------------


void __fastcall TKatalog::SpisItemSelectorKeyPress(TObject *Sender,char &Key)
{
  if (Key==VK_RETURN)
    {
    String ItemText=SpisItemSelector->Text;
    if (ItemText==NULL) return;
    while (ItemText.Length()<NameDigits) ItemText="0"+ItemText;
    if (ItemText.Length()>NameDigits) return;
    int ItemIndex=SpisCD->Items->IndexOf(ItemText);
    if (ItemIndex<0) return;
    SpisCD->Selected[ItemIndex]=true;
    Key=0;
    SpisCDClick(NULL);
    }
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::DebGenStatButtonClick(TObject *Sender)
{
  if (Synchronizer!=NULL)
      Synchronizer->GenerateDebugStats();
  if (SearchSystem!=NULL)
      SearchSystem->GenerateDebugStats();
}
//---------------------------------------------------------------------------

void __fastcall TKatalog::SearchSpdButtonMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Synchronizer!=NULL)
    Synchronizer->SetSearchAddKeyParams(Shift);
}
//---------------------------------------------------------------------------
void __fastcall TKatalog::TyperKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if (Synchronizer!=NULL)
    Synchronizer->SetSearchAddKeyParams(Shift);
}
//---------------------------------------------------------------------------

