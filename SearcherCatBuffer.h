#ifndef SearcherCatBufferH
#define SearcherCatBufferH

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

struct TSrchCatBufItem
  {
  char CatCombination[MaxCategories];
  TStringList *Lines;
  DynamicArray<int> ItemNums;
  };

class TSearcherCatBuffer : public Classes::TList
{
public:
	TSrchCatBufItem* operator[](int Index) { return Items[Index]; }

protected:
	TSrchCatBufItem* __fastcall GetItm(int Index);
	void __fastcall PutItm(int Index, TSrchCatBufItem *Item);

private:
	void __fastcall FreeItem(int Index);

public:
	__fastcall TSearcherCatBuffer(void);
	__fastcall virtual ~TSearcherCatBuffer(void);
	__property TSrchCatBufItem *Items[int Index] = {read=GetItm, write=PutItm/*, default*/};
	HIDESBASE void __fastcall Delete(int Index);
	void __fastcall ChangeItemsCount(int nCount);
};


//-- end unit ----------------------------------------------------------------
#endif	// SearcherCatBuffer
