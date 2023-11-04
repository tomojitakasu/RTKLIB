//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QFileDialog>

#include "browsmain.h"
#include "staoptdlg.h"


extern MainForm *mainForm;

//---------------------------------------------------------------------------
StaListDialog::StaListDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(BtnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(BtnLoad, SIGNAL(clicked(bool)), this, SLOT(btnLoadClicked()));
    connect(BtnOk, SIGNAL(clicked(bool)), this, SLOT(btnOkClicked()));
    connect(BtnSave, SIGNAL(clicked(bool)), this, SLOT(btnSaveClicked()));
}
//---------------------------------------------------------------------------
void StaListDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    StationList->clear();

    for (int i = 0; i < mainForm->stationList.count(); i++)
        StationList->addItem(mainForm->stationList.at(i));
}
//---------------------------------------------------------------------------
void StaListDialog::btnOkClicked()
{
    mainForm->stationList.clear();

    for (int i = 0; i < StationList->count(); i++)
        mainForm->stationList.append(StationList->item(i)->text());
}
//---------------------------------------------------------------------------
void StaListDialog::btnLoadClicked()
{
    QString filename;
    QFile file;
    QByteArray buffer;

    filename = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open...")));

    file.setFileName(filename);
    if (!file.open(QIODevice::ReadOnly)) return;

    StationList->clear();
    StationList->setVisible(false);

    while (!file.atEnd()) {
        buffer = file.readLine();
        buffer = buffer.mid(buffer.indexOf('#'));
        StationList->addItem(buffer);
    }

    StationList->setVisible(true);
}
//---------------------------------------------------------------------------
void StaListDialog::btnSaveClicked()
{
    QString filename = QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Save...")));
    QFile file;

    file.setFileName(filename);
    if (!file.open(QIODevice::WriteOnly)) return;

    for (int i = 0; i < StationList->count(); i++)
        file.write((StationList->item(i)->text() + "\n").toLatin1());
}
//---------------------------------------------------------------------------
