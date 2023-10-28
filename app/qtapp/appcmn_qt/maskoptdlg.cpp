//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann

#include <QShowEvent>

#include "maskoptdlg.h"
//---------------------------------------------------------------------------
MaskOptDialog::MaskOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    Mask.ena[0] = 0;
    Mask.ena[1] = 0;
    for (int i = 0; i < 3; i++) for (int j = 0; j < 9; j++)
            Mask.mask[i][j] = 0.0;

    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(btnOkClicked()));
    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(cBMaskEnabled1, SIGNAL(clicked(bool)), this, SLOT(MaskEnabledClicked()));
    connect(cBMaskEnabled2, SIGNAL(clicked(bool)), this, SLOT(MaskEnabledClicked()));
}
//---------------------------------------------------------------------------
void MaskOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    QDoubleSpinBox *mask[][9] = {
        { sBMask_1_1, sBMask_1_2, sBMask_1_3, sBMask_1_4, sBMask_1_5, sBMask_1_6, sBMask_1_7, sBMask_1_8, sBMask_1_9 },
        { sBMask_2_1, sBMask_2_2, sBMask_2_3, sBMask_2_4, sBMask_2_5, sBMask_2_6, sBMask_2_7, sBMask_2_8, sBMask_2_9 },
        { sBMask_3_1, sBMask_3_2, sBMask_3_3, sBMask_3_4, sBMask_3_5, sBMask_3_6, sBMask_3_7, sBMask_3_8, sBMask_3_9 }
    };

    cBMaskEnabled1->setChecked(Mask.ena[0]);
    cBMaskEnabled2->setChecked(Mask.ena[1]);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 9; j++) {
            mask[i][j]->setValue(Mask.mask[i][j]);
        }
	}

	updateEnable();
}
//---------------------------------------------------------------------------
void MaskOptDialog::btnOkClicked()
{
    QDoubleSpinBox *mask[][9] = {
        { sBMask_1_1, sBMask_1_2, sBMask_1_3, sBMask_1_4, sBMask_1_5, sBMask_1_6, sBMask_1_7, sBMask_1_8, sBMask_1_9 },
        { sBMask_2_1, sBMask_2_2, sBMask_2_3, sBMask_2_4, sBMask_2_5, sBMask_2_6, sBMask_2_7, sBMask_2_8, sBMask_2_9 },
        { sBMask_3_1, sBMask_3_2, sBMask_3_3, sBMask_3_4, sBMask_3_5, sBMask_3_6, sBMask_3_7, sBMask_3_8, sBMask_3_9 }
	};

    Mask.ena[0] = cBMaskEnabled1->isChecked();
    Mask.ena[1] = cBMaskEnabled2->isChecked();
    for (int i = 0; i < 3; i++) for (int j = 0; j < 9; j++)
            Mask.mask[i][j] = mask[i][j]->value();


    accept();
}
//---------------------------------------------------------------------------
void MaskOptDialog::MaskEnabledClicked()
{
	updateEnable();
}
//---------------------------------------------------------------------------
void MaskOptDialog::updateEnable(void)
{
    QDoubleSpinBox *mask[][9] = {
        { sBMask_1_1, sBMask_1_2, sBMask_1_3, sBMask_1_4, sBMask_1_5, sBMask_1_6, sBMask_1_7, sBMask_1_8, sBMask_1_9 },
        { sBMask_2_1, sBMask_2_2, sBMask_2_3, sBMask_2_4, sBMask_2_5, sBMask_2_6, sBMask_2_7, sBMask_2_8, sBMask_2_9 },
        { sBMask_3_1, sBMask_3_2, sBMask_3_3, sBMask_3_4, sBMask_3_5, sBMask_3_6, sBMask_3_7, sBMask_3_8, sBMask_3_9 }
	};

    for (int i = 0; i < 3; i++) for (int j = 0; j < 9; j++)
            mask[i][j]->setEnabled(cBMaskEnabled1->isChecked() || cBMaskEnabled2->isChecked());

}
//---------------------------------------------------------------------------
