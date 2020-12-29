//---------------------------------------------------------------------------

#include "satdlg.h"

#include <QShowEvent>

//---------------------------------------------------------------------------
 SatDialog::SatDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

	for (int i=0;i<36;i++) ValidSat[i]=1;

    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(BtnCancelClick()));
    connect(BtnChkAll,SIGNAL(clicked(bool)),this,SLOT(BtnChkAllClick()));
    connect(BtnUnchkAll,SIGNAL(clicked(bool)),this,SLOT(BtnUnchkAllClick()));
}
//---------------------------------------------------------------------------
void  SatDialog::showEvent(QShowEvent *event)
{
    QCheckBox *sat[]={
		PRN01,PRN02,PRN03,PRN04,PRN05,PRN06,PRN07,PRN08,PRN09,PRN10,
		PRN11,PRN12,PRN13,PRN14,PRN15,PRN16,PRN17,PRN18,PRN19,PRN20,
		PRN21,PRN22,PRN23,PRN24,PRN25,PRN26,PRN27,PRN28,PRN29,PRN30,
		PRN31,PRN32,SBAS,GLO,GAL,PRN33
	};

    if (event->spontaneous()) return;

    for (int i=0;i<36;i++) sat[i]->setChecked(ValidSat[i]);
}
//---------------------------------------------------------------------------
void  SatDialog::BtnChkAllClick()
{
    QCheckBox *sat[]={
		PRN01,PRN02,PRN03,PRN04,PRN05,PRN06,PRN07,PRN08,PRN09,PRN10,
		PRN11,PRN12,PRN13,PRN14,PRN15,PRN16,PRN17,PRN18,PRN19,PRN20,
		PRN21,PRN22,PRN23,PRN24,PRN25,PRN26,PRN27,PRN28,PRN29,PRN30,
		PRN31,PRN32,SBAS,GLO,GAL,PRN33
	};
    for (int i=0;i<36;i++) sat[i]->setChecked(true);
}
//---------------------------------------------------------------------------
void  SatDialog::BtnUnchkAllClick()
{
    QCheckBox *sat[]={
		PRN01,PRN02,PRN03,PRN04,PRN05,PRN06,PRN07,PRN08,PRN09,PRN10,
		PRN11,PRN12,PRN13,PRN14,PRN15,PRN16,PRN17,PRN18,PRN19,PRN20,
		PRN21,PRN22,PRN23,PRN24,PRN25,PRN26,PRN27,PRN28,PRN29,PRN30,
		PRN31,PRN32,SBAS,GLO,GAL,PRN33
	};
    for (int i=0;i<36;i++) sat[i]->setChecked(false);
}
//---------------------------------------------------------------------------
void  SatDialog::BtnOkClick()
{
    QCheckBox *sat[]={
		PRN01,PRN02,PRN03,PRN04,PRN05,PRN06,PRN07,PRN08,PRN09,PRN10,
		PRN11,PRN12,PRN13,PRN14,PRN15,PRN16,PRN17,PRN18,PRN19,PRN20,
		PRN21,PRN22,PRN23,PRN24,PRN25,PRN26,PRN27,PRN28,PRN29,PRN30,
		PRN31,PRN32,SBAS,GLO,GAL,PRN33
	};
    for (int i=0;i<36;i++) ValidSat[i]=sat[i]->isChecked();
    accept();
}
//---------------------------------------------------------------------------
void  SatDialog::BtnCancelClick()
{
    reject();
}
//---------------------------------------------------------------------------
