//---------------------------------------------------------------------------
#ifndef convmainH
#define convmainH
//---------------------------------------------------------------------------
#include <QMainWindow>

#include "ui_convmain.h"

#include "rtklib.h"

class QShowEvent;
class QCloseEvent;
class QSettings;
class QComboBox;
class ConvOptDialog;
class TimeDialog;
class KeyDialog;
class AboutDialog;
class StartDialog;

//---------------------------------------------------------------------------
class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
protected:
    void showEvent           (QShowEvent*);
    void closeEvent          (QCloseEvent*);

public slots:
    void FormCreate          ();

    void BtnPlotClick        ();
    void BtnConvertClick     ();
    void BtnOptionsClick     ();
    void BtnExitClick        ();
    void BtnAboutClick       ();
    void BtnTime1Click       ();
    void BtnTime2Click       ();
    void BtnInFileClick      ();
    void BtnOutFile1Click    ();
    void BtnOutFile2Click    ();
    void BtnOutFile3Click    ();
    void BtnOutFile4Click    ();
    void BtnOutFileView1Click();
    void BtnOutFileView2Click();
    void BtnOutFileView3Click();
    void BtnOutFileView4Click();
	
    void TimeStartFClick     ();
    void TimeEndFClick       ();
    void TimeIntFClick       ();
    void OutDirEnaClick     ();
	
    void InFileChange();
    void BtnOutFileView5Click();
    void BtnOutFile5Click();
    void FormatChange();
    void BtnOutFileView6Click();
    void BtnOutFile6Click();
    void OutDirChange();
    void BtnOutDirClick();
    void BtnKeyClick();
    void BtnPostClick();
    void BtnOutFile7Click();
    void BtnOutFileView7Click();
    void BtnInFileViewClick();
	
private:
    QString IniFile,CmdPostExe;
	
//    void DropFiles(TWMDropFiles msg); // for files drop
	
    void ReadList(QComboBox* combo, QSettings *ini, const QString &key);
    void WriteList(QSettings *ini, const QString &key, const QComboBox *combo);
    void AddHist(QComboBox *combo);
	
    int  AutoFormat(const QString &File);
    void ConvertFile(void);
    void SetOutFiles(const QString &infile);
    void UpdateEnable(void);
    void GetTime(gtime_t *ts, gtime_t *te, double *tint, double *tunit);
    int  ExecCmd(const QString &cmd);
    QString RepPath(const QString &File);
    void LoadOpt(void);
    void SaveOpt(void);
		
    ConvOptDialog *convOptDialog;
    TimeDialog *timeDialog;
    KeyDialog* keyDialog;
    AboutDialog* aboutDialog;
    StartDialog* startDialog;
public:
	gtime_t RnxTime;
    QString RunBy,Marker,MarkerNo,MarkerType,Name[2],Rec[3],Ant[3];
    QString RnxCode,Comment[2],RcvOption,ExSats;
    QString CodeMask[6];
	double AppPos[3],AntDel[3];
	int RnxVer,RnxFile,NavSys,ObsType,FreqType,TraceLevel,EventEna;
	int AutoPos,ScanObs,OutIono,OutTime,OutLeaps;
	
    MainWindow(QWidget *parent=0);
};
//---------------------------------------------------------------------------
#endif
