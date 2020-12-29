//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------


























#include <Vcl.Styles.hpp>
#include <Vcl.Themes.hpp>
USEFORM("..\appcmn\refdlg.cpp", RefDialog);
USEFORM("..\appcmn\mntpoptdlg.cpp", MntpOptDialog);
USEFORM("..\appcmn\maskoptdlg.cpp", MaskOptDialog);
USEFORM("..\appcmn\serioptdlg.cpp", SerialOptDialog);
USEFORM("..\appcmn\vieweropt.cpp", ViewerOptDialog);
USEFORM("..\appcmn\viewer.cpp", TextViewer);
USEFORM("..\appcmn\tcpoptdlg.cpp", TcpOptDialog);
USEFORM("..\appcmn\keydlg.cpp", KeyDialog);
USEFORM("..\appcmn\confdlg.cpp", ConfDialog);
USEFORM("..\appcmn\cmdoptdlg.cpp", CmdOptDialog);
USEFORM("..\appcmn\aboutdlg.cpp", AboutDialog);
USEFORM("..\appcmn\fileoptdlg.cpp", FileOptDialog);
USEFORM("..\appcmn\ftpoptdlg.cpp", FtpOptDialog);
USEFORM("..\appcmn\freqdlg.cpp", FreqDialog);
USEFORM("rcvoptdlg.cpp", RcvOptDialog);
USEFORM("markdlg.cpp", MarkDialog);
USEFORM("logstrdlg.cpp", LogStrDialog);
USEFORM("instrdlg.cpp", InputStrDialog);
USEFORM("mondlg.cpp", MonitorDialog);
USEFORM("outstrdlg.cpp", OutputStrDialog);
USEFORM("naviopt.cpp", OptDialog);
USEFORM("navimain.cpp", MainForm);
//---------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "RTKNAVI";
		Application->CreateForm(__classid(TMainForm), &MainForm);
		Application->CreateForm(__classid(TSerialOptDialog), &SerialOptDialog);
		Application->CreateForm(__classid(TFtpOptDialog), &FtpOptDialog);
		Application->CreateForm(__classid(TFileOptDialog), &FileOptDialog);
		Application->CreateForm(__classid(TAboutDialog), &AboutDialog);
		Application->CreateForm(__classid(TViewerOptDialog), &ViewerOptDialog);
		Application->CreateForm(__classid(TTextViewer), &TextViewer);
		Application->CreateForm(__classid(TTcpOptDialog), &TcpOptDialog);
		Application->CreateForm(__classid(TRefDialog), &RefDialog);
		Application->CreateForm(__classid(TMaskOptDialog), &MaskOptDialog);
		Application->CreateForm(__classid(TKeyDialog), &KeyDialog);
		Application->CreateForm(__classid(TConfDialog), &ConfDialog);
		Application->CreateForm(__classid(TCmdOptDialog), &CmdOptDialog);
		Application->CreateForm(__classid(TMntpOptDialog), &MntpOptDialog);
		Application->CreateForm(__classid(TOptDialog), &OptDialog);
		Application->CreateForm(__classid(TOutputStrDialog), &OutputStrDialog);
		Application->CreateForm(__classid(TLogStrDialog), &LogStrDialog);
		Application->CreateForm(__classid(TInputStrDialog), &InputStrDialog);
		Application->CreateForm(__classid(TRcvOptDialog), &RcvOptDialog);
		Application->CreateForm(__classid(TMarkDialog), &MarkDialog);
		Application->CreateForm(__classid(TFreqDialog), &FreqDialog);
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
