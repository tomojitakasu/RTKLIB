//---------------------------------------------------------------------------
#ifndef startdlgH
#define startdlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_startdlg.h"

#include "rtklib.h"
class QShowEvent;

//---------------------------------------------------------------------------
class StartDialog : public QDialog, private Ui::StartDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent*);

public slots:
    void btnOkClicked();
    void btnFileTimeClicked();

public:
    gtime_t time;
    QString filename;

    explicit StartDialog(QWidget *parent=NULL);
};
//---------------------------------------------------------------------------
#endif
