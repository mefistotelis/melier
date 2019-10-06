//---------------------------------------------------------------------------
/*
 Gdybym nie wiedzial co robic:

RemoveEquPartsOfNamesInChildren
RemoveMultipleChars (w FXProcs)
FldrNameAnalist
FileBodyAnalist
i wiele innych...

*/

#include <vcl.h>
#pragma hdrstop

#include "DiscAnalyzer.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TDiscAnalyzer::TDiscAnalyzer(TSynchronizer* nSynchronizer)
        : TPersistent()
//Konstruktor klasy
{
  Terminate=false;
  Synchronizer=nSynchronizer;
  DynamicStruct=new TDynamicStruct();
  InpMpeg=new InputFileMPEG();
  if (Synchronizer==NULL) return;
  EnableBodyAnalyst=Synchronizer->AnalyzerEnBody->Checked;
}
//---------------------------------------------------------------------------
__fastcall TDiscAnalyzer::~TDiscAnalyzer(void)
//Czysci wszystko i usuwa strukture
{
 __try
  {
  delete DynamicStruct;
  delete InpMpeg;
  Synchronizer=NULL;
  }
 __finally
  {
  };
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::AnalyzeDrive(System::TObject* Sender, AnsiString DriveRoot)
//Glówna funkcja analizuj¹ca
{
  if (Synchronizer==NULL) return false;
  Synchronizer->UpdateStatusBar("Rozpoczynam analizê dysku...");
  RootDir=DriveRoot;
  if (AnalyzeRootDir(Sender))
    __try
    {
    if (Terminate) return false;
    OptimizeStructure(Sender);
    Synchronizer->UpdateStatusBar("Analiza zakoñczona.");
    return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {};
  Synchronizer->UpdateStatusBar("Analiza nie powiod³a siê.");
  return false;
}
//---------------------------------------------------------------------------
unsigned int __fastcall TDiscAnalyzer::GetFromBufLittleEndianDWord(char *Buf, unsigned int Index)
//Pobiera liczbe DWORD z bufora
{
  return (((unsigned char)(Buf[Index+3]))<<24)| (((unsigned char)(Buf[Index+2]))<<16)|((unsigned char)(Buf[Index+1])<<8)|(unsigned char)(Buf[Index]);
}
//---------------------------------------------------------------------------
Word __fastcall TDiscAnalyzer::GetFromBufLittleEndianWord(char *Buf, unsigned Index)
//Pobiera liczbe WORD z bufora
{
  return (((unsigned char)(Buf[Index+1])) << 8)+(unsigned char)(Buf[Index]);
}
//---------------------------------------------------------------------------
unsigned int __fastcall TDiscAnalyzer::GetFromBufBigEndianDWord(char *Buf, unsigned int Index)
//Pobiera liczbe DWORD z bufora
{
  return (((unsigned char)(Buf[Index]))<<24)| (((unsigned char)(Buf[Index+1]))<<16)|((unsigned char)(Buf[Index+2])<<8)|(unsigned char)(Buf[Index+3]);
}
//---------------------------------------------------------------------------
Word __fastcall TDiscAnalyzer::GetFromBufBigEndianWord(char *Buf, unsigned Index)
//Pobiera liczbe WORD z bufora
{
  return (((unsigned char)(Buf[Index])) << 8)+(unsigned char)(Buf[Index+1]);
}
//---------------------------------------------------------------------------
AnsiString __fastcall TDiscAnalyzer::IdentifyACMCodec(Word identWord)
{
    switch (identWord)
      {
      case 0x0001:return "PCM";
      case 0x0007:return "CCITT";
      case 0x0101:return "IBM mu-Law";
      case 0x0102:return "IBM a-Law";
      case 0x0103:return "IBM ADPCM";
      case 0x0055:return "Layer-3";
      case 0x2000:return "AC3";
      case 0x0011:return "Ms ADPCM";
      case 0x0031:return "GSM";
      case 0x0161:return "WMA";
      default:return "unknSnd"+IntToHex(identWord,4);
      };
}
//---------------------------------------------------------------------------
ReplacePosition __fastcall TDiscAnalyzer::ShouldBeReplaced(PSearchRecList Entry)
// Okreœla, czy dany element powinien zmieniæ po³orzenie
//i gdzie nale¿y go umieœciæ
{
  return rpNone;
}
//---------------------------------------------------------------------------
void __fastcall TDiscAnalyzer::MakeReplacementsIn(PSearchRecList Entry)
//Zmienia po³o¿enie objektów w podstrukturze podanej
// parametrem. Co trzeba przenosi do g³ównego drzewa
// lub do okreœlonych kategorii.
{
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::FileBodyAnalist(TSearchRecList *Entry)
{
    TFileStream *FileData;
    char *FileStartPtr;
    String EntryPath;
  if ((Entry->F.Size<2)||(!EnableBodyAnalyst))
    {
    Entry->BodyAnalist.Probability=0;
    return true;
    };
   FileStartPtr=(char *)malloc(FileBodyHeaderSize);
  __try
    {
    EntryPath=Entry->Path;
    if (EntryPath=="") EntryPath=RootDir;
    FileData=new TFileStream(EntryPath+'\\'+Entry->F.Name,fmOpenRead|fmShareDenyNone);
    // odczytujemy pierwsze bajty pliku
    FileData->ReadBuffer(FileStartPtr,min(FileBodyHeaderSize,Entry->F.Size));
    delete FileData;
    }
  __except(EXCEPTION_EXECUTE_HANDLER)
    {
    Entry->BodyAnalist.Probability=0;
    return false;
    };
    // wczytane - pora na rozpoznawanie
  __try
    {
    if (!RecognizedArchiveStart(Entry,FileStartPtr))
    if (!RecognizedBitmapStart(Entry,FileStartPtr))
    if (!RecognizedTiffStart(Entry,FileStartPtr))
    if (!RecognizedRiffStart(Entry,FileStartPtr))
    if (!RecognizedVideoStart(Entry,FileStartPtr))
    if (!RecognizedMpegStart(Entry,FileStartPtr))
    if (!Recognized3DDataStart(Entry,FileStartPtr))
    if (!RecognizedVectGraphStart(Entry,FileStartPtr))
    if (!RecognizedExeCodeStart(Entry,FileStartPtr))
    if (!RecognizedMusicStart(Entry,FileStartPtr))
      {
        Entry->BodyAnalist.Probability=0;
      };
    }
  __except(EXCEPTION_EXECUTE_HANDLER)
    {//Taki b³¹d NIE MO¯E siê zdarzyæ, bo to NA PEWNO mój b³¹d
    Entry->BodyAnalist.Probability=999;
    Entry->BodyAnalist.DescBefore="WYJ¥TEK PODCZAS REKOGNIZACJI!!!";
    };
  free(FileStartPtr);
  return true;
}
//---------------------------------------------------------------------------
//===========================================
//  Na razie nie zajmowac sie przenoszeniem
//      Najpierw zrobic nazywanie wpisów
void __fastcall TDiscAnalyzer::MakeReplacements()
//Ogl¹da ca³¹ strukturê i zmienia po³o¿enie
// elementów, które wydaj¹ siê byæ nie na miejscu.
// Kategorie przenosi do g³ównego drzewa, itemy
// z g³ównego drzewa do którejœ kategorii
{
  uint HasStdKatStructure;//Prawdopodobieñstwo ¿e CD ma strukturê "Kategoriow¹"
  uint HasNotStdKatStructure;//i prawdopodobieñstwo ¿e nie ma
  HasStdKatStructure=0;
  HasNotStdKatStructure=0;
  ShouldBeReplaced(NULL);
  MakeReplacementsIn(NULL);
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::OptimizeStructure(TObject *Sender)
//Wykonuje wszelkie opeacje dostosowuj¹ce strukturê
//(Ustawia RealProperties i przebudowuje strukture)
//Uzywany w AnalyzeDrive po wykonaniu listy plików
{
  //Najpierw analiza wszystkich plików i okreœlenie czym na prawdê s¹
  SetAllEntryTypes();
  //Teraz zmiana po³o¿enia objektów w celu podzia³u na kategorie
  MakeReplacements();
  //No i tyle - teraz latwo wygenerowac wyniki
  return true;
}
//---------------------------------------------------------------------------
void __fastcall TDiscAnalyzer::SetAllEntryTypes()
//Przechodzi ca³¹ strukturê i okreœla typy
// wszystkoch wpisów. analizuje przy tym
// zawartoœæ plików.
//Po wykonaniu tej procedury wszystkie RealProperties
// powinny zawieraæ jakieœ informacje.
//U¿ywany w OptimizeStructure.
{
   String MostProbableCategory;
  //Analizujemy wpisy
  SetStructureAnalystProperties(DynamicStruct->RootEntryList);
  //Analiza skoñczona - ustawiamy RealProperties
  ChooseStructureRealProperties(DynamicStruct->RootEntryList,MostProbableCategory);
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::FldrContentAnalist(TSearchRecList *Entry)
//U¿ywany w SetFldrRealProperties
{
    Entry->BodyAnalist.Probability=0;
    return true;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::FldrNameAnalist(TSearchRecList *Entry)
//U¿ywany w SetFldrRealProperties
{
  //Folder mo¿e byc kategori¹ lub elementem kategorii
  //!! zrobic analize typu
  Entry->NameAnalist.Probability=10;
  //Mamy prawdopodobienstwa, teraz wystarczy wypelnic reszte pól
  Entry->NameAnalist.Name=PrepareNameFromFName(Entry->F);
  Entry->NameAnalist.EntryType=setCategory;
  Entry->NameAnalist.CategoryName=GetCatNameFromType(Entry->NameAnalist.EntryType);
  Entry->NameAnalist.DescBefore="";
  Entry->NameAnalist.DescAfter="";
  //no i koniec...
  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::IsFolder(TSearchRecList *Entry)
// okreœla, czy element reprezentuje katalog.
//jeœli zwraca fa³sz, to element jest plikiem
{
  if (Entry->F.Attr & faDirectory)
    return true;
   else
    return false;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TDiscAnalyzer::PrepareNameFromFName(const TSearchRec &F)
//  Dostaje plik lub katalog i okresla jak go nazwac
//  zakladaj¹c ¿e typ pliku nie jest rozpoznany
{
  AnsiString Result;
  __try
    {
    Result=ExtractFileName(F.Name);
    if ((F.Attr && faDirectory) != faDirectory)
        Result=RemoveExtensionAdv(Result);
    Result=Char2Space('_',Result);
    Result=Char2Space(',',Result);
    Result=Char2Space(';',Result);
    Result=Char2Space('~',Result);
    Result=RemoveMultipleChars(32,Result);
    Result=RemoveMultipleChars('-',Result);
    Result=CheckCase(Result);
    return Result;
    }
  __except(EXCEPTION_EXECUTE_HANDLER)
    {
    return "?";
    };
}
//---------------------------------------------------------------------------
AnsiString __fastcall TDiscAnalyzer::PrepareDescFromFName(const TSearchRec &F)
//  Dostaje plik lub katalog i okresla jaki daæ mu opis
//  zakladaj¹c ¿e typ pliku nie jest rozpoznany
{
  AnsiString Result;
  __try
    {
    Result=ExtractFileName(F.Name);
    if ((F.Attr && faDirectory) == faDirectory)
        Result="";
      else
        Result=ExtractExtensionAdv(Result);
    Result=RemoveMultipleChars(32,Result);
    Result=RemoveMultipleChars('-',Result);
    return Result.UpperCase();
    }
  __except(EXCEPTION_EXECUTE_HANDLER)
    {
    return "";
    };
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::GetGeneratedInfo(System::TObject* Sender, Classes::TStrings* Info)
//Pobiera utworzon¹ wczesniej strukture do listy stringów
//Uzywane w glównej funkcji analizuj¹cej, mo¿e pobierac kilka razy po jednej analizie
{
TSearchRecList *CurEntry; //Przetwarzany wpis tablicy plików
 __try
  {
  CurEntry=DynamicStruct->RootEntryList;
  while (CurEntry->Prev!=NULL) CurEntry=CurEntry->Prev;
  Info->Clear();
  while (CurEntry!=NULL)
    {
    if (Info->Count>1) Info->Add("");
    Info->Add(CurEntry->RealProperties.Name+":");
    GetItemList(Sender,CurEntry,Info);
    CurEntry=CurEntry->Next;
    }
  return true;
  }
 __except(EXCEPTION_EXECUTE_HANDLER)
  {
  return false;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::ChooseStructureRealProperties(Analyzerdynamics::PSearchRecList VirtDir, AnsiString &MostProbableCategory)
// Okresla czym na prawde s¹ pliki na podstawie wyników
// analiz nazw, zawartosci oraz polozenia.
// U¿ywany w SetAllEntryTypes
{
  TSearchRecList *CurEntry;//WskaŸnik na aktualny element
  String MostProbCatChild;//Pomocniczy, je¿eli aktualny objekt (którys CurEntry) to katalog
  //Przechodzimy po calej strukturze
  bool Result=true;
  CurEntry=VirtDir;
  DynamicStruct->MovetoFirstItem(CurEntry);
  while (CurEntry!=NULL)
    {
    //dla ka¿dego katalogu
    if (IsFolder(CurEntry))
      {
      //robimy analize wnetrza
      Result|=ChooseStructureRealProperties(CurEntry->Down,MostProbCatChild);
      //Wywalamy z nazw wnetrza zbedne elementy
      RemoveEquPartsOfNamesInChildren(CurEntry);
      //i ustawiamy RealProperties dla samego katalogu
      SetFldrRealProperties(CurEntry);
      }
     else
    //Je¿eli nie trafilismy na katalog
      {
      SetFileRealProperties(CurEntry);
      }
    //No i koniec - analizujemy nastepnego
    CurEntry=CurEntry->Next;
  }
 return Result;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::SetStructureAnalystProperties(Analyzerdynamics::PSearchRecList VirtDir)
//  Ustawia wpisy slu¿¹ce do wybrania RealProperties
{
  if (Synchronizer==NULL) return false;
  __try
    {
    TSearchRecList *CurEntry;//WskaŸnik na aktualny element
    //Przechodzimy po calej strukturze
    bool Result=true;
    CurEntry=VirtDir;
    DynamicStruct->MovetoFirstItem(CurEntry);
    while (CurEntry!=NULL)
      {
      if (Terminate) return false;
      //dla ka¿dego katalogu
      Synchronizer->UpdateStatusBar("Rozpoznajê "+CurEntry->Path+"\\"+CurEntry->F.Name);

      if (IsFolder(CurEntry))
        {
        //robimy analize wnetrza
        Result|=SetStructureAnalystProperties(CurEntry->Down);
        //i ustawiamy wpisy analizy dla samego katalogu
        FldrNameAnalist(CurEntry);
        FldrContentAnalist(CurEntry);
        }
       else
      //Je¿eli nie trafilismy na katalog
        {
        FileNameAnalist(CurEntry);
        FileBodyAnalist(CurEntry);
        }
      //No i koniec - analizujemy nastepnego
      CurEntry=CurEntry->Next;
    }
   return Result;
   }
  __except(EXCEPTION_EXECUTE_HANDLER)
   {return false;}
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::FileNameAnalist(Analyzerdynamics::PSearchRecList Entry)
{
  SearchEntryType ExtensionType;
  ExtensionType=GetTypeFromExtension(ExtractFileExt(Entry->F.Name));
  if (ExtensionType!=setUnknown)
    //Okreslamy prawdopodobienstwo ze ExtensionType jest poprawny
   {
    Entry->NameAnalist.Probability=500; //Wartosc poczatkowa: 50%
      //Kryterium: rozmiar pliku
    if ((ExtensionType==setAnimation)&&(Entry->F.Size>(512*1024)))
      Entry->NameAnalist.Probability+=((1000-Entry->NameAnalist.Probability)/2);
    if ((ExtensionType==setAnimation)&&(Entry->F.Size<(64*1024)))
      Entry->NameAnalist.Probability/=2;

    if ((ExtensionType==setPicture)&&(Entry->F.Size>(4*1024))&&(Entry->F.Size<(200*1024*1024)))
      Entry->NameAnalist.Probability+=((1000-Entry->NameAnalist.Probability)/2);
    if ((ExtensionType==setPicture)&&(Entry->F.Size<(1024)))
      Entry->NameAnalist.Probability/=2;

    if ((ExtensionType==setMusic)&&(Entry->F.Size>(256*1024))&&(Entry->F.Size<(400*1024*1024)))
      Entry->NameAnalist.Probability+=((1000-Entry->NameAnalist.Probability)/2);
    if ((ExtensionType==setPicture)&&(Entry->F.Size<(5*1024)))
      Entry->NameAnalist.Probability/=2;

    if ((ExtensionType==setLink)&&(Entry->F.Size>(50*1024))&&(Entry->F.Size<(9*1024)))
      Entry->NameAnalist.Probability+=((1000-Entry->NameAnalist.Probability)/2);
      else Entry->NameAnalist.Probability/=2;

   }
  else
    Entry->NameAnalist.Probability=10;
  //Mamy prawdopodobienstwa, teraz wystarczy wypelnic reszte pól
  Entry->NameAnalist.Name=PrepareNameFromFName(Entry->F);
  Entry->NameAnalist.EntryType=ExtensionType;
  Entry->NameAnalist.CategoryName=GetCatNameFromType(ExtensionType);
  Entry->NameAnalist.DescBefore="";
  Entry->NameAnalist.DescAfter=PrepareDescFromFName(Entry->F);
  //no i koniec...
  return true;
}
//---------------------------------------------------------------------------
SearchEntryType __fastcall TDiscAnalyzer::GetTypeFromExtension(AnsiString FileExt)
//  Rozpoznaje typ na podstawie rozszerzenia pliku.
//  Znane rozszerzenia ladowane s¹ przy starcie,
//  s¹ pamietane w klasie Synchronizer
{
  if (Synchronizer==NULL) return setUnknown;
  FileExt=AnsiUpperCase(FileExt);
    if (Synchronizer->ExtOfMusic->IndexOf(FileExt)>0)
      { return setMusic;}
    if (Synchronizer->ExtOfPicture->IndexOf(FileExt)>0)
      { return setPicture;}
    if (Synchronizer->ExtOfProgram->IndexOf(FileExt)>0)
      { return setProgram;}
    if (Synchronizer->ExtOfAnimation->IndexOf(FileExt)>0)
      { return setAnimation;}
    if (Synchronizer->ExtOfData->IndexOf(FileExt)>0)
      { return setData;}
    if (Synchronizer->ExtOfVarious->IndexOf(FileExt)>0)
      { return setVarious;}
    if (Synchronizer->ExtOfArchive->IndexOf(FileExt)>0)
      { return setArchive;}
  return setUnknown;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TDiscAnalyzer::GetCatNameFromType(Analyzerdynamics::SearchEntryType CatType)
//Na podstawie typu, pobiera jedn¹ z pasuj¹cych nazw tekstowych kategorii
//Jest to niejednoznaczne - u¿ywac jak najrzadziej
//  Najlepiej by bylo pozbyc sie tej funkcji
{
//  with (Synchronizer) do
  if (Synchronizer==NULL) return "Unknown";
  switch (CatType)
    {
    case setMusic:
        if (Synchronizer->ExtOfMusic->Count>0)
          return Synchronizer->ExtOfMusic->Strings[0];
    case setPicture:
        if (Synchronizer->ExtOfPicture->Count>0)
          return Synchronizer->ExtOfPicture->Strings[0];
    case setProgram:
        if (Synchronizer->ExtOfProgram->Count>0)
          return Synchronizer->ExtOfProgram->Strings[0];
    case setAnimation:
        if (Synchronizer->ExtOfAnimation->Count>0)
          return Synchronizer->ExtOfAnimation->Strings[0];
    case setData:
        if (Synchronizer->ExtOfData->Count>0)
          return Synchronizer->ExtOfData->Strings[0];
    case setVarious:
        if (Synchronizer->ExtOfVarious->Count>0)
          return Synchronizer->ExtOfVarious->Strings[0];
    case setArchive:
        if (Synchronizer->ExtOfArchive->Count>0)
          return Synchronizer->ExtOfArchive->Strings[0];
    default:
        return "Various";
    }
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::RemoveEquPartsOfNamesInChildren(TSearchRecList *ParentEntry)
//  Usuwa z nazw wpisów elementy powtarzaj¹ce sie,
//  takie jak nazwa wykonawcy w muzyce albo numery utworów
{
  TSearchRecList *CurEntry;//WskaŸnik na aktualny element
  TSimilarNamesList *SimList=new TSimilarNamesList();
  //Wysy³amy do SimListy wszystkie pliki z katalogu
  CurEntry=ParentEntry->Down;
  DynamicStruct->MovetoFirstItem(CurEntry);
  while (CurEntry!=NULL)
    {
    //dodajemy do SimListy
    SimList->AddNew(CurEntry);
    CurEntry=CurEntry->Next;
    }
  //Dokonujemy redukcji grup podobieñstwa
  SimList->ReduceGroups();
  //Robimy to, o co chodzi³o
  SimList->RenameGroupedFiles();
  //i czyœcimy
  delete SimList;
  return true;
}
//---------------------------------------------------------------------------
void __fastcall TDiscAnalyzer::SetFileRealProperties(TSearchRecList *Entry)
//Ustawia RealProperties na podstawie
//wczesniej przeprowadzonej analizy Name i Body.
//U¿ywany w SetStructureAnalystProperties

//Wszystkie prawdopodob. s¹ w promilach (pewnosc=1000)
{
  if (Entry->BodyAnalist.Probability>=Entry->NameAnalist.Probability)
    //Jezeli analiza Body jest dominuj¹ca
    {
    if (Entry->BodyAnalist.EntryType!=setUnknown) Entry->RealProperties.EntryType=Entry->BodyAnalist.EntryType;
     else Entry->RealProperties.EntryType=Entry->NameAnalist.EntryType;
    if (Entry->BodyAnalist.Name.Length()>0) Entry->RealProperties.Name=Entry->BodyAnalist.Name;
     else Entry->RealProperties.Name=Entry->NameAnalist.Name;
    if (Entry->BodyAnalist.CategoryName.Length()>0) Entry->RealProperties.CategoryName=Entry->BodyAnalist.CategoryName;
     else Entry->RealProperties.CategoryName=Entry->NameAnalist.CategoryName;
    if (Entry->BodyAnalist.DescBefore.Length()>0) Entry->RealProperties.DescBefore=Entry->BodyAnalist.DescBefore;
     else Entry->RealProperties.DescBefore=Entry->NameAnalist.DescBefore;
    if (Entry->BodyAnalist.DescAfter.Length()>0) Entry->RealProperties.DescAfter=Entry->BodyAnalist.DescAfter;
     else Entry->RealProperties.DescAfter=Entry->NameAnalist.DescAfter;
    }
   else
    //Jezeli analiza Name jest dominuj¹ca
    {
    if (Entry->NameAnalist.EntryType!=setUnknown) Entry->RealProperties.EntryType=Entry->NameAnalist.EntryType;
     else Entry->RealProperties.EntryType=Entry->BodyAnalist.EntryType;
    if (Entry->NameAnalist.Name.Length()>0) Entry->RealProperties.Name=Entry->NameAnalist.Name;
     else Entry->RealProperties.Name=Entry->BodyAnalist.Name;
    if (Entry->NameAnalist.CategoryName.Length()>0) Entry->RealProperties.CategoryName=Entry->NameAnalist.CategoryName;
     else Entry->RealProperties.CategoryName=Entry->BodyAnalist.CategoryName;
    if (Entry->NameAnalist.DescBefore.Length()>0) Entry->RealProperties.DescBefore=Entry->NameAnalist.DescBefore;
     else Entry->RealProperties.DescBefore=Entry->BodyAnalist.DescBefore;
    if (Entry->NameAnalist.DescAfter.Length()>0) Entry->RealProperties.DescAfter=Entry->NameAnalist.DescAfter;
     else Entry->RealProperties.DescAfter=Entry->BodyAnalist.DescAfter;
    }
}
//---------------------------------------------------------------------------
void __fastcall TDiscAnalyzer::SetFldrRealProperties(TSearchRecList *Entry)
//Ustawia RealProperties na podstawie
//wczesniejszej analizy Name i Content.
//U¿ywany w SetStructureAnalystProperties

//Wszystkie prawdopodob. s¹ w promilach (pewnosc=1000)
{
  if (Entry->BodyAnalist.Probability>=Entry->NameAnalist.Probability)
    //Jezeli analiza Content jest dominuj¹ca
    {
    if (Entry->BodyAnalist.EntryType!=setUnknown) Entry->RealProperties.EntryType=Entry->BodyAnalist.EntryType;
     else Entry->RealProperties.EntryType=Entry->NameAnalist.EntryType;
    if (Entry->BodyAnalist.Name.Length()>0) Entry->RealProperties.Name=Entry->BodyAnalist.Name;
     else Entry->RealProperties.Name=Entry->NameAnalist.Name;
    if (Entry->BodyAnalist.CategoryName.Length()>0) Entry->RealProperties.CategoryName=Entry->BodyAnalist.CategoryName;
     else Entry->RealProperties.CategoryName=Entry->NameAnalist.CategoryName;
    if (Entry->BodyAnalist.DescBefore.Length()>0) Entry->RealProperties.DescBefore=Entry->BodyAnalist.DescBefore;
     else Entry->RealProperties.DescBefore=Entry->NameAnalist.DescBefore;
    if (Entry->BodyAnalist.DescAfter.Length()>0) Entry->RealProperties.DescAfter=Entry->BodyAnalist.DescAfter;
     else Entry->RealProperties.DescAfter=Entry->NameAnalist.DescAfter;
    }
   else
    //Jezeli analiza Name jest dominuj¹ca
    {
    if (Entry->NameAnalist.EntryType!=setUnknown) Entry->RealProperties.EntryType=Entry->NameAnalist.EntryType;
     else Entry->RealProperties.EntryType=Entry->BodyAnalist.EntryType;
    if (Entry->NameAnalist.Name.Length()>0) Entry->RealProperties.Name=Entry->NameAnalist.Name;
     else Entry->RealProperties.Name=Entry->BodyAnalist.Name;
    if (Entry->NameAnalist.CategoryName.Length()>0) Entry->RealProperties.CategoryName=Entry->NameAnalist.CategoryName;
     else Entry->RealProperties.CategoryName=Entry->BodyAnalist.CategoryName;
    if (Entry->NameAnalist.DescBefore.Length()>0) Entry->RealProperties.DescBefore=Entry->NameAnalist.DescBefore;
     else Entry->RealProperties.DescBefore=Entry->BodyAnalist.DescBefore;
    if (Entry->NameAnalist.DescAfter.Length()>0) Entry->RealProperties.DescAfter=Entry->NameAnalist.DescAfter;
     else Entry->RealProperties.DescAfter=Entry->BodyAnalist.DescAfter;
    }
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::AnalyzeEntryList(TSearchRecList *UpEntry)
//U¿ywany przez AnalyzeRootDir do tworzenia listy plików
//wszystkich katalogów plyty z wyj¹tkiem glównego
{
  if (Synchronizer==NULL) return false;
  TSearchRec F;
  TSearchRecList *CurEntry; //Najnowszy wpis tablicy plików
  int ReadErrorCode;
  String Path;
  int Level;
  //ustaw zmienne lokalne
  CurEntry=UpEntry;
  Level=0;Path="";
  do
    {
    Level++;
    Path="\\"+CurEntry->F.Name+Path;
    CurEntry=CurEntry->Up;
    F.Name=" ";
    if (Level>128) return false;//zabezpieczenie w zasadzie zbedne, ale...
    } while (CurEntry!=NULL);
  CurEntry=UpEntry;
  Synchronizer->UpdateStatusBar("Czytam "+Path);
  Path=RootDir+Path;
  //utwórz liste plików
  ReadErrorCode=FindFirst(Path+"\\*.*",faAnyFile,F);
  while ((F.Name[1]=='.')&&(ReadErrorCode==0)) ReadErrorCode=FindNext(F);
  bool Result;
  if (ReadErrorCode==0)
    {
    DynamicStruct->NewSRecDown(CurEntry,F);
    CurEntry->Path=Path;
    while (FindNext(F)==0)
      {
      DynamicStruct->NewSRecAfter(CurEntry,F);
      CurEntry->Path=Path;
      }
    Result=true;
    }
   else
    {
    CurEntry=NULL;
    Result=false;
    }
  FindClose(F);
  //przeanalizuj listê katalogów
  while (CurEntry!=NULL)
    {
    if (Terminate) return false;
    //wywo³aj analizê dla ka¿dego katalogu
    if (IsFolder(CurEntry))
    Result=AnalyzeEntryList(CurEntry);
    CurEntry=CurEntry->Prev;
    }
  return Result;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::AnalyzeRootDir(System::TObject* Sender)
//Tworzy peln¹ liste plików i katalogów, ale bez ustawiania RealProperties
//Wywolywane przez AnalyzeDrive
{
  __try
    {
    TSearchRec F;
    TSearchRecList *CurEntry; //Ostatni wpis tablicy plików
    int ReadErrorCode;
    //utwórz liste plików w glownym katakogu
    CurEntry=NULL;ReadErrorCode=0;
    if (FindFirst(RootDir+"\\*.*",faAnyFile,F)==0)
      {
      while ((F.Name[1]=='.')&&(ReadErrorCode==0)) ReadErrorCode=FindNext(F);
      if (ReadErrorCode==0)
        {
        DynamicStruct->NewSRecAfter(CurEntry,F);
        while (FindNext(F)==0) DynamicStruct->NewSRecAfter(CurEntry,F);
        }
      FindClose(F);
      }
     else
      {
      FindClose(F);
      return false;
      }
    //przeanalizuj listê katalogów
    if (CurEntry!=NULL)
      {
      do
        {
        if (Terminate) return false;
        //wywolaj analizê dla ka¿dego katalogu
        if ((CurEntry->F.Attr & faDirectory)!=0)
           AnalyzeEntryList(CurEntry);
        CurEntry=CurEntry->Prev;
        } while (CurEntry!=NULL);
      }
     else
      return false;
    }
  __except(EXCEPTION_EXECUTE_HANDLER)
    {return false;}
  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::RecognizedArchiveStart(TSearchRecList *Entry,char *FileStart)
//U¿ywany w FileBodyAnalist
//sprawdza czy pliki s¹ archiwami
{
    //Do "podru¿y" po strukturze pliku video
    String bMagic;
    bool fileIdentified;
    uint bDataSize;
    //Do zapamiêtania wlasnosci strumieni
    fileIdentified=false;
    uint loadedBlockSize=min(FileBodyHeaderSize,uint(Entry->F.Size));

  //Format ZIP
  bMagic="xxxx";
  StrLCopy((char *)bMagic.data(),FileStart,4);
  if ((!fileIdentified)&&(bMagic=="PK\x03\x04")&&(Entry->F.Size>80))
      {
      fileIdentified=true;
      Entry->BodyAnalist.DescAfter="ZIP";
      Entry->BodyAnalist.Probability=800;
      };
  //Format RAR
  if ((!fileIdentified)&&(bMagic=="Rar!")&&(Entry->F.Size>80))
      {
      fileIdentified=true;
      Entry->BodyAnalist.DescAfter="RAR";
      Entry->BodyAnalist.Probability=800;
      };
  //Format HYP
  bMagic="xxx";
  StrLCopy((char *)bMagic.data(),FileStart,3);
  bDataSize=GetFromBufLittleEndianDWord(FileStart,4);//Na offsecie 4 jest rozmiar bez kompresji
  if ((!fileIdentified)&&((bMagic=="\x1AHP")||(bMagic=="\x1AST"))&&(Entry->F.Size>40)&&(bDataSize+128>(uint)Entry->F.Size)&&(bDataSize<(uint)Entry->F.Size+128))
      {
      fileIdentified=true;
      Entry->BodyAnalist.DescAfter="Hyper Archive";
      Entry->BodyAnalist.Probability=800;
      };
  //Format ARJ
  bMagic="xx";
  StrLCopy((char *)bMagic.data(),FileStart,2);
  if ((!fileIdentified)&&(bMagic=="\xEA\x60")&&(Entry->F.Size>80))
      {
      fileIdentified=true;
      Entry->BodyAnalist.DescAfter="ARJ";
      Entry->BodyAnalist.Probability=600;
      };
  if ((!fileIdentified)&&(bMagic=="!\x8B")&&(Entry->F.Size>40))
      {
      fileIdentified=true;
      Entry->BodyAnalist.DescAfter="GZIP";
      Entry->BodyAnalist.Probability=600;
      };

  //Format JAR (ale nie ten od Javy...)
  bMagic="xxxxxx";
  StrLCopy((char *)bMagic.data(),FileStart+14,6);
  if ((!fileIdentified)&&(bMagic=="\x1AJAR\x1B\x00")&&(Entry->F.Size>40))
      {
      fileIdentified=true;
      Entry->BodyAnalist.DescAfter="JAR";
      Entry->BodyAnalist.Probability=600;
      };


/*  //Format Animatic Film (nie testowane)
  bMagic="xxxx";
  StrLCopy((char *)bMagic.data(),FileStart+48,4);
  if ((!fileIdentified)&&((bMagic=="\x18\x28\x18\x27")))
      {
      fileIdentified=true;
      iWidth=GetFromBufLittleEndianWord(FileStart,40);
      iHeight=GetFromBufLittleEndianWord(FileStart,42);
      Entry->BodyAnalist.DescAfter="Animatic Film";
      Entry->BodyAnalist.Probability=700;
      };
*/
  if (fileIdentified)
    {
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        Entry->BodyAnalist.EntryType=setArchive;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
    return true;
    }
  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::RecognizedMpegStart(PSearchRecList Entry, char *FileStart)
{
  //ilosc bajtow w buforze
  uint loadedBlockSize=min(FileBodyHeaderSize,uint(Entry->F.Size));
  bool Result=false;

  InpMpeg->ReinitParser(FileStart,loadedBlockSize);
  InpMpeg->ParseMPEGFile();

  if ((InpMpeg->frame_rate>0)&&(InpMpeg->height>1)&&(InpMpeg->width>1))
    {
    String ChanlsStr="ST";
    if (InpMpeg->audChannels==3) ChanlsStr="MO";
    Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
    Entry->BodyAnalist.DescBefore="";
    String AudioStr="";
    String VideoStr="";
    if (InpMpeg->samprate>0)
        AudioStr=IntToStr(InpMpeg->samprate/1000)+"kHz "+ChanlsStr+" Layer-"+IntToStr(4-InpMpeg->layer)+" ";
    if ((InpMpeg->width>0)&&(InpMpeg->height>0))
        {
        if (InpMpeg->frame_rate>1)
            VideoStr=IntToStr(InpMpeg->width)+"x"+IntToStr(InpMpeg->height)+" "+FloatToStrF(Extended(InpMpeg->frame_rate)/1000, ffFixed, 18, 2)+"fps ";
          else
            VideoStr=IntToStr(InpMpeg->width)+"x"+IntToStr(InpMpeg->height)+" ";
        }
    Entry->BodyAnalist.DescAfter=AudioStr+VideoStr+"MPEG";
    Entry->BodyAnalist.EntryType=setAnimation;
    Entry->BodyAnalist.Probability=700;
    Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
    Result=true;
    }

  InpMpeg->ReinitParser(NULL,0);
  return Result;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::RecognizedRiffStart(TSearchRecList *Entry,char *FileStart)
//U¿ywany w FileBodyAnalist
//sprawdza czy pliki s¹ typu RIFF
{
    //Do "podru¿y" po strukturze RIFF-a
  uint loadedBlockSize;
  String bMagic;
  bool isBigEndian;
  bool fileTruncated;
  bool fileMultiplied;
  uint bDataSize;
    // sprawdzamy czy to w ogóle jest RIFF/RIFX/FORM
   bMagic="xxxx";
  StrLCopy((char *)bMagic.data(),FileStart,4);
  if ((bMagic=="RIFF"))
      {
      isBigEndian=false;
      }
    else
  if ((bMagic=="RIFX"))
      {
      isBigEndian=true;
      }
    else
  if ((bMagic=="FORM"))
      {
      isBigEndian=true;
      }
    else
      {
      //Jak nie ma odpowiedniego typu - koniec
      return false;
      };
  //Pobieramy rozmiar pliku
      if (isBigEndian)
          bDataSize=GetFromBufBigEndianDWord(FileStart,4);
        else
          bDataSize=GetFromBufLittleEndianDWord(FileStart,4);
  //i u¿ywamy do okreslenia czy jest to plik pojenynczy
      if (((uint)(Entry->F.Size) > bDataSize+(bDataSize/50)+1024))
          fileMultiplied=true;
        else
          fileMultiplied=false;
      if ((uint(Entry->F.Size)+1024 < bDataSize))
          fileTruncated=true;
        else
          fileTruncated=false;
  //ilosc bajtow w buforze
  loadedBlockSize=min(FileBodyHeaderSize,uint(Entry->F.Size));
  //Do podró¿y po strukturze
  String bMainType;
  uint bMacroChunkPos;
  String bMacroChunkName;
  String bMacroChunkType;
  uint bSubChunkPos;
  String bSubChunkName;
  String bSubChunkType;
  String currentStream;
    //Do zapamiêtania wlasnosci strumieni
   uint iWidth;
   uint iHeight;
   uint iColorBPP;
   String iCodec;
   float fFPS;
   uint iKBperSec;
   uint sSmpRate;
   uint sChannels;
   String sCodec;
   uint sBitsPesSmp;
   uint cLength;
   uint cTmpVal;
   String cTmpStr;
  //inicjacja zmiennych
  bMainType="xxxx";
  bMacroChunkName="xxxx";
  bMacroChunkType="xxxx";
  bSubChunkName="xxxx";
  bSubChunkType="xxxx";
  //Okreslamy typ pliku
  StrLCopy((char *)bMainType.data(),FileStart+8,4);
    iWidth=0;
    iHeight=0;
    iColorBPP=0;
    iCodec="";
    fFPS=0.0;
    iKBperSec=0;
    sSmpRate=0;
    sChannels=0;
    sCodec="";
    sBitsPesSmp=0;
    cLength=0;
  bMacroChunkPos=12;//Zaczynamy chunki od tej pozycji bo 12b ma naglowek
  //Teraz analizujemy kolejne chunksy z pliku
  while ((bMacroChunkPos+12<loadedBlockSize))
      {
      //Pobieramy rozmiar tego chunka
      if (isBigEndian)
          bDataSize=GetFromBufBigEndianDWord(FileStart,bMacroChunkPos+4);
        else
          bDataSize=GetFromBufLittleEndianDWord(FileStart,bMacroChunkPos+4);
      //pobieramy jego typ
      StrLCopy((char *)bMacroChunkName.data(),FileStart+bMacroChunkPos,4);
      StrLCopy((char *)bMacroChunkType.data(),FileStart+bMacroChunkPos+8,4);
      //i ustawiamy polo¿enie nastepnego
      if (bMacroChunkName=="LIST")
          bSubChunkPos=bMacroChunkPos+12;
        else
          bSubChunkPos=bMacroChunkPos;
      if (bDataSize>0)
          bMacroChunkPos=min(bMacroChunkPos+bDataSize+8,loadedBlockSize);//(bDataSize mod 2)+
        else
          bMacroChunkPos=loadedBlockSize;
      //Je¿eli typ tego chunka jest LIST to sprawdzamy podchunki
      if (bMacroChunkName=="LIST")
          {
          currentStream="xxxx";
          while ((bSubChunkPos+12<bMacroChunkPos))
              {
              //Pobieramy pozycje nastepnego subchunka
              if (isBigEndian)
                  bDataSize=GetFromBufBigEndianDWord(FileStart,bSubChunkPos+4);
                else
                  bDataSize=GetFromBufLittleEndianDWord(FileStart,bSubChunkPos+4);
              //Analizujemy aktualnego subchunka
              StrLCopy((char *)bSubChunkName.data(),FileStart+bSubChunkPos,4);
              // oczywiscie nie wszystkie subchunki maj¹ typ
              StrLCopy((char *)bSubChunkType.data(),FileStart+bSubChunkPos+8,4);
              if (bSubChunkName=="avih")
                  {
                  if (isBigEndian)
                    {
                    iWidth=GetFromBufBigEndianDWord(FileStart,bSubChunkPos+40);
                    iHeight=GetFromBufBigEndianDWord(FileStart,bSubChunkPos+44);
                    }
                   else
                    {
                    cTmpVal=GetFromBufLittleEndianDWord(FileStart,bSubChunkPos+8);
                    if (cTmpVal>0)
                        fFPS=1000000.0 / cTmpVal;
                    if (cTmpVal>0)
                        cLength=(Extended(GetFromBufLittleEndianDWord(FileStart,bSubChunkPos+24))*cTmpVal) / 1000000.0;
                    if (GetFromBufLittleEndianDWord(FileStart,bSubChunkPos+32)<2)
                        sCodec="noSnd";
                    iKBperSec=GetFromBufLittleEndianDWord(FileStart,bSubChunkPos+12) / 1024;
                    iWidth=GetFromBufLittleEndianDWord(FileStart,bSubChunkPos+40);
                    iHeight=GetFromBufLittleEndianDWord(FileStart,bSubChunkPos+44);
                    };
                  }
                else
              if (bSubChunkName=="strh")
                  {
                  StrLCopy((char *)currentStream.data(),FileStart+bSubChunkPos+8,4);
                  if (currentStream=="vids")
                       {
                       iCodec="xxxx";
                       StrLCopy((char *)iCodec.data(),FileStart+bSubChunkPos+12,4);
                       };
                  }
                else
              if (bSubChunkName=="strf")
                  {
                  if (currentStream=="auds")
                       {
                       //W³asnoœci strumienia Audio
                       sCodec=IdentifyACMCodec(GetFromBufLittleEndianWord(FileStart,bSubChunkPos+8));
                       sSmpRate=GetFromBufLittleEndianDWord(FileStart,bSubChunkPos+12);
                       sChannels=GetFromBufLittleEndianWord(FileStart,bSubChunkPos+10);
                       };
                  }
                else
              if (bSubChunkName=="strn")
                  {
                  }
                else
              if ((bSubChunkName=="00dc")||(bSubChunkName=="00db"))
                  {
                  }
                else
              if (bSubChunkName=="01wb")
                  {
                  }
                else
              if (bSubChunkName=="LIST")
                  {
                  //Jak jest LIST to pomijamy na tym poziomie...
                  //(dostepny tylko jeden poziom 'wgl¹b')
                  bDataSize=4;
                  currentStream="xxxx";
                  }
                else
              if (bSubChunkName=="JUNK")
                  {
                  }
                else
                  {
                  };
              //Przechodzimy na nastepnego subchunka
              bDataSize += 8+(bDataSize % 2);
              if (bDataSize>0)
                  bSubChunkPos=min(bSubChunkPos+bDataSize,loadedBlockSize);
                else
                  bSubChunkPos=bMacroChunkPos;
              };//while ((bSubChunkPos+...
          } else
      if (bMacroChunkName=="fmt ")
          {
                       //W³asnoœci strumienia Audio - ten sam kod co przy AVI AUDS
                       sCodec=IdentifyACMCodec(GetFromBufLittleEndianWord(FileStart,bSubChunkPos+8));
                       sSmpRate=GetFromBufLittleEndianDWord(FileStart,bSubChunkPos+12);
                       sChannels=GetFromBufLittleEndianWord(FileStart,bSubChunkPos+10);
                       sBitsPesSmp=GetFromBufLittleEndianWord(FileStart,bSubChunkPos+22);
          } else
      if (bMacroChunkName=="data")
          {
          if ((bMainType=="WAVE")&&(sCodec=="PCM")&&(cLength==0)&&(sBitsPesSmp>7)&&(sChannels>0))
              {
              cTmpVal=GetFromBufLittleEndianDWord(FileStart,bSubChunkPos+4);
              if (cTmpVal>0)
                  {
                  cLength=((cTmpVal / sSmpRate) / (sBitsPesSmp / 8)) / sChannels;
                  if (cLength<1) cLength=1;
                  };
              };
          } else
      if (bMacroChunkName=="fact")
          {
          if ((bMainType=="WAVE")&&(cLength==0)&&(sChannels>0))
              {
              cTmpVal=GetFromBufLittleEndianDWord(FileStart,bSubChunkPos+8);
              if (cTmpVal>0)
                  {
                  if (sBitsPesSmp>7)
                      cLength=((cTmpVal / sSmpRate) / (sBitsPesSmp / 8)) / sChannels;
                    else
                      cLength=((cTmpVal / sSmpRate)) / sChannels;
                  if (cLength<1) cLength=1;
                  };
              };
          } else
      if (bMacroChunkName=="ANNO")
          {
          } else
      if (bMacroChunkName=="BMHD")
          {
          if (bMainType=="ILBM")
              {
              if (isBigEndian)
                {
                iWidth=GetFromBufBigEndianWord(FileStart,bSubChunkPos+8);
                iHeight=GetFromBufBigEndianWord(FileStart,bSubChunkPos+10);
                }
               else
                {
                iWidth=GetFromBufLittleEndianWord(FileStart,bSubChunkPos+8);
                iHeight=GetFromBufLittleEndianWord(FileStart,bSubChunkPos+10);
                };
                iColorBPP=(unsigned char)(FileStart[bSubChunkPos+16]);
              };//if (bSubType='ILBM')
          } else
      if (bMacroChunkName=="JUNK")
          {
          } else
      if (bMacroChunkName=="idx1")
          {
          } else
          {
          //Wejscie tu oznacza nie rozpoznan¹ etykiete lub koniec pliku
          bMacroChunkPos=bMacroChunkPos;
          };
      };//while ((bMacroChunkPos+...
  //Koniec analizy - teraz sprawdzamy co rozpoznalismy
  if (bMainType=="AVI ")
      {
      Entry->BodyAnalist.EntryType=setAnimation;
      Entry->BodyAnalist.DescAfter="AVI";
      }
    else
  if (bMainType=="WAVE")
      {
      Entry->BodyAnalist.EntryType=setMusic;
      Entry->BodyAnalist.DescAfter="Waveform";
      }
    else
  if (bMainType=="ILBM")
      {
      Entry->BodyAnalist.EntryType=setPicture;
      Entry->BodyAnalist.DescAfter="iLBM";
      }
    else
  if (bMainType=="PAL ")
      {
      Entry->BodyAnalist.EntryType=setData;
      Entry->BodyAnalist.DescAfter=bMagic;
      Entry->BodyAnalist.DescAfter+=" Palette";
      }
    else
  if (bMainType=="RDIB")
      {
      Entry->BodyAnalist.EntryType=setPicture;
      Entry->BodyAnalist.DescAfter=bMagic;
      Entry->BodyAnalist.DescAfter+=" DIB";
      }
    else
  if (bMainType=="RMID")
      {
      Entry->BodyAnalist.EntryType=setMusic;
      Entry->BodyAnalist.DescAfter=bMagic;
      Entry->BodyAnalist.DescAfter+=" MIDI";
      }
    else
  if (bMainType=="RMMP")
      {
      Entry->BodyAnalist.EntryType=setAnimation;
      Entry->BodyAnalist.DescAfter=bMagic;
      Entry->BodyAnalist.DescAfter+=" MultimediaMovie";
      }
    else
  if (bMainType=="TDDD")
      {
      Entry->BodyAnalist.EntryType=setData;
      Entry->BodyAnalist.DescAfter="TurboSilver 3D Data";
      }
    else
  if (bMainType=="LWOB")
      {
      Entry->BodyAnalist.EntryType=setData;
      Entry->BodyAnalist.DescAfter="LightWave 3D object";
      }
    else
  if (bMainType=="CDR9")
      {
      Entry->BodyAnalist.EntryType=setData;
      Entry->BodyAnalist.DescAfter="CorelDraw9 Graphics";
      }
    else
  if (bMainType=="CMX1")
      {
      Entry->BodyAnalist.EntryType=setData;
      Entry->BodyAnalist.DescAfter="Corel Present. Exchange CMX";
      }
    else
      {
      Entry->BodyAnalist.EntryType=setData;
      Entry->BodyAnalist.DescAfter=bMainType;
      Entry->BodyAnalist.DescAfter+=" ";
      Entry->BodyAnalist.DescAfter+=bMagic;
      };
  Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
  /* Popularne typy:
     AVI            Audio Video Inter.
     PAL            RIFF Palette Format
     RDIB           RIFF Device Indep}ent Bitmap Format
     RMID           RIFF MIDI Format
     RMMP           RIFF Multimedia Movie File Format
     WAVE           Waveform Audio Format
     ILBM           iLBM
     TDDD           TurboSilver 3D Data */
  Entry->BodyAnalist.Probability=700;
  Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
  Entry->BodyAnalist.DescBefore="";
  if (fileMultiplied)
      {
      Entry->BodyAnalist.DescAfter="Multifile "+Entry->BodyAnalist.DescAfter;
      return true;
      };

  if ((UpperCase(iCodec)=="DIVX")) iCodec="DivX"; else
  if ((UpperCase(iCodec)=="DIV3")) iCodec="DivX3"; else
  if ((UpperCase(iCodec)=="DIV4")) iCodec="DivX4"; else
  if ((UpperCase(iCodec)=="DIV5")) iCodec="DivX5"; else
  if ((UpperCase(iCodec)=="DX50")) iCodec="DivX5"; else
  if ((UpperCase(iCodec)=="XVID")) iCodec="XviD"; else
  if ((UpperCase(iCodec)=="MP42")) iCodec="MP42"; else
  if ((UpperCase(iCodec)=="WMV3")) iCodec="WMVid3"; else
  if ((UpperCase(iCodec)=="MSVC")) iCodec="MsVideoC"; else
  if ((UpperCase(iCodec)=="CVID")) iCodec="CinepakVideo"; else
  if ((UpperCase(iCodec)=="MRLE")) iCodec="mRLE";
  if ((iCodec.Length()>0))
      {
      for (int n=1;n<=iCodec.Length();n++)
        if (iCodec[n]<32) iCodec[n]='0'+iCodec[n];
      if (Entry->BodyAnalist.DescAfter.Length()>0)
          Entry->BodyAnalist.DescAfter=" "+Entry->BodyAnalist.DescAfter;
      Entry->BodyAnalist.DescAfter=iCodec+Entry->BodyAnalist.DescAfter;
      };

  if ((iKBperSec>0))
      Entry->BodyAnalist.DescAfter=IntToStr(iKBperSec)+"kB/s "+Entry->BodyAnalist.DescAfter;
  if ((iColorBPP>0))
      Entry->BodyAnalist.DescAfter=IntToStr(iColorBPP)+"-bit "+Entry->BodyAnalist.DescAfter;
  if ((fFPS>0.1)&&(fFPS<180))
      Entry->BodyAnalist.DescAfter=FloatToStrF(fFPS,ffFixed,18,2)+"fps "+Entry->BodyAnalist.DescAfter;
  if ((iWidth>0)&&(iHeight>0))
      Entry->BodyAnalist.DescAfter=IntToStr(iWidth)+"x"+IntToStr(iHeight)+" "+Entry->BodyAnalist.DescAfter;

  if ((sChannels>0))
      {
      if ((sChannels==1))
          Entry->BodyAnalist.DescAfter="MO "+Entry->BodyAnalist.DescAfter;
        else
      if ((sChannels==2))
          Entry->BodyAnalist.DescAfter="ST "+Entry->BodyAnalist.DescAfter;
        else
          Entry->BodyAnalist.DescAfter=IntToStr(sChannels)+"chn "+Entry->BodyAnalist.DescAfter;
      };
  if ((sCodec!=""))
      Entry->BodyAnalist.DescAfter=sCodec+" "+Entry->BodyAnalist.DescAfter;
  if ((sBitsPesSmp>0))
      Entry->BodyAnalist.DescAfter=IntToStr(sBitsPesSmp)+"bps "+Entry->BodyAnalist.DescAfter;
  if ((sSmpRate>0)&&(sSmpRate<262145))
      Entry->BodyAnalist.DescAfter=IntToStr(sSmpRate / 1000)+"kHz "+Entry->BodyAnalist.DescAfter;

  if ((cLength>0))
      {
      cTmpStr=IntToStr(cLength % 60);
      if (cTmpStr.Length()<2) cTmpStr="0"+cTmpStr;
      Entry->BodyAnalist.DescAfter=cTmpStr+"s,"+Entry->BodyAnalist.DescAfter;
      cTmpVal=cLength / 60;
      cTmpStr=IntToStr(cTmpVal % 60);
      if (cTmpStr.Length()<2) cTmpStr="0"+cTmpStr;
      Entry->BodyAnalist.DescAfter=cTmpStr+":"+Entry->BodyAnalist.DescAfter;
      cTmpVal=cTmpVal / 60;
      cTmpStr=IntToStr(cTmpVal % 60);
      if (cTmpStr.Length()<2) cTmpStr="0"+cTmpStr;
      if (cTmpVal>0)
          Entry->BodyAnalist.DescAfter=cTmpStr+":"+Entry->BodyAnalist.DescAfter;
      };

  if (fileTruncated)
      Entry->BodyAnalist.DescAfter=Entry->BodyAnalist.DescAfter+", truncated";

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::Recognized3DDataStart(TSearchRecList *Entry,char *FileStart)
//U¿ywany w FileBodyAnalist
//sprawdza czy pliki s¹ danymi programu 3D
{
    // w razie czego mo¿na tu dodac dowolny nowy format
  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::RecognizedVideoStart(TSearchRecList *Entry,char *FileStart)
//U¿ywany w FileBodyAnalist
//sprawdza czy pliki s¹ filmami (oprócz RIFF)
{
    //Do "podru¿y" po strukturze pliku video
    //loadedBlockSize:LongWord;
    String bMagic="xxxx";
    bool fileIdentified;
    uint bDataSize;
    //Do zapamiêtania wlasnosci strumieni
    uint iWidth;
    uint iHeight;
    uint iColorBPP;
    String iCodec;
    float fFPS;
    uint iKBperSec;
    uint sSmpRate;
    uint sChannels;
    String sCodec;
    uint sBitsPesSmp;
    uint cLength;
    uint cTmpVal;
    String cTmpStr;

    fileIdentified=false;
    uint loadedBlockSize=min(FileBodyHeaderSize,uint(Entry->F.Size));
    iWidth=0;
    iHeight=0;
    iColorBPP=0;
    iCodec="";
    fFPS=0;
    iKBperSec=0;
    sSmpRate=0;
    sChannels=0;
    sCodec="";
    sBitsPesSmp=0;
    cLength=0;
  //Format Windows Media Video
  StrLCopy((char *)bMagic.data(),FileStart,4);
  if ((!fileIdentified)&&(bMagic=="\x30\x26\xB2\x75"))
      {
      fileIdentified=true;
      Entry->BodyAnalist.DescAfter="WMVideo";
      //if (loadedBlockSize>???)
      Entry->BodyAnalist.Probability=800;
      };
  //Format DeluxePaint Animation (nie testowane)
  if ((!fileIdentified)&&((bMagic=="LPF ")))
      {
      fileIdentified=true;
      Entry->BodyAnalist.DescAfter="DeluxePaint Animation";
      Entry->BodyAnalist.Probability=700;
      };
  //Format DVI (nie testowane)
  if ((!fileIdentified)&&(bMagic=="\x49\x56\x44\x56"))
      {
      fileIdentified=true;
      Entry->BodyAnalist.DescAfter="DVI Video";
      Entry->BodyAnalist.Probability=700;
      };
  bMagic="xx";
  StrLCopy((char *)bMagic.data(),FileStart+4,2);
  //Format Color Map (nie testowane)
  if ((!fileIdentified)&&((bMagic=="\x23\xB1")))
      {
      bDataSize=GetFromBufLittleEndianDWord(FileStart,1);
      if ((uint(Entry->F.Size)+16>bDataSize)&&(uint(Entry->F.Size)<bDataSize+128))
          {
          fileIdentified=true;
          Entry->BodyAnalist.DescAfter="Animator Color Map";
          Entry->BodyAnalist.Probability=600;
          };
      };
  //Format FLI/FLC
  bMagic="xx";
  StrLCopy((char *)bMagic.data(),FileStart+4,2);
  if ((!fileIdentified)&&((bMagic=="\x11\xAF")||(bMagic=="\x12\xAF")))
      {
      bDataSize=GetFromBufLittleEndianDWord(FileStart,0);
      if ((uint(Entry->F.Size)+128>bDataSize)&&(uint(Entry->F.Size)<bDataSize+1024))
          {
          fileIdentified=true;
          if ((bMagic=="\x11\xAF"))
              Entry->BodyAnalist.DescAfter="FLI Animation";
            else
              Entry->BodyAnalist.DescAfter="FLC Animation";
          iWidth=GetFromBufLittleEndianWord(FileStart,8);
          iHeight=GetFromBufLittleEndianWord(FileStart,10);
          iColorBPP=GetFromBufLittleEndianWord(FileStart,12);
          bDataSize=GetFromBufLittleEndianDWord(FileStart,16);
          if (bDataSize>0)
              {
              fFPS=1000.0 / bDataSize;
              cLength=GetFromBufLittleEndianWord(FileStart,6)*bDataSize / 1000;
              }

          Entry->BodyAnalist.Probability=600;
          }
        else
      if ((bDataSize==12))
          {
          fileIdentified=true;
          Entry->BodyAnalist.DescAfter="Modified FLI";
          iWidth=GetFromBufLittleEndianWord(FileStart,8);
          iHeight=GetFromBufLittleEndianWord(FileStart,10);
          iColorBPP=0;//GetFromBufLittleEndianWord(FileStart,12);
          bDataSize=GetFromBufLittleEndianDWord(FileStart,16);
          fFPS=0;
          cLength=0;
          Entry->BodyAnalist.Probability=600;
          }
      }
  //Format Animatic Film (nie testowane)
  bMagic="xxxx";
  StrLCopy((char *)bMagic.data(),FileStart+48,4);
  if ((!fileIdentified)&&((bMagic=="\x18\x28\x18\x27")))
      {
      fileIdentified=true;
      iWidth=GetFromBufLittleEndianWord(FileStart,40);
      iHeight=GetFromBufLittleEndianWord(FileStart,42);
      Entry->BodyAnalist.DescAfter="Animatic Film";
      Entry->BodyAnalist.Probability=700;
      };


  if (fileIdentified)
    {
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        Entry->BodyAnalist.EntryType=setAnimation;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);

    if ((iCodec!=""))
      Entry->BodyAnalist.DescAfter=iCodec+" "+Entry->BodyAnalist.DescAfter;

    if ((iKBperSec>0))
      Entry->BodyAnalist.DescAfter=IntToStr(iKBperSec)+"kB/s "+Entry->BodyAnalist.DescAfter;
    if ((iColorBPP>0))
      Entry->BodyAnalist.DescAfter=IntToStr(iColorBPP)+"-bit "+Entry->BodyAnalist.DescAfter;
    if ((fFPS>0.1)&&(fFPS<180.0))
      Entry->BodyAnalist.DescAfter=FloatToStrF(fFPS,ffFixed,18,2)+"fps "+Entry->BodyAnalist.DescAfter;
    if ((iWidth>0)&&(iHeight>0))
      Entry->BodyAnalist.DescAfter=IntToStr(iWidth)+"x"+IntToStr(iHeight)+" "+Entry->BodyAnalist.DescAfter;

    if ((sChannels>0))
      {
      if ((sChannels==1))
          Entry->BodyAnalist.DescAfter="MO "+Entry->BodyAnalist.DescAfter;
        else
      if ((sChannels==2))
          Entry->BodyAnalist.DescAfter="ST "+Entry->BodyAnalist.DescAfter;
        else
          Entry->BodyAnalist.DescAfter=IntToStr(sChannels)+"chn "+Entry->BodyAnalist.DescAfter;
      };
    if ((sCodec!=""))
      Entry->BodyAnalist.DescAfter=sCodec+" "+Entry->BodyAnalist.DescAfter;
    if ((sBitsPesSmp>0))
      Entry->BodyAnalist.DescAfter=IntToStr(sBitsPesSmp)+"bps "+Entry->BodyAnalist.DescAfter;
    if ((sSmpRate>0))
      Entry->BodyAnalist.DescAfter=IntToStr(sSmpRate / 1000)+"kHz "+Entry->BodyAnalist.DescAfter;

    if ((cLength>0))
      {
      cTmpStr=IntToStr(cLength % 60);
      if (cTmpStr.Length()<2) cTmpStr="0"+cTmpStr;
      Entry->BodyAnalist.DescAfter=cTmpStr+"s,"+Entry->BodyAnalist.DescAfter;
      cTmpVal=cLength / 60;
      cTmpStr=IntToStr(cTmpVal % 60);
      if (cTmpStr.Length()<2) cTmpStr="0"+cTmpStr;
      Entry->BodyAnalist.DescAfter=cTmpStr+":"+Entry->BodyAnalist.DescAfter;
      cTmpVal=cTmpVal / 60;
      cTmpStr=IntToStr(cTmpVal % 60);
      if (cTmpStr.Length()<2) cTmpStr="0"+cTmpStr;
      if (cTmpVal>0)
          Entry->BodyAnalist.DescAfter=cTmpStr+":"+Entry->BodyAnalist.DescAfter;
      }
    return true;
    }
  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::RecognizedTiffStart(TSearchRecList *Entry,char *FileStart)
//U¿ywany w FileBodyAnalist
//sprawdza czy pliki s¹ mapami bitowymi
{
   bool isBigEndian;
   uint nEntries;
   uint cEntry;
   uint loadedBlockSize;
   String bMagic;
   uint bSubPos;
   uint bSubTag;
   uint bSubDataType;
   ushort fieldDataW;
   uint fieldDataDW;
   uint bWidth;
   uint bHeight;
   ushort bColorBPP;

    // sprawdzamy Tiff - jak Magic nie pasuje to rezygnujemy
  bMagic="xx";
  StrLCopy((char *)bMagic.data(),FileStart,2);
  if ((bMagic=="II"))
      isBigEndian=false;
    else
  if ((bMagic=="MM"))
      isBigEndian=true;
    else
      {
      return false;
      };
  //ilosc bajtow w buforze
  loadedBlockSize=min(FileBodyHeaderSize,uint(Entry->F.Size));
  //do pelnego okreslenia TIFFa trzeba caly plik... ale probujemy
  //Pobieramy offset pierwszego "katalogu"
  if (isBigEndian)
        bSubPos=GetFromBufBigEndianDWord(FileStart,4);
      else
        bSubPos=GetFromBufLittleEndianDWord(FileStart,4);
  //chodzimy po katalogach (jak pierwszego uda sie wczytac to bedzie dobrze...)
    Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
    Entry->BodyAnalist.DescBefore="";
    Entry->BodyAnalist.Probability=700;
    Entry->BodyAnalist.DescAfter="Tiff";
    bWidth=0;bHeight=0;bColorBPP=0;
  while ((bSubPos>7)&&(bSubPos+14<loadedBlockSize))
    {
    //Pobieramy ilosc wpisow w katalogu
    if (isBigEndian)
        nEntries=GetFromBufBigEndianWord(FileStart,bSubPos);
      else
        nEntries=GetFromBufLittleEndianWord(FileStart,bSubPos);
    cEntry=0;
    bSubPos+=2;//omijamy ilosc wpisów

    //Zaczynamy analize wpisow (kazdy ma 12 bajtow)
    while (cEntry<nEntries)
        {
        cEntry++;
        if ((bSubPos+12>loadedBlockSize) || (bSubPos<7))
            break;
        if (isBigEndian)
            {
            bSubTag=GetFromBufBigEndianWord(FileStart,bSubPos);//to jest Tag
            bSubDataType=GetFromBufBigEndianWord(FileStart,bSubPos+2);
            fieldDataW=GetFromBufBigEndianWord(FileStart,bSubPos+8);
            fieldDataDW=GetFromBufBigEndianDWord(FileStart,bSubPos+8);
            }
          else
            {
            bSubTag=GetFromBufLittleEndianWord(FileStart,bSubPos);//to jest Tag
            bSubDataType=GetFromBufLittleEndianWord(FileStart,bSubPos+2);
            fieldDataW=GetFromBufLittleEndianWord(FileStart,bSubPos+8);
            fieldDataDW=GetFromBufLittleEndianDWord(FileStart,bSubPos+8);
            };
        switch (bSubTag)
            {
             case 277:{
                  if (bSubDataType==3)
                      bColorBPP=fieldDataW*8;
                    else if (bSubDataType==4)
                      bColorBPP=fieldDataDW*8;
                  }break;
             case 257:{
                  if (bSubDataType==3)
                      bHeight=fieldDataW;
                    else if (bSubDataType==4)
                      bHeight=fieldDataDW;
                  }break;
             case 256:{
                  if (bSubDataType==3)
                      bWidth=fieldDataW;
                    else if (bSubDataType==4)
                      bWidth=fieldDataDW;
                  }break;
            };//end case
        bSubPos+=12;
        };//while (cEntry<...

    //Pobieramy offset nastepnego katalogu
        if ((bSubPos+4>loadedBlockSize) || (bSubPos<9))
            break;
        if (isBigEndian)
              bSubPos=GetFromBufBigEndianDWord(FileStart,bSubPos);
            else
              bSubPos=GetFromBufLittleEndianDWord(FileStart,bSubPos);
        bSubPos++;//bo u nas pierwszy jest bajt 1 a nie 0
    };//while ((bSubPos>...

  //Analiza zakonczona - ustalamy opis tekstowy
    if (isBigEndian)
        {
        if ((bWidth>0)&&(bHeight>0))
            Entry->BodyAnalist.DescAfter="m"+Entry->BodyAnalist.DescAfter;
          else
            Entry->BodyAnalist.DescAfter="Motorola "+Entry->BodyAnalist.DescAfter;
        }
      else
        {
        if ((bWidth>0)&&(bHeight>0))
            Entry->BodyAnalist.DescAfter="i"+Entry->BodyAnalist.DescAfter;
          else
            Entry->BodyAnalist.DescAfter="Intel "+Entry->BodyAnalist.DescAfter;
        };
    if ((bColorBPP>0)&&(bColorBPP<129))
        Entry->BodyAnalist.DescAfter=IntToStr(bColorBPP)+"-bit "+Entry->BodyAnalist.DescAfter;
    if ((bWidth>0)&&(bHeight>0))
        Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+Entry->BodyAnalist.DescAfter;

    Entry->BodyAnalist.EntryType=setPicture;
    Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
    return true;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::RecognizedBitmapStart(TSearchRecList *Entry,char *FileStart)
//U¿ywany w FileBodyAnalist
//sprawdza czy pliki s¹ mapami bitowymi
{
   String bMagic;
   String bSubType;
   unsigned int bSubPos;
   unsigned int bWidth;
   unsigned int bHeight;
   unsigned int bColorBPP;
   unsigned int bDataSize; //Ta zmienna jest tylko do testowania czy bitmapa nie "udaje"

  uint loadedBlockSize;
  loadedBlockSize=min(FileBodyHeaderSize,uint(Entry->F.Size));

  bMagic="xx";
  StrLCopy((char *)bMagic.data(),FileStart,2);

    // sprawdzamy BMP
  if ((bMagic=="BM")||(bMagic=="BA")||(bMagic=="CI")||(bMagic=="CP")||
      (bMagic=="IC")||(bMagic=="PT"))
    {
    bWidth=GetFromBufLittleEndianDWord(FileStart,18);
    bHeight=GetFromBufLittleEndianDWord(FileStart,22);
    bColorBPP=GetFromBufLittleEndianWord(FileStart,28);
    //0012h  1 dword  Horizontal width of bitmap in pixels
    //0016h  1 dword  Vertical width of bitmap in pixels
    //001Ch  1 word   Bits per pixel ( thus the number of colors )
    Entry->BodyAnalist.Probability=900;
    Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
    Entry->BodyAnalist.DescBefore="";
      if (bMagic=="BM")
          {
          Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit BMP";
          }
       else
      if ((bMagic=="CI")||(bMagic=="IC"))
          {
          Entry->BodyAnalist.DescAfter="OS/2 Icon";
          }
       else
      if ((bMagic=="PT")||(bMagic=="CP"))
          {
          Entry->BodyAnalist.DescAfter="OS/2 Mouse Cursor";
          }
       else //bMagic=="BA"
          {
          Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" OS/2 Bitmap Array";
          }
      Entry->BodyAnalist.EntryType=setPicture;
      Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
      return true;
    }

    // sprawdzamy JPEG
  if ((bMagic=="\xFF\xD8"))
    {
    Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
    Entry->BodyAnalist.DescBefore="";
    bSubType="xxxxx";
    String cmparSub="JFIFx";
    cmparSub[5]=0;
    StrLCopy((char *)bSubType.data(),FileStart+6,5);
    if (bSubType==cmparSub)
        {
        Entry->BodyAnalist.Probability=900;
        Entry->BodyAnalist.DescAfter="JPEG";
        //Szukamy identyfikatora bloku informacyjnego
        bSubPos=FindSubMemPos("\xFF\xC0",2,FileStart,loadedBlockSize);
        if ((bSubPos>14)&&(bSubPos<FileBodyHeaderSize-9))
          {
          bHeight=GetFromBufBigEndianWord(FileStart,bSubPos+5);
          bWidth=GetFromBufBigEndianWord(FileStart,bSubPos+7);
          bColorBPP=8*Word(FileStart[bSubPos+9]);
          Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit JPEG";
          }
        }
      else
        {
        Entry->BodyAnalist.DescAfter="JPEG similar";
        Entry->BodyAnalist.Probability=450;
        }
    Entry->BodyAnalist.EntryType=setPicture;
    Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
    return true;
    }

    // sprawdzamy SGI
  if ((bMagic=="\x01\xDA"))
    {
    bWidth=GetFromBufBigEndianWord(FileStart,6);
    bHeight=GetFromBufBigEndianWord(FileStart,8);
    bColorBPP=(Word(FileStart[3])*8)*GetFromBufBigEndianWord(FileStart,10);
    if ((bColorBPP<65)&&(bColorBPP>0)&&
        (bHeight>0)&&(bWidth>0))
        {
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit SiliconGrphImg";
        Entry->BodyAnalist.Probability=700;
        Entry->BodyAnalist.EntryType=setPicture;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
        return true;
        }
    }

    // sprawdzamy PIC/CEL (NIE TESTOWANE!)
  if ((bMagic=="\x91\x19"))
    {
    bWidth=GetFromBufLittleEndianWord(FileStart,2);
    bHeight=GetFromBufLittleEndianWord(FileStart,4);
    bColorBPP=((uchar)FileStart[9]);
    Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
    Entry->BodyAnalist.DescBefore="";
    if ((bColorBPP<65))
        {
        Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit PIC/CEL Bitmap";
        Entry->BodyAnalist.Probability=700;
        }
      else
        {
        Entry->BodyAnalist.DescAfter="PIC/CEL Bitmap";
        Entry->BodyAnalist.Probability=600;
        }
    Entry->BodyAnalist.EntryType=setPicture;
    Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
    return true;
    }

  bMagic="xxx";
  StrLCopy((char *)bMagic.data(),FileStart,3);

    // sprawdzamy GIF
  if ((bMagic=="GIF"))
    {
    bWidth=GetFromBufLittleEndianWord(FileStart,6);
    bHeight=GetFromBufLittleEndianWord(FileStart,8);
    bColorBPP=(Word(FileStart[10]) & 7)+1;
    Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
    Entry->BodyAnalist.DescBefore="";
    if ((bColorBPP<65))
        {
        Entry->BodyAnalist.Probability=900;
        Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit GIF";
        }
      else
        {
        Entry->BodyAnalist.Probability=700;
        Entry->BodyAnalist.DescAfter="GIF";
        }
    Entry->BodyAnalist.EntryType=setPicture;
    Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
    }

    // sprawdzamy PCX
  if ((bMagic[1]==10)&&(bMagic[2]<6)&&(bMagic[3]==1))
    {
    bWidth=GetFromBufLittleEndianWord(FileStart,8)+1-GetFromBufLittleEndianWord(FileStart,4);
    bHeight=GetFromBufLittleEndianWord(FileStart,10)+1-GetFromBufLittleEndianWord(FileStart,6);
    bColorBPP=(Word(FileStart[3]));
    Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
    Entry->BodyAnalist.DescBefore="";
    if ((bColorBPP<65)&&(bColorBPP>0))
        {
        Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit PCX";
        Entry->BodyAnalist.Probability=700;
        }
      else
        {
        Entry->BodyAnalist.DescAfter="new PCX";
        Entry->BodyAnalist.Probability=600;
        }
    Entry->BodyAnalist.EntryType=setPicture;
    Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
    return true;
    }

    // sprawdzamy RIX (NIE TESTOWANE!)}
  if ((bMagic=="RIX"))
    {
    bWidth=GetFromBufLittleEndianWord(FileStart,3);
    bHeight=GetFromBufLittleEndianWord(FileStart,5);
    bColorBPP=(((uint)FileStart[7])& 7)+1;
    if ((bColorBPP<65))
        {
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit ColoRIX Image";
        Entry->BodyAnalist.Probability=700;
        Entry->BodyAnalist.EntryType=setPicture;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
        return true;
        }
    }

  bMagic="xxxx";
  StrLCopy((char *)bMagic.data(),FileStart,4);

    // sprawdzamy ImageAlchemy JPEG
  if ((bMagic=="HSI1"))
    {
    Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
    Entry->BodyAnalist.DescBefore="";
    Entry->BodyAnalist.DescAfter="ImageAlchemy JPEG";
    Entry->BodyAnalist.Probability=700;
    Entry->BodyAnalist.EntryType=setPicture;
    Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
    return true;
    }

    // sprawdzamy PSD
  if ((bMagic=="8BPS"))
    {
    Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
    Entry->BodyAnalist.DescBefore="";
    bHeight=GetFromBufBigEndianDWord(FileStart,14);
    bWidth=GetFromBufBigEndianDWord(FileStart,18);
    bColorBPP=GetFromBufBigEndianWord(FileStart,12)*GetFromBufBigEndianWord(FileStart,22);
    if ((bColorBPP<65)&&(bColorBPP>0)&&
        (bHeight<128000)&&(bHeight>0)&&
        (bWidth<128000)&&(bWidth>0))
        {
        Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit PSD";
        Entry->BodyAnalist.Probability=800;
        }
      else
        {
        Entry->BodyAnalist.DescAfter="Photoshop PSD";
        Entry->BodyAnalist.Probability=650;
        }
    Entry->BodyAnalist.EntryType=setPicture;
    Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
    return true;
    }

    // sprawdzamy RAS
  if ((bMagic=="\x59\xA6\x6A\x95"))
    {
    Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
    Entry->BodyAnalist.DescBefore="";
    bWidth=GetFromBufBigEndianDWord(FileStart,4);
    bHeight=GetFromBufBigEndianDWord(FileStart,8);
    bColorBPP=GetFromBufBigEndianWord(FileStart,14);
    if ((bColorBPP<65)&&(bColorBPP>0)&&
        (bHeight<128000)&&(bHeight>0)&&
        (bWidth<128000)&&(bWidth>0))
        {
        Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit SunRAS";
        Entry->BodyAnalist.Probability=700;
        }
      else
        {
        Entry->BodyAnalist.DescAfter="SunRAS(new)";
        Entry->BodyAnalist.Probability=600;
        }
    Entry->BodyAnalist.EntryType=setPicture;
    Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
    return true;
    }

    // sprawdzamy EPS - dla tego pliku szer, wys itd-> s¹ daleko (np bajt 781) wiec podaje tylko nazwe
  if ((bMagic=="\xC5\xD0\xD3\xC6"))
    {
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        Entry->BodyAnalist.DescAfter="EncapPostScrpt";
        Entry->BodyAnalist.Probability=700;
        Entry->BodyAnalist.EntryType=setPicture;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
        return true;
    }

    // sprawdzamy GRIB
  if ((bMagic=="GRIB"))
    {
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        Entry->BodyAnalist.DescAfter="GRIB Weather Info";
        Entry->BodyAnalist.Probability=700;
        Entry->BodyAnalist.EntryType=setData;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
        return true;
    }

    // sprawdzamy SCMImg (NIE TESTOWANE!)
  if ((bMagic=="SCMI"))
    {
    bSubPos=FindSubMemPos("AT",2,FileStart,loadedBlockSize);
    if ((bSubPos>0)&&(bSubPos+6<FileBodyHeaderSize))
        {
        Entry->BodyAnalist.DescAfter="SCMImage";
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        Entry->BodyAnalist.DescAfter="SCMImage";
        Entry->BodyAnalist.Probability=600;
        Entry->BodyAnalist.EntryType=setPicture;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
        bWidth=GetFromBufLittleEndianWord(FileStart,bSubPos+5);
        bHeight=GetFromBufLittleEndianWord(FileStart,bSubPos+7);
        bColorBPP=floorl(Log2( GetFromBufLittleEndianWord(FileStart,bSubPos+9) ));
        if ((bColorBPP<65)&&(bColorBPP>0))
            {
            Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit SCMImg";
            Entry->BodyAnalist.Probability=700;
           }
        return true;
        }
    }

    // sprawdzamy ICO
  if ((FileStart[0]==0)&&(FileStart[1]==0)&&(FileStart[2]==1)&&(FileStart[3]==0))
    {
    bSubPos=GetFromBufLittleEndianWord(FileStart,4);//ilosc ikon w pliku
    if ((bSubPos>0))
        {
        if (bSubPos>1)
            {
            Entry->BodyAnalist.DescAfter=IntToStr(bSubPos)+" Icon images";
            }
          else
            {
            //Co do parametrów ikony to nie jestem pewny czy to dobrze...
            bWidth=(uchar)FileStart[6];
            bHeight=(uchar)FileStart[7];
            //Dla starych ikon mamy zapisan¹ liczbê kolorów
            uchar nColors=(uchar)FileStart[8];
            if (nColors>1)
                {
                bColorBPP=Ceil(Log2(nColors));
                Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit Icon";
                }
              else
                {
                bColorBPP=GetFromBufLittleEndianWord(FileStart,12);
                bHeight=bHeight*4/bColorBPP;
                Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit Icon";
                }
            }
        Entry->BodyAnalist.Probability=600;
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        Entry->BodyAnalist.EntryType=setPicture;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
        return true;
        }
    }

    // sprawdzamy RSB
  if ((FileStart[0]==0)&&(FileStart[1]==0)&&(FileStart[2]==0)&&(FileStart[3]==0))
    {
    bWidth=GetFromBufLittleEndianDWord(FileStart,4);
    bHeight=GetFromBufLittleEndianDWord(FileStart,8);
    if ((FileStart[16]==5)&&(FileStart[17]==0)&&(FileStart[18]==0)&&(FileStart[19]==0)&&
        (FileStart[20]==6)&&(FileStart[21]==0)&&(FileStart[22]==0)&&(FileStart[23]==0)&&
        (FileStart[24]==5)&&(FileStart[25]==0)&&(FileStart[26]==0)&&(FileStart[27]==0))
        {
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        if ((bHeight<128000)&&(bHeight>0)&&
            (bWidth<128000)&&(bWidth>0))
            {
            Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" RedStormBitmap";
            Entry->BodyAnalist.Probability=650;
            }
          else
            {
            Entry->BodyAnalist.DescAfter="RedStormBitmap similar";
            Entry->BodyAnalist.Probability=550;
            }
        Entry->BodyAnalist.EntryType=setPicture;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
        return true;
        }
    }

    // sprawdzamy ICC (NIE TESTOWANE!)
  if ((FileStart[0]==0)&&(FileStart[1]=='\x5B')&&(FileStart[2]=='\x07')&&(FileStart[3]=='\x20'))
    {
    bSubPos=GetFromBufLittleEndianDWord(FileStart,6);
    if ((bSubPos>7)&&(bSubPos+40<FileBodyHeaderSize))
        {
        Entry->BodyAnalist.DescAfter="ICC Bitmap";
        Entry->BodyAnalist.Probability=600;
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        Entry->BodyAnalist.EntryType=setPicture;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
        bWidth=((uint)(FileStart[4+bSubPos+37]) << 8)+(uint)(FileStart[bSubPos+36]);
        bHeight=GetFromBufLittleEndianWord(FileStart,bSubPos+38);
        Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" ICC";
        Entry->BodyAnalist.Probability=700;
        return true;
        }
    }

    // sprawdzamy TarGA
  if ((FileStart[1]<2)&&(FileStart[2]<35)&&(FileStart[11]<64))
    {
    bSubPos=GetFromBufLittleEndianWord(FileStart,5);
    bWidth=GetFromBufLittleEndianWord(FileStart,12);
    bHeight=GetFromBufLittleEndianWord(FileStart,14);
    bColorBPP=Word(FileStart[16]);
    bDataSize=bWidth*bHeight*(bColorBPP / 8);
    if ((bColorBPP<65) && (bColorBPP>0) && (((bColorBPP % 8)==0)||(bColorBPP==15))&&
        (bSubPos >= (uint)GetFromBufLittleEndianWord(FileStart,4))&&
        (bWidth > 0) && (bHeight > 0) &&
        ((uint)Entry->F.Size < bDataSize+2048) && ((uint)(Entry->F.Size) > bDataSize/20))
        {
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit TarGA";
        Entry->BodyAnalist.Probability=700;
        Entry->BodyAnalist.EntryType=setPicture;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
        return true;
        }
    }

    // sprawdzamy PBM
  if ((FileStart[0]=='P')&&(FileStart[1]>='1')&&(FileStart[1]<'9'))
    {
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        Entry->BodyAnalist.DescAfter="PBM/PPM/PGM PixelMap";
        Entry->BodyAnalist.Probability=550;
        Entry->BodyAnalist.EntryType=setPicture;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
        return true;
    }

  bMagic="xxxxxx";
  StrLCopy((char *)bMagic.data(),FileStart,6);

    // sprawdzamy PNG
  if ((bMagic=="\x89PNG\x0D\x0A"))
    {
    Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
    Entry->BodyAnalist.DescBefore="";
    Entry->BodyAnalist.Probability=800;
    Entry->BodyAnalist.DescAfter="PNG bitmap";
    bSubPos=FindSubMemPos("IHDR",4,FileStart,loadedBlockSize);
    if ((bSubPos>5)&&(bSubPos<FileBodyHeaderSize-9))
        {
        bWidth=GetFromBufBigEndianDWord(FileStart,bSubPos+4);
        bHeight=GetFromBufBigEndianDWord(FileStart,bSubPos+8);
        bColorBPP=Word(FileStart[bSubPos+12]);
        Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit PNG";
        }
    Entry->BodyAnalist.EntryType=setPicture;
    Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
    return true;
    }

  bMagic="XXXXX_XXX";
  StrLCopy((char *)bMagic.data(),FileStart+2,9);

    // sprawdzamy CEG
  if ((bMagic.UpperCase()=="EDSUN CEG"))
    {
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        Entry->BodyAnalist.DescAfter="Edsun CEG Bitmap";
        Entry->BodyAnalist.Probability=800;
        Entry->BodyAnalist.EntryType=setPicture;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
        return true;
    }

  bMagic="XXXXXXXX";
  StrLCopy((char *)bMagic.data(),FileStart,8);

    // sprawdzamy QFX
  if ((bMagic.SubString(1,7)=="QLIIFAX")&&(bMagic[8]==0))
    {
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        Entry->BodyAnalist.DescAfter="Quicklink FaX Image";
        Entry->BodyAnalist.Probability=800;
        Entry->BodyAnalist.EntryType=setData;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
        return true;
    }

    // sprawdzamy PICPro/MSK (NIE TESTOWANE!)
  if ((FileStart[4]=='\x95')&&(FileStart[5]==0))
    {
    bDataSize=GetFromBufLittleEndianDWord(FileStart,0);
    bWidth=GetFromBufLittleEndianWord(FileStart,6);
    bHeight=GetFromBufLittleEndianWord(FileStart,8);
    bColorBPP=(((uchar)FileStart[17]));
    if ((bColorBPP<65)&&(bDataSize<((uint)Entry->F.Size)+512)&&(bDataSize+512>((uint)Entry->F.Size)))
        {
        Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
        Entry->BodyAnalist.DescBefore="";
        Entry->BodyAnalist.DescAfter=IntToStr(bWidth)+"x"+IntToStr(bHeight)+" "+IntToStr(bColorBPP)+"-bit PICPro/MSK";
        Entry->BodyAnalist.Probability=700;
        Entry->BodyAnalist.EntryType=setPicture;
        Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
        return true;
        }
    }

    // w razie czego mo¿na tu dodac dowolny nowy format
  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::RecognizedVectGraphStart(TSearchRecList *Entry,char *FileStart)
{
  String bMagic;
  bMagic="xxx";
  StrLCopy((char *)bMagic.data(),FileStart,3);

  // sprawdzamy SWF
  if ((bMagic=="FWS"))
      {
        int VerNum=FileStart[3];
        if (VerNum<20)  //Jak wyjdzie 20 wersji Flasha to siê zmieni (teraz jest 6)
          {
          Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
          Entry->BodyAnalist.DescBefore="";
          Entry->BodyAnalist.DescAfter="ShockWave Flash "+IntToStr(VerNum)+" file";
          Entry->BodyAnalist.Probability=800;
          Entry->BodyAnalist.EntryType=setAnimation;
          Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
          }
        return true;
      }

  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::RecognizedExeCodeStart(TSearchRecList *Entry,char *FileStart)
{
  String bMagic;
  String bSubType;
  bMagic="xx";
  StrLCopy((char *)bMagic.data(),FileStart,2);

  // sprawdzamy EXE dla DOS/Windows
  int newExePos=0;
  if ((bMagic=="MZ")||(bMagic=="ZM"))
    newExePos=GetFromBufLittleEndianWord(FileStart,60);
  bSubType="xx";
  StrLCopy((char *)bSubType.data(),FileStart+newExePos,2);
  //Mamy obydwa nag³ówki. Oczywiœcie bSubType mo¿e byæ niepoprawny...
  if (bSubType=="NE")
      {
      Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
      Entry->BodyAnalist.DescBefore="";
      Entry->BodyAnalist.DescAfter="Win3.x new executable";
      Entry->BodyAnalist.Probability=800;
      Entry->BodyAnalist.EntryType=setProgram;
      Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
      return true;
      }
    else
  if (bSubType=="LE")
      {
      Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
      Entry->BodyAnalist.DescBefore="";
      Entry->BodyAnalist.DescAfter="Windows VxD linear executable";
      Entry->BodyAnalist.Probability=800;
      Entry->BodyAnalist.EntryType=setProgram;
      Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
      return true;
      }
    else
  if (bSubType=="LX")
      {
      Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
      Entry->BodyAnalist.DescBefore="";
      Entry->BodyAnalist.DescAfter="OS/2 LE variant";
      Entry->BodyAnalist.Probability=800;
      Entry->BodyAnalist.EntryType=setProgram;
      Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
      return true;
      }
    else
  if (bSubType=="W3")
      {
      Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
      Entry->BodyAnalist.DescBefore="";
      Entry->BodyAnalist.DescAfter="LE files collection";
      Entry->BodyAnalist.Probability=800;
      Entry->BodyAnalist.EntryType=setProgram;
      Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
      return true;
      }
    else
  if (bSubType=="PE")
      {
      Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
      Entry->BodyAnalist.DescBefore="";
      Entry->BodyAnalist.DescAfter="Win32 portable executable";
      Entry->BodyAnalist.Probability=800;
      Entry->BodyAnalist.EntryType=setProgram;
      Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
      return true;
      }
    else
  if (bSubType=="DL")
      {
      Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
      Entry->BodyAnalist.DescBefore="";
      Entry->BodyAnalist.DescAfter="HP 100LX/200LX system manager compliant executable";
      Entry->BodyAnalist.Probability=800;
      Entry->BodyAnalist.EntryType=setProgram;
      Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
      return true;
      }
    else
  if (bSubType=="MP")
      {
      Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
      Entry->BodyAnalist.DescBefore="";
      Entry->BodyAnalist.DescAfter="old PharLap executable";
      Entry->BodyAnalist.Probability=800;
      Entry->BodyAnalist.EntryType=setProgram;
      Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
      return true;
      }
    else
  if (bSubType=="P2")
      {
      Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
      Entry->BodyAnalist.DescBefore="";
      Entry->BodyAnalist.DescAfter="PharLap 286 executable";
      Entry->BodyAnalist.Probability=800;
      Entry->BodyAnalist.EntryType=setProgram;
      Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
      return true;
      }
    else
  if (bSubType=="P3")
      {
      Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
      Entry->BodyAnalist.DescBefore="";
      Entry->BodyAnalist.DescAfter="PharLap 386 executable";
      Entry->BodyAnalist.Probability=800;
      Entry->BodyAnalist.EntryType=setProgram;
      Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
      return true;
      }
    else
  if ((bMagic=="MZ")||(bMagic=="ZM"))
      {
      Entry->BodyAnalist.Name=PrepareNameFromFName(Entry->F);
      Entry->BodyAnalist.DescBefore="";
      Entry->BodyAnalist.DescAfter="DOS executable";
      Entry->BodyAnalist.Probability=800;
      Entry->BodyAnalist.EntryType=setProgram;
      Entry->BodyAnalist.CategoryName=GetCatNameFromType(Entry->BodyAnalist.EntryType);
      return true;
      }

  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::RecognizedMusicStart(TSearchRecList *Entry,char *FileStart)
{
  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TDiscAnalyzer::GetItemList(TObject *Sender,TSearchRecList *Entry,TStrings *Info)
//Element GetGeneratedInfo, pobiera liste plikow do stringLista
{
 bool Result=true;
 if (IsFolder(Entry))
   {
    TSearchRecList *CurEntry; //Przetwarzany wpis tablicy plików
    TSearchRecList *SubEntry;
    String SubList;
    String SubDescBefore;
    String SubDescAfter;
    int SubCount;
    int SubSize;
   CurEntry=Entry->Down;
   DynamicStruct->MovetoFirstItem(CurEntry);
   while (CurEntry!=NULL)
     {
     SubEntry=CurEntry->Down;
     DynamicStruct->MovetoFirstItem(SubEntry);
     __try
     {
     SubList="";SubCount=0;
     SubSize=0;
     SubDescBefore="";SubDescAfter="";
     if (SubEntry==NULL)
        {
        if (CurEntry!=NULL) SubSize=CurEntry->F.Size;
        }
      else
       {
       while (SubEntry!=NULL)
         {
         if (SubList.Length()<1)
           SubList=SubEntry->RealProperties.Name;
          else
           SubList=SubList+","+SubEntry->RealProperties.Name;
         SubCount++;
         SubSize+=SubEntry->F.Size;
         if (SubEntry->RealProperties.DescAfter.Length()>0)
             {
             if (SubDescAfter.Length()>0)
                 {
                 if (SubDescAfter.Pos(SubEntry->RealProperties.DescAfter)<1)
                     SubDescAfter=SubDescAfter+","+SubEntry->RealProperties.DescAfter;
                 }
               else
                 SubDescAfter=SubEntry->RealProperties.DescAfter;
             }
         if (SubEntry->RealProperties.DescBefore.Length()>0)
             {
             if (SubDescBefore.Length()>0)
                 {
                 if (SubDescAfter.Pos(SubEntry->RealProperties.DescAfter)<1)
                     SubDescBefore=SubDescBefore+","+SubEntry->RealProperties.DescBefore;
                 }
               else
                 SubDescBefore=SubEntry->RealProperties.DescBefore;
             }
         SubEntry=SubEntry->Next;
         }
       }
//Musimy sie troche pobawic, bo nie wiemy ktore stringi sa puste
  //najpierw l¹czymy DescBefore i DescAfter z SubDescBefore/After
     if (CurEntry->RealProperties.DescAfter.Length()>0)
         {
         //wyl¹czenie tego kodu powoduje, ¿e jak jest opis do calej
         // kategorii, to opisy poszczegolnych plikow nie sa uznawane
             SubDescAfter=CurEntry->RealProperties.DescAfter;
         }
     if (CurEntry->RealProperties.DescBefore!="")
         {
         //wyl¹czenie tego kodu powoduje, ¿e jak jest opis do calej
         // kategorii, to opisy poszczegolnych plikow nie sa uznawane
             SubDescBefore=CurEntry->RealProperties.DescBefore;
         }
  //teraz SubDescBefore/After l¹czymy z SubList

     if (SubDescBefore.Length()>0)
       {
       if (SubList.Length()>0)
         SubList=SubDescBefore+";"+SubList;
        else
         SubList=SubDescBefore;
       }
     if (SubDescAfter!="")
       {
       if (SubList!="")
         SubList=SubList+";"+SubDescAfter;
        else
         SubList=SubDescAfter;
       }
      Extended SizeInMB;
      SizeInMB=SubSize /(1024.0*1024);
      int nDigits; //ilosc cyfr po kropce
      if (SizeInMB>1) nDigits=0; else nDigits=1;
      if (SizeInMB<0.1) SizeInMB=0.1;
     DecimalSeparator='.';
     //No i mamy gotowy string informacyjny - dodajemy do listy
     String SizeInMBStr;
     SizeInMBStr=FloatToStrF(SizeInMB,ffFixed,9,nDigits);
     if (SubCount>0)
       Info->Add(CurEntry->RealProperties.Name+"("+SubList+"("+IntToStr(SubCount)+","+SizeInMBStr+"MB))");
      else
       Info->Add(CurEntry->RealProperties.Name+"("+SubList+"("+SizeInMBStr+"MB))");
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {Result=false;};
     CurEntry=CurEntry->Next;
   } //end while (CurEntry!=NULL)
 } else // if (IsFolder(Entry))
   {
   Info->Add(Entry->RealProperties.Name);
   }
 return Result;
}
//---------------------------------------------------------------------------

