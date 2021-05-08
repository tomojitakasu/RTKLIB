//---------------------------------------------------------------------------
#ifndef timedlgH
#define timedlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "rtklib.h"

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

    explicit TimeDialog(QWidget *parent);
};
//---------------------------------------------------------------------------
#endif
