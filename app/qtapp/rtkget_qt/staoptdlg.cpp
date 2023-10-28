//---------------------------------------------------------------------------

#include "getmain.h"
#include "staoptdlg.h"

#include <QShowEvent>
#include <QFileDialog>

extern MainForm *mainForm;

//---------------------------------------------------------------------------
StaListDialog::StaListDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(btnLoad, SIGNAL(clicked(bool)), this, SLOT(btnLoadClicked()));
    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(btnOkClicked()));
    connect(btnSave, SIGNAL(clicked(bool)), this, SLOT(btnSaveClicked()));
}
//---------------------------------------------------------------------------
void StaListDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    stationListWidget->clear();

    for (int i = 0; i < mainForm->stationListWidget->count(); i++)
        stationListWidget->addItem(mainForm->stationListWidget->item(i)->text());
}
//---------------------------------------------------------------------------
void StaListDialog::btnOkClicked()
{
    mainForm->stationListWidget->clear();

    for (int i = 0; i < stationListWidget->count(); i++)
        mainForm->stationListWidget->addItem(stationListWidget->item(i)->text());
}
//---------------------------------------------------------------------------
void StaListDialog::btnLoadClicked()
{
    QString filename;
    QFile fp;
    QByteArray buff;

    filename = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open...")));

    fp.setFileName(filename);
    if (!fp.open(QIODevice::ReadOnly)) return;

    stationListWidget->clear();
    stationListWidget->setVisible(false);

    while (!fp.atEnd()) {
        buff = fp.readLine();
        buff = buff.mid(buff.indexOf('#'));
        stationListWidget->addItem(buff);
    }

    stationListWidget->setVisible(true);
}
//---------------------------------------------------------------------------
void StaListDialog::btnSaveClicked()
{
    QString file = QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Save...")));
    QFile fp;

    fp.setFileName(file);
    if (!fp.open(QIODevice::WriteOnly)) return;

    for (int i = 0; i < stationListWidget->count(); i++)
        fp.write((stationListWidget->item(i)->text() + "\n").toLatin1());
}
//---------------------------------------------------------------------------
