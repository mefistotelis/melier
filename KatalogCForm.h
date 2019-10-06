//---------------------------------------------------------------------------

#ifndef KatalogCFormH
#define KatalogCFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "Spin.hpp"
#include <Buttons.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <FileCtrl.hpp>
#include <Grids.hpp>
#include "SynchronizerUnit.h"
#include "Searcher.h"
#include "BorrowSystem.h"
#include "DiscAnalyzer.h"
#include "ExportDlg.hpp"
#include "FXProcsPas.hpp"
#include "SpisEditorC.h"
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TKatalog : public TForm
{
__published:	// IDE-managed Components
        TPageControl *MainPageControl;
        TTabSheet *TabFind;
        TTabSheet *TabEdit;
        TPanel *EditorToolBarPanel;
        TButton *EdRestoreBtn;
        TEdit *EdNumber;
        TButton *EdSaveBtn;
        TButton *AnalyseIntrBtn;
        TPanel *EFunctionPanel;
        TButton *EdFindButton;
        TButton *EdReplaceButton;
        TButton *EdCaseButton;
        TButton *Button2;
        TButton *AnalyseBtnOld;
        TPanel *EditorMemoPanel;
        TTabSheet *TabOptions;
        TPanel *OPanel2;
        TGroupBox *OSearchGroupBox;
        TLabel *OSearchLabel1;
        TLabel *OSearchLabel2;
        TCheckBox *ViewerWordWraper;
        TButton *SmallFontButton;
        TButton *NormFontButton;
        TMemo *OSearchNote;
        TCheckBox *ViewerShowNum;
        TCheckBox *ViewerOpRemove;
        TCheckBox *ViewerNumAtEnd;
        TSpinEdit *ViewerBufSize;
        TGroupBox *OEditGroupBox;
        TCheckBox *EditorWordWraper;
        TMemo *OEditNote;
        TPanel *OPanel1;
        TGroupBox *OGlobalGroupBox;
        TLabel *OGlobalLabel1;
        TLabel *OGlobalLabel2;
        TCheckBox *ScaleComponentsCheckBox;
        TMemo *OGlobalNote;
        TEdit *ScaleFactor;
        TGroupBox *OCPGroupBox;
        TRadioButton *CPWinButton;
        TRadioButton *CPDosButton;
        TMemo *OCPNote;
        TPanel *OPanel3;
        TGroupBox *OAnalyseGroupBox;
        TCheckBox *CheckBox1;
        TMemo *OAnalyseNote;
        TGroupBox *OPrefencesGroupBox;
        TButton *OSavePrefButton;
        TButton *OClearPrefButton;
        TMemo *OPrefNote;
        TTabSheet *TabBorrow;
        TPanel *PSpisPanel;
        TPanel *PodFindPanel;
        TLabel *PZwLabel;
        TSpeedButton *PZwButton;
        TLabel *PZwLabel2;
        TButton *PoRemoveButton;
        TEdit *PZwEdit;
        TRadioButton *PbdbRadioButton;
        TRadioButton *PdbRadioButton;
        TRadioButton *PdstRadioButton;
        TRadioButton *PzlyRadioButton;
        TButton *PoEditBtn;
        TPanel *PDispRightPanel;
        TGroupBox *PDispGroupBox;
        TSpeedButton *PVHSpeedButton;
        TSpeedButton *PVESpeedButton;
        TSpeedButton *PVPSpeedButton;
        TSpeedButton *PVZSpeedButton;
        TPanel *PVPanel1;
        TComboBox *PVEEdit;
        TGroupBox *PToolsGroupBox;
        TPanel *PPoPanel;
        TSpeedButton *PPoButton;
        TLabel *PPoLabel;
        TLabel *PBackLabel;
        TComboBox *PPoComboBox;
        TEdit *PPoEdit;
        TMemo *PPoComment;
        TSpinEdit *PBackEdit;
        TTabSheet *TabAbout;
        TPanel *HelpTitlePanel;
        TPanel *HelpWhatToViewPanel;
        TSpeedButton *HelpDescriptionButton;
        TSpeedButton *HelpVersionsButton;
        TPanel *HelpProgramNamePanel;
        TLabel *AboutProgramLabel;
        TLabel *AboutAuthorLabel;
        TPanel *HelpMainPanel;
        TRichEdit *HelpRichEdit;
        TFontDialog *NormFont;
        TFontDialog *SmallFont;
        TFindDialog *EdFindDialog;
        TReplaceDialog *EdReplaceDialog;
        TComboBox *DrvSelector;
        TListView *PMainListView;
        TStatusBar *StatusBarFull;
        TProgressBar *ProgressBar;
        TCheckBox *AnalyzerEnBody;
        TLabel *PVLabel1;
        TSpeedButton *PVLSpeedButton;
        TSpeedButton *PTolSprSpeedButton;
        TRichEdit *Editor;
        TTabSheet *TabDebug;
        TMemo *DebLogMemo;
        TGroupBox *RangeGroupBox;
        TLabel *Label1;
        TLabel *Label2;
        TEdit *LimitLow;
        TEdit *LimitHigh;
        TGroupBox *KatBox;
        TEdit *Typer;
        TSpeedButton *SearchSpdButton;
        TPanel *FOptionsPanel;
        TLabel *TyperOptionsLabel;
        TPanel *FButtonsPanel;
        TButton *FShowAllBtn;
        TButton *FExportBtn;
        TRadioButton *TyperAllRadioBtn;
        TRadioButton *TyperAnyRadioBtn;
        TCheckBox *TyperCommentsCheckBox;
        TPanel *FSpisPanel;
        TRichEdit *SearchViewer;
        TPanel *FSpisListPanel;
        TListBox *SpisCD;
        TPanel *FSpisListDownPanel;
        TSpeedButton *ShowSearchResultsBtn;
        TEdit *SpisItemSelector;
        TRichEdit *ItemViewer;
        TSpeedButton *DebGenStatButton;
        TButton *AnalyseExtrBtn;
        void __fastcall MainPageControlChange(TObject *Sender);
        void __fastcall HelpButtonClick(TObject *Sender);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall PSpisPanelResize(TObject *Sender);
        void __fastcall SpisCDKeyPress(TObject *Sender, char &Key);
        void __fastcall SpisCDClick(TObject *Sender);
        void __fastcall FindKeyPress(TObject *Sender, char &Key);
        void __fastcall EdRestoreBtnClick(TObject *Sender);
        void __fastcall EdSaveBtnClick(TObject *Sender);
        void __fastcall AnalyseIntrBtnClick(TObject *Sender);
        void __fastcall FShowAllBtnClick(TObject *Sender);
        void __fastcall FExportBtnClick(TObject *Sender);
        void __fastcall ViewerWordWraperClick(TObject *Sender);
        void __fastcall EditorWordWraperClick(TObject *Sender);
        void __fastcall SmallFontButtonClick(TObject *Sender);
        void __fastcall NormFontButtonClick(TObject *Sender);
        void __fastcall KatPanelResize(TObject *Sender);
        void __fastcall OSavePrefButtonClick(TObject *Sender);
        void __fastcall OClearPrefButtonClick(TObject *Sender);
        void __fastcall SearchSpdButtonClick(TObject *Sender);
        void __fastcall PPoEditKeyPress(TObject *Sender, char &Key);
        void __fastcall FormDestroy(TObject *Sender);
        void __fastcall KatBoxClick(TObject *Sender);
        void __fastcall KatBoxDblClick(TObject *Sender);
        void __fastcall PPoButtonClick(TObject *Sender);
        void __fastcall PZwEditKeyPress(TObject *Sender, char &Key);
        void __fastcall PZwButtonClick(TObject *Sender);
        void __fastcall PVSpeedButtonClick(TObject *Sender);
        void __fastcall PVEEditKeyPress(TObject *Sender, char &Key);
        void __fastcall AnalyseBtnOldClick(TObject *Sender);
        void __fastcall EdFindButtonClick(TObject *Sender);
        void __fastcall EdFindEvent(TObject *Sender);
        void __fastcall EdReplaceEvent(TObject *Sender);
        void __fastcall EdReplaceButtonClick(TObject *Sender);
        void __fastcall EdCaseButtonClick(TObject *Sender);
        void __fastcall PPoComboBoxEnter(TObject *Sender);
        void __fastcall PVEEditEnter(TObject *Sender);
        void __fastcall ShowSearchResultsBtnClick(TObject *Sender);
        void __fastcall StatusBarFullDrawPanel(TStatusBar *StatusBar,
          TStatusPanel *Panel, const TRect &Rect);
        void __fastcall SpisItemSelectorKeyPress(TObject *Sender,
          char &Key);
        void __fastcall DebGenStatButtonClick(TObject *Sender);
        void __fastcall SearchSpdButtonMouseUp(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall TyperKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
        void __fastcall AnalyseExtrBtnClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
    TSynchronizer *Synchronizer;
    TKatSearcher *SearchSystem;
    TBorrowSystem *BorrowSystem;
    TSpisEditor *EditSystem;
    void UpdateAnyBorrowLists();
    void __fastcall CreateDiskDriveList();
                //Konstrukcja i destrukcja
        __fastcall TKatalog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TKatalog *Katalog;
//---------------------------------------------------------------------------
#endif
