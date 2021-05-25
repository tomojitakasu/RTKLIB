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

// Conversion Thread Class ------------------------------------------------------------------

class ConversionThread : public QThread
{
    Q_OBJECT
public:
    char ifile[1024], *ofile[9];
    rnxopt_t rnxopt;
    int format;

    explicit ConversionThread(QObject *parent) : QThread(parent) {
        for (int i = 0; i < 9; i++)
        {
            ofile[i] = new char[1024];
            ofile[i][0] = '\0';
        };
        memset(&rnxopt, 0, sizeof(rnxopt_t));
        format = 0;
        ifile[0] = '\0';
    }

    ~ConversionThread() {
        for (int i = 0; i < 9; i++) delete[] ofile[i];
    }

protected:
    void run() {
        // convert to rinex
        convrnx(format, &rnxopt, ifile, ofile);
    }
};

//---------------------------------------------------------------------------
class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

protected:
    void showEvent           (QShowEvent*);
    void closeEvent          (QCloseEvent*);
    void dragEnterEvent      (QDragEnterEvent *);
    void dropEvent           (QDropEvent *);

public slots:
    void BtnPlotClick();
    void BtnConvertClick();
    void BtnOptionsClick();
    void BtnExitClick();
    void BtnAboutClick();
    void BtnTime1Click();
    void BtnTime2Click();
    void BtnInFileClick();
    void BtnOutFile1Click();
    void BtnOutFile2Click();
    void BtnOutFile3Click();
    void BtnOutFile4Click();
    void BtnOutFile5Click();
    void BtnOutFile6Click();
    void BtnOutFile7Click();
    void BtnOutFile8Click();
    void BtnOutFile9Click();
    void BtnOutFileView1Click();
    void BtnOutFileView2Click();
    void BtnOutFileView3Click();
    void BtnOutFileView4Click();
    void BtnOutFileView5Click();
    void BtnOutFileView6Click();
    void BtnOutFileView7Click();
    void BtnOutFileView8Click();
    void BtnOutFileView9Click();
    void BtnAbortClick();
	
    void TimeStartFClick();
    void TimeEndFClick();
    void TimeIntFClick();
    void OutDirEnaClick();
	
    void InFileChange();
    void FormatChange();
    void OutDirChange();
    void BtnOutDirClick();
    void BtnKeyClick();
    void BtnPostClick();
    void BtnInFileViewClick();
    void ConversionFinished();
    void UpdateEnable();

private:
    QString IniFile, CmdPostExe;
    ConversionThread *conversionThread;

    void ReadList(QComboBox* combo, QSettings *ini, const QString &key);
    void WriteList(QSettings *ini, const QString &key, const QComboBox *combo);
    void AddHist(QComboBox *combo);
	
    int  AutoFormat(const QString &File);
    void ConvertFile(void);
    void SetOutFiles(const QString &infile);
    void GetTime(gtime_t *ts, gtime_t *te, double *tint, double *tunit);
    int  ExecCmd(const QString &cmd, QStringList &opt);
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
    QString RunBy, Marker, MarkerNo, MarkerType, Name[2], Rec[3], Ant[3];
    QString RnxCode, Comment[2], RcvOption, ExSats;
    QString CodeMask[7];
    double AppPos[3], AntDel[3], TimeTol;
    int RnxVer, RnxFile, NavSys, ObsType, FreqType, TraceLevel;
    int AutoPos, PhaseShift, HalfCyc, OutIono, OutTime, OutLeaps, SepNav;
    int EnaGloFcn,GloFcn[27];
	
    explicit MainWindow(QWidget *parent=0);
};
//---------------------------------------------------------------------------
#endif
