//---------------------------------------------------------------------------
#ifndef mondlgH
#define mondlgH
//---------------------------------------------------------------------------
#include <QStringList>
#include <QDialog>
#include <QTimer>

#include "rtklib.h"

#include "ui_mondlg.h"

//---------------------------------------------------------------------------
class MonitorDialog : public QDialog, private Ui::MonitorDialog
{
    Q_OBJECT
protected:
    void showEvent(QShowEvent*);
    void closeEvent(QCloseEvent*);

public slots:
    void BtnCloseClick();
    void BtnClearClick();
    void BtnDownClick();
    void TypeChange(int);
    void Timer1Timer();
    void Timer2Timer();
    void SelObsChange(int);
    void SelFmtChange(int);

private:
	int TypeF,ConFmt,ScrollPos,FontScale,ObsMode;
    QStringList ConBuff;
    QStringList header;
	rtcm_t rtcm;
	raw_t raw;
    QTimer timer1,timer2;
	
    void ClearTable(void);
    void SetRtk(void);
    void SetSat(void);
    void SetEst(void);
    void SetCov(void);
    void SetObs(void);
    void SetNav(void);
    void SetGnav(void);
    void SetStr(void);
    void SetSbsMsg(void);
    void SetSbsLong(void);
    void SetSbsIono(void);
    void SetSbsFast(void);
    void SetSbsNav(void);
    void SetIonUtc(void);
    void SetRtcm(void);
    void SetRtcmDgps(void);
    void SetRtcmSsr(void);
    void SetLexMsg(void);
    void SetLexEph(void);
    void SetLexIon(void);
    void SetIonCorr(void);
    void ShowRtk(void);
    void ShowSat(int sys);
    void ShowEst(void);
    void ShowCov(void);
    void ShowObs(void);
    void ShowNav(int sys);
    void ShowGnav(void);
    void ShowSbsMsg(void);
    void ShowIonUtc(void);
    void ShowStr(void);
    void ShowSbsLong(void);
    void ShowSbsIono(void);
    void ShowSbsFast(void);
    void ShowSbsNav(void);
    void ShowRtcm(void);
    void ShowRtcmDgps(void);
    void ShowRtcmSsr(void);
    void ShowLexMsg(void);
    void ShowLexEph(void);
    void ShowLexIon(void);
    void ShowIonCorr(void);

    void AddConsole(unsigned char *msg, int n, int mode);
    void ViewConsole(void);

public:
    explicit MonitorDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
