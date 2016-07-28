//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QFileDialog>

#include "browsmain.h"
#include "staoptdlg.h"


extern MainForm *mainForm;

//---------------------------------------------------------------------------
StaListDialog::StaListDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnLoad,SIGNAL(clicked(bool)),this,SLOT(BtnLoadClick()));
    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnSave,SIGNAL(clicked(bool)),this,SLOT(BtnSaveClick()));
}
//---------------------------------------------------------------------------
void StaListDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;
    StaList->clear();
    
    for (int i=0;i<mainForm->StaList.count();i++) {
        StaList->addItem(mainForm->StaList.at(i));
    }
}
//---------------------------------------------------------------------------
void StaListDialog::BtnOkClick()
{
    mainForm->StaList.clear();
    
    for (int i=0;i<StaList->count();i++) {
        mainForm->StaList.append(StaList->item(i)->text());
    }
}
//---------------------------------------------------------------------------
void StaListDialog::BtnLoadClick()
{
    QString file;
    QFile fp;
    QByteArray buff;
    
    
    file=QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Open...")));

    fp.setFileName(file);
    if (!fp.open(QIODevice::ReadOnly)) return;
        
    StaList->clear();
    StaList->setVisible(false);
    
    while (!fp.atEnd())
    {
        buff=fp.readLine();
        buff=buff.mid(buff.indexOf('#'));
        StaList->addItem(buff);
    }

    StaList->setVisible(true);
}
//---------------------------------------------------------------------------
void StaListDialog::BtnSaveClick()
{
    QString file=QDir::toNativeSeparators(QFileDialog::getSaveFileName(this,tr("Save...")));
    QFile fp;

    fp.setFileName(file);
    if (!fp.open(QIODevice::WriteOnly)) return;

    for (int i=0;i<StaList->count();i++)
    {
        fp.write((StaList->item(i)->text()+"\n").toLatin1());
    }
}
//---------------------------------------------------------------------------
