//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <QShowEvent>
#include <QDoubleValidator>

#include "maskoptdlg.h"
//---------------------------------------------------------------------------
MaskOptDialog::MaskOptDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

	Mask.ena[0]=0;
	Mask.ena[1]=0;
	for (int i=0;i<3;i++) for (int j=0;j<9;j++) {
		Mask.mask[i][j]=0.0;
	}

    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(MaskEna1,SIGNAL(clicked(bool)),this,SLOT(MaskEnaClick()));
    connect(MaskEna2,SIGNAL(clicked(bool)),this,SLOT(MaskEnaClick()));
}
//---------------------------------------------------------------------------
void  MaskOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    QLineEdit *mask[][9]={
		{Mask_1_1,Mask_1_2,Mask_1_3,Mask_1_4,Mask_1_5,Mask_1_6,Mask_1_7,Mask_1_8,Mask_1_9},
		{Mask_2_1,Mask_2_2,Mask_2_3,Mask_2_4,Mask_2_5,Mask_2_6,Mask_2_7,Mask_2_8,Mask_2_9},
		{Mask_3_1,Mask_3_2,Mask_3_3,Mask_3_4,Mask_3_5,Mask_3_6,Mask_3_7,Mask_3_8,Mask_3_9}
	};    

    MaskEna1->setChecked(Mask.ena[0]);
    MaskEna2->setChecked(Mask.ena[1]);

    for (int i=0;i<3;i++)
    {
        for (int j=0;j<9;j++) {
            mask[i][j]->setValidator(new QDoubleValidator(-90,90,1));
            mask[i][j]->setText(QString::number(Mask.mask[i][j]));
        }
	}

	UpdateEnable();
}
//---------------------------------------------------------------------------
void  MaskOptDialog::BtnOkClick()
{
    QLineEdit *mask[][9]={
		{Mask_1_1,Mask_1_2,Mask_1_3,Mask_1_4,Mask_1_5,Mask_1_6,Mask_1_7,Mask_1_8,Mask_1_9},
		{Mask_2_1,Mask_2_2,Mask_2_3,Mask_2_4,Mask_2_5,Mask_2_6,Mask_2_7,Mask_2_8,Mask_2_9},
		{Mask_3_1,Mask_3_2,Mask_3_3,Mask_3_4,Mask_3_5,Mask_3_6,Mask_3_7,Mask_3_8,Mask_3_9}
	};
    Mask.ena[0]=MaskEna1->isChecked();
    Mask.ena[1]=MaskEna2->isChecked();
	for (int i=0;i<3;i++) for (int j=0;j<9;j++) {
        Mask.mask[i][j]=mask[i][j]->text().toDouble();
	}

    accept();
}
//---------------------------------------------------------------------------
void  MaskOptDialog::MaskEnaClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void  MaskOptDialog::UpdateEnable(void)
{
    QLineEdit *mask[][9]={
		{Mask_1_1,Mask_1_2,Mask_1_3,Mask_1_4,Mask_1_5,Mask_1_6,Mask_1_7,Mask_1_8,Mask_1_9},
		{Mask_2_1,Mask_2_2,Mask_2_3,Mask_2_4,Mask_2_5,Mask_2_6,Mask_2_7,Mask_2_8,Mask_2_9},
		{Mask_3_1,Mask_3_2,Mask_3_3,Mask_3_4,Mask_3_5,Mask_3_6,Mask_3_7,Mask_3_8,Mask_3_9}
	};
	for (int i=0;i<3;i++) for (int j=0;j<9;j++) {
        mask[i][j]->setEnabled(MaskEna1->isChecked()||MaskEna2->isChecked());
	}
}
//---------------------------------------------------------------------------

