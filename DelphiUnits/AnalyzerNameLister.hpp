// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'AnalyzerNameLister.pas' rev: 6.00

#ifndef AnalyzerNameListerHPP
#define AnalyzerNameListerHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <FXProcsPas.hpp>	// Pascal unit
#include <AnalyzerDynamics.hpp>	// Pascal unit
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

namespace Analyzernamelister
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum NamePartType { nptConstant, nptSpecial, nptNumber };
#pragma option pop

struct TSimilarityItem;
typedef TSimilarityItem *PSimilarityItem;

#pragma pack(push, 4)
struct TSimilarityItem
{
	NamePartType Typ;
	int Size;
	AnsiString Value;
} ;
#pragma pack(pop)

class DELPHICLASS TSimilarityList;
class PASCALIMPLEMENTATION TSimilarityList : public Classes::TList 
{
	typedef Classes::TList inherited;
	
public:
	PSimilarityItem operator[](int Index) { return Items[Index]; }
	
protected:
	PSimilarityItem __fastcall GetSim(int Index);
	void __fastcall PutSim(int Index, PSimilarityItem Item);
	
public:
	__fastcall TSimilarityList(void);
	__fastcall virtual ~TSimilarityList(void);
	int __fastcall AddSim(NamePartType nTyp, int nSize, AnsiString nValue);
	__property PSimilarityItem Items[int Index] = {read=GetSim, write=PutSim/*, default*/};
	HIDESBASE void __fastcall Delete(int Index);
};


class DELPHICLASS TSimilarNamesListItem;
class PASCALIMPLEMENTATION TSimilarNamesListItem : public Classes::TList 
{
	typedef Classes::TList inherited;
	
public:
	Analyzerdynamics::PSearchRecList operator[](int Index) { return Items[Index]; }
	
protected:
	Analyzerdynamics::PSearchRecList __fastcall GetSrchRec(int Index);
	void __fastcall PutSrchRec(int Index, Analyzerdynamics::PSearchRecList Item);
	
public:
	TSimilarityList* StartSimList;
	TSimilarityList* EndSimList;
	__fastcall TSimilarNamesListItem(void);
	__fastcall virtual ~TSimilarNamesListItem(void);
	__property Analyzerdynamics::PSearchRecList Items[int Index] = {read=GetSrchRec, write=PutSrchRec/*, default*/};
	HIDESBASE void __fastcall Delete(int Index);
	void __fastcall FillStartSimList(void);
	void __fastcall FillEndSimList(void);
	void __fastcall DetectSimilarItemAtStart(int &StartChar, AnsiString NameStr);
	void __fastcall DetectSimilarItemAtEnd(int &StartChar, AnsiString NameStr);
	NamePartType __fastcall GetCharType(char chr);
};


class DELPHICLASS TSimilarNamesList;
class PASCALIMPLEMENTATION TSimilarNamesList : public Classes::TList 
{
	typedef Classes::TList inherited;
	
public:
	TSimilarNamesListItem* operator[](int Index) { return Items[Index]; }
	
private:
	int __fastcall CountSimListSimilars(TSimilarityList* firstSList, TSimilarityList* secondSList);
	int __fastcall CountSimilars(TSimilarNamesListItem* firstSNLItem, TSimilarNamesListItem* secondSNLItem);
	void __fastcall MergeItems(int firstSNLIndex, int secondSNLIndex);
	
protected:
	TSimilarNamesListItem* __fastcall GetSN(int Index);
	void __fastcall PutSN(int Index, TSimilarNamesListItem* Item);
	
public:
	__fastcall TSimilarNamesList(void);
	__fastcall virtual ~TSimilarNamesList(void);
	__property TSimilarNamesListItem* Items[int Index] = {read=GetSN, write=PutSN/*, default*/};
	HIDESBASE void __fastcall Delete(int Index);
	int __fastcall AddNew(Analyzerdynamics::PSearchRecList nItemSearchRec);
	void __fastcall ReduceGroups(void);
	void __fastcall RenameGroupedFiles(void);
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Analyzernamelister */
using namespace Analyzernamelister;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// AnalyzerNameLister
