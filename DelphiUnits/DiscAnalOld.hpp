// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'DiscAnalOld.pas' rev: 6.00

#ifndef DiscAnalOldHPP
#define DiscAnalOldHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SynchronizerUnitPas.hpp>	// Pascal unit
#include <FXProcsPas.hpp>	// Pascal unit
#include <Grids.hpp>	// Pascal unit
#include <Buttons.hpp>	// Pascal unit
#include <Registry.hpp>	// Pascal unit
#include <FileCtrl.hpp>	// Pascal unit
#include <ExtCtrls.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
#include <TabNotBk.hpp>	// Pascal unit
#include <ComCtrls.hpp>	// Pascal unit
#include <Dialogs.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Discanalold
{
//-- type declarations -------------------------------------------------------
struct TTreeSpis;
typedef TTreeSpis *PTreeSpis;

#pragma pack(push, 4)
struct TTreeSpis
{
	int Size;
	int Num;
	AnsiString Name;
	AnsiString Text;
	TTreeSpis *Items;
	TTreeSpis *Up;
	TTreeSpis *Next;
} ;
#pragma pack(pop)

class DELPHICLASS TDiscAnalOld;
class PASCALIMPLEMENTATION TDiscAnalOld : public System::TObject 
{
	typedef System::TObject inherited;
	
public:
	Synchronizerunitpas::TSynchronizerPas* Synchronizer;
	void __fastcall CreateListOld(Classes::TStrings* List);
	__fastcall TDiscAnalOld(Synchronizerunitpas::TSynchronizerPas* nSynchronizer);
	__fastcall virtual ~TDiscAnalOld(void);
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Discanalold */
using namespace Discanalold;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// DiscAnalOld
