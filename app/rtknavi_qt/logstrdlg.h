//---------------------------------------------------------------------------
#ifndef logstrdlgH
#define logstrdlgH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_logstrdlg.h"

class KeyDialog;
class SerialOptDialog;
class TcpOptDialog;

//---------------------------------------------------------------------------
class LogStrDialog : public QDialog, private Ui::LogStrDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent *);

    KeyDialog *keyDialog;
    SerialOptDialog *serialOptDialog;
    TcpOptDialog *tcpOptDialog;

public slots:
    void BtnOkClick();
    void Stream1Change(int);
    void Stream2Change(int);
    void BtnStr1Click();
    void BtnStr2Click();
    void BtnFile1Click();
    void BtnFile2Click();
    void Stream1CClick();
    void Stream2CClick();
    void BtnKeyClick();
    void BtnStr3Click();
    void BtnFile3Click();
    void Stream3CClick();
    void Stream3Change(int);

private:
    QString GetFilePath(const QString &path);
    QString SetFilePath(const QString &path);

    void SerialOpt(int index, int opt);
    void TcpOpt(int index, int opt);
    void UpdateEnable(void);

public:
	int StreamC[3],Stream[3],LogTimeTag,LogAppend;
    QString Paths[3][4],SwapInterval;
    QString History[10],MntpHist[10];

    explicit LogStrDialog(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
