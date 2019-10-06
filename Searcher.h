//---------------------------------------------------------------------------

#ifndef SearcherH
#define SearcherH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <SysUtils.hpp>
#include "SearcherItmBuffer.h"
#include "SynchronizerUnit.h"
#include "FXProcs.h"
//---------------------------------------------------------------------------
enum TSearcherTask { stskNone, stskSearch, stskReadFile, stskCountNum };
//---------------------------------------------------------------------------
class TKatSearcher : public TThread
{
private:
protected:
	AnsiString LiniaUpd;
	TSearcherItmBuffer* ItmBuffer;

	void __fastcall PrepareToSearch();
	TStringList* __fastcall ListSelectedCategories();
	void __fastcall OpenCDDataFile(TStringList* Plik,long Num);
	void __fastcall OpenCDDataFileQuick(TStringList* Plik,long Num);
	void __fastcall FindCategoryInList(TStringList* Plik,TStringList* CatList,long &KatStart,long &KatEnd);
	void __fastcall SearchForTextInLineRange(TStringList* Plik,long KatTxtStart,long KatTxtEnd,String SrchText);
	void __fastcall UpdateSearchResults();
	void __fastcall RefreshAfterSearch();
      // Elementy funkcji wyszukuj¹cej
	bool __fastcall inline ContainsTypedText(AnsiString Text, AnsiString TypedText);
	void __fastcall StartNewTask();
    //Podstawowe metody wyszukuj¹ce
	void __fastcall SearchOneCDDatabase(TStringList* Plik,TStringList* CatList,String SrchText);
	void __fastcall SearchItemRange();
    //Funkcje niskiego poziomu
        //do komponentu z list¹ p³yt
	void __fastcall AddFileToSpisCD();
	void __fastcall ClearSpisCD();
    //Funkcje wywo³ywane w g³ównej funkcji w¹tku
        // Okreslenie ilosci plyt
	void __fastcall CountNum(void);
        //Czyta jeden plik bazy
	void __fastcall ReadFile();
        //G³ówna funkcja w¹tku
        virtual void __fastcall Execute();
public:
        //Konstrukcja i destrukcja
        __fastcall TKatSearcher(TSynchronizer *nSynchronizer);
        __fastcall ~TKatSearcher();
        //Tworzenie zadañ dla w¹tku
       	void __fastcall MakeTaskSearchCD();
	void __fastcall MakeTaskReadFileView();
	void __fastcall MakeTaskReadFileEdit();
	void __fastcall MakeTaskCountNum();
        //Debuging i logowanie
        void __fastcall GenerateDebugStats();
        // funkcje uwidaczniania/chowania komponentów
  	void __fastcall MakeSearcherVisible();
	void __fastcall MakeItmViewerVisible();

	bool CancelAction;
	bool Working;
	TSearcherTask ActiveTask;
	TSearcherTask NewTask;
	TSearcherTask TmpTask;
	bool Busy;
	TSynchronizer* Synchronizer;
	int MaxNum;
	int MinCD;
	int MaxCD;
        bool currSearchQuick;
        bool FullListInSpisCD;
	TStringList* ErrorList;
	unsigned long AddErrorNum;

        //Do formatowania tekstu
        TMemoryStream *FormattedReslt;
        String NrmFontStr;
        String SmlFontStr;

	void __fastcall UpdateKatHeight();

	TRichEdit* ItemDestination;
        String ItemStr;
};
//---------------------------------------------------------------------------
#endif
