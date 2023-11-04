//---------------------------------------------------------------------------

#include <QFileDialog>
#include <QDir>

#include "rtklib.h"
#include "glofcndlg.h"

//---------------------------------------------------------------------------
GloFcnDialog::GloFcnDialog(QWidget* parent) : QDialog(parent)
{
    setupUi(this);
	EnaGloFcn=0;

	for (int i=0;i<27;i++) {
		GloFcn[i]=0;
	}

    connect(BtnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(BtnOk, SIGNAL(clicked(bool)), this, SLOT(BtnOkClick()));
    connect(BtnClear, SIGNAL(clicked(bool)), this, SLOT(BtnClearClick()));
    connect(BtnRead, SIGNAL(clicked(bool)), this, SLOT(BtnReadClick()));
    connect(EnaFcn, SIGNAL(stateChanged(int)), this, SLOT(UpdateEnable()));
}
//---------------------------------------------------------------------------

void GloFcnDialog::showEvent(QShowEvent*)
{
    QString text;

    EnaFcn->setChecked(EnaGloFcn);

	for (int i=0;i<27;i++) {
        if (GloFcn[i]) GetFcn(i+1)->setText(QString::number(GloFcn[i]-8));
        else GetFcn(i+1)->setText("");
	}
    UpdateEnable();
}
//---------------------------------------------------------------------------
void GloFcnDialog::BtnOkClick()
{
    QString text;
	int no;
    bool ok;
	
    EnaGloFcn=EnaFcn->isChecked();
	
	for (int i=0;i<27;i++) {
        text=GetFcn(i+1)->text();
        no = text.toInt(&ok);
        if (ok&&no>=-7&&no<=6) {
			GloFcn[i]=no+8;
		}
		else GloFcn[i]=0; // GLONASS FCN+8 (0:none)
	}
    accept();
}
//---------------------------------------------------------------------------
void GloFcnDialog::BtnReadClick()
{
    QString file,text;
    nav_t nav;
	int prn;

    memset(&nav, 0, sizeof(nav_t));

    file=QFileDialog::getOpenFileName(this);
    
    if (!readrnx(qPrintable(file),0,"",NULL,&nav,NULL)) return;
	
	for (int i=0;i<nav.ng;i++) {
		if (satsys(nav.geph[i].sat,&prn)!=SYS_GLO) continue;
        GetFcn(prn)->setText(QString::number(nav.geph[i].frq));
	}
	freenav(&nav,0xFF);
}
//---------------------------------------------------------------------------
void GloFcnDialog::BtnClearClick()
{
	for (int i=0;i<27;i++) {
        GetFcn(i+1)->setText("");
	}
}
//---------------------------------------------------------------------------
void GloFcnDialog::EnaFcnClick()
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void GloFcnDialog::UpdateEnable(void)
{
    BtnClear->setEnabled(EnaFcn->isChecked());
    BtnRead ->setEnabled(EnaFcn->isChecked());
    Label21 ->setEnabled(EnaFcn->isChecked());
    Label22 ->setEnabled(EnaFcn->isChecked());
	
	for (int i=0;i<27;i++) {
        GetFcn(i+1)->setEnabled(EnaFcn->isChecked());
	}
}
//---------------------------------------------------------------------------
QLineEdit * GloFcnDialog::GetFcn(int prn)
{
    QLineEdit *fcn[]={
		Fcn01,Fcn02,Fcn03,Fcn04,Fcn05,Fcn06,Fcn07,Fcn08,Fcn09,Fcn10,
		Fcn11,Fcn12,Fcn13,Fcn14,Fcn15,Fcn16,Fcn17,Fcn18,Fcn19,Fcn20,
		Fcn21,Fcn22,Fcn23,Fcn24,Fcn25,Fcn26,Fcn27
	};
	return fcn[prn-1];
}
//---------------------------------------------------------------------------

