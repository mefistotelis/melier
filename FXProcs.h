//---------------------------------------------------------------------------

#ifndef FXProcsH
#define FXProcsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include "libiconv_converters.h"
//---------------------------------------------------------------------------
  typedef unsigned int uint;
  typedef unsigned short ushort;
  typedef unsigned char uchar;
//---------------------------------------------------------------------------
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))  
//---------------------------------------------------------------------------
  bool IsAsText(int,String);
  bool IsAsCategory(int PosInStr,String Str);
  bool IsBeforeEqu(int PosInStr,String Str);
//---------------------------------------------------------------------------
  String GetBefore(String Chr,String Str);
  String GetAfter(String Chr,String Str);
  String GetAfterLast(String Chr,String Str);
  String GetBetween(String Ch1,String Ch2,String Str);
  int BackPos(String Chr,String Str);
  String Char2Space(char DChar,const String &SrcStr);
//---------------------------------------------------------------------------
  int ListItemsIndexOfCapt(String CaptStr,TListItems *Items);
  int ListItemsIndexOfSub(int SubNum,String SubStr,TListItems *Items);
//---------------------------------------------------------------------------
void ConvWin1250ToCP852(char *strbuf,unsigned int maxlength);
void ConvCP852ToWin1250(char *strbuf,unsigned int maxlength);
//---------------------------------------------------------------------------
String RmNationalWin1250(unsigned char chr);
String RmNationalCharsWin1250(const String Source);
//---------------------------------------------------------------------------
void PrepareRTFString(String &Dest,const String Source);
//---------------------------------------------------------------------------
  String GetDAPathFromStr(String,int);
  void PrepareMathLine(String &);
  int WhereIsClosing(String Line);
  String RemoveWhatInBrackets(String);
  bool StrIsDecNum(String Str);
  bool StrIsHexNum(String Str);
  bool StrIsBinNum(String Str);
  bool StrIsAnyNum(String Str);
  int BinToInt(String Str);
  String IntToBin(int Num,int MinDigits);
  String FormatString(String str);
  String DeFormatString(String str);
  bool StrToBoolean(const String &str);
  String BooleanToStr(bool val);
  TThreadPriority StrToPriority(const String &str);
  String PriorityToStr(TThreadPriority val);
  TControl* FindAnyChildControl(TWinControl *Parent,const AnsiString &ControlName);
  int FindSubstringPosFrom(int startIndex,String SubStr,String MainString);
  int FindSubMemPos(const char *SubBuf,uint SubBufSize,const char *MainBuf,uint MainBufSize);
  String ChangeCaseWordsStartsBig(const String &SrcStr);
//---------------------------------------------------------------------------
enum FileCopyOptions
      {
      FC_OVERWRITE           =  1,
      FC_ACTUALDATE          =  2
      };
const int FileCopyReadBufSize=65536;
//---------------------------------------------------------------------------
int FileCopy(String SourceFile,String DestFile,int Options);
String RemoveExtension(String FullFName);
String RemoveExtensionAdv(String FullFName);
String ExtractExtensionAdv(String FullFName);
//---------------------------------------------------------------------------
#endif
