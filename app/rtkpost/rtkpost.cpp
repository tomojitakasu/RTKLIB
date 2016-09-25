//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------

USEFORM("kmzconv.cpp", ConvDialog);
USEFORM("postmain.cpp", MainForm);
USEFORM("extopt.cpp", ExtOptDialog);
USEFORM("postopt.cpp", OptDialog);
USEFORM("..\appcmn\keydlg.cpp", KeyDialog);
USEFORM("..\appcmn\maskoptdlg.cpp", MaskOptDialog);
USEFORM("..\appcmn\aboutdlg.cpp", AboutDialog);
USEFORM("..\appcmn\confdlg.cpp", ConfDialog);
USEFORM("..\appcmn\viewer.cpp", TextViewer);
USEFORM("..\appcmn\vieweropt.cpp", ViewerOptDialog);
USEFORM("..\appcmn\refdlg.cpp", RefDialog);
USEFORM("..\appcmn\timedlg.cpp", TimeDialog);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "RTKPOST";
		Application->CreateForm(__classid(TMainForm), &MainForm);
		Application->CreateForm(__classid(TOptDialog), &OptDialog);
		Application->CreateForm(__classid(TConvDialog), &ConvDialog);
		Application->CreateForm(__classid(TOptDialog), &OptDialog);
		Application->CreateForm(__classid(TTextViewer), &TextViewer);
		Application->CreateForm(__classid(TViewerOptDialog), &ViewerOptDialog);
		Application->CreateForm(__classid(TRefDialog), &RefDialog);
		Application->CreateForm(__classid(TTimeDialog), &TimeDialog);
		Application->CreateForm(__classid(TConfDialog), &ConfDialog);
		Application->CreateForm(__classid(TAboutDialog), &AboutDialog);
		Application->CreateForm(__classid(TKeyDialog), &KeyDialog);
		Application->CreateForm(__classid(TExtOptDialog), &ExtOptDialog);
		Application->CreateForm(__classid(TMaskOptDialog), &MaskOptDialog);
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
