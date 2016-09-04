//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
//---------------------------------------------------------------------------

#include <Vcl.Styles.hpp>
#include <Vcl.Themes.hpp>















USEFORM("launchmain.cpp", MainForm);
USEFORM("launchoptdlg.cpp", LaunchOptDialog);
//---------------------------------------------------------------------------
WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	try
	{
		Application->Initialize();
		Application->MainFormOnTaskBar = true;
		Application->Title = "RTKLAUNCH";
		Application->CreateForm(__classid(TMainForm), &MainForm);
		Application->CreateForm(__classid(TLaunchOptDialog), &LaunchOptDialog);
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
