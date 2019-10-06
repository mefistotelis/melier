//---------------------------------------------------------------------------

#ifndef CaseConvertUnitH
#define CaseConvertUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <FileCtrl.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TCaseConvertDialog : public TForm
{
__published:	// IDE-managed Components
        TRadioGroup *ConvertionType;
        TBitBtn *OKBtn;
        TBitBtn *CancelBtn;
        TPanel *BtnPanel;
        TPanel *BtnRightPanel;
private:	// User declarations
public:		// User declarations
        __fastcall TCaseConvertDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TCaseConvertDialog *CaseConvertDialog;
//---------------------------------------------------------------------------
#endif
