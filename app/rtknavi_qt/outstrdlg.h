//---------------------------------------------------------------------------
#ifndef outstrdlgH
#define outstrdlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_outstrdlg.h"

class KeyDialog;
class SerialOptDialog;
class TcpOptDialog;

//---------------------------------------------------------------------------
class OutputStrDialog : public QDialog, private Ui::OutputStrDialog
{
    Q_OBJECT
protected:
    void showEvent(QShowEvent *);

    KeyDialog *keyDialog;
    SerialOptDialog *serialOptDialog;
    TcpOptDialog *tcpOptDialog;

public slots:
    void BtnOkClick();
    void BtnStr1Click();
    void BtnStr2Click();
    void Stream1Change(int);
    void Stream2Change(int);
    void BtnFile1Click();
    void BtnFile2Click();
    void Stream1CClick();
    void Stream2CClick();
    void BtnKeyClick();

private:
    QString GetFilePath(const QString path);
    QString SetFilePath(const QString path);
    void SerialOpt(int index, int opt);
    void TcpOpt(int index, int opt);
    void UpdateEnable(void);

public:
	int StreamC[2],Stream[2],Format[2],OutTimeTag,OutAppend;
    QString Paths[2][4],SwapInterval;
    QString History[10],MntpHist[10];

    explicit OutputStrDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
