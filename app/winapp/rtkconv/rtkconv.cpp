//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------

USEFORM("..\appcmn\timedlg.cpp", TimeDialog);
USEFORM("..\appcmn\viewer.cpp", TextViewer);
USEFORM("..\appcmn\vieweropt.cpp", ViewerOptDialog);
USEFORM("codeopt.cpp", CodeOptDialog);
USEFORM("..\appcmn\keydlg.cpp", KeyDialog);
USEFORM("..\appcmn\aboutdlg.cpp", AboutDialog);
USEFORM("..\appcmn\confdlg.cpp", ConfDialog);
USEFORM("..\appcmn\freqdlg.cpp", FreqDialog);
USEFORM("..\appcmn\glofcndlg.cpp", GloFcnDialog);
USEFORM("convmain.cpp", MainWindow);
USEFORM("convopt.cpp", ConvOptDialog);
USEFORM("startdlg.cpp", StartDialog);
//---------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "RTKCONV";
		Application->CreateForm(__classid(TMainWindow), &MainWindow);
		Application->CreateForm(__classid(TConvOptDialog), &ConvOptDialog);
		Application->CreateForm(__classid(TTextViewer), &TextViewer);
		Application->CreateForm(__classid(TViewerOptDialog), &ViewerOptDialog);
		Application->CreateForm(__classid(TAboutDialog), &AboutDialog);
		Application->CreateForm(__classid(TTimeDialog), &TimeDialog);
		Application->CreateForm(__classid(TConfDialog), &ConfDialog);
		Application->CreateForm(__classid(TStartDialog), &StartDialog);
		Application->CreateForm(__classid(TKeyDialog), &KeyDialog);
		Application->CreateForm(__classid(TCodeOptDialog), &CodeOptDialog);
		Application->CreateForm(__classid(TFreqDialog), &FreqDialog);
		Application->CreateForm(__classid(TGloFcnDialog), &GloFcnDialog);
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
