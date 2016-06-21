//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------


























USEFORM("logstrdlg.cpp", LogStrDialog);
USEFORM("markdlg.cpp", MarkDialog);
USEFORM("mondlg.cpp", MonitorDialog);
USEFORM("..\appcmn\vieweropt.cpp", ViewerOptDialog);
USEFORM("instrdlg.cpp", InputStrDialog);
USEFORM("rcvoptdlg.cpp", RcvOptDialog);
USEFORM("navimain.cpp", MainForm);
USEFORM("naviopt.cpp", OptDialog);
USEFORM("outstrdlg.cpp", OutputStrDialog);
USEFORM("..\appcmn\confdlg.cpp", ConfDialog);
USEFORM("..\appcmn\ftpoptdlg.cpp", FtpOptDialog);
USEFORM("..\appcmn\aboutdlg.cpp", AboutDialog);
USEFORM("..\appcmn\cmdoptdlg.cpp", CmdOptDialog);
USEFORM("..\appcmn\serioptdlg.cpp", SerialOptDialog);
USEFORM("..\appcmn\tcpoptdlg.cpp", TcpOptDialog);
USEFORM("..\appcmn\viewer.cpp", TextViewer);
USEFORM("..\appcmn\keydlg.cpp", KeyDialog);
USEFORM("..\appcmn\maskoptdlg.cpp", MaskOptDialog);
USEFORM("..\appcmn\refdlg.cpp", RefDialog);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "RTKNAVI";
		Application->CreateForm(__classid(TMainForm), &MainForm);
		Application->CreateForm(__classid(TOutputStrDialog), &OutputStrDialog);
		Application->CreateForm(__classid(TSerialOptDialog), &SerialOptDialog);
		Application->CreateForm(__classid(TCmdOptDialog), &CmdOptDialog);
		Application->CreateForm(__classid(TTcpOptDialog), &TcpOptDialog);
		Application->CreateForm(__classid(TOptDialog), &OptDialog);
		Application->CreateForm(__classid(TRefDialog), &RefDialog);
		Application->CreateForm(__classid(TTextViewer), &TextViewer);
		Application->CreateForm(__classid(TViewerOptDialog), &ViewerOptDialog);
		Application->CreateForm(__classid(TConfDialog), &ConfDialog);
		Application->CreateForm(__classid(TAboutDialog), &AboutDialog);
		Application->CreateForm(__classid(TInputStrDialog), &InputStrDialog);
		Application->CreateForm(__classid(TLogStrDialog), &LogStrDialog);
		Application->CreateForm(__classid(TKeyDialog), &KeyDialog);
		Application->CreateForm(__classid(TFtpOptDialog), &FtpOptDialog);
		Application->CreateForm(__classid(TRcvOptDialog), &RcvOptDialog);
		Application->CreateForm(__classid(TMaskOptDialog), &MaskOptDialog);
		Application->CreateForm(__classid(TMarkDialog), &MarkDialog);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
