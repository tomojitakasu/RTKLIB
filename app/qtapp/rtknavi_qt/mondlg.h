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
    void SelStrChange();
    void SelStr2Change();

private:
    int TypeF, ConFmt, Str1, Str2, FontScale, ObsMode;
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
    void SetRefSta(void);
    void ShowRtk(void);
    void ShowSat(void);
    void ShowEst(void);
    void ShowCov(void);
    void ShowObs(void);
    void ShowNav(void);
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
    void ShowRefSta(void);

    void AddConsole(const unsigned char *msg, int n, int mode);
    void ViewConsole(void);

public:
    explicit MonitorDialog(QWidget* parent);
    ~MonitorDialog();
};
//---------------------------------------------------------------------------
#endif
