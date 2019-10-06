//---------------------------------------------------------------------------

#ifndef SpisEditorCH
#define SpisEditorCH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "SynchronizerUnit.h"
#include "DiscAnalyzer.h"
#include "CaseConvertUnit.h"
#include "DiscAnalOld.hpp"
//---------------------------------------------------------------------------
enum TSpisEdThrdFunction
   {
   etfNone,
   etfCreateList,
   etfCreateListOld,
   etfFindInRichEd,
   etfReplaceInRichEd,
   };
//---------------------------------------------------------------------------
class TSpisEditor : public TThread
{
private:
protected:
        void __fastcall Execute();
public:
        TSynchronizer *Synchronizer;
        TDiscAnalyzer *DiscAnalyzer;
           //Zmienne u¿ywane przez w¹tek podczas pracy
        TSpisEdThrdFunction Function;
        TStrings *FuncParamList;
        TRichEdit *FuncParamEditor;
           //Funkcje wywo³uj¹ce analizê CD
        void CreateList(TObject *Sender,TStrings *List);
        void CreateListOld(TStrings *List);
           //Funkcje dodatkowe
        void ChangeSelectionCase(TObject *Sender,TRichEdit *Editor);
        void ChangeSelectionCase(TObject *Sender,TMemo *Editor);
        void DoFindInRichEdit(TFindDialog *Dialog,TRichEdit *Editor);
        void DoReplaceInRichEdit(TReplaceDialog *Dialog,TRichEdit *Editor);
        void DoFindInMemo(TFindDialog *Dialog,TMemo *Editor);
        void DoReplaceInMemo(TReplaceDialog *Dialog,TMemo *Editor);
           //Konstrukcja/destrukcja
        __fastcall TSpisEditor(TSynchronizer *nSynchronizer);
        __fastcall ~TSpisEditor();
};
//---------------------------------------------------------------------------
#endif
