//---------------------------------------------------------------------------
#ifndef navioptH
#define navioptH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_naviopt.h"
#include "rtklib.h"

class TextViewer;
//---------------------------------------------------------------------------
class OptDialog : public QDialog, private Ui::OptDialog
{
    Q_OBJECT
protected:
    void showEvent(QShowEvent*);

public slots:
    void BtnOkClick();
    void RovAntPcvClick();
    void BtnAntPcvFileClick();
    void BtnDCBFileClick();
    void BtnAntPcvViewClick();
    void AmbResChange(int);
    void PosModeChange(int);
    void SolFormatChange(int);
    void BtnLoadClick();
    void BtnSaveClick();
    void FreqChange();
    void BtnRefPosClick();
    void BtnRovPosClick();
    void BtnStaPosViewClick();
    void BtnStaPosFileClick();
    void OutputHeightClick();
    void RefPosTypePChange(int);
    void RovPosTypePChange(int);
    void GetPos(int type, QLineEdit **edit, double *pos);
    void SetPos(int type, QLineEdit **edit, double *pos);
    void BtnFontClick();
    void BtnGeoidDataFileClick();
    void NavSys2Click();
    void BaselineConstClick();
    void BtnSatPcvViewClick();
    void BtnSatPcvFileClick();
    void BtnLocalDirClick();
    void BtnEOPFileClick();
    void BtnEOPViewClick();
    void BtnTLESatFileClick();
    void BtnTLEFileClick();
    void BtnSnrMaskClick();
    void NavSys6Click();

private:
    void GetOpt(void);
    void SetOpt(void);
    void LoadOpt(const QString &file);
    void SaveOpt(const QString &file);
    void ReadAntList(void);
    void UpdateEnable(void);

public:
	prcopt_t PrcOpt;
	solopt_t SolOpt;
    QFont PosFont;
    TextViewer *textViewer;

	int SvrCycle,SvrBuffSize,SolBuffSize,NavSelect,SavedSol;
	int NmeaReq,NmeaCycle,TimeoutTime,ReconTime,DgpsCorr,SbasCorr;
	int DebugTraceF,DebugStatusF;
	int RovPosTypeF,RefPosTypeF,RovAntPcvF,RefAntPcvF,BaselineC;
    int MoniPort,FileSwapMargin,PanelStack;

    QString ExSats,LocalDirectory;
    QString RovAntF,RefAntF,SatPcvFileF,AntPcvFileF,StaPosFileF;
    QString GeoidDataFileF,DCBFileF,EOPFileF,TLEFileF,TLESatFileF;
    QString ProxyAddr;

	double RovAntDel[3],RefAntDel[3],RovPos[3],RefPos[3];
	double Baseline[2],NmeaIntv[2];

    explicit OptDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
