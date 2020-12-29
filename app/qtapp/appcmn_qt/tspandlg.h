//---------------------------------------------------------------------------
#ifndef tspandlgH
#define tspandlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_tspandlg.h"

#include "rtklib.h"

class QShowEvent;

//---------------------------------------------------------------------------
class SpanDialog : public QDialog, public Ui::SpanDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent*);

public slots:
    void BtnOkClick();
    void TimeStartFClick();
    void TimeEndFClick();
    void TimeIntFClick();

private:
    void UpdateEnable(void);

public:
	int TimeEna[3],TimeVal[3];
	gtime_t TimeStart,TimeEnd;
	double TimeInt;

    explicit SpanDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
