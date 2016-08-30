//---------------------------------------------------------------------------
#ifndef mondlgH
#define mondlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QStringList>

#include "rtklib.h"

#include "ui_mondlg.h"

#define MAX_MSG_BUFF	4096

//---------------------------------------------------------------------------
class StrMonDialog : public QDialog, private Ui::StrMonDialog
{
    Q_OBJECT
public slots:
    void BtnCloseClick();
    void BtnClearClick();
    void BtnDownClick();
    void SelFmtChange();
private:
    QStringList ConBuff;
    int Stop,ScrollPos,StrFmt;
    rtcm_t rtcm;
    raw_t raw;

    void AddConsole(unsigned char *msg, int len, int mode);
public:
    explicit StrMonDialog(QWidget *parent);
    void AddMsg(unsigned char *buff, int n);
};
//---------------------------------------------------------------------------
#endif
