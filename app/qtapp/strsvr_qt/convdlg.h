//---------------------------------------------------------------------------
#ifndef convdlgH
#define convdlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_convdlg.h"

class QShowEvent;
//---------------------------------------------------------------------------
class ConvDialog : public QDialog, private Ui::ConvDialog
{
    Q_OBJECT

public slots:
    void btnOkClicked();
    void conversionClicked();

protected:
    void showEvent(QShowEvent*);

private:
    void updateEnable(void);

public:
    QString conversionMessage, conversionOptions, antennaType, receiverType;
    int conversionEnabled, conversionInputFormat, conversionOutputFormat, stationId;
    double antennaPos[3], antennaOffset[3];
	
    explicit ConvDialog(QWidget *parent);
};
//---------------------------------------------------------------------------
#endif
