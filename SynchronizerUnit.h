//---------------------------------------------------------------------------

#ifndef SynchronizerUnitH
#define SynchronizerUnitH
#include "SynchronizerUnitPas.hpp"
#include "FXProcs.h"
//---------------------------------------------------------------------------
class TSynchronizer : public TSynchronizerPas
{
private:
                //Komponenty z okna glownego
        TStrings *DebLogLines;
                //Pobieranie wskaünikow komponentow
        void __fastcall MainFormComponentsGet();
        void __fastcall MainFormComponentsClear();
                //Tworzenie struktur i ladowanie pliku z kategoriami
	void __fastcall KatCreationAnalyseRozszerzenia(String &Extensions,String &ContentType,unsigned int lnstart,unsigned  int lnend);
public:
                //Konstrukcja i destrukcja
        __fastcall TSynchronizer(TForm *nMainForm);
        __fastcall ~TSynchronizer();
                //Konwersja matrycy znakow
	String __fastcall WinCPConvertLine(String s);
	String __fastcall DosCPConvertLine(String s);
        void __fastcall WinCPConvertBuffer(char *buf,int bufSize);
        void __fastcall DosCPConvertBuffer(char *buf,int bufSize);
                //Tworzenie kategorii
	void __fastcall CreateKat();
                //Aktualizacja paska stanu i LOGÛw
	void __fastcall UpdateStatusBar(AnsiString Text);
        void __fastcall AddStringsToStata(TStrings *StrList);
        void __fastcall GenerateDebugStats();
        void __fastcall SetSearchAddKeyParams(TShiftState Shift);
        bool quickSearch;

};
//---------------------------------------------------------------------------
#endif
