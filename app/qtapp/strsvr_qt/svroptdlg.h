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
    void btnOkClicked();
    void btnPosClicked();
    void nmeaReqChecked();
    void btnLocalDirClicked();
    void stationIdChecked();
    void btnLogFileClicked();

protected:
    void showEvent(QShowEvent*);

private:
    void updateEnable(void);

public:
    QString stationPositionFile, exeDirectory, localDirectory, proxyAddress;
    QString antennaType, receiverType, logFile;
    int serverOptions[6], traceLevel, NmeaReq, fileSwapMargin, stationId, StaSel, RelayBack;
    int progressBarRange;
    double antennaPos[3], antennaOffset[3];

    explicit SvrOptDialog(QWidget *parent);
};
//---------------------------------------------------------------------------
#endif
