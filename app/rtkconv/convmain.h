//---------------------------------------------------------------------------
#ifndef convmainH
#define convmainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <Graphics.hpp>
#include <ComCtrls.hpp>
#include <FileCtrl.hpp>
#ifdef TCPP
#include <vcl\inifiles.hpp>
#else
#include <inifiles.hpp>
#endif

#include "rtklib.h"
//---------------------------------------------------------------------------
class TMainWindow : public TForm
{
__published:
	TPanel *Panel1;
	TPanel *Panel3;
	TPanel *Panel2;
	TButton *BtnInFile;
	TButton *BtnOutFile1;
	TButton *BtnOutFile2;
	TButton *BtnOutFile4;
	TButton *BtnOutFile3;
	TSpeedButton *BtnAbout;
	TSpeedButton *BtnTime1;
	TSpeedButton *BtnTime2;
	TSpeedButton *BtnOutFileView3;
	TSpeedButton *BtnOutFileView1;
	TSpeedButton *BtnOutFileView2;
	TSpeedButton *BtnOutFileView4;
	
	TCheckBox *TimeStartF;
	TCheckBox *TimeEndF;
	TCheckBox *TimeIntF;
	TCheckBox *OutFileEna1;
	TCheckBox *OutFileEna2;
	TCheckBox *OutFileEna3;
	TCheckBox *OutFileEna4;
	
	TEdit *TimeY1;
	TEdit *TimeH1;
	TEdit *TimeY2;
	TEdit *TimeH2;
	TUpDown *TimeY1UD;
	TUpDown *TimeH1UD;
	TUpDown *TimeY2UD;
	TUpDown *TimeH2UD;
	
	TLabel *LabelInFile;
	TLabel *LabelOutFile;
	TLabel *LabelTimeInt;
	TLabel *LabelFormat;
	TLabel *Message;
	
	TComboBox *TimeInt;
	TComboBox *Format;
	TEdit *OutFile1;
	TEdit *OutFile2;
	TEdit *OutFile3;
	TEdit *OutFile4;
	
	TOpenDialog *OpenDialog;
	TOpenDialog *OpenDialog2;
	TCheckBox *OutFileEna5;
	TEdit *OutFile5;
	TSpeedButton *BtnOutFileView5;
	TButton *BtnOutFile5;
	TCheckBox *OutFileEna6;
	TEdit *OutFile6;
	TSpeedButton *BtnOutFileView6;
	TButton *BtnOutFile6;
	TComboBox *InFile;
	TCheckBox *OutDirEna;
	TEdit *OutDir;
	TLabel *LabelOutDir;
	TButton *BtnOutDir;
	TSpeedButton *BtnKey;
	TCheckBox *TimeUnitF;
	TLabel *LabelTimeUnit;
	TEdit *TimeUnit;
	TCheckBox *OutFileEna7;
	TEdit *OutFile7;
	TSpeedButton *BtnOutFileView7;
	TButton *BtnOutFile7;
	TSpeedButton *BtnInFileView;
	TPanel *Panel4;
	TBitBtn *BtnAbort;
	TBitBtn *BtnConvert;
	TBitBtn *BtnOptions;
	TBitBtn *BtnPlot;
	TBitBtn *BtnPost;
	TBitBtn *BtnExit;
	TCheckBox *OutFileEna8;
	TEdit *OutFile8;
	TSpeedButton *BtnOutFileView8;
	TButton *BtnOutFile8;
	TCheckBox *OutFileEna9;
	TEdit *OutFile9;
	TSpeedButton *BtnOutFileView9;
	TButton *BtnOutFile9;
	
	void __fastcall FormCreate          (TObject *Sender);
	void __fastcall FormShow            (TObject *Sender);
	void __fastcall FormClose           (TObject *Sender, TCloseAction &Action);
	
	void __fastcall BtnPlotClick        (TObject *Sender);
	void __fastcall BtnConvertClick     (TObject *Sender);
	void __fastcall BtnOptionsClick     (TObject *Sender);
	void __fastcall BtnExitClick        (TObject *Sender);
	void __fastcall BtnAboutClick       (TObject *Sender);
	void __fastcall BtnTime1Click       (TObject *Sender);
	void __fastcall BtnTime2Click       (TObject *Sender);
	void __fastcall BtnInFileClick      (TObject *Sender);
	void __fastcall BtnOutFile1Click    (TObject *Sender);
	void __fastcall BtnOutFile2Click    (TObject *Sender);
	void __fastcall BtnOutFile3Click    (TObject *Sender);
	void __fastcall BtnOutFile4Click    (TObject *Sender);
	void __fastcall BtnOutFileView1Click(TObject *Sender);
	void __fastcall BtnOutFileView2Click(TObject *Sender);
	void __fastcall BtnOutFileView3Click(TObject *Sender);
	void __fastcall BtnOutFileView4Click(TObject *Sender);
	
	void __fastcall TimeStartFClick     (TObject *Sender);
	void __fastcall TimeEndFClick       (TObject *Sender);
	void __fastcall TimeIntFClick       (TObject *Sender);
	void __fastcall OutDirEnaClick      (TObject *Sender);
	void __fastcall InFileChange(TObject *Sender);
	void __fastcall BtnOutFileView5Click(TObject *Sender);
	void __fastcall BtnOutFile5Click(TObject *Sender);
	void __fastcall FormatChange(TObject *Sender);
	void __fastcall BtnOutFileView6Click(TObject *Sender);
	void __fastcall BtnOutFile6Click(TObject *Sender);
	void __fastcall OutDirChange(TObject *Sender);
	void __fastcall BtnOutDirClick(TObject *Sender);
	void __fastcall BtnKeyClick(TObject *Sender);
	void __fastcall BtnPostClick(TObject *Sender);
	void __fastcall BtnOutFile7Click(TObject *Sender);
	void __fastcall BtnOutFileView7Click(TObject *Sender);
	void __fastcall BtnInFileViewClick(TObject *Sender);
	void __fastcall BtnAbortClick(TObject *Sender);
	void __fastcall Panel4Resize(TObject *Sender);
	void __fastcall Panel2Resize(TObject *Sender);
	void __fastcall TimeY1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall TimeH1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall TimeY2KeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall TimeH2KeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall BtnOutFile8Click(TObject *Sender);
	void __fastcall BtnOutFile9Click(TObject *Sender);
	void __fastcall BtnOutFileView8Click(TObject *Sender);
	void __fastcall BtnOutFileView9Click(TObject *Sender);
	void __fastcall TimeY1UDChangingEx(TObject *Sender, bool &AllowChange, int NewValue,
          TUpDownDirection Direction);
	void __fastcall TimeH1UDChangingEx(TObject *Sender, bool &AllowChange, int NewValue,
          TUpDownDirection Direction);
	void __fastcall TimeY2UDChangingEx(TObject *Sender, bool &AllowChange, int NewValue,
          TUpDownDirection Direction);
	void __fastcall TimeH2UDChangingEx(TObject *Sender, bool &AllowChange, int NewValue,
          TUpDownDirection Direction);
	
private:
	AnsiString IniFile,CmdPostExe;
	
	void __fastcall DropFiles(TWMDropFiles msg); // for files drop
	
	TStringList * __fastcall ReadList(TIniFile *ini, AnsiString cat,
		AnsiString key);
	void __fastcall WriteList(TIniFile *ini, AnsiString cat,
		AnsiString key, TStrings *list);
	void __fastcall AddHist(TComboBox *combo);
	
	int  __fastcall AutoFormat(AnsiString File);
	void __fastcall ConvertFile(void);
	void __fastcall SetOutFiles(AnsiString infile);
	void __fastcall UpdateEnable(void);
	void __fastcall GetTime(gtime_t *ts, gtime_t *te, double *tint, double *tunit);
	int  __fastcall ExecCmd(AnsiString cmd);
	AnsiString __fastcall RepPath(AnsiString File);
	void __fastcall LoadOpt(void);
	void __fastcall SaveOpt(void);
	
	BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(WM_DROPFILES,TWMDropFiles,DropFiles);
	END_MESSAGE_MAP(TForm);
	
public:
	gtime_t RnxTime;
	AnsiString RunBy,Marker,MarkerNo,MarkerType,Name[2],Rec[3],Ant[3];
	AnsiString RnxCode,Comment[2],RcvOption,ExSats;
	AnsiString CodeMask[7];
	double AppPos[3],AntDel[3],TimeTol;
	int RnxVer,RnxFile,NavSys,ObsType,FreqType,TraceLevel,EventEna;
	int AutoPos,ScanObs,HalfCyc,OutIono,OutTime,OutLeaps,SepNav;
	
	__fastcall TMainWindow(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainWindow *MainWindow;
//---------------------------------------------------------------------------
#endif
