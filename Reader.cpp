//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USEFORM("KatalogCForm.cpp", Katalog);
USEFORM("CaseConvertUnit.cpp", CaseConvertDialog);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "Katalog CD";
                 Application->CreateForm(__classid(TKatalog), &Katalog);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------
