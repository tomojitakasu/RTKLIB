//---------------------------------------------------------------------------
#ifndef aboutdlgH
#define aboutdlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>

#include "ui_aboutdlg.h"

//---------------------------------------------------------------------------
class AboutDialog : public QDialog, private Ui::AboutDlg
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent *);

public:
	int IconIndex;
    QString About;
    explicit AboutDialog(QWidget*);
};
#endif
