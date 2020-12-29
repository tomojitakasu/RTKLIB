//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "viewer.h"
#include "mapviewopt.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMapViewOptDialog *MapViewOptDialog;
//---------------------------------------------------------------------------
__fastcall TMapViewOptDialog::TMapViewOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TMapViewOptDialog::FormShow(TObject *Sender)
{
	TEdit *titles[]={
		MapTitle1,MapTitle2,MapTitle3,MapTitle4,MapTitle5,MapTitle6
	};
	TEdit *tiles[]={
		MapTile1,MapTile2,MapTile3,MapTile4,MapTile5,MapTile6
	};
	for (int i=0;i<6;i++) {
	    titles[i]->Text=MapStrs[i][0];
	    tiles [i]->Text=MapStrs[i][1];
	}
    EditApiKey->Text=ApiKey;
}
//---------------------------------------------------------------------------
void __fastcall TMapViewOptDialog::BtnOkClick(TObject *Sender)
{
	TEdit *titles[]={
		MapTitle1,MapTitle2,MapTitle3,MapTitle4,MapTitle5,MapTitle6
	};
	TEdit *tiles[]={
		MapTile1,MapTile2,MapTile3,MapTile4,MapTile5,MapTile6
	};
	for (int i=0;i<6;i++) {
		MapStrs[i][0]=titles[i]->Text;
		MapStrs[i][1]=tiles [i]->Text;
	}
	ApiKey=EditApiKey->Text;
}
//---------------------------------------------------------------------------
void __fastcall TMapViewOptDialog::BtnNotesClick(TObject *Sender)
{
	AnsiString file;
    TTextViewer *viewer;
    char dir[1024],*p;
    
    ::GetModuleFileName(NULL,dir,sizeof(dir));
    if ((p=strrchr(dir,'\\'))) *p='\0';
	file=dir;
	file+="\\gmview_notes.txt";
    viewer=new TTextViewer(Application);
    viewer->Caption=file;
    viewer->Option=0;
    viewer->Show();
    viewer->Read(file);	
}
//---------------------------------------------------------------------------


