//---------------------------------------------------------------------------
#ifndef confdlgH
#define confdlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_confdlg.h"

//---------------------------------------------------------------------------
class ConfDialog : public QDialog, public Ui::ConfDialog
{
    Q_OBJECT
public:
    ConfDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
