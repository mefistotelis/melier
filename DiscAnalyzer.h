//---------------------------------------------------------------------------

#ifndef DiscAnalyzerH
#define DiscAnalyzerH
//---------------------------------------------------------------------------
#include <math.h>
#include <math.hpp>
#include "SynchronizerUnit.h"
#include "AnalyzerDynamics.hpp"
#include "FXProcs.h"
#include "InputFileMPEG.h"
#include "AnalyzerNameLister.hpp"
//---------------------------------------------------------------------------
//const FileBodyHeaderSize=1576800;
const FileBodyHeaderSize=32768;
//---------------------------------------------------------------------------
class TDiscAnalyzer : public TPersistent
{
protected:
	TDynamicStruct* DynamicStruct;
	TSynchronizer* Synchronizer;
	AnsiString RootDir;
            // do stworzenia listy
	bool __fastcall AnalyzeRootDir(System::TObject* Sender);
	bool __fastcall AnalyzeEntryList(TSearchRecList *UpEntry);
//	bool __fastcall GetItemList(System::TObject* Sender, Analyzerdynamics::PSearchRecList Entry, Classes::TStrings* Info);
            // dodatkowe/konwersji
	unsigned int __fastcall GetFromBufLittleEndianDWord(char *Buf, unsigned Index);
	Word __fastcall GetFromBufLittleEndianWord(char *Buf, unsigned Index);
	unsigned int __fastcall GetFromBufBigEndianDWord(char *Buf, unsigned Index);
	Word __fastcall GetFromBufBigEndianWord(char *Buf, unsigned Index);
            // do optymalizacji
                // funkcje pomocnicze do okreslania nazwy i typu objektu
	bool __fastcall IsFolder(TSearchRecList *Entry);
	SearchEntryType __fastcall GetTypeFromExtension(AnsiString FileExt);
	AnsiString __fastcall GetCatNameFromType(Analyzerdynamics::SearchEntryType CatType);

	AnsiString __fastcall PrepareNameFromFName(const TSearchRec &F);
	AnsiString __fastcall PrepareDescFromFName(const TSearchRec &F);

                // okreœlanie nazwy i typu pliku na podstawie nazwy i zawartosci
	bool __fastcall FileNameAnalist(Analyzerdynamics::PSearchRecList Entry);
	void __fastcall SetFileRealProperties(TSearchRecList *Entry);

	bool __fastcall FileBodyAnalist(TSearchRecList *Entry);
                // okreœlanie nazwy i kategorii folderu na podstawie nazwy i zawartosci
	bool __fastcall FldrNameAnalist(TSearchRecList *Entry);
	bool __fastcall FldrContentAnalist(TSearchRecList *Entry);
	void __fastcall SetFldrRealProperties(TSearchRecList *Entry);

                // analizuje strukturê ustawiaj¹c nazwy i typy objektów
	void __fastcall SetAllEntryTypes(void);
	bool __fastcall SetStructureAnalystProperties(PSearchRecList VirtDir);
	bool __fastcall ChooseStructureRealProperties(PSearchRecList VirtDir, AnsiString &MostProbableCategory);
	bool __fastcall RemoveEquPartsOfNamesInChildren(TSearchRecList *ParentEntry);

                // podfunkcje FileBodyAnalist do rozpoznawania ró¿nych typów
	bool __fastcall RecognizedRiffStart(TSearchRecList *Entry,char *FileStart);
        bool __fastcall Recognized3DDataStart(TSearchRecList *Entry,char *FileStart);
        bool __fastcall RecognizedVectGraphStart(TSearchRecList *Entry,char *FileStart);
        bool __fastcall RecognizedTiffStart(TSearchRecList *Entry,char *FileStart);
	bool __fastcall RecognizedBitmapStart(TSearchRecList *Entry,char *FileStart);
        bool __fastcall RecognizedMusicStart(TSearchRecList *Entry,char *FileStart);
	bool __fastcall RecognizedMpegStart(TSearchRecList *Entry,char *FileStart);
        bool __fastcall RecognizedVideoStart(TSearchRecList *Entry,char *FileStart);
        bool __fastcall RecognizedExeCodeStart(TSearchRecList *Entry,char *FileStart);
	bool __fastcall RecognizedArchiveStart(TSearchRecList *Entry,char *FileStart);
               //Specjalnie do analizy MPEG
        InputFileMPEG *InpMpeg;

	AnsiString __fastcall IdentifyACMCodec(Word identWord);
                // zmienia kolejnoœæ "katalogów" struktury dynam.
	void __fastcall MakeReplacements(void);
	void __fastcall MakeReplacementsIn(PSearchRecList Entry);
	ReplacePosition __fastcall ShouldBeReplaced(Analyzerdynamics::PSearchRecList Entry);
public:
        // do u¿ywania klasy z zewn¹trz
	bool __fastcall AnalyzeDrive(System::TObject* Sender, AnsiString DriveRoot);
	bool __fastcall OptimizeStructure(System::TObject* Sender);
	bool __fastcall GetGeneratedInfo(System::TObject* Sender, Classes::TStrings* Info);
        bool __fastcall GetItemList(TObject *Sender,TSearchRecList *Entry,TStrings *Info);
        bool EnableBodyAnalyst;
        bool Terminate;
          //Konstrukcja/destrukcja
	__fastcall TDiscAnalyzer(TSynchronizer* nSynchronizer);
	__fastcall virtual ~TDiscAnalyzer(void);
};
//---------------------------------------------------------------------------
#endif
