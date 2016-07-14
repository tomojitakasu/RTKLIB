//---------------------------------------------------------------------------
#ifndef convmainH
#define convmainH
//---------------------------------------------------------------------------
#include <QMainWindow>
#include <QThread>

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
class TextViewer;

//Helper Class ------------------------------------------------------------------

class ConversionThread : public QThread
{
    Q_OBJECT
public:
    char ifile[1024],*ofile[7];
    rnxopt_t rnxopt;
    int format;

    explicit ConversionThread(QObject *parent):QThread(parent){
        for (int i=0;i<7;i++)
        {
            ofile[i]=new char[1024];
            ofile[i][0]='\0';
        };
        memset(&rnxopt,0,sizeof(rnxopt_t));
        format=0;
        ifile[0]='\0';
    }

    ~ConversionThread() {
        for (int i=0;i<7;i++) delete[] ofile[i];
    }

protected:
    void run() {
        // convert to rinex
        convrnx(format,&rnxopt,ifile,ofile);
    }
};

//---------------------------------------------------------------------------
class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
protected:
    void showEvent           (QShowEvent*);
    void closeEvent          (QCloseEvent*);
    void dragEnterEvent      (QDragEnterEvent *event);
    void dropEvent           (QDropEvent *event);
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
    void BtnAbortClick       ();
	
    void TimeStartFClick     ();
    void TimeEndFClick       ();
    void TimeIntFClick       ();
    void OutDirEnaClick      ();
	
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
    void ConversionFinished();
    void UpdateEnable();

private:
    QString IniFile,CmdPostExe;
    ConversionThread *conversionThread;
//    void DropFiles(TWMDropFiles msg); // for files drop
	
    void ReadList(QComboBox* combo, QSettings *ini, const QString &key);
    void WriteList(QSettings *ini, const QString &key, const QComboBox *combo);
    void AddHist(QComboBox *combo);
	
    int  AutoFormat(const QString &File);
    void ConvertFile(void);
    void SetOutFiles(const QString &infile);
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
    TextViewer *viewer;
public:
	gtime_t RnxTime;
    QString RunBy,Marker,MarkerNo,MarkerType,Name[2],Rec[3],Ant[3];
    QString RnxCode,Comment[2],RcvOption,ExSats;
    QString CodeMask[6];
	double AppPos[3],AntDel[3];
	int RnxVer,RnxFile,NavSys,ObsType,FreqType,TraceLevel,EventEna;
	int AutoPos,ScanObs,OutIono,OutTime,OutLeaps;
	
    explicit MainWindow(QWidget *parent=0);
};
//---------------------------------------------------------------------------
#endif
