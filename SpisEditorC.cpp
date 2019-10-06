//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SpisEditorC.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//      Synchronize(UpdateCaption);
//---------------------------------------------------------------------------
__fastcall TSpisEditor::TSpisEditor(TSynchronizer *nSynchronizer)
        : TThread(true)
{
  Synchronizer=nSynchronizer;
  CaseConvertDialog=new TCaseConvertDialog(Synchronizer->MainForm);
  DiscAnalyzer=NULL;
}
//---------------------------------------------------------------------------
__fastcall TSpisEditor::~TSpisEditor()
{
  delete CaseConvertDialog;
  CaseConvertDialog=NULL;
  if (DiscAnalyzer!=NULL)
      {
      DiscAnalyzer->Terminate=true;
      delete DiscAnalyzer;
      DiscAnalyzer=NULL;
      }
}
//---------------------------------------------------------------------------
void __fastcall TSpisEditor::Execute()
{
 do
  {
  switch (Function)
    {
    case etfCreateList:
        Function=etfNone;
        CreateList(this,FuncParamList);
        break;
    case etfCreateListOld:
        Function=etfNone;
        CreateListOld(FuncParamList);
        break;
    }
  Suspend();
  } while (!Terminated);
}
//---------------------------------------------------------------------------
void TSpisEditor::CreateList(TObject *Sender,TStrings *List)
{
  String Drv;
 __try
  {
  if (DiscAnalyzer!=NULL)
    {
    DiscAnalyzer->Terminate=true;
    return;
    }
  Drv=Synchronizer->DrvSelector->Text;
  if (Drv.Length()<1) return;
  if (Drv.SubString(2,3)==": [")
    Drv=Drv.SubString(1,2);
  DiscAnalyzer=new TDiscAnalyzer(Synchronizer);
  if (DiscAnalyzer->AnalyzeDrive(Sender,Drv))
      {
      }
    else
      MessageBox(Synchronizer->MainForm->Handle,"Wyst¹pi³ b³¹d podczas próby analizy dysku.","Cosik ni tok jok czeba...",MB_OK|MB_ICONERROR|MB_APPLMODAL);
  List->Capacity=MaxInt; //Nie wiem czy to cokolwiek daje....
  if ((Terminated)||(DiscAnalyzer==NULL)) return;
  DiscAnalyzer->GetGeneratedInfo(Sender,List);
  }
 __finally
  {
  delete DiscAnalyzer;
  DiscAnalyzer=NULL;
  };
}
//---------------------------------------------------------------------------
void TSpisEditor::CreateListOld(TStrings *List)
{
  TDiscAnalOld *DiscAnalOld;
 __try
  {
  DiscAnalOld=new TDiscAnalOld(Synchronizer);
  DiscAnalOld->CreateListOld(List);
  }
 __finally
  {
  delete DiscAnalOld;
  };

}
//---------------------------------------------------------------------------
void TSpisEditor::ChangeSelectionCase(TObject *Sender,TRichEdit *Editor)
{
  if (CaseConvertDialog->ShowModal()==mrOk)
    {
    switch (CaseConvertDialog->ConvertionType->ItemIndex)
      {
      case 0:Editor->SelText=AnsiLowerCase(Editor->SelText);break;
      case 1:Editor->SelText=AnsiUpperCase(Editor->SelText);break;
      case 2:Editor->SelText=ChangeCaseWordsStartsBig(Editor->SelText);break;
      default:
        MessageBox(Synchronizer->MainForm->Handle,"Coœ jest nie tak z okienkiem dialogowym czmiany wielkoœci liter.","No nie... Ale zwa³a...",MB_OK|MB_ICONERROR);break;
      };
    };
}
//---------------------------------------------------------------------------
void TSpisEditor::ChangeSelectionCase(TObject *Sender,TMemo *Editor)
{
  if (CaseConvertDialog->ShowModal()==mrOk)
    {
    switch (CaseConvertDialog->ConvertionType->ItemIndex)
      {
      case 0:Editor->SelText=AnsiLowerCase(Editor->SelText);break;
      case 1:Editor->SelText=AnsiUpperCase(Editor->SelText);break;
      case 2:Editor->SelText=ChangeCaseWordsStartsBig(Editor->SelText);break;
      default:
        MessageBox(Synchronizer->MainForm->Handle,"Coœ jest nie tak z okienkiem dialogowym czmiany wielkoœci liter.","No nie... Ale zwa³a...",MB_OK|MB_ICONERROR);break;
      };
    };
}
//---------------------------------------------------------------------------
void TSpisEditor::DoFindInRichEdit(TFindDialog *Dialog,TRichEdit *Editor)
{
  long FoundAt;
  int StartPos, ToEnd;
  TSearchTypes st;
    // Jak mamy cos zaznaczone - szukamy za tym.
    // inaczej szukamy od poczatku
    if (Editor->SelLength > 0)
      StartPos = Editor->SelStart + Editor->SelLength;
     else
      StartPos = 0;
    // ToEnd is the length from StartPos to the end of the text in the rich edit control
    ToEnd = Editor->Text.Length() - StartPos;
    // Jeszcze opcje wyszukiwania
    st.Clear();
    if (Dialog->Options.Contains(frMatchCase))
        st<<stMatchCase;
    if (Dialog->Options.Contains(frWholeWord))
        st<<stWholeWord;
    // No i szukamy
    FoundAt = Editor->FindText(Dialog->FindText, StartPos, ToEnd, st);
    // Udalo sie lub nie...
    if (FoundAt > -1)
      {
      Editor->SetFocus();
      Editor->SelStart = FoundAt;
      Editor->SelLength = Dialog->FindText.Length();
      }
     else
      {
      MessageBox(Dialog->Handle,"Nie znaleziono wiecej odpowiedników.","Durny ty jestes!",MB_OK|MB_ICONINFORMATION);
      Editor->SelStart=0;
      };
}
//---------------------------------------------------------------------------
void TSpisEditor::DoFindInMemo(TFindDialog *Dialog,TMemo *Editor)
{
  long FoundAt;
  int StartPos, ToEnd;
  TSearchTypes st;
    // Jak mamy cos zaznaczone - szukamy za tym.
    // inaczej szukamy od poczatku
    if (Editor->SelLength > 0)
      StartPos = Editor->SelStart + Editor->SelLength;
     else
      StartPos = 0;
    // ToEnd is the length from StartPos to the end of the text in the rich edit control
//    ToEnd = Editor->Text.Length() - StartPos;
    // Jeszcze opcje wyszukiwania
    // No i szukamy
    if (Dialog->Options.Contains(frMatchCase))
      {
      FoundAt = FindSubstringPosFrom(StartPos,Dialog->FindText,Editor->Lines->Text)-1;
      }
     else
      {
      FoundAt = FindSubstringPosFrom(StartPos,Dialog->FindText.UpperCase(),Editor->Lines->Text.UpperCase())-1;
      }
    // Udalo sie lub nie...
    if (FoundAt > -1)
      {
      Editor->SetFocus();
      Editor->SelStart = FoundAt;
      Editor->SelLength = Dialog->FindText.Length();
      }
     else
      {
      MessageBox(Dialog->Handle,"Nie znaleziono wiecej odpowiedników.","Durny ty jestes!",MB_OK|MB_ICONINFORMATION);
      Editor->SelStart=0;
      };
}
//---------------------------------------------------------------------------
void TSpisEditor::DoReplaceInRichEdit(TReplaceDialog *Dialog,TRichEdit *Editor)
{
  if (Editor->SelLength == 0)
    DoFindInRichEdit(Dialog,Editor);
   else
    {
    Editor->SelText=Dialog->ReplaceText;
    DoFindInRichEdit(Dialog,Editor);
    };

    if (Dialog->Options.Contains(frReplaceAll))
        while ((Editor->SelLength)>0)
            DoReplaceInRichEdit(Dialog,Editor);
}
//---------------------------------------------------------------------------
void TSpisEditor::DoReplaceInMemo(TReplaceDialog *Dialog,TMemo *Editor)
{
  if (Editor->SelLength == 0)
    DoFindInMemo(Dialog,Editor);
   else
    {
    Editor->SelText=Dialog->ReplaceText;
    DoFindInMemo(Dialog,Editor);
    };

    if (Dialog->Options.Contains(frReplaceAll))
        while ((Editor->SelLength)>0)
            DoReplaceInMemo(Dialog,Editor);
}
//---------------------------------------------------------------------------

