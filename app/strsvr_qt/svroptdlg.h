//---------------------------------------------------------------------------
#ifndef svroptdlgH
#define svroptdlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_svroptdlg.h"

class QShowEvent;
//---------------------------------------------------------------------------
class SvrOptDialog : public QDialog, public Ui::SvrOptDialog
{
    Q_OBJECT
public slots:
    void BtnOkClick();
    void BtnPosClick();
    void NmeaReqTClick();
    void BtnLocalDirClick();
    void StaInfoSelClick();
protected:
    void showEvent(QShowEvent*);
private:
    void UpdateEnable(void);
public:
    QString StaPosFile,ExeDirectory,LocalDirectory,ProxyAddress;
    QString AntType,RcvType;
	int SvrOpt[6],TraceLevel,NmeaReq,FileSwapMargin,StaId,StaSel;
	double AntPos[3],AntOff[3];
    explicit SvrOptDialog(QWidget *parent);
};
//---------------------------------------------------------------------------
#endif
