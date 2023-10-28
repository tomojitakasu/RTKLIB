//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <stdio.h>

#include <QShowEvent>
#include <QProcess>
#include <QIntValidator>
#include <QUrl>

#include "rtklib.h"
#include "tcpoptdlg.h"
#include "mntpoptdlg.h"

//---------------------------------------------------------------------------

#define NTRIP_TIMEOUT   10000                           // response timeout (ms)
#define NTRIP_CYCLE             50                      // processing cycle (ms)
#define MAXSRCTBL               512000                  // max source table size (bytes)
#define ENDSRCTBL               "ENDSOURCETABLE"        // end marker of table
#define MAXLINE                 1024                    // max line size (byte)

//---------------------------------------------------------------------------
TcpOptDialog::TcpOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    mntpOptDialog = new MntpOptDialog(this);

    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(btnOkClicked()));
    connect(btnNtrip, SIGNAL(clicked(bool)), this, SLOT(btnNtripClicked()));
    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
}
//---------------------------------------------------------------------------
void TcpOptDialog::showEvent(QShowEvent *event)
{
    QString ti[] = { tr("TCP Server Options "),  tr("TCP Client Options"),
             tr("NTRIP Server Options"), tr("NTRIP Client Options"),
                "NTRIP Caster Client Options",
                "NTRIP Caster Server Options", "UDP Server Options",
                "UDP Client Options"};

    if (event->spontaneous()) return;

    cBAddress->clear();
    cBMountPoint->clear();

    for (int i = 0; i < MAXHIST; i++)
        if (history[i] != "") cBAddress->addItem(history[i]);

    int index = path.lastIndexOf("@");

    QStringList tokens= path.mid(0,index).split(':'); // separate user and password
    if (tokens.size() == 2)
    {
        lEUser->setText(tokens.at(0));
        lEPassword->setText(tokens.at(1));
    } else if (tokens.size() == 1)
        lEUser->setText(tokens.at(0));

    QString url_str = path.mid(index); // use the rest

    index = url_str.lastIndexOf(":"); // separate "str"
    mountpoint=url_str.mid(index + 1);

    QUrl url(QString("ftp://") + url_str.mid(0,index));

    cBAddress->insertItem(0, url.host()); cBAddress->setCurrentText(url.host());
    sBPort->setValue(url.port());
    if (options==2||options==4) {
        cBMountPoint->insertItem(0, url.path().mid(1)); cBMountPoint->setCurrentText(url.path().mid(1));
    }

    setWindowTitle(ti[options]);
    lblAddress->setText((options>=2&&options<=5)?"NTRIP Caster Address":"Server Address");
    lblAddress->setEnabled((options>=1&&options<=3)||options==7);

    cBAddress->setEnabled((options >= 1 && options <= 3) || options == 7);
    lblMountPoint->setEnabled(options >= 2 && options <= 4);
    cBMountPoint->setEnabled(options >= 2 && options <= 4);
    lblUser->setEnabled(options >= 3 && options <= 4);
    lEUser->setEnabled(options >= 3 && options <= 4);
    lblPassword->setEnabled(options >= 2 && options <= 5);
    lEPassword->setEnabled(options >= 2 && options <= 5);
    btnNtrip->setVisible((options==3));
    btnBrowse->setVisible((options==3));
    btnMountpointOptions->setVisible((options==2||options==4));  //FIXME: w/o function

    setWindowTitle(ti[options]);

    btnNtrip->setVisible(options == 2 || options == 3);
}
//---------------------------------------------------------------------------
void TcpOptDialog::btnOkClicked()
{
    QString User_Text = lEUser->text(), Passwd_Text = lEPassword->text();
    QString Addr_Text = cBAddress->currentText(), Port_Text = sBPort->text();
    QString MntPnt_Text = cBMountPoint->currentText();

    path = QString("%1:%2@%3:%4/%5:%6").arg(User_Text).arg(Passwd_Text)
           .arg(Addr_Text).arg(Port_Text).arg(MntPnt_Text)
           .arg(mountpoint);

    addHistory(cBAddress, history);

    accept();
}
//---------------------------------------------------------------------------
void TcpOptDialog::addHistory(QComboBox *list, QString *hist)
{
    for (int i = 0; i < MAXHIST; i++) {
        if (list->currentText() != hist[i]) continue;
        for (int j = i + 1; j < MAXHIST; j++) hist[j - 1] = hist[j];
        hist[MAXHIST - 1] = "";
	}
    for (int i = MAXHIST - 1; i > 0; i--) hist[i] = hist[i - 1];
    hist[0] = list->currentText();

    list->clear();
    for (int i = 0; i < MAXHIST; i++)
        if (hist[i] != "") list->addItem(hist[i]);
}
//---------------------------------------------------------------------------
void TcpOptDialog::btnNtripClicked()
{
    QPushButton *btn = (QPushButton *)sender();
    QString path = cBAddress->currentText() + ":" + sBPort->text();
    stream_t str;
    uint32_t tick = tickget();
    static char buff[MAXSRCTBL];
    char *p = buff, mntpnt[256];

    strinit(&str);
    if (!stropen(&str, STR_NTRIPCLI,STR_MODE_R, qPrintable(path))) return;

    btn->setEnabled(false);
    *p='\0';
    while (p<buff+MAXSRCTBL-1) {
        p+=strread(&str,(uint8_t *)p,buff+MAXSRCTBL-p-1);
        *p='\0';
        sleepms(NTRIP_CYCLE);
        if (strstr(buff,ENDSRCTBL)) break;
        if ((int)(tickget()-tick)>NTRIP_TIMEOUT) break;
    }
    strclose(&str);

    cBMountPoint->clear();
    for (p=buff;(p=strstr(p,"STR;"));p+=4) {
        if (sscanf(p,"STR;%255[^;]",mntpnt)==1) {
            cBMountPoint->addItem(mntpnt);
        }
    }
    btn->setEnabled(true);
}
//---------------------------------------------------------------------------
void TcpOptDialog::btnBrowseClicked()
{
    QString Addr_Text = cBAddress->currentText();
    QString Port_Text = sBPort->text();

    if (Port_Text!="") Addr_Text+=":"+Port_Text;
    ExecCommand("srctblbrows_qt ", QStringList(Addr_Text), 1);
}
//---------------------------------------------------------------------------
void TcpOptDialog::btnMountpointClicked()
{
    mntpOptDialog->mountPoint=cBMountPoint->currentText();
    mntpOptDialog->MntpStr=mountpoint;
    mntpOptDialog->exec();
    if (mntpOptDialog->result()!=QDialog::Accepted) return;
    cBMountPoint->setCurrentText(mntpOptDialog->mountPoint);
    mountpoint=mntpOptDialog->MntpStr;
}
//---------------------------------------------------------------------------
int TcpOptDialog::ExecCommand(const QString &cmd, const QStringList &opt, int show)
{
    QProcess prog;

    Q_UNUSED(show);

    prog.start(cmd, opt); /* FIXME: show option not yet supported */
    return 1;
}
//---------------------------------------------------------------------------
