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
        SrcTbl1,SrcTbl2,SrcTbl3,NULL,SrcTbl5,SrcTbl6,SrcTbl7,SrcTbl8,SrcTbl9,
        NULL,NULL,SrcTbl12,SrcTbl13,NULL,NULL,SrcTbl16
	};
    QComboBox *box[]={
        NULL,NULL,NULL,SrcTbl4,NULL,NULL,NULL,NULL,NULL,SrcTbl10,SrcTbl11,NULL,
		NULL,SrcTbl14,SrcTbl15,NULL
	};
    
    MntPntE->setText(MntPnt);

    QStringList tokens=MntpStr.split(";");
	
	for (int i=0;i<16;i++) {
        if (edit[i]==NULL) edit[i]->setText(""); else box[i]->setCurrentIndex(0);
	}

    for (int i=0;i<tokens.size()&& i<16;i++) {
        if (edit[i]) edit[i]->setText(tokens.at(i)); else box[i]->setCurrentText(tokens.at(i));
	}
}
//---------------------------------------------------------------------------
void MntpOptDialog::BtnOkClick()
{
    MntPnt=MntPntE->text();
    MntpStr=SrcTbl1->text()+";"+SrcTbl2->text()+";"+SrcTbl3->text()+";"+SrcTbl4->currentText()+";"+
            SrcTbl5->text()+";"+SrcTbl6->text()+";"+SrcTbl7->text()+";"+SrcTbl8->text()+";"+
            SrcTbl9->text()+";"+SrcTbl10->currentText()+";"+SrcTbl11->currentText()+";"+SrcTbl12->text()+";"+
            SrcTbl13->text()+";"+SrcTbl14->currentText()+";"+SrcTbl15->currentText()+";"+SrcTbl16->text();
}
//---------------------------------------------------------------------------
