//---------------------------------------------------------------------------
#ifndef getoptdlgH
#define getoptdlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_getoptdlg.h"

//---------------------------------------------------------------------------
class DownOptDialog : public QDialog, private Ui::DownOptDialog
{
    Q_OBJECT

protected:
    void  showEvent(QShowEvent*);

public slots:
    void  btnOkClicked();
    void  btnUrlFileClicked();
    void  btnLogFileClicked();

public:
    explicit DownOptDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
