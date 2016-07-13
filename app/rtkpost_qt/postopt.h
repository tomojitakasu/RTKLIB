//---------------------------------------------------------------------------
#ifndef postoptH
#define postoptH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_postopt.h"

#include "rtklib.h"
//---------------------------------------------------------------------------
class OptDialog : public QDialog, public Ui::OptDialog
{
    Q_OBJECT

public slots:
    void BtnOkClick();
    void RovAntPcvClick();
    void BtnAntPcvFileClick();
    void BtnIonoFileClick();
    void BtnAntPcvViewClick();
    void NetRSCorrClick();
    void SatClkCorrClick();

    void PosModeChange();
    void SolFormatChange();
    void AmbResChange();

    void BtnLoadClick();
    void BtnSaveClick();
    void FreqChange();
    void BtnRefPosClick();
    void BtnRovPosClick();
    void RovPosClick();
    void RefPosClick();
    void BtnStaPosViewClick();
    void BtnStaPosFileClick();
    void SbasCorrClick();
    void OutputHeightClick();

    void RefPosTypeChange();
    void RovPosTypeChange();
    void GetPos(int type, QLineEdit **edit, double *pos);
    void SetPos(int type, QLineEdit **edit, double *pos);

    void BtnSatPcvFileClick();
    void BtnSatPcvViewClick();
    void SatEphemClick();
    void BtnGeoidDataFileClick();
    void BaselineConstClick();
    void NavSys2Click();

    void IonoOptChange();
    void TropOptChange();
    void DynamicModelChange();

    void SatEphemChange();
    void RovAntClick();
    void RefAntClick();
    void BtnDCBViewClick();
    void BtnDCBFileClick();
    void BtnHelpClick();
    void ExtEna0Click();
    void ExtEna1Click();
    void ExtEna2Click();
    void BtnBLQFileViewClick();
    void BtnBLQFileClick();
    void BtnEOPFileClick();
    void BtnEOPViewClick();
    void BtnExtOptClick();
    void BtnMaskClick();
    void NavSys6Click();

protected:
    void showEvent(QShowEvent*);

private:
	snrmask_t SnrMask;
	int RovPosTypeP,RefPosTypeP;

    void GetOpt(void);
    void SetOpt(void);
    void LoadOpt(const QString &file);
    void SaveOpt(const QString &file);
    void ReadAntList(void);
    void UpdateEnable(void);
    void UpdateEnableExtErr(void);
public:
	exterr_t ExtErr;
	
    explicit OptDialog(QWidget *parent);
};
//---------------------------------------------------------------------------
#endif
