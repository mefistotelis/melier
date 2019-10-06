// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'AnalyzerDynamics.pas' rev: 6.00

#ifndef AnalyzerDynamicsHPP
#define AnalyzerDynamicsHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <ComCtrls.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
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

namespace Analyzerdynamics
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum SearchEntryType { setUnknown, setLink, setVarious, setCategory, setMusic, setPicture, setProgram, setAnimation, setData, setArchive };
#pragma option pop

#pragma option push -b-
enum ReplacePosition { rpNone, rpMain, rpCategory };
#pragma option pop

#pragma pack(push, 4)
struct TAnalistResult
{
	AnsiString Name;
	SearchEntryType EntryType;
	AnsiString CategoryName;
	AnsiString DescBefore;
	AnsiString DescAfter;
	int Probability;
} ;
#pragma pack(pop)

struct TSearchRecList;
typedef TSearchRecList *PSearchRecList;

#pragma pack(push, 4)
struct TSearchRecList
{
	TSearchRecList *Next;
	TSearchRecList *Prev;
	TSearchRecList *Up;
	TSearchRecList *Down;
	TAnalistResult NameAnalist;
	TAnalistResult BodyAnalist;
	TAnalistResult RealProperties;
	int AverageChildSize;
	AnsiString Path;
	Sysutils::TSearchRec F;
} ;
#pragma pack(pop)

class DELPHICLASS TDynamicStruct;
class PASCALIMPLEMENTATION TDynamicStruct : public Classes::TPersistent 
{
	typedef Classes::TPersistent inherited;
	
private:
	void __fastcall CreateUnassignedSRec(PSearchRecList &NewRec, const Sysutils::TSearchRec &Value);
	void __fastcall CutSRecSubTree(PSearchRecList &Rec);
	void __fastcall PasteSRecSubTreeAfter(PSearchRecList &NewRec, PSearchRecList RecBefore);
	void __fastcall PasteSRecSubTreeBefore(PSearchRecList &NewRec, PSearchRecList RecAfter);
	
public:
	TSearchRecList *RootEntryList;
	void __fastcall MovetoFirstItem(PSearchRecList &Rec);
	void __fastcall MovetoLastItem(PSearchRecList &Rec);
	void __fastcall NewSRecAfter(PSearchRecList &NewRec, const Sysutils::TSearchRec &Value);
	void __fastcall NewSRecBefore(PSearchRecList &NewRec, const Sysutils::TSearchRec &Value);
	void __fastcall NewSRecDown(PSearchRecList &UpRec, const Sysutils::TSearchRec &Value);
	void __fastcall DelSearchRec(PSearchRecList &Rec);
	void __fastcall DelLastSearchRec(PSearchRecList &Rec);
	void __fastcall DelAllSearchRec(PSearchRecList &Rec);
	void __fastcall ChangeItemPositionToAfter(PSearchRecList Rec, PSearchRecList RecBefore);
	void __fastcall ChangeItemPositionToBefore(PSearchRecList Rec, PSearchRecList RecAfter);
	bool __fastcall Clear(void);
	__fastcall virtual ~TDynamicStruct(void);
public:
	#pragma option push -w-inl
	/* TObject.Create */ inline __fastcall TDynamicStruct(void) : Classes::TPersistent() { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Analyzerdynamics */
using namespace Analyzerdynamics;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// AnalyzerDynamics
