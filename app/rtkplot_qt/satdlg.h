//---------------------------------------------------------------------------
#ifndef satdlgH
#define satdlgH
//---------------------------------------------------------------------------

#include <QDialog>

#include "ui_satdlg.h"
class QShowEvent;

//---------------------------------------------------------------------------
class SatDialog : public QDialog, private Ui::SatDialog
{
    Q_OBJECT

public slots:
    void BtnOkClick();
    void BtnCancelClick();
    void BtnChkAllClick();
    void BtnUnchkAllClick();

protected:
    void showEvent(QShowEvent*);

public:
	int ValidSat[36];
    explicit SatDialog(QWidget * parent=NULL);
};
//---------------------------------------------------------------------------
#endif
