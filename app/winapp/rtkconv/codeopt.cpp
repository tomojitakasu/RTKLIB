//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "convopt.h"
#include "codeopt.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCodeOptDialog *CodeOptDialog;
//---------------------------------------------------------------------------
__fastcall TCodeOptDialog::TCodeOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TCodeOptDialog::FormShow(TObject *Sender)
{
	char mask[7][MAXCODE+1]={""};
	
	for (int i=0;i<7;i++) strcpy(mask[i],ConvOptDialog->CodeMask[i].c_str());
	G01->Checked=mask[0][ 0]=='1';
	G02->Checked=mask[0][ 1]=='1';
	G03->Checked=mask[0][ 2]=='1';
	G04->Checked=mask[0][ 3]=='1';
	G05->Checked=mask[0][ 4]=='1';
	G06->Checked=mask[0][ 5]=='1';
	G07->Checked=mask[0][ 6]=='1';
	G08->Checked=mask[0][ 7]=='1';
	G14->Checked=mask[0][13]=='1';
	G15->Checked=mask[0][14]=='1';
	G16->Checked=mask[0][15]=='1';
	G17->Checked=mask[0][16]=='1';
	G18->Checked=mask[0][17]=='1';
	G19->Checked=mask[0][18]=='1';
	G20->Checked=mask[0][19]=='1';
	G21->Checked=mask[0][20]=='1';
	G22->Checked=mask[0][21]=='1';
	G23->Checked=mask[0][22]=='1';
	G24->Checked=mask[0][23]=='1';
	G25->Checked=mask[0][24]=='1';
	G26->Checked=mask[0][25]=='1';
	R01->Checked=mask[1][ 0]=='1';
	R02->Checked=mask[1][ 1]=='1';
	R14->Checked=mask[1][13]=='1';
	R19->Checked=mask[1][18]=='1';
	R44->Checked=mask[1][43]=='1';
	R45->Checked=mask[1][44]=='1';
	R46->Checked=mask[1][45]=='1';
	R66->Checked=mask[1][65]=='1'; //
	R67->Checked=mask[1][66]=='1'; //
	R68->Checked=mask[1][67]=='1'; //
	R30->Checked=mask[1][29]=='1'; //
	R31->Checked=mask[1][30]=='1'; //
	R33->Checked=mask[1][32]=='1'; //
	E01->Checked=mask[2][ 0]=='1';
	E10->Checked=mask[2][ 9]=='1';
	E11->Checked=mask[2][10]=='1';
	E12->Checked=mask[2][11]=='1';
	E13->Checked=mask[2][12]=='1';
	E24->Checked=mask[2][23]=='1';
	E25->Checked=mask[2][24]=='1';
	E26->Checked=mask[2][25]=='1';
	E27->Checked=mask[2][26]=='1';
	E28->Checked=mask[2][27]=='1';
	E29->Checked=mask[2][28]=='1';
	E30->Checked=mask[2][29]=='1';
	E31->Checked=mask[2][30]=='1';
	E32->Checked=mask[2][31]=='1';
	E33->Checked=mask[2][32]=='1';
	E34->Checked=mask[2][33]=='1';
	E37->Checked=mask[2][36]=='1';
	E38->Checked=mask[2][37]=='1';
	E39->Checked=mask[2][38]=='1';
	J01->Checked=mask[3][ 0]=='1';
	J07->Checked=mask[3][ 6]=='1';
	J08->Checked=mask[3][ 7]=='1';
	J13->Checked=mask[3][12]=='1';
	J12->Checked=mask[3][11]=='1';
	J16->Checked=mask[3][15]=='1';
	J17->Checked=mask[3][16]=='1';
	J18->Checked=mask[3][17]=='1';
	J24->Checked=mask[3][23]=='1';
	J25->Checked=mask[3][24]=='1';
	J26->Checked=mask[3][25]=='1';
	J57->Checked=mask[3][56]=='1'; //
	J58->Checked=mask[3][57]=='1'; //
	J59->Checked=mask[3][58]=='1'; //
	J60->Checked=mask[3][59]=='1'; //
	J34->Checked=mask[3][33]=='1'; //
	J35->Checked=mask[3][34]=='1';
	J36->Checked=mask[3][35]=='1';
	J33->Checked=mask[3][32]=='1';
	C40->Checked=mask[5][39]=='1'; //
	C41->Checked=mask[5][40]=='1'; //
	C18->Checked=mask[5][17]=='1'; //
	C27->Checked=mask[5][26]=='1';
	C28->Checked=mask[5][27]=='1';
	C29->Checked=mask[5][28]=='1';
	C42->Checked=mask[5][41]=='1';
	C43->Checked=mask[5][42]=='1';
	C33->Checked=mask[5][32]=='1';
	C56->Checked=mask[5][55]=='1'; //
	C02->Checked=mask[5][ 1]=='1'; //
	C12->Checked=mask[5][11]=='1'; //
	C10->Checked=mask[5][ 9]=='1'; //
	C06->Checked=mask[5][ 5]=='1'; //
	C57->Checked=mask[5][56]=='1'; //
	C58->Checked=mask[5][57]=='1'; //
	C26->Checked=mask[5][25]=='1'; //
	C61->Checked=mask[5][60]=='1'; //
	C62->Checked=mask[5][61]=='1'; //
	C63->Checked=mask[5][62]=='1'; //
	C64->Checked=mask[5][63]=='1'; //
	C65->Checked=mask[5][64]=='1'; //
	C39->Checked=mask[5][38]=='1'; //
	C30->Checked=mask[5][29]=='1'; //
	I49->Checked=mask[6][48]=='1';
	I50->Checked=mask[6][49]=='1';
	I51->Checked=mask[6][50]=='1';
	I26->Checked=mask[6][25]=='1';
	I52->Checked=mask[6][51]=='1';
	I53->Checked=mask[6][52]=='1';
	I54->Checked=mask[6][53]=='1';
	I55->Checked=mask[6][54]=='1';
	S01->Checked=mask[4][ 0]=='1';
	S24->Checked=mask[4][23]=='1';
	S25->Checked=mask[4][24]=='1';
	S26->Checked=mask[4][25]=='1';
	
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TCodeOptDialog::BtnOkClick(TObject *Sender)
{
	char mask[7][MAXCODE+1]={""};
	
	for (int i=0;i<7;i++) for (int j=0;j<MAXCODE;j++) mask[i][j]='0';
	if (G01->Checked) mask[0][ 0]='1';
	if (G02->Checked) mask[0][ 1]='1';
	if (G03->Checked) mask[0][ 2]='1';
	if (G04->Checked) mask[0][ 3]='1';
	if (G05->Checked) mask[0][ 4]='1';
	if (G06->Checked) mask[0][ 5]='1';
	if (G07->Checked) mask[0][ 6]='1';
	if (G08->Checked) mask[0][ 7]='1';
	if (G14->Checked) mask[0][13]='1';
	if (G15->Checked) mask[0][14]='1';
	if (G16->Checked) mask[0][15]='1';
	if (G17->Checked) mask[0][16]='1';
	if (G18->Checked) mask[0][17]='1';
	if (G19->Checked) mask[0][18]='1';
	if (G20->Checked) mask[0][19]='1';
	if (G21->Checked) mask[0][20]='1';
	if (G22->Checked) mask[0][21]='1';
	if (G23->Checked) mask[0][22]='1';
	if (G24->Checked) mask[0][23]='1';
	if (G25->Checked) mask[0][24]='1';
	if (G26->Checked) mask[0][25]='1';
	if (R01->Checked) mask[1][ 0]='1';
	if (R02->Checked) mask[1][ 1]='1';
	if (R14->Checked) mask[1][13]='1';
	if (R19->Checked) mask[1][18]='1';
	if (R44->Checked) mask[1][43]='1';
	if (R45->Checked) mask[1][44]='1';
	if (R46->Checked) mask[1][45]='1';
	if (R66->Checked) mask[1][65]='1'; //
	if (R67->Checked) mask[1][66]='1'; //
	if (R68->Checked) mask[1][67]='1'; //
	if (R30->Checked) mask[1][29]='1'; //
	if (R31->Checked) mask[1][30]='1'; //
	if (R33->Checked) mask[1][32]='1'; //
	if (E01->Checked) mask[2][ 0]='1';
	if (E10->Checked) mask[2][ 9]='1';
	if (E11->Checked) mask[2][10]='1';
	if (E12->Checked) mask[2][11]='1';
	if (E13->Checked) mask[2][12]='1';
	if (E24->Checked) mask[2][23]='1';
	if (E25->Checked) mask[2][24]='1';
	if (E26->Checked) mask[2][25]='1';
	if (E27->Checked) mask[2][26]='1';
	if (E28->Checked) mask[2][27]='1';
	if (E29->Checked) mask[2][28]='1';
	if (E30->Checked) mask[2][29]='1';
	if (E31->Checked) mask[2][30]='1';
	if (E32->Checked) mask[2][31]='1';
	if (E33->Checked) mask[2][32]='1';
	if (E34->Checked) mask[2][33]='1';
	if (E37->Checked) mask[2][36]='1';
	if (E38->Checked) mask[2][37]='1';
	if (E39->Checked) mask[2][38]='1';
	if (J01->Checked) mask[3][ 0]='1';
	if (J07->Checked) mask[3][ 6]='1';
	if (J08->Checked) mask[3][ 7]='1';
	if (J13->Checked) mask[3][12]='1';
	if (J12->Checked) mask[3][11]='1';
	if (J16->Checked) mask[3][15]='1';
	if (J17->Checked) mask[3][16]='1';
	if (J18->Checked) mask[3][17]='1';
	if (J24->Checked) mask[3][23]='1';
	if (J25->Checked) mask[3][24]='1';
	if (J26->Checked) mask[3][25]='1';
	if (J57->Checked) mask[3][56]='1'; //
	if (J58->Checked) mask[3][57]='1'; //
	if (J59->Checked) mask[3][58]='1'; //
	if (J60->Checked) mask[3][59]='1'; //
	if (J34->Checked) mask[3][33]='1'; //
	if (J35->Checked) mask[3][34]='1';
	if (J36->Checked) mask[3][35]='1';
	if (J33->Checked) mask[3][32]='1';
	if (C40->Checked) mask[5][39]='1'; //
	if (C41->Checked) mask[5][40]='1'; //
	if (C18->Checked) mask[5][17]='1'; //
	if (C27->Checked) mask[5][26]='1';
	if (C28->Checked) mask[5][27]='1';
	if (C29->Checked) mask[5][28]='1';
	if (C42->Checked) mask[5][41]='1';
	if (C43->Checked) mask[5][42]='1';
	if (C33->Checked) mask[5][32]='1';
	if (C56->Checked) mask[5][55]='1'; //
	if (C02->Checked) mask[5][ 1]='1'; //
	if (C12->Checked) mask[5][11]='1'; //
	if (C10->Checked) mask[5][ 9]='1'; //
	if (C06->Checked) mask[5][ 5]='1'; //
	if (C57->Checked) mask[5][56]='1'; //
	if (C58->Checked) mask[5][57]='1'; //
	if (C26->Checked) mask[5][25]='1'; //
	if (C61->Checked) mask[5][60]='1'; //
	if (C62->Checked) mask[5][61]='1'; //
	if (C63->Checked) mask[5][62]='1'; //
	if (C64->Checked) mask[5][63]='1'; //
	if (C65->Checked) mask[5][64]='1'; //
	if (C39->Checked) mask[5][38]='1'; //
	if (C30->Checked) mask[5][29]='1'; //
	if (I49->Checked) mask[6][48]='1';
	if (I50->Checked) mask[6][49]='1';
	if (I51->Checked) mask[6][50]='1';
	if (I26->Checked) mask[6][25]='1';
	if (I52->Checked) mask[6][51]='1';
	if (I53->Checked) mask[6][52]='1';
	if (I54->Checked) mask[6][53]='1';
	if (I55->Checked) mask[6][54]='1';
	if (S01->Checked) mask[4][ 0]='1';
	if (S24->Checked) mask[4][23]='1';
	if (S25->Checked) mask[4][24]='1';
	if (S26->Checked) mask[4][25]='1';
	for (int i=0;i<7;i++) ConvOptDialog->CodeMask[i]=mask[i];
}
//---------------------------------------------------------------------------
void __fastcall TCodeOptDialog::BtnSetAllClick(TObject *Sender)
{
	int set=BtnSetAll->Caption=="Set All";
	
	G01->Checked=set;
	G02->Checked=set;
	G03->Checked=set;
	G04->Checked=set;
	G05->Checked=set;
	G06->Checked=set;
	G07->Checked=set;
	G08->Checked=set;
	G14->Checked=set;
	G15->Checked=set;
	G16->Checked=set;
	G17->Checked=set;
	G18->Checked=set;
	G19->Checked=set;
	G20->Checked=set;
	G21->Checked=set;
	G22->Checked=set;
	G23->Checked=set;
	G24->Checked=set;
	G25->Checked=set;
	G26->Checked=set;
	R01->Checked=set;
	R02->Checked=set;
	R14->Checked=set;
	R19->Checked=set;
	R44->Checked=set;
	R45->Checked=set;
	R46->Checked=set;
	R66->Checked=set; //
	R67->Checked=set; //
	R68->Checked=set; //
	R30->Checked=set; //
	R31->Checked=set; //
	R33->Checked=set; //
	E01->Checked=set;
	E10->Checked=set;
	E11->Checked=set;
	E12->Checked=set;
	E13->Checked=set;
	E24->Checked=set;
	E25->Checked=set;
	E26->Checked=set;
	E27->Checked=set;
	E28->Checked=set;
	E29->Checked=set;
	E30->Checked=set;
	E31->Checked=set;
	E32->Checked=set;
	E33->Checked=set;
	E34->Checked=set;
	E37->Checked=set;
	E38->Checked=set;
	E39->Checked=set;
	J01->Checked=set;
	J07->Checked=set;
	J08->Checked=set;
	J13->Checked=set;
	J12->Checked=set;
	J16->Checked=set;
	J17->Checked=set;
	J18->Checked=set;
	J24->Checked=set;
	J25->Checked=set;
	J26->Checked=set;
	J57->Checked=set; //
	J58->Checked=set; //
	J59->Checked=set; //
	J60->Checked=set; //
	J34->Checked=set; //
	J35->Checked=set;
	J36->Checked=set;
	J33->Checked=set;
	C40->Checked=set; //
	C41->Checked=set; //
	C18->Checked=set; //
	C27->Checked=set;
	C28->Checked=set;
	C29->Checked=set;
	C42->Checked=set;
	C43->Checked=set;
	C33->Checked=set;
	C56->Checked=set; //
	C02->Checked=set; //
	C12->Checked=set; //
	C10->Checked=set; //
	C06->Checked=set; //
	C57->Checked=set; //
	C58->Checked=set; //
	C26->Checked=set; //
	C61->Checked=set; //
	C62->Checked=set; //
	C63->Checked=set; //
	C64->Checked=set; //
	C65->Checked=set; //
	C39->Checked=set; //
	C30->Checked=set; //
	I49->Checked=set;
	I50->Checked=set;
	I51->Checked=set;
	I26->Checked=set;
	I52->Checked=set;
	I53->Checked=set;
	I54->Checked=set;
	I55->Checked=set;
	S01->Checked=set;
	S24->Checked=set;
	S25->Checked=set;
	S26->Checked=set;
	BtnSetAll->Caption=BtnSetAll->Caption=="Set All"?"Unset All":"Set All";
}
//---------------------------------------------------------------------------
void __fastcall TCodeOptDialog::UpdateEnable(void)
{
	G01->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L1);
	G02->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L1);
	G03->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L1);
	G04->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L1);
	G05->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L1);
	G06->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L1);
	G07->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L1);
	G08->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L1);
	G14->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L2);
	G15->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L2);
	G16->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L2);
	G17->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L2);
	G18->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L2);
	G19->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L2);
	G20->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L2);
	G21->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L2);
	G22->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L2);
	G23->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L2);
	G24->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L3);
	G25->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L3);
	G26->Enabled=(NavSys&SYS_GPS)&&(FreqType&FREQTYPE_L3);
	R01->Enabled=(NavSys&SYS_GLO)&&(FreqType&FREQTYPE_L1);
	R02->Enabled=(NavSys&SYS_GLO)&&(FreqType&FREQTYPE_L1);
	R14->Enabled=(NavSys&SYS_GLO)&&(FreqType&FREQTYPE_L2);
	R19->Enabled=(NavSys&SYS_GLO)&&(FreqType&FREQTYPE_L2);
	R44->Enabled=(NavSys&SYS_GLO)&&(FreqType&FREQTYPE_L3);
	R45->Enabled=(NavSys&SYS_GLO)&&(FreqType&FREQTYPE_L3);
	R46->Enabled=(NavSys&SYS_GLO)&&(FreqType&FREQTYPE_L3);
	R66->Enabled=(NavSys&SYS_GLO)&&(FreqType&FREQTYPE_L1); //
	R67->Enabled=(NavSys&SYS_GLO)&&(FreqType&FREQTYPE_L1); //
	R68->Enabled=(NavSys&SYS_GLO)&&(FreqType&FREQTYPE_L1); //
	R30->Enabled=(NavSys&SYS_GLO)&&(FreqType&FREQTYPE_L2); //
	R31->Enabled=(NavSys&SYS_GLO)&&(FreqType&FREQTYPE_L2); //
	R33->Enabled=(NavSys&SYS_GLO)&&(FreqType&FREQTYPE_L2); //
	E01->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L1);
	E10->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L1);
	E11->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L1);
	E12->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L1);
	E13->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L1);
	E24->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L3);
	E25->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L3);
	E26->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L3);
	E27->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L2);
	E28->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L2);
	E29->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L2);
	E30->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L4);
	E31->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L4);
	E32->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L4);
	E33->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L4);
	E34->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L4);
	E37->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L5);
	E38->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L5);
	E39->Enabled=(NavSys&SYS_GAL)&&(FreqType&FREQTYPE_L5);
	J01->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L1);
	J07->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L1);
	J08->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L1);
	J13->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L1);
	J12->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L1);
	J16->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L2);
	J17->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L2);
	J18->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L2);
	J24->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L3);
	J25->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L3);
	J26->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L3);
	J57->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L3); //
	J58->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L3); //
	J59->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L3); //
	J60->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L4); //
	J34->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L4); //
	J35->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L4);
	J36->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L4);
	J33->Enabled=(NavSys&SYS_QZS)&&(FreqType&FREQTYPE_L4);
	C40->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L1); //
	C41->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L1); //
	C18->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L1); //
	C27->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L2);
	C28->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L2);
	C29->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L2);
	C42->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L4);
	C43->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L4);
	C33->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L4);
	C56->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L1); //
	C02->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L1); //
	C12->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L1); //
	C10->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L1); //
	C06->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L1); //
	C57->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L3); //
	C58->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L3); //
	C26->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L3); //
	C61->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L2); //
	C62->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L2); //
	C63->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L2); //
	C64->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L5); //
	C65->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L5); //
	C39->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L5); //
	C30->Enabled=(NavSys&SYS_CMP)&&(FreqType&FREQTYPE_L4); //
	I49->Enabled=(NavSys&SYS_IRN)&&(FreqType&FREQTYPE_L1);
	I50->Enabled=(NavSys&SYS_IRN)&&(FreqType&FREQTYPE_L1);
	I51->Enabled=(NavSys&SYS_IRN)&&(FreqType&FREQTYPE_L1);
	I26->Enabled=(NavSys&SYS_IRN)&&(FreqType&FREQTYPE_L1);
	I52->Enabled=(NavSys&SYS_IRN)&&(FreqType&FREQTYPE_L2);
	I53->Enabled=(NavSys&SYS_IRN)&&(FreqType&FREQTYPE_L2);
	I54->Enabled=(NavSys&SYS_IRN)&&(FreqType&FREQTYPE_L2);
	I55->Enabled=(NavSys&SYS_IRN)&&(FreqType&FREQTYPE_L2);
	S01->Enabled=(NavSys&SYS_SBS)&&(FreqType&FREQTYPE_L1);
	S24->Enabled=(NavSys&SYS_SBS)&&(FreqType&FREQTYPE_L3);
	S25->Enabled=(NavSys&SYS_SBS)&&(FreqType&FREQTYPE_L3);
	S26->Enabled=(NavSys&SYS_SBS)&&(FreqType&FREQTYPE_L3);
}
//---------------------------------------------------------------------------

