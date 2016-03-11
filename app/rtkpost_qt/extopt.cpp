//---------------------------------------------------------------------------

#include "extopt.h"
#include "postopt.h"

#include <QShowEvent>
//---------------------------------------------------------------------------
ExtOptDialog::ExtOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(ExtEna0,SIGNAL(clicked(bool)),this,SLOT(ExtEna0Click()));
    connect(ExtEna1,SIGNAL(clicked(bool)),this,SLOT(ExtEna1Click()));
    connect(ExtEna2,SIGNAL(clicked(bool)),this,SLOT(ExtEna2Click()));
    connect(ExtEna3,SIGNAL(clicked(bool)),this,SLOT(ExtEna3Click()));
}
//---------------------------------------------------------------------------
void ExtOptDialog::showEvent(QShowEvent* event)
{
    if (event->spontaneous()) return;

	GetExtErrOpt();

	UpdateEnable();
}
//---------------------------------------------------------------------------
void ExtOptDialog::BtnOkClick()
{
	SetExtErrOpt();
}
//---------------------------------------------------------------------------
void ExtOptDialog::ExtEna0Click()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void ExtOptDialog::ExtEna1Click()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void ExtOptDialog::ExtEna3Click()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void ExtOptDialog::ExtEna2Click()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void ExtOptDialog::GetExtErrOpt(void)
{
    QLineEdit *editc[][6]={
		{CodeErr00,CodeErr01,CodeErr02,CodeErr03,CodeErr04,CodeErr05},
		{CodeErr10,CodeErr11,CodeErr12,CodeErr13,CodeErr14,CodeErr15},
		{CodeErr20,CodeErr21,CodeErr22,CodeErr23,CodeErr24,CodeErr25}
	};
    QLineEdit *editp[][6]={
		{PhaseErr00,PhaseErr01,PhaseErr02,PhaseErr03,PhaseErr04,PhaseErr05},
		{PhaseErr10,PhaseErr11,PhaseErr12,PhaseErr13,PhaseErr14,PhaseErr15},
		{PhaseErr20,PhaseErr21,PhaseErr22,PhaseErr23,PhaseErr24,PhaseErr25}
	};
	
    ExtEna0->setChecked(optDialog->ExtErr.ena[0]);
    ExtEna1->setChecked(optDialog->ExtErr.ena[1]);
    ExtEna2->setChecked(optDialog->ExtErr.ena[2]);
    ExtEna3->setChecked(optDialog->ExtErr.ena[3]);
	
	for (int i=0;i<3;i++) for (int j=0;j<6;j++) {
        editc[i][j]->setText(QString::number(optDialog->ExtErr.cerr[i][j],'f',3));
        editp[i][j]->setText(QString::number(optDialog->ExtErr.perr[i][j],'f',3));
	}
    GpsGloB0->setText(QString::number(optDialog->ExtErr.gpsglob[0],'f',3));
    GpsGloB1->setText(QString::number(optDialog->ExtErr.gpsglob[1],'f',3));
    GloICB0->setText(QString::number(optDialog->ExtErr.gloicb[0],'f',3));
    GloICB1->setText(QString::number(optDialog->ExtErr.gloicb[1],'f',3));
}
//---------------------------------------------------------------------------
void ExtOptDialog::SetExtErrOpt(void)
{
    QLineEdit *editc[][6]={
		{CodeErr00,CodeErr01,CodeErr02,CodeErr03,CodeErr04,CodeErr05},
		{CodeErr10,CodeErr11,CodeErr12,CodeErr13,CodeErr14,CodeErr15},
		{CodeErr20,CodeErr21,CodeErr22,CodeErr23,CodeErr24,CodeErr25}
	};
    QLineEdit *editp[][6]={
		{PhaseErr00,PhaseErr01,PhaseErr02,PhaseErr03,PhaseErr04,PhaseErr05},
		{PhaseErr10,PhaseErr11,PhaseErr12,PhaseErr13,PhaseErr14,PhaseErr15},
		{PhaseErr20,PhaseErr21,PhaseErr22,PhaseErr23,PhaseErr24,PhaseErr25}
	};
    optDialog->ExtErr.ena[0]=ExtEna0->isChecked();
    optDialog->ExtErr.ena[1]=ExtEna1->isChecked();
    optDialog->ExtErr.ena[2]=ExtEna2->isChecked();
    optDialog->ExtErr.ena[3]=ExtEna3->isChecked();
	
	for (int i=0;i<3;i++) for (int j=0;j<6;j++) {
        optDialog->ExtErr.cerr[i][j]=editc[i][j]->text().toDouble();
        optDialog->ExtErr.perr[i][j]=editp[i][j]->text().toDouble();
	}
    optDialog->ExtErr.gloicb[0]=GloICB0->text().toDouble();
    optDialog->ExtErr.gloicb[1]=GloICB1->text().toDouble();
    optDialog->ExtErr.gpsglob[0]=GpsGloB0->text().toDouble();
    optDialog->ExtErr.gpsglob[1]=GpsGloB1->text().toDouble();
}
//---------------------------------------------------------------------------
void ExtOptDialog::UpdateEnable(void)
{
    QLineEdit *editc[][6]={
		{CodeErr00,CodeErr01,CodeErr02,CodeErr03,CodeErr04,CodeErr05},
		{CodeErr10,CodeErr11,CodeErr12,CodeErr13,CodeErr14,CodeErr15},
		{CodeErr20,CodeErr21,CodeErr22,CodeErr23,CodeErr24,CodeErr25}
	};
    QLineEdit *editp[][6]={
		{PhaseErr00,PhaseErr01,PhaseErr02,PhaseErr03,PhaseErr04,PhaseErr05},
		{PhaseErr10,PhaseErr11,PhaseErr12,PhaseErr13,PhaseErr14,PhaseErr15},
		{PhaseErr20,PhaseErr21,PhaseErr22,PhaseErr23,PhaseErr24,PhaseErr25}
	};
	for (int i=0;i<3;i++) for (int j=0;j<6;j++) {
        editc[i][j]->setEnabled(ExtEna0->isChecked());
        editp[i][j]->setEnabled(ExtEna1->isChecked());
	}
    GloICB0->setEnabled(ExtEna2->isChecked());
    GloICB1->setEnabled(ExtEna2->isChecked());
    GpsGloB0->setEnabled(ExtEna3->isChecked());
    GpsGloB1->setEnabled(ExtEna3->isChecked());
}
//---------------------------------------------------------------------------

