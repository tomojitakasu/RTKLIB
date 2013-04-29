//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "timedlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTimeDialog *TimeDialog;
//---------------------------------------------------------------------------
__fastcall TTimeDialog::TTimeDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TTimeDialog::FormShow(TObject *Sender)
{
	gtime_t utc;
	double tow,doy;
	int week;
	char msg[1024],s1[64],s2[64],*p=msg;
	utc=gpst2utc(Time);
	time2str(Time,s1,0);
	time2str(utc,s2,0);
	tow=time2gpst(Time,&week);
	doy=time2doy(Time);
	p+=sprintf(p,"%s GPST\n",s1);
	p+=sprintf(p,"%s UTC\n\n",s2);
	p+=sprintf(p,"GPS Week: %d\n",week);
	p+=sprintf(p,"GPS Time: %.0f s\n",tow);
	p+=sprintf(p,"Day of Year: %03d\n",(int)floor(doy));
	p+=sprintf(p,"Day of Week: %d\n",(int)floor(tow/86400.0));
	p+=sprintf(p,"Time of Day: %.0f s\n",fmod(tow,86400.0));
	sprintf(p,"Leap Seconds: %.0f s\n",timediff(Time,utc));
	Message->Caption=msg;
}
//---------------------------------------------------------------------------
