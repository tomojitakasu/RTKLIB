//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------








USEFORM("..\appcmn\keydlg.cpp", KeyDialog);
USEFORM("..\appcmn\ftpoptdlg.cpp", FtpOptDialog);
USEFORM("..\appcmn\serioptdlg.cpp", SerialOptDialog);
USEFORM("..\appcmn\refdlg.cpp", RefDialog);
USEFORM("..\appcmn\fileoptdlg.cpp", FileOptDialog);
USEFORM("..\appcmn\cmdoptdlg.cpp", CmdOptDialog);
USEFORM("..\appcmn\aboutdlg.cpp", AboutDialog);
USEFORM("..\appcmn\console.cpp", Console);
USEFORM("..\appcmn\confdlg.cpp", ConfDialog);
USEFORM("..\appcmn\tcpoptdlg.cpp", TcpOptDialog);
USEFORM("svroptdlg.cpp", SvrOptDialog);
USEFORM("convdlg.cpp", ConvDialog);
USEFORM("svrmain.cpp", MainForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
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
		Application->CreateForm(__classid(TConfDialog), &ConfDialog);
		Application->CreateForm(__classid(TAboutDialog), &AboutDialog);
		Application->CreateForm(__classid(TRefDialog), &RefDialog);
		Application->CreateForm(__classid(TTcpOptDialog), &TcpOptDialog);
		Application->CreateForm(__classid(TKeyDialog), &KeyDialog);
		Application->CreateForm(__classid(TConsole), &Console);
		Application->CreateForm(__classid(TConvDialog), &ConvDialog);
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
