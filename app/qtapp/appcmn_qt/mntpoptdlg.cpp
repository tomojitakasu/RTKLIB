//---------------------------------------------------------------------------
#include <stdio.h>

#include <QShowEvent>

#include "rtklib.h"
#include "mntpoptdlg.h"
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
MntpOptDialog::MntpOptDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
}
//---------------------------------------------------------------------------
void MntpOptDialog::showEvent(QShowEvent* event)
{
    if (event->spontaneous()) return;
    QLineEdit *edit[]={
        lESourceTable1, lESourceTable2, lESourceTable3, NULL, lESourceTable5,
        lESourceTable6, lESourceTable7, lESourceTable8, lESourceTable9,
        NULL, NULL, lESourceTable12, lESourceTable13, NULL, NULL, lESourceTable16
    };
    QComboBox *box[]={
        NULL, NULL, NULL, lESourceTable4, NULL, NULL, NULL, NULL, NULL, lESourceTable10,
        lESourceTable11, NULL, NULL, lESourceTable14, lESourceTable15, NULL
    };
    
    lEMountPoint->setText(mountPoint);

    QStringList tokens=MntpStr.split(";");

    for (int i=0;i<16;i++) {
        if (edit[i]==NULL) edit[i]->setText(""); else box[i]->setCurrentIndex(0);
    }

    for (int i=0;i<tokens.size()&& i<16;i++) {
        if (edit[i]) edit[i]->setText(tokens.at(i)); else box[i]->setCurrentText(tokens.at(i));
    }
}
//---------------------------------------------------------------------------
void MntpOptDialog::btnOkClicked()
{
    mountPoint = lEMountPoint->text();
    MntpStr = lESourceTable1->text()+";"+lESourceTable2->text()+";"+lESourceTable3->text()+";"+lESourceTable4->currentText()+";"+
              lESourceTable5->text()+";"+lESourceTable6->text()+";"+lESourceTable7->text()+";"+lESourceTable8->text()+";"+
              lESourceTable9->text()+";"+lESourceTable10->currentText()+";"+lESourceTable11->currentText()+";"+lESourceTable12->text()+";"+
              lESourceTable13->text()+";"+lESourceTable14->currentText()+";"+lESourceTable15->currentText()+";"+lESourceTable16->text();
}
//---------------------------------------------------------------------------

