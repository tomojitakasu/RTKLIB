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

    void btnOkClicked();
    void btnAntennaPcvFileClicked();
    void btnIonophereFileClicked();
    void btnAntennaPcvViewClicked();

    void btnLoadClicked();
    void btnSaveClicked();
    void btnReferencePositionClicked();
    void btnRoverPositionClicked();
    void btnStationPositionViewClicked();
    void btnStationPositionFileClicked();

    void referencePositionTypeChanged();
    void roverPositionTypeChanged();
    void getPosition(int type, QLineEdit **edit, double *pos);
    void setPosition(int type, QLineEdit **edit, double *pos);

    void btnSatellitePcvFileClicked();
    void btnSatelitePcvViewClicked();
    void btnGeoidDataFileClicked();

    void btnDCBViewClicked();
    void btnDCBFileClicked();
    void btnHelpClicked();
    void extEna0Clicked();
    void extEna1Clicked();
    void extEna2Clicked();
    void btnBLQFileViewClicked();
    void btnBLQFileClicked();
    void btnEOPFileClicked();
    void btnEOPViewClicked();
    void btnFrequenciesClicked();
    void btnMaskClicked();
    void NavSys6Click();
    void updateEnable();

protected:

    void showEvent(QShowEvent*);

private:

    snrmask_t snrMask;
    int roverPositionType, ReferencePositionType;

    void getOptions(void);
    void setOptions(void);
    void loadOptions(const QString &file);
    void saveOptions(const QString &file);
    void readAntennaList(void);
    void updateEnableExtErr(void);

public:
	
    explicit OptDialog(QWidget *parent);
};
//---------------------------------------------------------------------------
#endif
