//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------











USEFORM("staoptdlg.cpp", StaListDialog);
USEFORM("..\appcmn\aboutdlg.cpp", AboutDialog);
USEFORM("..\appcmn\gmview.cpp", GoogleMapView);
USEFORM("browsmain.cpp", MainForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "NTRIP Source Table Browser";
		Application->CreateForm(__classid(TMainForm), &MainForm);
		Application->CreateForm(__classid(TAboutDialog), &AboutDialog);
		Application->CreateForm(__classid(TGoogleMapView), &GoogleMapView);
		Application->CreateForm(__classid(TStaListDialog), &StaListDialog);
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
