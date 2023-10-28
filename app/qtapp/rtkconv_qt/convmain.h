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
    void btnPlotClicked();
    void btnConvertClicked();
    void btnOptionsClicked();
    void btnExitClicked();
    void btnAboutClicked();
    void btnTimeStartClicked();
    void btnTimeStopClicked();
    void btnInputFileClicked();
    void btnOutputFile1Clicked();
    void btnOutputFile2Clicked();
    void btnOutputFile3Clicked();
    void btnOutputFile4Clicked();
    void btnOutputFile5Clicked();
    void btnOutputFile6Clicked();
    void btnOutputFile7Clicked();
    void btnOutputFile8Clicked();
    void btnOutputFile9Clicked();
    void btnOutputFileView1Clicked();
    void btnOutputFileView2Clicked();
    void btnOutputFileView3Clicked();
    void btnOutputFileView4Clicked();
    void btnOutputFileView5Clicked();
    void btnOutputFileView6Clicked();
    void btnOutputFileView7Clicked();
    void btnOutputFileView8Clicked();
    void btnOutputFileView9Clicked();
    void btnAbortClicked();
	
    void timeStartClicked();
    void timeEndClicked();
    void TimeIntervalClicked();
    void outputDirrectoryEnableClicked();
	
    void inputFileChanged();
    void formatChanged();
    void outputDirectoryChanged();
    void btnOutputDirrectoryClicked();
    void btnKeyClicked();
    void btnPostClicked();
    void btnInputFileViewClicked();
    void conversionFinished();
    void updateEnable();

private:
    QString iniFile, commandPostExe;
    ConversionThread *conversionThread;

    void readList(QComboBox* combo, QSettings *ini, const QString &key);
    void writeList(QSettings *ini, const QString &key, const QComboBox *combo);
    void addHistory(QComboBox *combo);
	
    int autoFormat(const QString &file);
    void convertFile(void);
    void setOutputFiles(const QString &infile);
    void getTime(gtime_t *ts, gtime_t *te, double *tint, double *tunit);
    int  execCommand(const QString &cmd, QStringList &opt);
    QString repPath(const QString &File);
    void loadOptions(void);
    void saveOptions(void);
		
    ConvOptDialog *convOptDialog;
    TimeDialog *timeDialog;
    KeyDialog* keyDialog;
    AboutDialog* aboutDialog;
    StartDialog* startDialog;
    TextViewer *viewer;
public:
    gtime_t rinexTime;
    QString runBy, marker, markerNo, markerType, name[2], receiver[3], antenna[3];
    QString rinexStationCode, comment[2], receiverOptions, excludedSatellites;
    QString modeMask[7];
    double approxPosition[3], antennaDelta[3], timeTolerance;
    int rinexVersion, rinexFile, navSys, observationType, frequencyType, traceLevel;
    int autoPosition, phaseShift, halfCycle, outputIonoCorr, outputTimeCorr, outputLeapSeconds, separateNavigation;
    int enableGlonassFrequency, glonassFrequency[27];
	
    explicit MainWindow(QWidget *parent=0);
};
//---------------------------------------------------------------------------
#endif
