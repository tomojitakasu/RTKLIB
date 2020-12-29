//---------------------------------------------------------------------------
#ifndef conndlgH
#define conndlgH
//---------------------------------------------------------------------------

#define MAXHIST		10

#include "ui_conndlg.h"

#include <QDialog>

class QShowEvent;

//---------------------------------------------------------------------------
class ConnectDialog : public QDialog, private Ui::ConnectDialog
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent*);

public slots:
    void BtnOpt1Click();
    void BtnOkClick();
    void BtnCmd1Click();
    void BtnOpt2Click();
    void BtnCmd2Click();
    void SelStream1Change();
    void SolFormat1Change();
    void SolFormat2Change();
    void SelStream2Change();

private:
    void SerialOpt1(int opt);
    void SerialOpt2(int opt);
    void TcpOpt1(int opt);
    void TcpOpt2(int opt);
    void FileOpt1(int opt);
    void FileOpt2(int opt);
    void UpdateEnable(void);

public:
	int Stream1,Stream2,Format1,Format2,CmdEna1[2],CmdEna2[2];
	int TimeForm,DegForm,TimeOutTime,ReConnTime;
    QString Path,Paths1[4],Paths2[4];
    QString TcpHistory[MAXHIST],TcpMntpHist[MAXHIST];
    QString Cmds1[2],Cmds2[2],FieldSep;

    explicit ConnectDialog(QWidget *parent=NULL);
};
//---------------------------------------------------------------------------
#endif
