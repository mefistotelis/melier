//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SpisEditorC.h"
#pragma package(smart_init)

#define MAX_BUFFER_SIZE 512
//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//      Synchronize(UpdateCaption);
//---------------------------------------------------------------------------
String ExecuteExternalFile(String csExeName, String csArguments)
{
  String csExecute;
  csExecute=csExeName + " " + csArguments;

  SECURITY_ATTRIBUTES secattr;
  ZeroMemory(&secattr,sizeof(secattr));
  secattr.nLength = sizeof(secattr);
  secattr.bInheritHandle = TRUE;

  HANDLE rPipe, wPipe;

  //Create pipes to write and read data
  CreatePipe(&rPipe,&wPipe,&secattr,0);
  //
  STARTUPINFO sInfo;
  ZeroMemory(&sInfo,sizeof(sInfo));
  PROCESS_INFORMATION pInfo;
  ZeroMemory(&pInfo,sizeof(pInfo));
  sInfo.cb=sizeof(sInfo);
  sInfo.dwFlags=STARTF_USESTDHANDLES;
  sInfo.hStdInput=NULL;
  sInfo.hStdOutput=wPipe;
  sInfo.hStdError=wPipe;
  char command[1024];
  strcpy(command,csExecute.c_str());

  //Create the process here.
  CreateProcess(0,command,0,0,TRUE,
          NORMAL_PRIORITY_CLASS|CREATE_NO_WINDOW,0,0,&sInfo,&pInfo);
  CloseHandle(wPipe);

  //now read the output pipe here.
  char buf[100];
  unsigned long reDword;
  String m_csOutput,csTemp;
  bool res;
  do
  {
                  res=::ReadFile(rPipe,buf,100,&reDword,0);
                  csTemp=buf;
                  m_csOutput+=csTemp.SubString(0,reDword);
  }while(res);
  return m_csOutput;
}
int EmulateCommandPrompt(LPSTR cmdline)
{
	STARTUPINFO sti = { 0 };
	SECURITY_ATTRIBUTES sats = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	HANDLE pipin_w, pipin_r, pipout_w, pipout_r;
	char buffer[MAX_BUFFER_SIZE];
	DWORD writ, excode, read, available;
	int ret = 0;
	
	pipin_w = pipin_r = pipout_w = pipout_r = NULL;
	
	for(;;)
	{
		//set SECURITY_ATTRIBUTES struct fields
		sats.nLength = sizeof(sats);
		sats.bInheritHandle = TRUE;
		sats.lpSecurityDescriptor = NULL;
		
		//create child's stdout pipes
		if(!CreatePipe(&pipout_r, &pipout_w, &sats, 0)) break;
		//and its stdin pipes
		if(!CreatePipe(&pipin_r, &pipin_w, &sats, 0)) break;
		printf("Created pipes\n");
		
		//now set STARTUPINFO struct fields (from the child's point of view)
		sti.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		sti.wShowWindow = SW_HIDE;
		sti.hStdInput = pipin_r;
		sti.hStdOutput = pipout_w;
		sti.hStdError = pipout_w;
		
		//create the process...
		if(!CreateProcess(NULL, "cmd.exe", NULL, NULL, TRUE, 
			0, NULL, NULL, &sti, &pi)) break;
		printf("Created process (%s)\n", cmdline);
		
		//now have a continuous loop to get and recieve info
		for(;;)
		{
			//make sure process is still running
			GetExitCodeProcess(pi.hProcess, &excode);
			if(excode != STILL_ACTIVE) break;
			//printf("Process still running\n");
			
			//give it time to set up/react
			Sleep(500);
			
			//now check to see if process has anything to say
			if(!PeekNamedPipe(pipout_r, buffer, 
				sizeof(buffer), &read, &available, NULL)) ret = 10;
			//printf("Peeked\n");
			
			//is there anything to be read in the pipe?
			if(read)
			{
				do
				{
					ZeroMemory(buffer, sizeof(buffer));
					//read it and print to stdout
					if(!ReadFile(pipout_r, buffer, sizeof(buffer), &read, NULL) || !read) ret = 7;
					buffer[read] = 0;
					fprintf(stdout, "%s", buffer);
					if(ret) break;
				}
				while(read >= sizeof(buffer));
			}
			
			//make sure we didn't run into any errors
			if(!ret)
			{
				//get info and write it to pipe
				ZeroMemory(buffer, sizeof(buffer));
				fgets(buffer, sizeof(buffer), stdin);
				if(!strnicmp(buffer, "exit", 4)) ret = 12;
				if(!WriteFile(pipin_w, buffer, strlen(buffer), &writ, NULL)) ret = 8;
			}
			if(ret) break;
		}
		
		break;
	}
	
	//clean up any unfinished business
	if(pipin_w != NULL) CloseHandle(pipin_w);
	if(pipin_r != NULL) CloseHandle(pipin_r);
	if(pipout_w != NULL) CloseHandle(pipout_w);
	if(pipout_r != NULL) CloseHandle(pipout_r);
	if(pi.hProcess != NULL) CloseHandle(pi.hProcess);
	if(pi.hThread != NULL) CloseHandle(pi.hThread);

	return ret;
}
//---------------------------------------------------------------------------
__fastcall TSpisEditor::TSpisEditor(TSynchronizer *nSynchronizer)
        : TThread(true)
{
  Synchronizer=nSynchronizer;
  CaseConvertDialog=new TCaseConvertDialog(Synchronizer->MainForm);
  DiscAnalyzer=NULL;
}
//---------------------------------------------------------------------------
__fastcall TSpisEditor::~TSpisEditor()
{
  delete CaseConvertDialog;
  CaseConvertDialog=NULL;
  if (DiscAnalyzer!=NULL)
      {
      DiscAnalyzer->Terminate=true;
      delete DiscAnalyzer;
      DiscAnalyzer=NULL;
      }
}
//---------------------------------------------------------------------------
void __fastcall TSpisEditor::Execute()
{
 do
  {
  switch (Function)
    {
    case etfCreateList:
        Function=etfNone;
        CreateList(this,FuncParamList);
        break;
    case etfCreateListOld:
        Function=etfNone;
        CreateListOld(FuncParamList);
        break;
    case etfCrListMenader:
        Function=etfNone;
        CreateListMenader(FuncParamList);
        break;
    }
  Suspend();
  } while (!Terminated);
}
//---------------------------------------------------------------------------
void TSpisEditor::CreateListMenader(TStrings *List)
{
  String Drv;
 __try
  {
  if (DiscAnalyzer!=NULL)
    {
    DiscAnalyzer->Terminate=true;
    return;
    }
  Drv=Synchronizer->DrvSelector->Text;
  if (Drv.Length()<1) return;
  if (Drv.SubString(2,3)==": [")
    Drv=Drv.SubString(1,2);
  char full_cmdline[2048];
  strcpy(full_cmdline,"\"");
  strncat(full_cmdline,Drv.c_str(),1500);
  strcat(full_cmdline,"\" \"antemp.txt\"");
  //create the process...
  ExecuteExternalFile("medaner", full_cmdline);
//  EmulateCommandPrompt(full_cmdline);
//      MessageBox(Synchronizer->MainForm->Handle,"Wyst¹pi³ b³¹d podczas próby analizy dysku.","Cosik ni tok jok czeba...",MB_OK|MB_ICONERROR|MB_APPLMODAL);
  List->Capacity=MaxInt; //Nie wiem czy to cokolwiek daje....
  if ((Terminated)) return;
  List->LoadFromFile("antemp.txt");
//  DiscAnalyzer->GetGeneratedInfo(Sender,List);
  }
 __finally
  {
  };
}
//---------------------------------------------------------------------------
void TSpisEditor::CreateList(TObject *Sender,TStrings *List)
{
  String Drv;
 __try
  {
  if (DiscAnalyzer!=NULL)
    {
    DiscAnalyzer->Terminate=true;
    return;
    }
  Drv=Synchronizer->DrvSelector->Text;
  if (Drv.Length()<1) return;
  if (Drv.SubString(2,3)==": [")
    Drv=Drv.SubString(1,2);
  DiscAnalyzer=new TDiscAnalyzer(Synchronizer);
  if (DiscAnalyzer->AnalyzeDrive(Sender,Drv))
      {
      }
    else
      MessageBox(Synchronizer->MainForm->Handle,"Wyst¹pi³ b³¹d podczas próby analizy dysku.","Cosik ni tok jok czeba...",MB_OK|MB_ICONERROR|MB_APPLMODAL);
  List->Capacity=MaxInt; //Nie wiem czy to cokolwiek daje....
  if ((Terminated)||(DiscAnalyzer==NULL)) return;
  DiscAnalyzer->GetGeneratedInfo(Sender,List);
  }
 __finally
  {
  delete DiscAnalyzer;
  DiscAnalyzer=NULL;
  };
}
//---------------------------------------------------------------------------
void TSpisEditor::CreateListOld(TStrings *List)
{
  TDiscAnalOld *DiscAnalOld;
 __try
  {
  DiscAnalOld=new TDiscAnalOld(Synchronizer);
  DiscAnalOld->CreateListOld(List);
  }
 __finally
  {
  delete DiscAnalOld;
  };

}
//---------------------------------------------------------------------------
void TSpisEditor::ChangeSelectionCase(TObject *Sender,TRichEdit *Editor)
{
  if (CaseConvertDialog->ShowModal()==mrOk)
    {
    switch (CaseConvertDialog->ConvertionType->ItemIndex)
      {
      case 0:Editor->SelText=AnsiLowerCase(Editor->SelText);break;
      case 1:Editor->SelText=AnsiUpperCase(Editor->SelText);break;
      case 2:Editor->SelText=ChangeCaseWordsStartsBig(Editor->SelText);break;
      default:
        MessageBox(Synchronizer->MainForm->Handle,"Coœ jest nie tak z okienkiem dialogowym czmiany wielkoœci liter.","No nie... Ale zwa³a...",MB_OK|MB_ICONERROR);break;
      };
    };
}
//---------------------------------------------------------------------------
void TSpisEditor::ChangeSelectionCase(TObject *Sender,TMemo *Editor)
{
  if (CaseConvertDialog->ShowModal()==mrOk)
    {
    switch (CaseConvertDialog->ConvertionType->ItemIndex)
      {
      case 0:Editor->SelText=AnsiLowerCase(Editor->SelText);break;
      case 1:Editor->SelText=AnsiUpperCase(Editor->SelText);break;
      case 2:Editor->SelText=ChangeCaseWordsStartsBig(Editor->SelText);break;
      default:
        MessageBox(Synchronizer->MainForm->Handle,"Coœ jest nie tak z okienkiem dialogowym czmiany wielkoœci liter.","No nie... Ale zwa³a...",MB_OK|MB_ICONERROR);break;
      };
    };
}
//---------------------------------------------------------------------------
void TSpisEditor::DoFindInRichEdit(TFindDialog *Dialog,TRichEdit *Editor)
{
  long FoundAt;
  int StartPos, ToEnd;
  TSearchTypes st;
    // Jak mamy cos zaznaczone - szukamy za tym.
    // inaczej szukamy od poczatku
    if (Editor->SelLength > 0)
      StartPos = Editor->SelStart + Editor->SelLength;
     else
      StartPos = 0;
    // ToEnd is the length from StartPos to the end of the text in the rich edit control
    ToEnd = Editor->Text.Length() - StartPos;
    // Jeszcze opcje wyszukiwania
    st.Clear();
    if (Dialog->Options.Contains(frMatchCase))
        st<<stMatchCase;
    if (Dialog->Options.Contains(frWholeWord))
        st<<stWholeWord;
    // No i szukamy
    FoundAt = Editor->FindText(Dialog->FindText, StartPos, ToEnd, st);
    // Udalo sie lub nie...
    if (FoundAt > -1)
      {
      Editor->SetFocus();
      Editor->SelStart = FoundAt;
      Editor->SelLength = Dialog->FindText.Length();
      }
     else
      {
      MessageBox(Dialog->Handle,"Nie znaleziono wiecej odpowiedników.","Durny ty jestes!",MB_OK|MB_ICONINFORMATION);
      Editor->SelStart=0;
      };
}
//---------------------------------------------------------------------------
void TSpisEditor::DoFindInMemo(TFindDialog *Dialog,TMemo *Editor)
{
  long FoundAt;
  int StartPos, ToEnd;
  TSearchTypes st;
    // Jak mamy cos zaznaczone - szukamy za tym.
    // inaczej szukamy od poczatku
    if (Editor->SelLength > 0)
      StartPos = Editor->SelStart + Editor->SelLength;
     else
      StartPos = 0;
    // ToEnd is the length from StartPos to the end of the text in the rich edit control
//    ToEnd = Editor->Text.Length() - StartPos;
    // Jeszcze opcje wyszukiwania
    // No i szukamy
    if (Dialog->Options.Contains(frMatchCase))
      {
      FoundAt = FindSubstringPosFrom(StartPos,Dialog->FindText,Editor->Lines->Text)-1;
      }
     else
      {
      FoundAt = FindSubstringPosFrom(StartPos,Dialog->FindText.UpperCase(),Editor->Lines->Text.UpperCase())-1;
      }
    // Udalo sie lub nie...
    if (FoundAt > -1)
      {
      Editor->SetFocus();
      Editor->SelStart = FoundAt;
      Editor->SelLength = Dialog->FindText.Length();
      }
     else
      {
      MessageBox(Dialog->Handle,"Nie znaleziono wiecej odpowiedników.","Durny ty jestes!",MB_OK|MB_ICONINFORMATION);
      Editor->SelStart=0;
      };
}
//---------------------------------------------------------------------------
void TSpisEditor::DoReplaceInRichEdit(TReplaceDialog *Dialog,TRichEdit *Editor)
{
  if (Editor->SelLength == 0)
    DoFindInRichEdit(Dialog,Editor);
   else
    {
    Editor->SelText=Dialog->ReplaceText;
    DoFindInRichEdit(Dialog,Editor);
    };

    if (Dialog->Options.Contains(frReplaceAll))
        while ((Editor->SelLength)>0)
            DoReplaceInRichEdit(Dialog,Editor);
}
//---------------------------------------------------------------------------
void TSpisEditor::DoReplaceInMemo(TReplaceDialog *Dialog,TMemo *Editor)
{
  if (Editor->SelLength == 0)
    DoFindInMemo(Dialog,Editor);
   else
    {
    Editor->SelText=Dialog->ReplaceText;
    DoFindInMemo(Dialog,Editor);
    };

    if (Dialog->Options.Contains(frReplaceAll))
        while ((Editor->SelLength)>0)
            DoReplaceInMemo(Dialog,Editor);
}
//---------------------------------------------------------------------------

