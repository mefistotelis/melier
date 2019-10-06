// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'ExportDlg.pas' rev: 6.00

#ifndef ExportDlgHPP
#define ExportDlgHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Dialogs.hpp>	// Pascal unit
#include <Buttons.hpp>	// Pascal unit
#include <ExtCtrls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Exportdlg
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TExportDialog;
class PASCALIMPLEMENTATION TExportDialog : public Dialogs::TOpenDialog 
{
	typedef Dialogs::TOpenDialog inherited;
	
private:
	Extctrls::TPanel* ExpSourcePanel;
	
protected:
	DYNAMIC void __fastcall DoClose(void);
	DYNAMIC void __fastcall DoShow(void);
	
public:
	Stdctrls::TRadioButton* ExpCurrent;
	Stdctrls::TRadioButton* ExpNewSearch;
	__fastcall virtual TExportDialog(Classes::TComponent* AOwner);
	__fastcall virtual ~TExportDialog(void);
	virtual bool __fastcall Execute(void);
};


//-- var, const, procedure ---------------------------------------------------

}	/* namespace Exportdlg */
using namespace Exportdlg;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// ExportDlg
