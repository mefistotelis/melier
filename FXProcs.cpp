//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FXProcs.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
  const char Digits[]={'0','1','2','3','4','5','6','7','8','9',0};
  const char Symbols[]={'-','_','.',' ','~',',',';','|','\\','/','!','#','%','^','&','*','?','`','\'',0};
  const char SmallLetters[]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',0};
  const char LargeLetters[]={'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',0};
  const char Letters[]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
                        'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',0};
  const char WordCharsPl[] = {'0','1','2','3','4','5','6','7','8','9',
                              'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
                              'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
                              '_','\'','π','Ê','Í','≥','Ò','Û','ú','ü','ø','•','∆',' ','£','—','”','å','è','Ø',0};
//---------------------------------------------------------------------------
  const unsigned __int64 MaxSafeIntVal=0x7fffffffffffffff;
  const int HugeIntSize=8;
//---------------------------------------------------------------------------
  const String PriorTimeCriticalText="TimeCritical";
  const String PriorHighestText="Highest";
  const String PriorHigherText="Higher";
  const String PriorNormalText="Normal";
  const String PriorLowerText="Lower";
  const String PriorLowestText="Lowest";
  const String PriorIdleText="Idle";
//---------------------------------------------------------------------------
bool IsAsText(int PosInStr,String Str)
{
  bool IsText=false;
  int CurrPos;
  for (CurrPos=1;CurrPos<=PosInStr;CurrPos++)
    {
       if (CurrPos<=(Str.Length())&&(Str[CurrPos]==34))
       IsText=!IsText;
    };
  //Jeszcze tylko korekcja dla cudzyslowu zamykajπcego
  if ((!IsText) && ((CurrPos<=Str.Length())&&(Str[CurrPos]==34))) IsText=true;
  return IsText;
};

bool IsAsCategory(int PosInStr,String Str)
{
  bool IsCat=false;
  int CurrPos;
  for (CurrPos=1;CurrPos<=PosInStr;CurrPos++)
    {
       if (CurrPos<=(Str.Length()))
         {
         if (Str[CurrPos]==91)// to "["
           IsCat=true;
           else
           if (Str[CurrPos]==93)// to "]"
             IsCat=false;
         };
    };
  //I korekcja dla znaku zamykajπcego
  if ((!IsCat) && ((CurrPos<=Str.Length())&&(Str[CurrPos]==93))) IsCat=true;
  return IsCat;
};

bool IsBeforeEqu(int PosInStr,String Str)
{
 bool IsEq=false;
 int EPos=Str.Pos("=");
 if (EPos>0)
  if (PosInStr<EPos) IsEq=true;
 return IsEq;
};

String GetBefore(String Chr,String Str)
{
 int EPos=Str.Pos(Chr);
 if (EPos>1)
  return Str.SubString(1,EPos-1);
  else return "";
};

String GetAfter(String Chr,String Str)
{
 int EPos=Str.Pos(Chr);
 if (EPos>0)
  return Str.SubString(EPos+Chr.Length(),Str.Length()-EPos+1);
  else return "";
};

String GetAfterLast(String Chr,String Str)
{
 int EPos=BackPos(Chr,Str);
 if (EPos>0)
  return Str.SubString(EPos+Chr.Length(),Str.Length()-EPos+1);
  else return Str;
};

int BackPos(String Chr,String Str)
{
 int bpos=Str.Length()-Chr.Length()+1;
 do
   {
   if (Str.SubString(bpos,Chr.Length())==Chr)
     return bpos;
   bpos--;
   }
   while (bpos>0);
 return 0;
};

String GetDAPathFromStr(String Str,int StrNum)
{
  int Index=1,Count=0,Num=1;
  while (Num<StrNum)
    {
    if (Index>Str.Length()) return "";
    if (isspace(Str[Index]))
      {
      Num++;
      do Index++; while ((Index<=Str.Length())&&(isspace(Str[Index])));
      } else Index++;
    };
  //Dobra, mamy laÒcuch, teraz jego dlugosc
  while(((Index+Count)<=Str.Length())&&(!isspace(Str[Index+Count])))
    Count++;
  return Str.SubString(Index,Count);
};

String GetBetween(String Ch1,String Ch2,String Str)
{
  int Index=Str.Pos(Ch1)+Ch1.Length();
  if (Index<=0) return "";
  int Count;
  if (Ch1==Ch2)
    {
    String Str2=Str.SubString(Index,Str.Length());
    Count=Str2.Pos(Ch2)-1;
    } else
    {
    Count=Str.LastDelimiter(Ch2)-Index;
    };
  if (Count<=0) return "";
  return Str.SubString(Index,Count);
};

void PrepareMathLine(String &Line)
/*
Usuwa najbardziej zewnÍtrzne spacje i nawiasy
 z linii tekstu
*/
{
  Line.Trim();
  if (Line.Length()>0)
    {
     // usuniÍcie ewentualnego zewnÍtrznego nawiasu
     if ((Line[1]=='(')&&(WhereIsClosing(Line)==Line.Length()))
       {
       Line.Delete(1,1);
       Line.Delete(Line.Length(),1);
       };
    };
};

int WhereIsClosing(String Line)
/*
Powinien dostac stringa, w ktorym [1] to '('.
Poda wtedy, gdzie siÍ ten nawias koÒczy.
*/
{
  int where=2;
  while (where<=Line.Length())
    {
    if (Line[where]==')')
      return where;
    if (Line[where]=='(')
      {
      int wadd=WhereIsClosing(Line.SubString(where,Line.Length()));
      if (wadd<0) return -1;
      where+=wadd;
      }
     else
      where++;
    };
  return -1;
};

String RemoveWhatInBrackets(String Line)
/*
ZastÍpuje wszystko pomiÍdzy nawiasami stringa
przez '_'
*/
{
  int BracketCounter=0;
  for (int Pos=1;Pos<=Line.Length();Pos++)
    {
    if (Line[Pos]=='(') BracketCounter++;
      else
      if (Line[Pos]==')') BracketCounter--;
        else
        if (BracketCounter>0) Line[Pos]='_';
    };
  return Line;
};

bool StrIsHexNum(String Str)
{
  bool Result=false;
  if (UpCase(Str[Str.Length()])=='H')
    {
    Result=true;
    for (int n=1;n<Str.Length();n++)
      {
      if (!isxdigit(Str[n]))
        {
        Result=false;
        break;
        };
      };
    };
  return Result;
};

bool StrIsAnyNum(String Str)
{
  return StrIsBinNum(Str)||StrIsDecNum(Str)||StrIsHexNum(Str);
};

bool StrIsBinNum(String Str)
{
  bool Result=false;
  if ((Str!="")&&(UpCase(Str[Str.Length()])=='Z'))
    {
    Result=true;
    for (int n=1;n<Str.Length();n++)
      {
      if ((Str[n]!='0')&&(Str[n]!='1'))
        {
        Result=false;
        break;
        };
      };
    };
  return Result;
};

bool StrIsDecNum(String Str)
{
  bool Result=true;
  for (int n=1;n<=Str.Length();n++)
    {
    if (!isdigit(Str[n]))
      {
      Result=false;
      break;
      };
    };
  return Result;
};

String IntToBin(int Num,int MinDigits)
{
  String Result="";
  int n=Num;
  int pow=1;
  while (n>0)
    {
    if (n & pow)
      {
      Result="1"+Result;
      n-=pow;
      }
      else Result="0"+Result;
    pow*=2;
    };
  while (Result.Length()<MinDigits) Result="0"+Result;
  return Result;
};

int BinToInt(String Str)
{
  int Result=0;
  int pow=1;
  // ostatni znak pomijamy bo powinno byc nim Z
  for (int n=Str.Length();n>0;n--)
      {
      if (Str[n]=='1')
        Result+=pow;
      pow*=2;
      };
  return Result;
};

String FormatString(String str)
/*
  Formatuje laÒcuch zawierajπcy kody<32 do wyswietlenia pod Windows
*/
{
  String dest="";
  int strPos=1;
  bool lastWasNum=false;
  while (strPos <= str.Length())
    {
    String tmp=str[strPos];
    if ((str[strPos]<32)&&(str[strPos]>=0) || (lastWasNum&&StrIsDecNum(tmp)))
      {
      dest+="\\"+IntToStr((unsigned __int8)str[strPos]);
      lastWasNum=true;
      }
     else
      {
      if (str[strPos]=='\\')
        dest+="\\";
       else
        dest+=str[strPos];
      lastWasNum=false;
      };
    strPos++;
    };
  return dest;
}

String DeFormatString(String str)
/*
  Usuwa z laÒcucha kody do wyswietlania
*/
{
  String dest="";
  int strPos=1;
  while (strPos <= str.Length())
    {
    if (str[strPos]=='\\')
      {
      strPos++;
      if (str[strPos]=='\\')
        {
        dest+="\\";
        strPos++;
        }
       else
        {
        String tmp=str[strPos];
        while ((StrIsDecNum(tmp)) && (strPos<str.Length()))
          {
          strPos++;
          tmp+=str[strPos];
          };
        tmp.Delete(tmp.Length(),1);
        if (tmp.Length()>0)
          {
          dest+=(char)tmp.ToInt();
          }
         else
          {
          dest+="\\";
          strPos++;
          };
        };
      }
     else
      {
      dest+=str[strPos];
      strPos++;
      };
    };
  return dest;
}
//---------------------------------------------------------------------------
bool StrToBoolean(const String &str)
{
  String nstr=str.UpperCase();
  if ((nstr[1]=='Y')||(nstr[1]=='T')||(nstr=="ON"))
    return true;
  return false;
}
//---------------------------------------------------------------------------
String BooleanToStr(bool val)
{
  if (val) return "yes";
  return "no";
}
//---------------------------------------------------------------------------
TControl* FindAnyChildControl(TWinControl *Parent,const AnsiString &ControlName)
{
  for (int cIndex=0;cIndex<Parent->ControlCount;cIndex++)
    {
    TControl *Control=Parent->Controls[cIndex];
    if ((Control!=NULL)&&(Control->Name==ControlName))
      return Control;
    };
  return NULL;
}
//---------------------------------------------------------------------------
int FindSubstringPosFrom(int startIndex,String SubStr,String MainString)
{
  int Result=-1;
  int i=startIndex;
  while (i <= MainString.Length())
    {
    if (MainString.SubString(i,SubStr.Length())==SubStr)
      {
      Result=i;
      break;
      };
    i++;
    };
  return Result;
}
//---------------------------------------------------------------------------
int FindSubMemPos(const char *SubBuf,uint SubBufSize,const char *MainBuf,uint MainBufSize)
{
  uint i=0;
  while (i <= (MainBufSize-SubBufSize))
    {
    uint k=0;
    while (k<SubBufSize)
      {
      //Jak sπ rÛøne - nastÍpna pozycja
      if (MainBuf[i+k]!=SubBuf[k])
          break;
        else
      //Jak sπ rÛwne i to koniec - znaleziono
      if (k >= SubBufSize-1)
          return i;
        else
      //Jak sπ rÛwne i to nie koniec, porÛwnujemy dalej
          k++;
      }
    i++;
    };
  return -1;
}
//---------------------------------------------------------------------------
TThreadPriority StrToPriority(const String &str)
{
  String strVal=str.UpperCase();
  if (strVal==PriorIdleText.UpperCase())
    return tpIdle;
  if (strVal==PriorLowestText.UpperCase())
    return tpLowest;
  if (strVal==PriorLowerText.UpperCase())
    return tpLower;
  if (strVal==PriorNormalText.UpperCase())
    return tpNormal;
  if (strVal==PriorHigherText.UpperCase())
    return tpHigher;
  if (strVal==PriorHighestText.UpperCase())
    return tpHighest;
  if (strVal==PriorTimeCriticalText.UpperCase())
    return tpTimeCritical;
  return tpNormal;
}
//---------------------------------------------------------------------------
String PriorityToStr(TThreadPriority val)
{
  switch (val)
    {
    case tpIdle:return PriorIdleText;
    case tpLowest:return PriorLowestText;
    case tpLower:return PriorLowerText;
    case tpNormal:return PriorNormalText;
    case tpHigher:return PriorHigherText;
    case tpHighest:return PriorHighestText;
    case tpTimeCritical:return PriorTimeCriticalText;
    default: return "Unknown";
    };
}
//---------------------------------------------------------------------------
int FileCopy(String SourceFile,String DestFile,int Options)
// Kopiuje plik. Nazwy podawaÊ ze úcieøkπ!
// Zwroty:
//0 - OK
//1 - Plik docelowy istnieje
//2 - Nie moøna usunπÊ pliku
//3 - B≥πd podczas kopiowania
{
 //Testy poczπtkowe - tu zapewniamy, by plik docelowy nie istnia≥
 __try
  {
  if (FileExists(DestFile))
    {
    if (Options & FC_OVERWRITE)
        {
        bool IsDeleted=DeleteFile(DestFile);
        if (!IsDeleted) return 2;
        }
      else
        return 1;
    };
  }__except(EXCEPTION_EXECUTE_HANDLER) {return 2;};
  TFileStream *srcStrm=NULL;
  TFileStream *destStrm=NULL;
 __try
  {
  srcStrm=new TFileStream(SourceFile,fmOpenRead|fmShareDenyWrite);
  destStrm=new TFileStream(DestFile,fmOpenWrite|fmShareDenyWrite|fmCreate);
  destStrm->CopyFrom(srcStrm, 0); //0 oznacza ca≥y stream
  }__except(EXCEPTION_EXECUTE_HANDLER)
  {
  delete srcStrm;
  delete destStrm;
  return 2;
  };
 if (!(Options & FC_ACTUALDATE))
   __try
    {
    int prevDate=FileGetDate(srcStrm->Handle);
    FileSetDate(destStrm->Handle,prevDate);
    }__except(EXCEPTION_EXECUTE_HANDLER) {};
  delete srcStrm;
  delete destStrm;
  return 0;
}
//---------------------------------------------------------------------------
String ChangeCaseWordsStartsBig(const String &SrcStr)
{
  bool WordStart=true;
  String Result=AnsiLowerCase(SrcStr);
  String UpperStr=AnsiUpperCase(SrcStr);
  uint j=1;
    for (uint i=1;i<=(uint)SrcStr.Length();i++)
      {
      if (strchr(WordCharsPl,SrcStr[j])!=NULL)
        {
        if ((WordStart)&&(strchr(WordCharsPl,SrcStr[i])!=NULL))
          {  //Jeúli mamy wyraz i jest on d≥uøszy niø 1 znak
          Result[j]=UpperStr[j];
          WordStart=false;
          }
        }
       else
        WordStart=true;
      j=i;
      }
  return Result;
}
//---------------------------------------------------------------------------
String Char2Space(char DChar,const String &SrcStr)
{
  String Result=SrcStr;
  int chrPos=Result.Pos(DChar);
  while (chrPos>0)
    {
    Result[chrPos]=32;
    chrPos=Result.Pos(DChar);
    }
  return Result;
}
//---------------------------------------------------------------------------
String CharReplace(char DChar,char SChar,const String &SrcStr)
{
  String Result=SrcStr;
  int chrPos=Result.Pos(DChar);
  if ((SChar==DChar)) return Result;
  while (chrPos>0)
    {
    Result[chrPos]=SChar;
    chrPos=Result.Pos(DChar);
    }
  return Result;
}
//---------------------------------------------------------------------------
String RemoveExtension(String FullFName)
//Usuwa rozszerzenie z nazwy pliku, tzn.
//wywala ze stringa wszystko po ostatniej kropce
{
  int DotPos;
  String Result;
  DotPos=BackPos(".",FullFName);
  if (DotPos<=1)
    Result=FullFName;
   else
    Result=FullFName.SubString(1,max(DotPos-1,0));
  return Result;
}
//---------------------------------------------------------------------------
String RemoveExtensionAdv(String FullFName)
//Usuwa rozszerzenie z nazwy pliku, tzn.
//wywala ze stringa wszystko po ostatniej kropce
{
  int DotPos;
  DotPos=BackPos(".",FullFName);
  if (DotPos<=1)
    return FullFName.Trim();
   else
    {
    String Result;
    int ExtentionLength=FullFName.Length()-DotPos;
    String Extention=FullFName.SubString(DotPos+1,ExtentionLength);
    if ((ExtentionLength<5)&&(!StrIsDecNum(Extention)))
        {
        Result=FullFName.SubString(1,max(DotPos-1,0));
        if ((Extention.UpperCase()=="GZ")||(Extention.UpperCase()=="BZ2")||(Extention.UpperCase()=="BZ"))
          {
          DotPos=BackPos(".",Result);
          if (DotPos>1)
            {
            ExtentionLength=Result.Length()-DotPos;
            Extention=Result.SubString(DotPos+1,ExtentionLength);
            if (Extention.UpperCase()=="TAR")
                Result=Result.SubString(1,max(DotPos-1,0));
            }
          }
        }
      else
        {
        Result=FullFName;
        }
    return Result.Trim();
    }
}
//---------------------------------------------------------------------------
String ExtractExtensionAdv(String FullFName)
//Usuwa rozszerzenie z nazwy pliku, tzn.
//wywala ze stringa wszystko po ostatniej kropce
{
  int DotPos;
  DotPos=BackPos(".",FullFName);
  if (DotPos<=1)
    return "";
   else
    {
    String Result;
    int ExtentionLength=FullFName.Length()-DotPos;
    String Extention=FullFName.SubString(DotPos+1,ExtentionLength);
    if ((ExtentionLength<5)&&(!StrIsDecNum(Extention)))
        {
        Result=FullFName.SubString(1,max(DotPos-1,0));
        if ((Extention.UpperCase()=="GZ")||(Extention.UpperCase()=="BZ2")||(Extention.UpperCase()=="BZ"))
          {
          DotPos=BackPos(".",Result);
          if (DotPos>1)
            {
            ExtentionLength=Result.Length()-DotPos;
            String SecExt=Result.SubString(DotPos+1,ExtentionLength);
            if (SecExt.UpperCase()=="TAR")
                Extention=SecExt+" "+Extention;
            }
          }
        }
      else
        {
        Extention="";
        }
    return Extention.Trim();
    }
}
//---------------------------------------------------------------------------
int ListItemsIndexOfCapt(String CaptStr,TListItems *Items)
{
  for (int cntr=0;cntr<Items->Count;cntr++)
    {
    TListItem *Item=Items->Item[cntr];
    if (Item->Caption==CaptStr) return cntr;
    }
  return -1;
}
//---------------------------------------------------------------------------
int ListItemsIndexOfSub(int SubNum,String SubStr,TListItems *Items)
{
  for (int cntr=0;cntr<Items->Count;cntr++)
    {
    TListItem *Item=Items->Item[cntr];
    if (Item->SubItems->Strings[SubNum]==SubStr) return cntr;
    }
  return -1;
}
//---------------------------------------------------------------------------
void ConvWin1250ToCP852(char *strbuf,unsigned int maxlength)
{
  unsigned int pos=0;
  while ((pos<maxlength)&&(strbuf[pos]!=0))
    {
    ucs4_t UnicodeStr;
    cp1250_mbtowc(NULL,&UnicodeStr,strbuf+pos,0);
    cp852_wctomb(NULL,(strbuf+pos),UnicodeStr,0);
    pos++;
    }
}
//---------------------------------------------------------------------------
void ConvCP852ToWin1250(char *strbuf,unsigned int maxlength)
{
  unsigned int pos=0;
  while ((pos<maxlength)&&(strbuf[pos]!=0))
    {
    ucs4_t UnicodeStr;
    cp852_mbtowc(NULL,&UnicodeStr,strbuf+pos,0);
    cp1250_wctomb(NULL,(strbuf+pos),UnicodeStr,0);
    pos++;
    }
}
//---------------------------------------------------------------------------
void PrepareRTFString(String &Dest,const String Source)
{
  unsigned int maxlength=Source.Length();
  unsigned int pos;
  Dest="";
  for (pos=1;pos<=maxlength;pos++)
    {
    unsigned char oneChar=Source[pos];
    if (oneChar<128)
      {
      switch (oneChar)
        {
        case '\\':Dest+="\\\\";break;
        case '{':Dest+="\\{";  break;
        default: Dest+=(char)oneChar;break;
        }
      }
     else
      Dest+="\\'"+IntToHex(oneChar,2);
    }
}
//---------------------------------------------------------------------------
String RmNationalWin1250(unsigned char chr)
{
  switch (chr)
    {
    case 129:return "_";
    case 130:return ",";
    case 131:return "_";
    case 132:return "\"";
    case 133:return "...";
    case 134:return "t";
    case 135:return "++";
    case 136:return "_";
    case 137:return "%%";
    case 138:return "S";
    case 139:return "<";
    case 140:return "S";
    case 141:return "T";
    case 142:return "Z";
    case 143:return "Z";
    case 144:return "_";
    case 145:return "'";
    case 146:return "'";
    case 147:return "\"";
    case 148:return "\"";
    case 149:return "\'";
    case 150:return "-";
    case 151:return "-";
    case 152:return "_";
    case 153:return "TM";
    case 154:return "s";
    case 155:return ">";
    case 156:return "s";
    case 157:return "t";
    case 158:return "z";
    case 159:return "z";
    case 160:return " ";
    case 161:return "^";
    case 162:return "^";
    case 163:return "L";
    case 164:return "#";
    case 165:return "A";
    case 166:return "|";
    case 167:return "$";
    case 168:return "\"";
    case 169:return "(c)";
    case 170:return "S";
    case 171:return "<<";
    case 172:return "-";
    case 173:return "-";
    case 174:return "(R)";
    case 175:return "Z";
    case 176:return "\'";
    case 177:return "+/-";
    case 178:return ",";
    case 179:return "l";
    case 180:return "\'";
    case 181:return "u";
    case 182:return "P";
    case 183:return "\'";
    case 184:return ",";
    case 185:return "a";
    case 186:return "s";
    case 187:return ">>";
    case 188:return "L";
    case 189:return "\"";
    case 190:return "l";
    case 191:return "z";
    case 192:return "R";
    case 193:return "A";
    case 194:return "A";
    case 195:return "A";
    case 196:return "A";
    case 197:return "L";
    case 198:return "C";
    case 199:return "C";
    case 200:return "C";
    case 201:return "E";
    case 202:return "E";
    case 203:return "E";
    case 204:return "E";
    case 205:return "I";
    case 206:return "I";
    case 207:return "D";
    case 208:return "D";
    case 209:return "N";
    case 210:return "N";
    case 211:return "O";
    case 212:return "O";
    case 213:return "O";
    case 214:return "O";
    case 215:return "x";
    case 216:return "R";
    case 217:return "U";
    case 218:return "U";
    case 219:return "U";
    case 220:return "U";
    case 221:return "Y";
    case 222:return "T";
    case 223:return "b";
    case 224:return "r";
    case 225:return "a";
    case 226:return "a";
    case 227:return "a";
    case 228:return "a";
    case 229:return "l";
    case 230:return "c";
    case 231:return "c";
    case 232:return "c";
    case 233:return "e";
    case 234:return "e";
    case 235:return "e";
    case 236:return "e";
    case 237:return "i";
    case 238:return "i";
    case 239:return "d";
    case 240:return "d";
    case 241:return "n";
    case 242:return "n";
    case 243:return "o";
    case 244:return "o";
    case 245:return "o";
    case 246:return "o";
    case 247:return "/";
    case 248:return "r";
    case 249:return "u";
    case 250:return "u";
    case 251:return "u";
    case 252:return "u";
    case 253:return "y";
    case 254:return "t";
    case 255:return "\'";
    default: return (char)chr;
    }
}
//---------------------------------------------------------------------------
String RmNationalCharsWin1250(const String Source)
{
  unsigned int pos;
  unsigned int maxlength=Source.Length();
  String Dest;
  for (pos=1;pos<=maxlength;pos++)
    Dest+=RmNationalWin1250(Source[pos]);
  return Dest;
}
//---------------------------------------------------------------------------


