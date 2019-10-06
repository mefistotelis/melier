#ifndef SearcherItmBufferH
#define SearcherItmBufferH

#include <Registry.hpp>	// Pascal unit
#include <FileCtrl.hpp>	// Pascal unit
#include <ExtCtrls.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
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
#include "SynchronizerUnit.h"
//#include "FXProcs.h"

struct TSrchItmBufItem
  {
  int Time;
  int Size;
  TStringList *FileBody;
  };

class TSearcherItmBuffer : public Classes::TList
{
public:
	TSrchItmBufItem* operator[](int Index) { return Items[Index]; }

protected:
	TSrchItmBufItem* __fastcall GetItm(int Index);
	void __fastcall PutItm(int Index, TSrchItmBufItem *Item);

private:
	void __fastcall FreeItem(int Index);

public:
	int BufferUsed;
	int MaxBuffer;
	__fastcall TSearcherItmBuffer(void);
	__fastcall virtual ~TSearcherItmBuffer(void);
	bool __fastcall CanLoad(int nItmNumber, AnsiString nFileName);
	int __fastcall ConsiderAddingItem(int nItmNumber, AnsiString nFileName, Classes::TStrings* nFileBody);
	int __fastcall AddItm(int nItmNumber, int nDate, int nFileSize, Classes::TStrings* &nFileBody);
	__property TSrchItmBufItem *Items[int Index] = {read=GetItm, write=PutItm/*, default*/};
	HIDESBASE void __fastcall Delete(int Index);
	void __fastcall ChangeItemsCount(int nCount);
};


//-- end unit ----------------------------------------------------------------
#endif	// SearcherItmBuffer
