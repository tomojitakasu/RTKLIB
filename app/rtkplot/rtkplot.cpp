//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------



























USEFORM("skydlg.cpp", SkyImgDialog);
USEFORM("satdlg.cpp", SatDialog);
USEFORM("..\appcmn\serioptdlg.cpp", SerialOptDialog);
USEFORM("..\appcmn\refdlg.cpp", RefDialog);
USEFORM("..\appcmn\keydlg.cpp", KeyDialog);
USEFORM("..\appcmn\tcpoptdlg.cpp", TcpOptDialog);
USEFORM("..\appcmn\viewer.cpp", TextViewer);
USEFORM("..\appcmn\tspandlg.cpp", SpanDialog);
USEFORM("..\appcmn\timedlg.cpp", TimeDialog);
USEFORM("..\appcmn\confdlg.cpp", ConfDialog);
USEFORM("..\appcmn\cmdoptdlg.cpp", CmdOptDialog);
USEFORM("..\appcmn\aboutdlg.cpp", AboutDialog);
USEFORM("..\appcmn\console.cpp", Console);
USEFORM("..\appcmn\ftpoptdlg.cpp", FtpOptDialog);
USEFORM("..\appcmn\fileoptdlg.cpp", FileOptDialog);
USEFORM("..\appcmn\vieweropt.cpp", ViewerOptDialog);
USEFORM("plotmain.cpp", Plot);
USEFORM("plotopt.cpp", PlotOptDialog);
USEFORM("pntdlg.cpp", PntDialog);
USEFORM("geview.cpp", GoogleEarthView);
USEFORM("fileseldlg.cpp", FileSelDialog);
USEFORM("conndlg.cpp", ConnectDialog);
USEFORM("gmview.cpp", GoogleMapView);
USEFORM("mapdlg.cpp", MapAreaDialog);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "RTKPLOT";
		Application->CreateForm(__classid(TPlot), &Plot);
		Application->CreateForm(__classid(TPlotOptDialog), &PlotOptDialog);
		Application->CreateForm(__classid(TSatDialog), &SatDialog);
		Application->CreateForm(__classid(TRefDialog), &RefDialog);
		Application->CreateForm(__classid(TAboutDialog), &AboutDialog);
		Application->CreateForm(__classid(TSpanDialog), &SpanDialog);
		Application->CreateForm(__classid(TTimeDialog), &TimeDialog);
		Application->CreateForm(__classid(TConnectDialog), &ConnectDialog);
		Application->CreateForm(__classid(TSerialOptDialog), &SerialOptDialog);
		Application->CreateForm(__classid(TTcpOptDialog), &TcpOptDialog);
		Application->CreateForm(__classid(TCmdOptDialog), &CmdOptDialog);
		Application->CreateForm(__classid(TFileOptDialog), &FileOptDialog);
		Application->CreateForm(__classid(TKeyDialog), &KeyDialog);
		Application->CreateForm(__classid(TTextViewer), &TextViewer);
		Application->CreateForm(__classid(TViewerOptDialog), &ViewerOptDialog);
		Application->CreateForm(__classid(TPntDialog), &PntDialog);
		Application->CreateForm(__classid(TConfDialog), &ConfDialog);
		Application->CreateForm(__classid(TFileSelDialog), &FileSelDialog);
		Application->CreateForm(__classid(TGoogleEarthView), &GoogleEarthView);
		Application->CreateForm(__classid(TFtpOptDialog), &FtpOptDialog);
		Application->CreateForm(__classid(TConsole), &Console);
		Application->CreateForm(__classid(TGoogleMapView), &GoogleMapView);
		Application->CreateForm(__classid(TMapAreaDialog), &MapAreaDialog);
		Application->CreateForm(__classid(TSkyImgDialog), &SkyImgDialog);
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

