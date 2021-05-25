//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QDebug>

#include "rtklib.h"
#include "convopt.h"
#include "codeopt.h"
//---------------------------------------------------------------------------
CodeOptDialog::CodeOptDialog(QWidget *parent, ConvOptDialog *c)
    : QDialog(parent), convOptDialog(c)
{
    setupUi(this);

    connect(BtnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(BtnOk, SIGNAL(clicked(bool)), this, SLOT(BtnOkClick()));
    connect(BtnSetAll, SIGNAL(clicked(bool)), this, SLOT(BtnSetAllClick()));
}
//---------------------------------------------------------------------------
void CodeOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    char mask[7][MAXCODE+1] = { "" };

    for (int i = 0; i < 7; i++) strcpy(mask[i], qPrintable(convOptDialog->CodeMask[i]));

    G01->setChecked(mask[0][0] == '1');
    G02->setChecked(mask[0][1] == '1');
    G03->setChecked(mask[0][2] == '1');
    G04->setChecked(mask[0][3] == '1');
    G05->setChecked(mask[0][4] == '1');
    G06->setChecked(mask[0][5] == '1');
    G07->setChecked(mask[0][6] == '1');
    G08->setChecked(mask[0][7] == '1');
    G14->setChecked(mask[0][13] == '1');
    G15->setChecked(mask[0][14] == '1');
    G16->setChecked(mask[0][15] == '1');
    G17->setChecked(mask[0][16] == '1');
    G18->setChecked(mask[0][17] == '1');
    G19->setChecked(mask[0][18] == '1');
    G20->setChecked(mask[0][19] == '1');
    G21->setChecked(mask[0][20] == '1');
    G22->setChecked(mask[0][21] == '1');
    G23->setChecked(mask[0][22] == '1');
    G24->setChecked(mask[0][23] == '1');
    G25->setChecked(mask[0][24] == '1');
    G26->setChecked(mask[0][25] == '1');
    R01->setChecked(mask[1][0] == '1');
    R02->setChecked(mask[1][1] == '1');
    R14->setChecked(mask[1][13] == '1');
    R19->setChecked(mask[1][18] == '1');
    R44->setChecked(mask[1][43] == '1');
    R45->setChecked(mask[1][44] == '1');
    R46->setChecked(mask[1][45] == '1');
    R66->setChecked(mask[1][65] == '1');
    R67->setChecked(mask[1][66] == '1');
    R68->setChecked(mask[1][67] == '1');
    R30->setChecked(mask[1][29] == '1');
    R31->setChecked(mask[1][30] == '1');
    R33->setChecked(mask[1][32] == '1');
    E01->setChecked(mask[2][0] == '1');
    E10->setChecked(mask[2][9] == '1');
    E11->setChecked(mask[2][10] == '1');
    E12->setChecked(mask[2][11] == '1');
    E13->setChecked(mask[2][12] == '1');
    E24->setChecked(mask[2][23] == '1');
    E25->setChecked(mask[2][24] == '1');
    E26->setChecked(mask[2][25] == '1');
    E27->setChecked(mask[2][26] == '1');
    E28->setChecked(mask[2][27] == '1');
    E29->setChecked(mask[2][28] == '1');
    E30->setChecked(mask[2][29] == '1');
    E31->setChecked(mask[2][30] == '1');
    E32->setChecked(mask[2][31] == '1');
    E33->setChecked(mask[2][32] == '1');
    E34->setChecked(mask[2][33] == '1');
    E37->setChecked(mask[2][36] == '1');
    E38->setChecked(mask[2][37] == '1');
    E39->setChecked(mask[2][38] == '1');
    J01->setChecked(mask[3][0] == '1');
    J07->setChecked(mask[3][6] == '1');
    J08->setChecked(mask[3][7] == '1');
    J13->setChecked(mask[3][12] == '1');
    J12->setChecked(mask[3][11] == '1');
    J16->setChecked(mask[3][15] == '1');
    J17->setChecked(mask[3][16] == '1');
    J18->setChecked(mask[3][17] == '1');
    J24->setChecked(mask[3][23] == '1');
    J25->setChecked(mask[3][24] == '1');
    J26->setChecked(mask[3][25] == '1');
    J57->setChecked(mask[3][56] == '1');
    J58->setChecked(mask[3][57] == '1');
    J59->setChecked(mask[3][58] == '1');
    J60->setChecked(mask[3][59] == '1');
    J34->setChecked(mask[3][33] == '1');
    J35->setChecked(mask[3][34] == '1');
    J36->setChecked(mask[3][35] == '1');
    J33->setChecked(mask[3][32] == '1');
    C40->setChecked(mask[5][39] == '1');
    C41->setChecked(mask[5][40] == '1');
    C18->setChecked(mask[5][17] == '1');
    C27->setChecked(mask[5][26] == '1');
    C28->setChecked(mask[5][27] == '1');
    C29->setChecked(mask[5][28] == '1');
    C42->setChecked(mask[5][41] == '1');
    C43->setChecked(mask[5][42] == '1');
    C33->setChecked(mask[5][32] == '1');
    C56->setChecked(mask[5][55] == '1'); //
    C02->setChecked(mask[5][ 1] == '1'); //
    C12->setChecked(mask[5][11] == '1'); //
    C10->setChecked(mask[5][ 9] == '1'); //
    C06->setChecked(mask[5][ 5] == '1'); //
    C57->setChecked(mask[5][56] == '1'); //
    C58->setChecked(mask[5][57] == '1'); //
    C26->setChecked(mask[5][25] == '1'); //
    C61->setChecked(mask[5][60] == '1'); //
    C62->setChecked(mask[5][61] == '1'); //
    C63->setChecked(mask[5][62] == '1'); //
    C64->setChecked(mask[5][63] == '1'); //
    C65->setChecked(mask[5][64] == '1'); //
    C39->setChecked(mask[5][38] == '1'); //
    C30->setChecked(mask[5][29] == '1'); //
    I49->setChecked(mask[6][48] == '1');
    I50->setChecked(mask[6][49] == '1');
    I51->setChecked(mask[6][50] == '1');
    I26->setChecked(mask[6][25] == '1');
    I52->setChecked(mask[6][51] == '1');
    I53->setChecked(mask[6][52] == '1');
    I54->setChecked(mask[6][53] == '1');
    I55->setChecked(mask[6][54] == '1');
    S01->setChecked(mask[4][0] == '1');
    S24->setChecked(mask[4][23] == '1');
    S25->setChecked(mask[4][24] == '1');
    S26->setChecked(mask[4][25] == '1');

	UpdateEnable();
}
//---------------------------------------------------------------------------
void CodeOptDialog::BtnOkClick()
{
    char mask[7][MAXCODE+1] = { "" };

    for (int i = 0; i < 7; i++)
        for (int j = 0; j < MAXCODE; j++) mask[i][j] = '0';

    if (G01->isChecked()) mask[0][0] = '1';
    if (G02->isChecked()) mask[0][1] = '1';
    if (G03->isChecked()) mask[0][2] = '1';
    if (G04->isChecked()) mask[0][3] = '1';
    if (G05->isChecked()) mask[0][4] = '1';
    if (G06->isChecked()) mask[0][5] = '1';
    if (G07->isChecked()) mask[0][6] = '1';
    if (G08->isChecked()) mask[0][7] = '1';
    if (G14->isChecked()) mask[0][13] = '1';
    if (G15->isChecked()) mask[0][14] = '1';
    if (G16->isChecked()) mask[0][15] = '1';
    if (G17->isChecked()) mask[0][16] = '1';
    if (G18->isChecked()) mask[0][17] = '1';
    if (G19->isChecked()) mask[0][18] = '1';
    if (G20->isChecked()) mask[0][19] = '1';
    if (G21->isChecked()) mask[0][20] = '1';
    if (G22->isChecked()) mask[0][21] = '1';
    if (G23->isChecked()) mask[0][22] = '1';
    if (G24->isChecked()) mask[0][23] = '1';
    if (G25->isChecked()) mask[0][24] = '1';
    if (G26->isChecked()) mask[0][25] = '1';
    if (R01->isChecked()) mask[1][0] = '1';
    if (R02->isChecked()) mask[1][1] = '1';
    if (R14->isChecked()) mask[1][13] = '1';
    if (R19->isChecked()) mask[1][18] = '1';
    if (R44->isChecked()) mask[1][43] = '1';
    if (R45->isChecked()) mask[1][44] = '1';
    if (R46->isChecked()) mask[1][45] = '1';
    if (R66->isChecked()) mask[1][65] = '1'; //
    if (R67->isChecked()) mask[1][66] = '1'; //
    if (R68->isChecked()) mask[1][67] = '1'; //
    if (R30->isChecked()) mask[1][29] = '1'; //
    if (R31->isChecked()) mask[1][30] = '1'; //
    if (R33->isChecked()) mask[1][32] = '1'; //
    if (E01->isChecked()) mask[2][0] = '1';
    if (E10->isChecked()) mask[2][9] = '1';
    if (E11->isChecked()) mask[2][10] = '1';
    if (E12->isChecked()) mask[2][11] = '1';
    if (E13->isChecked()) mask[2][12] = '1';
    if (E24->isChecked()) mask[2][23] = '1';
    if (E25->isChecked()) mask[2][24] = '1';
    if (E26->isChecked()) mask[2][25] = '1';
    if (E27->isChecked()) mask[2][26] = '1';
    if (E28->isChecked()) mask[2][27] = '1';
    if (E29->isChecked()) mask[2][28] = '1';
    if (E30->isChecked()) mask[2][29] = '1';
    if (E31->isChecked()) mask[2][30] = '1';
    if (E32->isChecked()) mask[2][31] = '1';
    if (E33->isChecked()) mask[2][32] = '1';
    if (E34->isChecked()) mask[2][33] = '1';
    if (E37->isChecked()) mask[2][36] = '1';
    if (E38->isChecked()) mask[2][37] = '1';
    if (E39->isChecked()) mask[2][38] = '1';
    if (J01->isChecked()) mask[3][0] = '1';
    if (J07->isChecked()) mask[3][6] = '1';
    if (J08->isChecked()) mask[3][7] = '1';
    if (J13->isChecked()) mask[3][12] = '1';
    if (J12->isChecked()) mask[3][11] = '1';
    if (J16->isChecked()) mask[3][15] = '1';
    if (J17->isChecked()) mask[3][16] = '1';
    if (J18->isChecked()) mask[3][17] = '1';
    if (J24->isChecked()) mask[3][23] = '1';
    if (J25->isChecked()) mask[3][24] = '1';
    if (J26->isChecked()) mask[3][25] = '1';
    if (J57->isChecked()) mask[3][56] = '1'; //
    if (J58->isChecked()) mask[3][57] = '1'; //
    if (J59->isChecked()) mask[3][58] = '1'; //
    if (J60->isChecked()) mask[3][59] = '1'; //
    if (J34->isChecked()) mask[3][33] = '1'; //
    if (J35->isChecked()) mask[3][34] = '1';
    if (J36->isChecked()) mask[3][35] = '1';
    if (J33->isChecked()) mask[3][32] = '1';
    if (C40->isChecked()) mask[5][39] = '1';
    if (C41->isChecked()) mask[5][40] = '1';
    if (C18->isChecked()) mask[5][17] = '1';
    if (C27->isChecked()) mask[5][26] = '1';
    if (C28->isChecked()) mask[5][27] = '1';
    if (C29->isChecked()) mask[5][28] = '1';
    if (C42->isChecked()) mask[5][41] = '1';
    if (C43->isChecked()) mask[5][42] = '1';
    if (C33->isChecked()) mask[5][32] = '1';
    if (C56->isChecked()) mask[5][55] = '1'; //
    if (C02->isChecked()) mask[5][ 1] = '1'; //
    if (C12->isChecked()) mask[5][11] = '1'; //
    if (C10->isChecked()) mask[5][ 9] = '1'; //
    if (C06->isChecked()) mask[5][ 5] = '1'; //
    if (C57->isChecked()) mask[5][56] = '1'; //
    if (C58->isChecked()) mask[5][57] = '1'; //
    if (C26->isChecked()) mask[5][25] = '1'; //
    if (C61->isChecked()) mask[5][60] = '1'; //
    if (C62->isChecked()) mask[5][61] = '1'; //
    if (C63->isChecked()) mask[5][62] = '1'; //
    if (C64->isChecked()) mask[5][63] = '1'; //
    if (C65->isChecked()) mask[5][64] = '1'; //
    if (C39->isChecked()) mask[5][38] = '1'; //
    if (C30->isChecked()) mask[5][29] = '1'; //
    if (I49->isChecked()) mask[6][48] = '1';
    if (I50->isChecked()) mask[6][49] = '1';
    if (I51->isChecked()) mask[6][50] = '1';
    if (I26->isChecked()) mask[6][25] = '1';
    if (I52->isChecked()) mask[6][51] = '1';
    if (I53->isChecked()) mask[6][52] = '1';
    if (I54->isChecked()) mask[6][53] = '1';
    if (I55->isChecked()) mask[6][54] = '1';
    if (S01->isChecked()) mask[4][0] = '1';
    if (S24->isChecked()) mask[4][23] = '1';
    if (S25->isChecked()) mask[4][24] = '1';
    if (S26->isChecked()) mask[4][25] = '1';

    for (int i = 0; i < 7; i++) convOptDialog->CodeMask[i] = mask[i];

    accept();
}
//---------------------------------------------------------------------------
void CodeOptDialog::BtnSetAllClick()
{
    bool set = (BtnSetAll->text() == tr("&Set All"));

    G01->setChecked(set);
    G02->setChecked(set);
    G03->setChecked(set);
    G04->setChecked(set);
    G05->setChecked(set);
    G06->setChecked(set);
    G07->setChecked(set);
    G08->setChecked(set);
    G14->setChecked(set);
    G15->setChecked(set);
    G16->setChecked(set);
    G17->setChecked(set);
    G18->setChecked(set);
    G19->setChecked(set);
    G20->setChecked(set);
    G21->setChecked(set);
    G22->setChecked(set);
    G23->setChecked(set);
    G24->setChecked(set);
    G25->setChecked(set);
    G26->setChecked(set);
    R01->setChecked(set);
    R02->setChecked(set);
    R14->setChecked(set);
    R19->setChecked(set);
    R44->setChecked(set);
    R45->setChecked(set);
    R46->setChecked(set);
    R66->setChecked(set);
    R67->setChecked(set);
    R68->setChecked(set);
    R30->setChecked(set);
    R31->setChecked(set);
    R33->setChecked(set);
    E01->setChecked(set);
    E10->setChecked(set);
    E11->setChecked(set);
    E12->setChecked(set);
    E13->setChecked(set);
    E24->setChecked(set);
    E25->setChecked(set);
    E26->setChecked(set);
    E27->setChecked(set);
    E28->setChecked(set);
    E29->setChecked(set);
    E30->setChecked(set);
    E31->setChecked(set);
    E32->setChecked(set);
    E33->setChecked(set);
    E34->setChecked(set);
    E37->setChecked(set);
    E38->setChecked(set);
    E39->setChecked(set);
    J01->setChecked(set);
    J07->setChecked(set);
    J08->setChecked(set);
    J13->setChecked(set);
    J12->setChecked(set);
    J16->setChecked(set);
    J17->setChecked(set);
    J18->setChecked(set);
    J24->setChecked(set);
    J25->setChecked(set);
    J26->setChecked(set);
    J57->setChecked(set);
    J58->setChecked(set);
    J59->setChecked(set);
    J60->setChecked(set);
    J34->setChecked(set);
    J35->setChecked(set);
    J36->setChecked(set);
    J33->setChecked(set);
    C40->setChecked(set);
    C41->setChecked(set);
    C18->setChecked(set);
    C27->setChecked(set);
    C28->setChecked(set);
    C29->setChecked(set);
    C42->setChecked(set);
    C43->setChecked(set);
    C33->setChecked(set);
    C56->setChecked(set);
    C02->setChecked(set);
    C12->setChecked(set);
    C10->setChecked(set);
    C06->setChecked(set);
    C57->setChecked(set);
    C58->setChecked(set);
    C26->setChecked(set);
    C61->setChecked(set);
    C62->setChecked(set);
    C63->setChecked(set);
    C64->setChecked(set);
    C65->setChecked(set);
    C39->setChecked(set);
    C30->setChecked(set);
    I49->setChecked(set);
    I50->setChecked(set);
    I26->setChecked(set);
    I51->setChecked(set);
    I52->setChecked(set);
    I53->setChecked(set);
    I54->setChecked(set);
    I55->setChecked(set);
    S01->setChecked(set);
    S24->setChecked(set);
    S25->setChecked(set);
    S26->setChecked(set);

    BtnSetAll->setText(BtnSetAll->text() == tr("&Set All") ? tr("Un&set All") : tr("&Set All"));
}
//---------------------------------------------------------------------------
void CodeOptDialog::UpdateEnable(void)
{
    G01->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L1));
    G02->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L1));
    G03->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L1));
    G04->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L1));
    G05->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L1));
    G06->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L1));
    G07->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L1));
    G08->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L1));
    G14->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L2));
    G15->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L2));
    G16->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L2));
    G17->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L2));
    G18->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L2));
    G19->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L2));
    G20->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L2));
    G21->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L2));
    G22->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L2));
    G23->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L2));
    G24->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L3));
    G25->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L3));
    G26->setEnabled((NavSys & SYS_GPS) && (FreqType & FREQTYPE_L3));
    R01->setEnabled((NavSys & SYS_GLO) && (FreqType & FREQTYPE_L1));
    R02->setEnabled((NavSys & SYS_GLO) && (FreqType & FREQTYPE_L1));
    R14->setEnabled((NavSys & SYS_GLO) && (FreqType & FREQTYPE_L2));
    R19->setEnabled((NavSys & SYS_GLO) && (FreqType & FREQTYPE_L2));
    R44->setEnabled((NavSys & SYS_GLO) && (FreqType & FREQTYPE_L3));
    R45->setEnabled((NavSys & SYS_GLO) && (FreqType & FREQTYPE_L3));
    R46->setEnabled((NavSys & SYS_GLO) && (FreqType & FREQTYPE_L3));
    R66->setEnabled((NavSys & SYS_GLO) && (FreqType & FREQTYPE_L1));
    R67->setEnabled((NavSys & SYS_GLO) && (FreqType & FREQTYPE_L1));
    R68->setEnabled((NavSys & SYS_GLO) && (FreqType & FREQTYPE_L1));
    R30->setEnabled((NavSys & SYS_GLO) && (FreqType & FREQTYPE_L2));
    R31->setEnabled((NavSys & SYS_GLO) && (FreqType & FREQTYPE_L2));
    R33->setEnabled((NavSys & SYS_GLO) && (FreqType & FREQTYPE_L2));
    E01->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L1));
    E10->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L1));
    E11->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L1));
    E12->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L1));
    E13->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L1));
    E24->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L3));
    E25->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L3));
    E26->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L3));
    E27->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L2));
    E28->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L2));
    E29->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L2));
    E30->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L4));
    E31->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L4));
    E32->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L4));
    E33->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L4));
    E34->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L4));
    E37->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L5));
    E38->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L5));
    E39->setEnabled((NavSys & SYS_GAL) && (FreqType & FREQTYPE_L5));
    J01->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L1));
    J07->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L1));
    J08->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L1));
    J13->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L1));
    J12->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L1));
    J16->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L2));
    J17->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L2));
    J18->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L2));
    J24->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L3));
    J25->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L3));
    J26->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L3));
    J57->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L3));
    J58->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L3));
    J59->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L3));
    J60->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L4));
    J34->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L4));
    J35->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L4));
    J36->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L4));
    J33->setEnabled((NavSys & SYS_QZS) && (FreqType & FREQTYPE_L4));
    C40->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L1));
    C41->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L1));
    C18->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L1));
    C27->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L2));
    C28->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L2));
    C29->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L2));
    C42->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L4));
    C43->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L4));
    C33->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L4));
    C56->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L1));
    C02->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L1));
    C12->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L1));
    C10->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L1));
    C06->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L1));
    C57->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L3));
    C58->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L3));
    C26->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L3));
    C61->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L2));
    C62->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L2));
    C63->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L2));
    C64->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L5));
    C65->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L5));
    C39->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L5));
    C30->setEnabled((NavSys & SYS_CMP) && (FreqType & FREQTYPE_L4));
    I49->setEnabled((NavSys & SYS_IRN) && (FreqType & FREQTYPE_L1));
    I50->setEnabled((NavSys & SYS_IRN) && (FreqType & FREQTYPE_L1));
    I51->setEnabled((NavSys & SYS_IRN) && (FreqType & FREQTYPE_L1));
    I26->setEnabled((NavSys & SYS_IRN) && (FreqType & FREQTYPE_L1));
    I52->setEnabled((NavSys & SYS_IRN) && (FreqType & FREQTYPE_L2));
    I53->setEnabled((NavSys & SYS_IRN) && (FreqType & FREQTYPE_L2));
    I54->setEnabled((NavSys & SYS_IRN) && (FreqType & FREQTYPE_L2));
    I55->setEnabled((NavSys & SYS_IRN) && (FreqType & FREQTYPE_L3));
    S01->setEnabled((NavSys & SYS_SBS) && (FreqType & FREQTYPE_L1));
    S24->setEnabled((NavSys & SYS_SBS) && (FreqType & FREQTYPE_L3));
    S25->setEnabled((NavSys & SYS_SBS) && (FreqType & FREQTYPE_L3));
    S26->setEnabled((NavSys & SYS_SBS) && (FreqType & FREQTYPE_L3));
}
//---------------------------------------------------------------------------
