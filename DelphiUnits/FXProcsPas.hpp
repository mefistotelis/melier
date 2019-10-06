// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'FXProcsPas.pas' rev: 6.00

#ifndef FXProcsPasHPP
#define FXProcsPasHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SysUtils.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Fxprocspas
{
//-- type declarations -------------------------------------------------------
typedef bool *pBoolean;

//-- var, const, procedure ---------------------------------------------------
extern PACKAGE System::Set<char, 0, 255>  WordChars;
#define DigitsPas (System::Set<char, 0, 255> () << '\x30' << '\x31' << '\x32' << '\x33' << '\x34' << '\x35' << '\x36' << '\x37' << '\x38' << '\x39' )
#define SymbolsPas (System::Set<char, 0, 255> () << '\x20' << '\x21' << '\x23' << '\x25' << '\x26' << '\x2a' << '\x2c' << '\x2d' << '\x2e' << '\x2f' << '\x3b' << '\x3f' << '\x40' << '\x41' << '\x43' << '\x45' << '\x46' << '\x4a' << '\x4c' << '\x4d' << '\x4e' << '\x4f' << '\x5b' << '\x5f' << '\x60' << '\x61' << '\x63' << '\x65' << '\x66' << '\x6a' << '\x6c' << '\x6d' << '\x6e' << '\x6f' << '\x7b' << '\x7f' )
#define LettersPas (System::Set<char, 0, 255> () << '\x41' << '\x42' << '\x43' << '\x44' << '\x45' << '\x46' << '\x47' << '\x48' << '\x49' << '\x4a' << '\x4b' << '\x4c' << '\x4d' << '\x4e' << '\x4f' << '\x50' << '\x51' << '\x52' << '\x53' << '\x54' << '\x55' << '\x56' << '\x57' << '\x58' << '\x59' << '\x5a' << '\x61' << '\x62' << '\x63' << '\x64' << '\x65' << '\x66' << '\x67' << '\x68' << '\x69' << '\x6a' << '\x6b' << '\x6c' << '\x6d' << '\x6e' << '\x6f' << '\x70' << '\x71' << '\x72' << '\x73' << '\x74' << '\x75' << '\x76' << '\x77' << '\x78' << '\x79' << '\x7a' )
extern PACKAGE int __fastcall max(int x1, int x2);
extern PACKAGE int __fastcall min(int x1, int x2);
extern PACKAGE AnsiString __fastcall GetParam(int Number, AnsiString Src);
extern PACKAGE AnsiString __fastcall GetTabParam(int Number, AnsiString Src);
extern PACKAGE AnsiString __fastcall GetSpaceParam(int Number, AnsiString Src);
extern PACKAGE AnsiString __fastcall GetBefore(char x, AnsiString Src);
extern PACKAGE AnsiString __fastcall GetAfter(char x, AnsiString Src);
extern PACKAGE bool __fastcall FindNextBlockBorders(Classes::TStrings* Lines, int &lnStart, int &lnEnd);
extern PACKAGE AnsiString __fastcall ChangeCaseWordsStartsBig(AnsiString SrcStr);
extern PACKAGE AnsiString __fastcall CheckCase(AnsiString SrcStr);
extern PACKAGE bool __fastcall IsNearlyInRightCase(AnsiString Str);
extern PACKAGE AnsiString __fastcall RemoveMultipleChars(char x, AnsiString SrcStr);

}	/* namespace Fxprocspas */
using namespace Fxprocspas;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// FXProcsPas
