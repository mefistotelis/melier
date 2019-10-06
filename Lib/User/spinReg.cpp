//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Spin.hpp"

//---------------------------------------------------------------------------
namespace Spin
{
        void __fastcall PACKAGE Register()
        {
                 TComponentClass classes[1] = {__classid(TSpinEdit)};
                 RegisterComponents("Samples", classes, 0);
        }
}
//---------------------------------------------------------------------------
