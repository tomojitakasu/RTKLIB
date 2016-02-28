//---------------------------------------------------------------------------

#ifndef keydlgH
#define keydlgH
#include "ui_keydlg.h"

#include <QDialog>

//---------------------------------------------------------------------------
class KeyDialog : public QDialog, private Ui::KeyDialog
{
    Q_OBJECT

public slots:
    void  BtnOkClick();

protected:
    void showEvent(QShowEvent*);

public:
	int Flag;
    KeyDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
