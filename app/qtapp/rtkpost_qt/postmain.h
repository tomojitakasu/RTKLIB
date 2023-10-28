//---------------------------------------------------------------------------
#ifndef postmainH
#define postmainH
//---------------------------------------------------------------------------
#include <QString>
#include <QDialog>
#include <QThread>

#include "rtklib.h"

#include "ui_postmain.h"

class QShowEvent;
class QCloseEvent;
class QSettings;
class OptDialog;
class TextViewer;
class ConvDialog;


//Helper Class ------------------------------------------------------------------

class ProcessingThread : public QThread
{
    Q_OBJECT

public:
    prcopt_t prcopt;
    solopt_t solopt;
    filopt_t filopt;
    gtime_t ts, te;
    double ti, tu;
    int n, stat;
    char *infile[6], outfile[1024];
    char *rov, *base;

    explicit ProcessingThread(QObject *parent);
    ~ProcessingThread();

    void addInput(const QString &);
    void addList(char * &sta, const QString & list);

protected:
    void run();

signals:
    void done(int);
};
//---------------------------------------------------------------------------

class MainForm : public QDialog, public Ui::MainForm
{
    Q_OBJECT

public slots:
    void btnPlotClicked();
    void btnViewClicked();
    void btnToKMLClicked();
    void btnOptionClicked();
    void btnExecClicked();
    void btnAbortClicked();
    void btnExitClicked();
    void btnAboutClicked();
	
    void btnTimeStartClicked();
    void btnTimeStopClicked();
    void btnInputFile1Clicked();
    void btnInputFile3Clicked();
    void btnInputFile2Clicked();
    void btnInputFile4Clicked();
    void btnInputFile5Clicked();
    void btnOutputFileClicked();
    void btnInputView1Clicked();
    void btnInputView3Clicked();
    void btnInputView2Clicked();
    void btnInputView4Clicked();
    void btnInputView5Clicked();
    void btnOutputView1Clicked();
    void btnOutputView2Clicked();
    void btnInputPlot1Clicked();
    void btnInputPlot2Clicked();
    void btnKeywordClicked();
	
    void timeStartClicked();
    void timeEndClicked();
    void timeIntervalFClicked();
    void timeUnitFClicked();
	
    void inputFile1Changed();
    void outputDirectoryEnableClicked();
    void btnOutputDirectoryClicked();
    void outputDirectoryChanged();
    void btnInputFile6Clicked();
    void btnInputView6Clicked();

    void formCreate();
    void processingFinished(int);
    void showMessage(const QString  &msg);

protected:
    void showEvent(QShowEvent*);
    void closeEvent(QCloseEvent*);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    OptDialog  *optDialog;
    ConvDialog *convDialog;
    TextViewer *textViewer;

    void execProcessing (void);
    int  getOption(prcopt_t &prcopt, solopt_t &solopt, filopt_t &filopt);
    int  obsToNav (const QString &obsfile, QString &navfile);
	
    QString filePath(const QString &file);
    void readList(QComboBox *, QSettings *ini,  const QString &key);
    void writeList(QSettings *ini, const QString &key, const QComboBox *combo);
    void addHistory(QComboBox *combo);
    int execCommand(const QString &cmd, const QStringList &opt, int show);
	
    gtime_t getTimeStart(void);
    gtime_t getTimeStop(void);
    void setOutputFile(void);
    void setTimeStart(gtime_t time);
    void setTimeStop(gtime_t time);
    void updateEnable(void);
    void loadOptions(void);
    void saveOptions(void);
	
public:
    QString iniFile;
    bool abortFlag;
	
    // options
    int positionMode, frequencies, solution, dynamicModel, ionosphereOption, troposphereOption, receiverBiasEstimation;
    int ARIter, numIter, codeSmooth, tideCorrection;
    int outputCntResetAmbiguity, fixCntHoldAmbiguity, LockCntFixAmbiguity, roverPositionType, referencePositionType;
    int satelliteEphemeris, navigationSystems;
    int roverAntennaPcv, referenceAntennaPcv, ambiguityResolutionGPS, ambiguityResolutionGLO, ambiguityResolutionBDS;
    int outputHeader, outputOptions, outputVelocity, outputSingle, outputDatum;
    int outputHeight, outputGeoid, debugTrace, debugStatus, baseLineConstrain;
    int solutionFormat, timeFormat, latLonFormat, intpolateReferenceObs, netRSCorr, satelliteClockCorrection;
    int sbasCorrection, sbasCorrection1, sbasCorrection2, sbasCorrection3, sbasCorrection4, timeDecimal;
    int solutionStatic, sbasSat, mapFunction;
    int positionOption[6];
    double elevationMask, maxAgeDiff, rejectThres, rejectGdop;
    double measurementErrorR1, measurementErrorR2, measurementError2, measurementError3, measurementError4, measurementError5;
    double satelliteClockStability, roverAntennaE, roverAntennaN, roverAntennaU, referenceAntennaE, referenceAntennaN, referenceAntennaU;
    double processNoise1, processNoise2, processNoise3, processNoise4, processNoise5;
    double validThresAR, elevationMaskAR, elevationMaskHold, slipThres;
    double thresAR2, thresAR3;
    double roverPosition[3], referencePosition[3], baseLine[2];
    double maxSolutionStd;
    snrmask_t snrMask;
	
    QString rnxOptions1, rnxOptions2, pppOptions;
    QString fieldSeperator, roverAntenna, referenceAntenna, antennaPcvFile, stationPositionFile, PrecEphFile;
    QString netRSCorrFile1, netRSCorrFile2, satelliteClockCorrectionFile, googleEarthFile;
    QString geoidDataFile, ionosphereFile, dcbFile, eopFile, blqFile;
    QString sbasCorrectionFile, satellitePcvFile, excludedSatellites;
    QString roverList, baseList;
	
    void viewFile(const QString &file);

    explicit MainForm(QWidget *parent = 0);
};

//---------------------------------------------------------------------------
#endif
