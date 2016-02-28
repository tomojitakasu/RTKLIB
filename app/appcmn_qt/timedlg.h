//---------------------------------------------------------------------------
#ifndef timedlgH
#define timedlgH
//---------------------------------------------------------------------------
#include "rtklib.h"

#include <QDialog>

#include "ui_timedlg.h"

class QShowEvent;

//---------------------------------------------------------------------------
class TimeDialog : public QDialog, public Ui::TimeDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent *);

public:
	gtime_t Time;

    TimeDialog(QWidget *parent);
};
//---------------------------------------------------------------------------
#endif
