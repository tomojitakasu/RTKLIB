//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------


USEFORM("..\appcmn\refdlg.cpp", RefDialog);
USEFORM("..\appcmn\keydlg.cpp", KeyDialog);
USEFORM("..\appcmn\tcpoptdlg.cpp", TcpOptDialog);
USEFORM("..\appcmn\serioptdlg.cpp", SerialOptDialog);
USEFORM("..\appcmn\ftpoptdlg.cpp", FtpOptDialog);
USEFORM("..\appcmn\cmdoptdlg.cpp", CmdOptDialog);
USEFORM("..\appcmn\aboutdlg.cpp", AboutDialog);
USEFORM("..\appcmn\fileoptdlg.cpp", FileOptDialog);
USEFORM("..\appcmn\confdlg.cpp", ConfDialog);
USEFORM("convdlg.cpp", ConvDialog);
USEFORM("svroptdlg.cpp", SvrOptDialog);
USEFORM("mondlg.cpp", StrMonDialog);
USEFORM("svrmain.cpp", MainForm);
USEFORM("..\appcmn\viewer.cpp", TextViewer);
USEFORM("..\appcmn\vieweropt.cpp", ViewerOptDialog);
//---------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "STRSVR";
		Application->CreateForm(__classid(TMainForm), &MainForm);
		Application->CreateForm(__classid(TSvrOptDialog), &SvrOptDialog);
		Application->CreateForm(__classid(TCmdOptDialog), &CmdOptDialog);
		Application->CreateForm(__classid(TFileOptDialog), &FileOptDialog);
		Application->CreateForm(__classid(TSerialOptDialog), &SerialOptDialog);
		Application->CreateForm(__classid(TFtpOptDialog), &FtpOptDialog);
		Application->CreateForm(__classid(TAboutDialog), &AboutDialog);
		Application->CreateForm(__classid(TRefDialog), &RefDialog);
		Application->CreateForm(__classid(TTcpOptDialog), &TcpOptDialog);
		Application->CreateForm(__classid(TKeyDialog), &KeyDialog);
		Application->CreateForm(__classid(TConvDialog), &ConvDialog);
		Application->CreateForm(__classid(TStrMonDialog), &StrMonDialog);
		Application->CreateForm(__classid(TTextViewer), &TextViewer);
		Application->CreateForm(__classid(TViewerOptDialog), &ViewerOptDialog);
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
