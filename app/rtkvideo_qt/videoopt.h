//---------------------------------------------------------------------------
#ifndef videooptH
#define videooptH

#include <QDialog>

#include "ui_videoopt.h"

class QShowEvent;

//---------------------------------------------------------------------------
class VideoOptDlg : public QDialog, protected Ui::VideoOptDlg
{
    Q_OBJECT
public slots:
    void BtnOkClick();
    void ChkTcpPortChange();
    void BtnFileClick();
    void UpdateEnable(void);
    void SelCapPosChange();
    void SelDevChange();
    void SelCapColorClicked();

protected:
    void showEvent(QShowEvent *);

    void UpdateProf(void);

    QColor CapColor;

public:
    VideoOptDlg(QWidget *parent);
};
//---------------------------------------------------------------------------
#endif
