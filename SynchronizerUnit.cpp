//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SynchronizerUnit.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------
const String CategoryFile="\\Spis\\0000.txt";
//---------------------------------------------------------------------------
__fastcall TSynchronizer::TSynchronizer(TForm *nMainForm)
  : TSynchronizerPas(nMainForm)
{
  MainFormComponentsClear();
  MainFormComponentsGet();
}
//---------------------------------------------------------------------------
__fastcall TSynchronizer::~TSynchronizer()
{
  //deinicjacja list do nazw
  delete NameOfMusic;
  delete NameOfPicture;
  delete NameOfProgram;
  delete NameOfAnimation;
  delete NameOfData;
  delete NameOfVarious;
  delete NameOfArchive;
  delete ExtOfMusic;
  delete ExtOfPicture;
  delete ExtOfProgram;
  delete ExtOfAnimation;
  delete ExtOfData;
  delete ExtOfVarious;
  delete ExtOfArchive;
  delete UpdFont;
  MainFormComponentsClear();
}
//---------------------------------------------------------------------------
String __fastcall TSynchronizer::WinCPConvertLine(String s)
{
  unsigned int strSize=s.Length();
  if ((strSize>0)&&(CpDosButton->Checked))
    {
    String OutStr=s;
    ConvCP852ToWin1250((char *)OutStr.data(),strSize);
    return OutStr;
    }
  else
   return s;
}
//---------------------------------------------------------------------------
void __fastcall TSynchronizer::WinCPConvertBuffer(char *buf,int bufSize)
{
  if ((bufSize>0)&&(CpDosButton->Checked))
    {
    ConvCP852ToWin1250(buf,bufSize);
    }
}
//---------------------------------------------------------------------------
String __fastcall TSynchronizer::DosCPConvertLine(String s)
{
  unsigned int strSize=s.Length();
  if ((strSize>0)&&(CpDosButton->Checked))
    {
    String OutStr=s;
    ConvWin1250ToCP852((char *)OutStr.data(),strSize);
    return OutStr;
    }
  else
   return s;
}
//---------------------------------------------------------------------------
void __fastcall TSynchronizer::CreateKat()
//  Pobiera z pliku nazwy kategorii, st¹d tworzy kategorie programu
//  oraz listy u¾ywane przez DiscAnalyzera
{
 __try
  {
  if (FileExists(SpisPath+CategoryFile))
     ItemViewer->Lines->LoadFromFile(SpisPath+CategoryFile);
    else
     {
     String MessageText="Plik z kategoriami nie istnieje ("+CategoryFile+")";
     MessageBox(MainForm->Handle,MessageText.c_str(),"Kurwa! Ale z ciebie zjeb!",MB_OK | MB_ICONERROR);
     }
  ItemViewer->Lines->Text=WinCPConvertLine(ItemViewer->Lines->Text);
//  ConvertStrList2W(ItemViewer.Lines);
  }
 __finally
  {
  String KatName,ContentType,Extensions;
  bool HasCategories;
  bool HasExtensions;
  int lnStart=0;
  int lnEnd=0;
  HasCategories=false;
  HasExtensions=false;
  while (FindNextBlockBorders(ItemViewer->Lines,lnStart,lnEnd))
    {
    if (ItemViewer->Lines->Strings[lnStart].UpperCase()=="KATEGORIE:")
      {
      HasCategories=true;
      KatCreationAnalyseKategorie(KatName,ContentType,lnStart,lnEnd);
      }
    if (ItemViewer->Lines->Strings[lnStart].UpperCase()=="ROZSZERZENIA:")
      {
      HasExtensions=true;
      KatCreationAnalyseRozszerzenia(Extensions,ContentType,lnStart,lnEnd);
      }
    }
  if (!HasCategories)
    {
    String MessageText="W pliku "+CategoryFile+" nie znaleziono segmentu z kategoriami. Popraw plik, bo kategorii nie bêdzie.";
    MessageBox(MainForm->Handle,MessageText.c_str(),"Piepszony zboczeniec!",MB_OK | MB_ICONERROR);
    }
  if (!HasExtensions)
    {
    String MessageText="W pliku "+CategoryFile+" nie znaleziono segmentu z rozszerzeniami. Nowy modu³ analizy CD wymaga tego segmentu i mo¿e nie dzia³aæ poprawnie.";
    MessageBox(MainForm->Handle,MessageText.c_str(),"Peda³ jebany!",MB_OK | MB_ICONERROR);
    }
  }

}
//---------------------------------------------------------------------------
void __fastcall TSynchronizer::UpdateStatusBar(AnsiString Text)
{
 __try
  {
  if (DebLogLines!=NULL)
    DebLogLines->Add("["+Time()+"] "+Text);
  StatusBar->Text=Text;
  MainForm->Refresh();
  }
 __except(EXCEPTION_EXECUTE_HANDLER)
  {
  }
}
//---------------------------------------------------------------------------
void __fastcall TSynchronizer::MainFormComponentsGet()
{
  //Pobieramy wskaŸniki
  TComponent *DebLogMemo=MainForm->FindComponent("DebLogMemo");
  if ((DebLogMemo!=NULL)&&(DebLogMemo->ClassType()==__classid(TMemo)))
    DebLogLines=((TMemo *)DebLogMemo)->Lines;
  //Kategorie do analizy plików na podstawie nazwy
  NameOfMusic=new TStringList();
  NameOfPicture=new TStringList();
  NameOfProgram=new TStringList();
  NameOfAnimation=new TStringList();
  NameOfData=new TStringList();
  NameOfVarious=new TStringList();
  NameOfArchive=new TStringList();
  ExtOfMusic=new TStringList();
  ExtOfPicture=new TStringList();
  ExtOfProgram=new TStringList();
  ExtOfAnimation=new TStringList();
  ExtOfData=new TStringList();
  ExtOfVarious=new TStringList();
  ExtOfArchive=new TStringList();
}
//---------------------------------------------------------------------------
void __fastcall TSynchronizer::MainFormComponentsClear()
{
  //Ustawiamy wszystko na NULL
  DebLogLines=NULL;
  //Kategorie do analizy plików na podstawie nazwy
  NameOfMusic=NULL;
  NameOfPicture=NULL;
  NameOfProgram=NULL;
  NameOfAnimation=NULL;
  NameOfData=NULL;
  NameOfVarious=NULL;
  NameOfArchive=NULL;
  ExtOfMusic=NULL;
  ExtOfPicture=NULL;
  ExtOfProgram=NULL;
  ExtOfAnimation=NULL;
  ExtOfData=NULL;
  ExtOfVarious=NULL;
  ExtOfArchive=NULL;
}
//---------------------------------------------------------------------------
void __fastcall TSynchronizer::GenerateDebugStats()
{
  UpdateStatusBar("-= Modu³ synchronizuj¹cy =-");
  UpdateStatusBar("   Nazwy kategorii z muzyk¹:");
  AddStringsToStata(NameOfMusic);
  UpdateStatusBar("   Nazwy kategorii z obrazkami:");
  AddStringsToStata(NameOfPicture);
  UpdateStatusBar("   Nazwy kategorii z programami:");
  AddStringsToStata(NameOfProgram);
  UpdateStatusBar("   Nazwy kategorii z animacjami:");
  AddStringsToStata(NameOfAnimation);
  UpdateStatusBar("   Nazwy kategorii z danymi:");
  AddStringsToStata(NameOfData);
  UpdateStatusBar("   Nazwy kategorii roznych:");
  AddStringsToStata(NameOfVarious);
  UpdateStatusBar("   Nazwy kategorii z archiwami:");
  AddStringsToStata(NameOfArchive);
  UpdateStatusBar("   Rozszerzenia muzyki:");
  AddStringsToStata(ExtOfMusic);
  UpdateStatusBar("   Rozszerzenia obrazkow:");
  AddStringsToStata(ExtOfPicture);
  UpdateStatusBar("   Rozszerzenia programow:");
  AddStringsToStata(ExtOfProgram);
  UpdateStatusBar("   Rozszerzenia animacji:");
  AddStringsToStata(ExtOfAnimation);
  UpdateStatusBar("   Rozszerzenia danych:");
  AddStringsToStata(ExtOfData);
  UpdateStatusBar("   Rozszerzenia roznych:");
  AddStringsToStata(ExtOfVarious);
  UpdateStatusBar("   Rozszerzenia archiwow:");
  AddStringsToStata(ExtOfArchive);
}
//---------------------------------------------------------------------------
void __fastcall TSynchronizer::SetSearchAddKeyParams(TShiftState Shift)
{
  if (Shift.Contains(ssShift))
      quickSearch=true;
    else
      quickSearch=false;
}
//---------------------------------------------------------------------------
void __fastcall TSynchronizer::AddStringsToStata(TStrings *StrList)
{
  String Str="      ";
  if (StrList==NULL) Str+="WskaŸnik na strukturê jest pusty.";
    else
  if (StrList->Count<1) Str+="(lista pusta)";
    else
  for (int itm=0 ; (itm<StrList->Count)&&(Str.Length()<8192) ; itm++)
    {
    Str+=StrList->Strings[itm];
    if (itm+1<StrList->Count) Str+=", ";
    }
  UpdateStatusBar(Str);
}
//---------------------------------------------------------------------------
void __fastcall TSynchronizer::KatCreationAnalyseRozszerzenia(String &Extensions,String &ContentType,unsigned int lnstart,unsigned  int lnend)
{
;
   for (unsigned long LineNum=1;LineNum<=(lnend-lnstart);LineNum++)
    if (ItemViewer->Lines->Strings[lnstart+LineNum]!="")
      {
      ContentType=GetBefore('=',ItemViewer->Lines->Strings[lnstart+LineNum]);
      Extensions=GetAfter('=',ItemViewer->Lines->Strings[lnstart+LineNum]);
      unsigned long ExtNum=0;
      do
         {
         ExtNum++;
         String ExtStr=GetSpaceParam(ExtNum,Extensions);
         if (ExtStr=="") break;
         AddSingleExtToExtensionList(ExtStr,ContentType);
         } while (ExtNum<8192);
      }
}
//---------------------------------------------------------------------------

