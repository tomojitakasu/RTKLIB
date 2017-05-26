//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------







































USEFORM("vmapdlg.cpp", VecMapDialog);
USEFORM("..\appcmn\tcpoptdlg.cpp", TcpOptDialog);
USEFORM("..\appcmn\serioptdlg.cpp", SerialOptDialog);
USEFORM("..\appcmn\refdlg.cpp", RefDialog);
USEFORM("..\appcmn\timedlg.cpp", TimeDialog);
USEFORM("..\appcmn\vieweropt.cpp", ViewerOptDialog);
USEFORM("..\appcmn\viewer.cpp", TextViewer);
USEFORM("..\appcmn\tspandlg.cpp", SpanDialog);
USEFORM("..\appcmn\keydlg.cpp", KeyDialog);
USEFORM("..\appcmn\confdlg.cpp", ConfDialog);
USEFORM("..\appcmn\cmdoptdlg.cpp", CmdOptDialog);
USEFORM("..\appcmn\aboutdlg.cpp", AboutDialog);
USEFORM("..\appcmn\console.cpp", Console);
USEFORM("..\appcmn\ftpoptdlg.cpp", FtpOptDialog);
USEFORM("..\appcmn\fileoptdlg.cpp", FileOptDialog);
USEFORM("pntdlg.cpp", PntDialog);
USEFORM("plotopt.cpp", PlotOptDialog);
USEFORM("plotmain.cpp", Plot);
USEFORM("skydlg.cpp", SkyImgDialog);
USEFORM("satdlg.cpp", SatDialog);
USEFORM("gmview.cpp", GoogleMapView);
USEFORM("geview.cpp", GoogleEarthView);
USEFORM("conndlg.cpp", ConnectDialog);
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
		Application->CreateForm(__classid(TGoogleEarthView), &GoogleEarthView);
		Application->CreateForm(__classid(TFtpOptDialog), &FtpOptDialog);
		Application->CreateForm(__classid(TConsole), &Console);
		Application->CreateForm(__classid(TGoogleMapView), &GoogleMapView);
		Application->CreateForm(__classid(TMapAreaDialog), &MapAreaDialog);
		Application->CreateForm(__classid(TSkyImgDialog), &SkyImgDialog);
		Application->CreateForm(__classid(TVecMapDialog), &VecMapDialog);
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

