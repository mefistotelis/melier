//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Searcher.h"
#include "KatalogCForm.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
const int MinItemNum=1;
const int MaxCategoryNameLen=32;
const unsigned int MaxErrorList=32;
const char* ConvFormat="%04li";  // czwórka oznacza wartoœæ NameDigits
//---------------------------------------------------------------------------
__fastcall TKatSearcher::TKatSearcher(TSynchronizer *nSynchronizer)
        : TThread(true)
{
    // Ustawiam thread
    Priority = tpLowest;
    FreeOnTerminate = false;
    CancelAction=false;
    Working=false;
    ActiveTask=stskNone;
    NewTask=stskNone;
    Busy=false;
    //Pocz¹tkowe wartosci propertiesow
  FullListInSpisCD=false;
    // no i pozosta³e
  Synchronizer=nSynchronizer;
  ItmBuffer=new TSearcherItmBuffer();
  FormattedReslt=new TMemoryStream();
  ErrorList=new TStringList();
  AddErrorNum=0;
}
//---------------------------------------------------------------------------
__fastcall TKatSearcher::~TKatSearcher()
{
  Busy=true;
  Priority=tpNormal;
  delete ItmBuffer;
  ItmBuffer=NULL;
  delete FormattedReslt;
  FormattedReslt=NULL;
  delete ErrorList;
  ErrorList=NULL;
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::MakeTaskSearchCD()
// Publiczna funkcja do wyszukiwania
{
  Busy=true;
  if (Working)
    if ((ActiveTask==stskSearch)||(ActiveTask==stskReadFile)) CancelAction=true;
  NewTask=stskSearch;
  Busy=false;
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::MakeTaskReadFileView()
// Publiczna funkcja do czytania pliku
{
  Busy=true;
  if (Working)
    {
    if ((ActiveTask==stskSearch)||(ActiveTask==stskReadFile))
        CancelAction=true;
      else
        return; //Podczas innych zdarzeñ nie mo¿na czytaæ pliku
    }
  int NumIdx=Synchronizer->SpisCD->ItemIndex;
  if ((NumIdx<0)||(NumIdx>Synchronizer->SpisCD->Items->Count)) return;
  ItemStr=Synchronizer->SpisCD->Items->Strings[NumIdx];
  ItemDestination=Synchronizer->ItemViewer;
  NewTask=stskReadFile;
  Busy=false;
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::MakeTaskReadFileEdit()
// Publiczna funkcja do czytania pliku
{
  Busy=true;
  if (Working)
    if ((ActiveTask==stskSearch)||(ActiveTask==stskReadFile)) CancelAction=true;
  ItemStr=Katalog->EdNumber->Text;
  if (ItemStr.Length()!=NameDigits) return;
  ItemDestination=Synchronizer->Editor;
  NewTask=stskReadFile;
  Busy=false;
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::MakeTaskCountNum()
// Publiczna funkcja do wyliczania plyt
{
  Busy=true;
  if (Working)
    if ((ActiveTask==stskSearch)||(ActiveTask==stskReadFile)||(ActiveTask==stskCountNum)) CancelAction=true;
  NewTask=stskCountNum;
  Busy=false;
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::Execute()
// G³ówna funkcja w¹tku
{
 do
  {
  try
    {
    // System usuwania b³êdów i szczególnych sytuacji
    if ((Working)||(CancelAction))
      {
      Working=false;
      TmpTask=stskNone;
      Synchronize(StartNewTask);
      CancelAction=false;
      }
    // Jak nie ma zleceñ to mo¿na sie uspic
    if (NewTask==stskNone) Suspend();
    // No i normalne wykonywanie poleceñ
    if (!Busy)
      {
      TmpTask=NewTask;
      NewTask=stskNone;
      Synchronize(StartNewTask);
      Priority = tpNormal;
      Working=true;
      switch (ActiveTask)
        {
        case stskSearch:
            try
              {
              SearchItemRange();
              TmpTask=stskNone;
              }
            catch (Exception &E)
             {
             if ((ErrorList!=NULL)&&(ErrorList->Count<MaxErrorList))
                 ErrorList->Add("Wyst¹pi³ b³¹d (wyj¹tek) podczas wyszukiwania w bazie danych. Oryginalny komunikat b³êdu: "+E.Message);
               else
                 AddErrorNum++;
             };break;
        case stskReadFile:
          try
            {
            Synchronize(ReadFile);
            TmpTask=stskNone;
            }
           catch (Exception &E)
            {
             if ((ErrorList!=NULL)&&(ErrorList->Count<MaxErrorList))
                 ErrorList->Add("Wyst¹pi³ b³¹d (wyj¹tek) podczas wyœwietlania pliku bazy danych. Oryginalny komunikat b³êdu: "+E.Message);
               else
                 AddErrorNum++;
            };break;
        case stskCountNum:
          try
            {
            CountNum();
            TmpTask=stskNone;
            }
           catch (Exception &E)
            {
             if ((ErrorList!=NULL)&&(ErrorList->Count<MaxErrorList))
                 ErrorList->Add("Wyst¹pi³ b³¹d (wyj¹tek) podczas wyliczania iloœci plików w bazie danych. Oryginalny komunikat b³êdu: "+E.Message);
               else
                 AddErrorNum++;
            };break;
          }
        }//end if

       if (AddErrorNum>0)
         {
         if (ErrorList!=NULL)
             ErrorList->Add("...\n(jest jeszcze "+IntToStr(AddErrorNum)+" nastêpnych b³êdów)\n");
           else
             MessageBox(Synchronizer->MainForm->Handle,"Nast¹pi³a seria b³êdów, obs³uga b³êdów jednak nie dzia³a i nie da siê odtworzyæ informacji o b³êdach.","Modu³ Searcher siê wypierdoli³",MB_OK | MB_ICONERROR | MB_APPLMODAL);
         }
       AddErrorNum=0;
       if ((ErrorList!=NULL)&&(ErrorList->Count>0))
         {
         MessageBox(Synchronizer->MainForm->Handle,ErrorList->Text.c_str(),"Modu³ Searcher siê zjeba³",MB_OK | MB_ICONERROR | MB_APPLMODAL);
         ErrorList->Clear();
         }
      Synchronize(StartNewTask);
      Working=false;
      CancelAction=false;
      Priority = tpIdle;
    }
  catch (Exception &E)
    {
    String tmpStr="Wyst¹pi³ b³¹d podczas pracy w¹tku wyszukuj¹cego. Prawdopodobnie problem zwi¹zany jest z wielow¹tkowoœci¹.\nOryginalny komunikat b³êdu:\n\n"+E.Message;
    MessageBox(Synchronizer->MainForm->Handle,tmpStr.c_str(),"Co ty kurwa znowu rozpierdoli³eœ?",MB_OK | MB_ICONWARNING | MB_APPLMODAL);
    TmpTask=stskNone;
    Synchronize(StartNewTask);
    }
  }
 while (!Terminated);
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::CountNum()
{
 unsigned int Num=MinItemNum;
 ItemStr=IntToStr(Num);
 while (ItemStr.Length() < NameDigits) ItemStr="0"+ItemStr;
 ClearSpisCD();
 while (FileExists(Synchronizer->SpisSubPath+ItemStr+".txt"))
   {
   if ((CancelAction)||(Terminated)) return;
   AddFileToSpisCD();
   Num++;
   ItemStr=IntToStr(Num);
   while (ItemStr.Length() < NameDigits) ItemStr="0"+ItemStr;
   }
  MaxNum=Num-1;
  FullListInSpisCD=true;
  Synchronizer->LimitLow->Text=IntToStr(MinItemNum);
  Synchronizer->LimitHigh->Text=IntToStr(MaxNum);

}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::ReadFile()
//  Wyswietla plik bazy danych.
//  Potrafi wykorzystac bufor.
{
  if (Synchronizer==NULL) return;
  ItmBuffer->MaxBuffer=Synchronizer->ViewerBufSize->Value*1024;
  ItemDestination->DefAttributes->Assign(Synchronizer->NormFont->Font);
  unsigned int Num=ItemStr.ToIntDef(0);
  MakeItmViewerVisible();
  String FullFileName=Synchronizer->SpisSubPath+ItemStr+".txt";
  //Jak mo¿na to ladujemy plik z bufora
  if (ItmBuffer->CanLoad(Num,FullFileName))
    ItemDestination->Lines->Assign(ItmBuffer->Items[Num]->FileBody);
   else
    {
    //Inaczej normalnie z pliku
    TMemoryStream *FileContent=new TMemoryStream();
    FileContent->LoadFromFile(FullFileName);
    //Konwersja z blokowaniem odœwierzania
    Synchronizer->WinCPConvertBuffer((char *)FileContent->Memory,FileContent->Size);
    FileContent->Seek(0,soFromBeginning);
    ItemDestination->Lines->LoadFromStream(FileContent);
    delete FileContent;
    //Jak przerwano to wychodzimy
    if ((CancelAction)||(Terminated)) return;
    //W tym wypadku mo¿na zastanowiæ siê nad dodaniem pliku do bufora
    ItmBuffer->ConsiderAddingItem(Num,FullFileName,ItemDestination->Lines);
    //Powtorzenie - blad Buildera wymusza cos takiego.
    }
  ItemDestination->SelStart=0;
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::MakeItmViewerVisible()
{
 __try
  {
  Synchronizer->ShowSearchResultsBtn->Enabled=false;
  if (Synchronizer->ShowSearchResultsBtn->Down)
    Synchronizer->ShowSearchResultsBtn->Down=false;
  if (Synchronizer->SearchViewer->Visible) Synchronizer->SearchViewer->Hide();
  }
 __finally
  {
  if (!Synchronizer->ItemViewer->Visible) Synchronizer->ItemViewer->Show();
  Synchronizer->ShowSearchResultsBtn->Enabled=true;
  }
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::AddFileToSpisCD()
{
  Synchronizer->SpisCD->Items->Add(ItemStr);
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::ClearSpisCD()
{
  Synchronizer->SpisCD->Items->Clear();
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::UpdateKatHeight()
{
// funkcja wywolywana z zewnatrz
 Busy=true;
 if (Synchronizer->KatBox->ControlCount > 0)
   {
   int NewHeight=(Synchronizer->KatBox->Height-2) / Synchronizer->KatBox->ControlCount;
   int KatSize=0;
   if (NewHeight>4)
    while (KatSize < (Synchronizer->KatBox->ControlCount))
      {
      TSpeedButton *CurrButt=(TSpeedButton *)(Synchronizer->KatBox->Controls[KatSize]);
      CurrButt->Top=((Synchronizer->KatBox->Height-2)*(KatSize+1)) / (Synchronizer->KatBox->ControlCount+1);
      CurrButt->Height=NewHeight;
      KatSize++;
      }
   }
 Busy=false;
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::SearchItemRange()
// Glówna funkcja wyszukuj¹ca. Wywolywana przez ten w¹tek
//  (czyli w funkcji EXECUTE). Wykorzystuje wiele funkcji,
//  w tym SearchOneCDDatabase.
//  Wewn¹trz glownej petli, bledy nie sa od razu wyswietlane,
//  ale id¹ do ErrorList i s¹ wyswietlane na raz, w funkcji nadrzêdnej.
{
 TStringList *Plik=new TStringList();
 TStringList *CatList=NULL;
 __try
   {
   // Przygotowania do wyszukiwania
   PrepareToSearch();
   String SrchText=RmNationalCharsWin1250(Synchronizer->Typer->Text);
   // Tworzenie listy kategorii
   CatList=ListSelectedCategories();
   if ((CatList==NULL)||(CatList->Count<=0))
     {
     String ResltAdd="\\b\\f0\\fs24 Nie zaznaczy\\\'b3e\\\'9c \\\'bfadnej kategorii.\\par\n";
     FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
     ResltAdd="Nie mo\\\'bfesz wyszukiwa\\\'e6 w niczym.\\par\n";
     FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
     ResltAdd="\\b0\\fs22 Wyboru kategorii dokonujesz przez klikni\\\'eacie na jej nazwie z lewej strony ekranu.\\par\n";
     FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
     ResltAdd="\\fs24\\par\n";
     FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
     ResltAdd="\\fs20 PS. \\i Cienias jeste\\\'9c. \\\'8crednio g\\\'b3upi cz\\\'b3owiek dawno by za\\\'b3apa\\\'b3 o co chodzi.\\par\n";
     FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
     return; //Idzie do FINALLY
     }
   // No i przechodzimy po wszystkich plytach
   if (currSearchQuick)
     {//Tryb szybki
     for (unsigned long Num=MinCD;Num<=(unsigned long)MaxCD;Num++)
       {
       // Obsluga anulowania wyszukiwania
       if ((CancelAction)||(Terminated))
         {
         String ResltAdd="\\b\\f0\\fs24 --= WYSZUKIWANIE ANULOWANO NA P£YCIE "+IntToStr(Num)+" =--\\par\n";
         if (FormattedReslt!=NULL)
           FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
         return; //Idzie do FINALLY
         }
       __try
         {
         // Otwieramy plik bazy
         OpenCDDataFileQuick(Plik,Num);
         // I szukamy w nim tego co trzeba
         SearchOneCDDatabase(Plik,CatList,SrchText);
         }
        catch (Exception &E)
         {
             if ((ErrorList!=NULL)&&(ErrorList->Count<MaxErrorList))
                 ErrorList->Add("Wyst¹pi³ b³¹d podczas wyszukiwania w pliku "+IntToStr(Num)+". Oryginalny komunikat b³êdu: "+E.Message);
               else
                 AddErrorNum++;
         }
       // Jeszcze zabiegi kosmetyczne
       Synchronizer->ProgressBar->StepIt();
       }//end for (Num...)
     }
    else
     {//Tryb standardowy - nie szybki
     for (unsigned long Num=MinCD;Num<=(unsigned long)MaxCD;Num++)
       {
       // Obsluga anulowania wyszukiwania
       if ((CancelAction)||(Terminated))
         {
         String ResltAdd="\\b\\f0\\fs24 --= WYSZUKIWANIE ANULOWANO NA P£YCIE "+IntToStr(Num)+" =--\\par\n";
         if (FormattedReslt!=NULL)
           FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
         return; //Idzie do FINALLY
         }
       __try
         {
         // Otwieramy plik bazy
         OpenCDDataFile(Plik,Num);
         // I szukamy w nim tego co trzeba
         SearchOneCDDatabase(Plik,CatList,SrchText);
         }
        catch (Exception &E)
         {
             if ((ErrorList!=NULL)&&(ErrorList->Count<MaxErrorList))
                 ErrorList->Add("Wyst¹pi³ b³¹d podczas wyszukiwania w pliku "+IntToStr(Num)+". Oryginalny komunikat b³êdu: "+E.Message);
               else
                 AddErrorNum++;
         }
       // Jeszcze zabiegi kosmetyczne
       Synchronizer->ProgressBar->StepIt();
       }//end for (Num...)
     }
   Synchronizer->UpdateStatusBar("Wstawianie sformatowanych wyników...");
   }
 __finally
   {
   delete Plik;
   if (CatList!=NULL) delete CatList;
   RefreshAfterSearch();
   }
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::RefreshAfterSearch()
{
   if (FormattedReslt==NULL) return;
   Synchronizer->SearchViewer->PlainText=false;
   Synchronizer->ProgressBar->Position=0;
   FullListInSpisCD=false;
   String NumOfItemsStr=IntToStr(Synchronizer->SearchViewer->Lines->Count);
   Synchronizer->UpdateStatusBar("Operacja zakoñczona, znaleziono "+NumOfItemsStr+" elementów.");
   String ResltAdd="\\fs16 \\par }\n";
   if (FormattedReslt==NULL) return;
   FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
   FormattedReslt->Seek(0,soFromBeginning);
   Synchronizer->SearchViewer->Lines->LoadFromStream(FormattedReslt);
   if (FormattedReslt->Size>65535)
     {
     //Ta pêtla 'if' ma za zadanie eliminacjê blêdu w Builderze 6
     FormattedReslt->Seek(0,soFromBeginning);
     Synchronizer->SearchViewer->Lines->LoadFromStream(FormattedReslt);
     }
//   Synchronizer->MainForm->Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::PrepareToSearch()
{
   Synchronizer->SpisCD->Clear();
   Synchronizer->SearchViewer->Clear();
   Synchronizer->SearchViewer->DefAttributes->Assign(Synchronizer->NormFont->Font);
   Synchronize(Synchronizer->Typer->SetFocus);
   MakeSearcherVisible();
   MinCD=StrToInt(Synchronizer->LimitLow->Text);
   MaxCD=StrToInt(Synchronizer->LimitHigh->Text);
   currSearchQuick=Synchronizer->quickSearch;
   FullListInSpisCD=false;
   ItmBuffer->MaxBuffer=Synchronizer->ViewerBufSize->Value*1024;
   //Bufor na sformatowane wyniki
   {
   TFont *SmlFont=Synchronizer->SmallFont->Font;
   TFont *NrmFont=Synchronizer->NormFont->Font;
   FormattedReslt->Clear();
   String ResltAdd="{\\rtf1\\ansi\\ansicpg1250\\deff0\\deflang1045\n";
   FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
   ResltAdd="{\\fonttbl{\\f0\\fnil "+NrmFont->Name+";}\n";
   FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
   ResltAdd="{\\f1\\fnil "+SmlFont->Name+";}\n";
   FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
   ResltAdd="{\\f2\\fmodern\\fprq6 SimSun;}}\n";
   FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
   unsigned long RGBColor=ColorToRGB(NrmFont->Color);
   unsigned int BlueVal= ((((RGBColor)>>16)&255));
   unsigned int GreenVal=((((RGBColor)>>8) &255));
   unsigned int RedVal=  ((((RGBColor))    &255));
   ResltAdd="{\\colortbl ;\\red"+IntToStr(RedVal)+"\\green"+IntToStr(GreenVal)+"\\blue"+IntToStr(BlueVal)+";\n";
   FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
   RGBColor=ColorToRGB(SmlFont->Color);
   BlueVal= ((((RGBColor)>>16)&255));
   GreenVal=((((RGBColor)>>8) &255));
   RedVal=  ((((RGBColor))    &255));
   ResltAdd="\\red"+IntToStr(RedVal)+"\\green"+IntToStr(GreenVal)+"\\blue"+IntToStr(BlueVal)+";}\n";
   FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
   ResltAdd="\\viewkind4\\uc1\\pard\n";
   NrmFontStr="\\cf1\\f0\\fs"+IntToStr(NrmFont->Size*2)+" ";
   SmlFontStr="\\cf2\\f1\\fs"+IntToStr(SmlFont->Size*2)+" ";
   if (currSearchQuick)
     {
     ResltAdd=NrmFontStr+"-->SZYBKIE WYSZUKIWANIE<--\\par \n";
     FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
     }
   }
   // Paski statusu
   Synchronizer->UpdateStatusBar("Trwa wyszukiwanie...");
   Synchronizer->ProgressBar->Min=0;
   Synchronizer->ProgressBar->Max=max(MaxCD-MinCD+1,1);
   Synchronizer->ProgressBar->Position=0;
   Synchronizer->ProgressBar->Step=1;
   // Poprawiamy ustawienia bufora
   if (ItmBuffer->Count<MaxCD) ItmBuffer->ChangeItemsCount(MaxCD);
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::SearchOneCDDatabase(TStringList* Plik,TStringList* CatList,String SrchText)
{
 __try
   {
   // Warunki pocz¹tkowe
   long KatEnd=-1;
   long KatStart=-1;
   FindCategoryInList(Plik,CatList,KatStart,KatEnd);
   while (KatEnd>0)
     {
     if ((KatEnd-KatStart)>0)
       {
       SearchForTextInLineRange(Plik,KatStart+1,KatEnd,SrchText);
       }
     FindCategoryInList(Plik,CatList,KatStart,KatEnd);
     }
  }
 __except(EXCEPTION_EXECUTE_HANDLER)
  {
   CancelAction=true;
  }
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::FindCategoryInList(TStringList* Plik,TStringList* CatList,long &KatStart,long &KatEnd)
{
// Przeszukuje liste 'Plik' poczawszy od 'KatEnd+1'
//  w poszukiwaniu elementow 'CatList'. Jak nie znajdzie zwraca KatStart/End=-1
  long Cntr=KatEnd+1;
  bool FoundCategory=false;
  char *TmpPChr=new char[MaxCategoryNameLen+1];
  __try
    {
    int PlikLinesCount=Plik->Count;
    // Szukanie pocz¹tku
    while (Cntr<PlikLinesCount)
      {
      char *CurrStr=Plik->Strings[Cntr].c_str();
      int CurStrLen=strlen(CurrStr);
      if ((CurStrLen>1)&&(CurStrLen<=MaxCategoryNameLen))
        if (CurrStr[CurStrLen-1]==':')
          {
          StrLCopy(TmpPChr,CurrStr,CurStrLen-1);
          TmpPChr[CurStrLen-1]=0;
          if (CatList->IndexOf(AnsiLowerCase(TmpPChr))>=0)
            {
            FoundCategory=true;
            break;
            }
          }
      Cntr++;
      } //end while (Cntr<...)
    }
  __finally
    {
    delete TmpPChr;
    }
  if (FoundCategory)
    {
    KatStart=Cntr;
    // Znaleziono pocz¹tek - szukanie konca
    Cntr++;
    while (Cntr < Plik->Count)
      {
      char *CurrStr=Plik->Strings[Cntr].c_str();
      int CurStrLen=strlen(CurrStr);
      if (CurStrLen>0)
        if (CurrStr[CurStrLen-1]==':')
            break;
      Cntr++;
      }
    KatEnd=Cntr-1;
    }
   else
    {
    // drugi przypadek - nic nie znaleziono
    KatStart=-1;
    KatEnd=-1;
    }
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::SearchForTextInLineRange(TStringList* Plik,long KatTxtStart,long KatTxtEnd,String SrchText)
{
// Wykonuje wyszukiwanie w wybranym bloku linii,
//   jak trzeba wywoluje funkcje dodaj¹c¹ do wyników
  for (long KatLnNum=KatTxtStart;KatLnNum<=KatTxtEnd;KatLnNum++)
    {
    LiniaUpd=Plik->Strings[KatLnNum];
    if (ContainsTypedText(RmNationalCharsWin1250(LiniaUpd),SrchText))
      Synchronize(UpdateSearchResults);
    }
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::OpenCDDataFile(TStringList* Plik,long Num)
// £aduje plik bazy danych.
// wykorzystuje bufor jezeli sie da
{
  try
    {
    // Tworzymy nazwê pliku
    ItemStr.sprintf(ConvFormat,Num);
    String FileName=Synchronizer->SpisSubPath+ItemStr+".txt";
    //Jak mo¿na to ladujemy plik z bufora
    if (ItmBuffer->CanLoad(Num,FileName))
      {
      Plik->Assign(ItmBuffer->Items[Num]->FileBody);
      }
     else
      {
      //Inaczej normalnie z pliku
      Plik->LoadFromFile(FileName);
      Plik->Text=Synchronizer->WinCPConvertLine(Plik->Text);
      //W tym wypadku mo¿na zastanowiæ siê nad dodaniem pliku do bufora
      ItmBuffer->ConsiderAddingItem(Num,FileName,Plik);
      }
    }
  catch (Exception &E)
    {
    if ((ErrorList!=NULL)&&(ErrorList->Count<MaxErrorList))
        ErrorList->Add("Wyst¹pi³ b³¹d podczas otwierania jednego z plików bazy danych. Oryginalny komunikat b³êdu: "+E.Message);
      else
        AddErrorNum++;
    }
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::OpenCDDataFileQuick(TStringList* Plik,long Num)
// £aduje plik bazy danych.
// wykorzystuje bufor nie sprawdzaj¹c czy tak mo¿na - Szybka wersja
{
  try
    {
    // Tworzymy nazwê pliku
    ItemStr.sprintf(ConvFormat,Num);
    //£adujemy plik z bufora
    Plik->Assign(ItmBuffer->Items[Num]->FileBody);
    }
  catch (Exception &E)
    {
    if ((ErrorList!=NULL)&&(ErrorList->Count<MaxErrorList))
        ErrorList->Add("B³¹d podczas SZYBKIEGO otwierania jednego z plików bazy danych. Oryginalny komunikat b³êdu: "+E.Message);
      else
        AddErrorNum++;
    }
}
//---------------------------------------------------------------------------
TStringList* __fastcall TKatSearcher::ListSelectedCategories()
{
 __try
   {
   TStringList *CatList=new TStringList();
   long KatSize=0;
    while ((KatSize<(Synchronizer->KatBox->ControlCount-1))&&(KatSize<MaxCategories))
      {
      TSpeedButton *KatBtn=(TSpeedButton *)(Synchronizer->KatBox->Controls[KatSize]);
      if (KatBtn->Down)
        CatList->Add(AnsiLowerCase(KatBtn->Caption));
      KatSize++;
      }
    return CatList;
   }
 __except(EXCEPTION_EXECUTE_HANDLER)
  {
  return NULL;
  }
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::StartNewTask()
{
  ActiveTask=TmpTask;
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::UpdateSearchResults()
// Je¿eli podczas wyszukiwania znaleziono cos, wywolywana jest ta funkcja
//   w w¹tku glównym by bezpiecznie dodac nowy wpis do wyników
{
  TStrings *SrchLines=Synchronizer->SearchViewer->Lines;
  if (!CancelAction)
    try
      {
      String Nawias;
      String NawiasRTF;
      String LiniaUpdRTF;
      {
      int NawStart=LiniaUpd.Pos("(");
      int NawEnd=BackPos(')',LiniaUpd);
      if (!Synchronizer->ViewerOpRemove->Checked)
        Nawias=LiniaUpd.SubString(NawStart,NawEnd-NawStart+1);
      LiniaUpd.Delete(NawStart,NawEnd-NawStart+1);
      PrepareRTFString(NawiasRTF,Nawias);
      PrepareRTFString(LiniaUpdRTF,LiniaUpd);
      }
      if (Synchronizer->ViewerShowNum->Checked)
        {
         if (Synchronizer->ViewerNumAtEnd->Checked)
            {
            if (!currSearchQuick)
                SrchLines->Add(LiniaUpd+Nawias+" ["+ItemStr+"]");
            String ResltAdd=NrmFontStr+LiniaUpdRTF+SmlFontStr+NawiasRTF+" ["+ItemStr+"]\\par \n";
            FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
            }
           else
            {
            if (!currSearchQuick)
                SrchLines->Add("["+ItemStr+"] "+LiniaUpd+Nawias);

            String ResltAdd=SmlFontStr+"["+ItemStr+"]"+NrmFontStr+" "+LiniaUpdRTF+SmlFontStr+NawiasRTF+"\\par \n";
            FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
            }
        }
       else
        {
        if (!currSearchQuick)
            SrchLines->Add(LiniaUpd);

        String ResltAdd=NrmFontStr+" "+LiniaUpdRTF+SmlFontStr+NawiasRTF+"\\par \n";
        FormattedReslt->Write(ResltAdd.c_str(),ResltAdd.Length());
        }
      }
    catch (Exception &E)
      {
      CancelAction=true;
      if ((ErrorList!=NULL)&&(ErrorList->Count<MaxErrorList))
          ErrorList->Add("B³¹d okienka wyswietlaj¹cego rezultaty. Nie bêd¹ w nim dostêpne ca³e wyniki. Oryginalny komunikat b³êdu: "+E.Message);
        else
          AddErrorNum++;
      }
  if (Synchronizer->SpisCD->Items->IndexOf(ItemStr)<0) Synchronizer->SpisCD->Items->Add(ItemStr);
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::MakeSearcherVisible()
{
 __try
  {
  Synchronizer->ShowSearchResultsBtn->Enabled=false;
  if (!Synchronizer->ShowSearchResultsBtn->Down)
    Synchronizer->ShowSearchResultsBtn->Down=true;
  if (Synchronizer->ItemViewer->Visible) Synchronizer->ItemViewer->Hide();
  }
 __finally
  {
  if (!Synchronizer->SearchViewer->Visible) Synchronizer->SearchViewer->Show();
  Synchronizer->ShowSearchResultsBtn->Enabled=true;
  }
}
//---------------------------------------------------------------------------
bool __fastcall inline TKatSearcher::ContainsTypedText(AnsiString Text,AnsiString TypedText)
{
 String CmpWith=AnsiLowerCase(TypedText);
 String SearchText=AnsiLowerCase(Text);
 if (!Synchronizer->TyperCommentsCheckBox->Checked)
    SearchText.Delete(SearchText.Pos("("),BackPos(")",SearchText));
 bool Founded;
 int WNum;
 String Wrd;
 if (Synchronizer->TyperAllRadioBtn->Checked)
    Founded=Synchronizer->ContainsText(SearchText,CmpWith);
  else
    {
    WNum=1;
    Wrd=GetParam(WNum,CmpWith);
   do
     {
      Founded=Synchronizer->ContainsText(SearchText,Wrd);
      Wrd=GetParam(WNum,CmpWith);
      WNum++;
     }
    while((!Founded)&&(Wrd.Length()>0));
   }
 return Founded;
}
//---------------------------------------------------------------------------
void __fastcall TKatSearcher::GenerateDebugStats()
{
  Synchronizer->UpdateStatusBar("-= Modu³ wyszukuj¹cy =-");
  Synchronizer->UpdateStatusBar("   Wykorzystany bufor plików bazy: "+IntToStr(ItmBuffer->BufferUsed)+" b");
  Synchronizer->UpdateStatusBar("   Maksimum rozmiaru bufora plików bazy: "+IntToStr(ItmBuffer->MaxBuffer)+" b");
  Synchronizer->UpdateStatusBar("   Zarezerwowanych pól bufora plików bazy: "+IntToStr(ItmBuffer->Count));
  Synchronizer->UpdateStatusBar("   Rozmiar ostatnich wyników: "+IntToStr(FormattedReslt->Size));
  Synchronizer->UpdateStatusBar("   W liœcie s¹ wszystkie p³yty: "+BoolToStr(FullListInSpisCD,true));
}
//---------------------------------------------------------------------------

