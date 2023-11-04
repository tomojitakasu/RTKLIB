//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <stdio.h>

#include <QShowEvent>
#include <QUrl>
#include <QIntValidator>

#include "rtklib.h"
#include "ftpoptdlg.h"
#include "keydlg.h"

//---------------------------------------------------------------------------
FtpOptDialog::FtpOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    keyDlg = new KeyDialog(this);

    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(btnOkClicked()));
    connect(btnKey, SIGNAL(clicked(bool)), this, SLOT(btnKeyClicked()));
    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));

    cBPathOffset->setValidator(new QIntValidator(this));
    cBInterval->setValidator(new QIntValidator(this));
    cBOffset->setValidator(new QIntValidator(this));
}
//---------------------------------------------------------------------------
void FtpOptDialog::showEvent(QShowEvent *event)
{
    QString cap[] = { tr("FTP Option"), tr("HTTP Option") };
    int topts[4] = { 0, 3600, 0, 0 };

    if (event->spontaneous()) return;

    setWindowTitle(cap[options]);

    QStringList tokens = path.split("::");
    if (tokens.size() > 1) {
        QString t = tokens.at(1);
        if (t.contains("T=")) {
            QStringList values = t.mid(2).split(",");
            for (int i = 0; (i < 4) || (i < values.size()); i++)
                topts[i] = values.at(i).toInt();
        }
    }
    QUrl url(QString("ftp://") + path);

    cBAddress->clear();
    cBAddress->addItem(url.host() + url.path());
    for (int i = 0; i < MAXHIST; i++)
        if (history[i] != "") cBAddress->addItem(history[i]);
    ;

    cBAddress->setCurrentIndex(0);
    lEUser->setText(url.userName());
    lEPassword->setText(url.password());
    cBPathOffset->insertItem(0, QString::number(topts[0] / 3600.0, 'g', 2)); cBPathOffset->setCurrentIndex(0);
    cBInterval->insertItem(0, QString::number(topts[1] / 3600.0, 'g', 2)); cBInterval->setCurrentIndex(0);
    cBOffset->insertItem(0, QString::number(topts[2] / 3600.0, 'g', 2)); cBOffset->setCurrentIndex(0);
    cBRetryInterval->setValue(topts[3]);
	updateEnable();
}
//---------------------------------------------------------------------------
void FtpOptDialog::btnOkClicked()
{
    QString PathOffset_Text = cBPathOffset->currentText();
    QString Interval_Text = cBInterval->currentText();
    QString Offset_Text = cBOffset->currentText();
    QString User_Text = lEUser->text(), Passwd_Text = lEPassword->text();
    QString Addr_Text = cBAddress->currentText(), s;
	int topts[4];
    bool ok;

    topts[0] = PathOffset_Text.toInt(&ok) * 3600.0;
    topts[1] = Interval_Text.toInt(&ok) * 3600.0;
    topts[2] = Offset_Text.toInt(&ok) * 3600.0;
    topts[3] = cBRetryInterval->value();

    path = QString("%1:%2@%3::T=%4,%5,%6,%7").arg(User_Text)
           .arg(Passwd_Text).arg(Addr_Text)
           .arg(topts[0]).arg(topts[1]).arg(topts[2]).arg(topts[3]);

    addHistory(cBAddress, history);

    accept();
}
//---------------------------------------------------------------------------
void FtpOptDialog::btnKeyClicked()
{
    keyDlg->exec();
}
//---------------------------------------------------------------------------
void FtpOptDialog::addHistory(QComboBox *list, QString *hist)
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
void FtpOptDialog::updateEnable(void)
{
    lEUser->setEnabled(options == 0);
    lEPassword->setEnabled(options == 0);
    lbUser->setEnabled(options == 0);
    lbPassword->setEnabled(options == 0);
}
