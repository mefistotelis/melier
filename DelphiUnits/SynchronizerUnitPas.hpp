// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SynchronizerUnitPas.pas' rev: 6.00

#ifndef SynchronizerUnitPasHPP
#define SynchronizerUnitPasHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <AnalyzerDynamics.hpp>	// Pascal unit
#include <ExportDlg.hpp>	// Pascal unit
#include <FXProcsPas.hpp>	// Pascal unit
#include <Spin.hpp>	// Pascal unit
#include <Grids.hpp>	// Pascal unit
#include <Buttons.hpp>	// Pascal unit
#include <Registry.hpp>	// Pascal unit
#include <FileCtrl.hpp>	// Pascal unit
#include <ExtCtrls.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
#include <TabNotBk.hpp>	// Pascal unit
#include <ComCtrls.hpp>	// Pascal unit
#include <Dialogs.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Synchronizerunitpas
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TBorrVisMode { gmHistory, gmElement, gmPerson, gmUnreturned, gmLost };
#pragma option pop

#pragma option push -b-
enum TNeededUpdate { nuFull, nuLastLine, nuNone };
#pragma option pop

class DELPHICLASS TSynchronizerPas;
class PASCALIMPLEMENTATION TSynchronizerPas : public Classes::TThread 
{
	typedef Classes::TThread inherited;
	
public:
	AnsiString SpisPath;
	AnsiString SpisSubPath;
	Comctrls::TStatusPanel* StatusBar;
	Comctrls::TProgressBar* ProgressBar;
	Dialogs::TFontDialog* SmallFont;
	Dialogs::TFontDialog* NormFont;
	Stdctrls::TRadioButton* CPWinButton;
	Stdctrls::TRadioButton* CpDosButton;
	Stdctrls::TCheckBox* ScaleComponentsCheckBox;
	Stdctrls::TEdit* ScaleFactor;
	Stdctrls::TGroupBox* KatBox;
	Forms::TForm* MainForm;
	Classes::TStringList* NameOfMusic;
	Classes::TStringList* NameOfPicture;
	Classes::TStringList* NameOfProgram;
	Classes::TStringList* NameOfAnimation;
	Classes::TStringList* NameOfData;
	Classes::TStringList* NameOfVarious;
	Classes::TStringList* NameOfArchive;
	Classes::TStringList* ExtOfMusic;
	Classes::TStringList* ExtOfPicture;
	Classes::TStringList* ExtOfProgram;
	Classes::TStringList* ExtOfAnimation;
	Classes::TStringList* ExtOfData;
	Classes::TStringList* ExtOfVarious;
	Classes::TStringList* ExtOfArchive;
	Stdctrls::TListBox* SpisCD;
	Comctrls::TRichEdit* SearchViewer;
	Comctrls::TRichEdit* ItemViewer;
	Buttons::TSpeedButton* ShowSearchResultsBtn;
	Stdctrls::TEdit* LimitLow;
	Stdctrls::TEdit* LimitHigh;
	Stdctrls::TEdit* Typer;
	Stdctrls::TRadioButton* TyperAllRadioBtn;
	Stdctrls::TCheckBox* TyperCommentsCheckBox;
	Stdctrls::TCheckBox* ViewerShowNum;
	Stdctrls::TCheckBox* ViewerOpRemove;
	Stdctrls::TCheckBox* ViewerNumAtEnd;
	Stdctrls::TCheckBox* ViewerWordWraper;
	Spin::TSpinEdit* ViewerBufSize;
	Comctrls::TRichEdit* Editor;
	Stdctrls::TCheckBox* EditorWordWraper;
	Stdctrls::TComboBox* DrvSelector;
	Stdctrls::TCheckBox* AnalyzerEnBody;
	Grids::TStringGrid* SpisStringGrid;
	Stdctrls::TComboBox* PPoComboBox;
	Stdctrls::TComboBox* PVEEdit;
	int UpdStart;
	int UpdLength;
	Comctrls::TRichEdit* UpdEditor;
	Dialogs::TFontDialog* UpdFont;
	void __fastcall KatCreationAnalyseKategorie(AnsiString &KatName, AnsiString &ContentType, int lnstart, int lnend);
	void __fastcall NewKat(Word num, AnsiString capt);
	bool __fastcall AddKatToContentList(AnsiString ContentType, AnsiString KatName);
	bool __fastcall AddSingleExtToExtensionList(AnsiString ExtStr, AnsiString ContentType);
	void __fastcall LoadDefaultOpt(void);
	void __fastcall LoadOptions(void);
	void __fastcall SaveOptions(void);
	void __fastcall RemoveSavedOptions(void);
	void __fastcall ScaleComponents(int ScaleFactor);
	bool __fastcall ContainsText(AnsiString Text, AnsiString CmpWith);
	void __fastcall UpdateREditFormat(void);
	__fastcall TSynchronizerPas(Forms::TForm* nMainForm);
	__fastcall virtual ~TSynchronizerPas(void);
	virtual void __fastcall Execute(void);
};


//-- var, const, procedure ---------------------------------------------------
#define ProgramRegistryPath "Software\\FX Corp\\Spis CD"
static const Shortint NameDigits = 0x4;
static const Word MaxCategories = 0x400;
#define BorrowFileName "\\Borrow.txt"
#define DescriptFile "\\Readme.txt"
#define VersionFile "\\History.txt"
#define SpisFolder "\\Spis"
#define MusicStr "MUSIC"
#define ProgramStr "PROGRAM"
#define PictureStr "PICTURE"
#define AnimationStr "ANIMATION"
#define DataStr "DATA"
#define VariousStr "VARIOUS"
#define ArchiveStr "ARCHIVE"
#define PozyczDefaultText "<wybierz osobê>"
#define CRLF "\r\n"
extern PACKAGE Analyzerdynamics::SearchEntryType __fastcall GetContentTypeFromString(AnsiString ContentType);

}	/* namespace Synchronizerunitpas */
using namespace Synchronizerunitpas;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SynchronizerUnitPas
