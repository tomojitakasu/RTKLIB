//---------------------------------------------------------------------------

#include <QShowEvent>

#include "viewer.h"
#include "mapviewopt.h"

//---------------------------------------------------------------------------
MapViewOptDialog::MapViewOptDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(BtnOk, SIGNAL(clicked(bool)), this, SLOT(BtnOkClick()));
    connect(BtnCancel, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(BtnNotes, SIGNAL(clicked(bool)), this, SLOT(BtnNotesClick()));
}
//---------------------------------------------------------------------------
void MapViewOptDialog::showEvent(QShowEvent *)
{
    QLineEdit *titles[]={
		MapTitle1,MapTitle2,MapTitle3,MapTitle4,MapTitle5,MapTitle6
	};
    QLineEdit *tiles[]={
		MapTile1,MapTile2,MapTile3,MapTile4,MapTile5,MapTile6
	};
	for (int i=0;i<6;i++) {
        titles[i]->setText(MapStrs[i][0]);
        tiles [i]->setText(MapStrs[i][1]);
	}
    EditApiKey->setText(ApiKey);
}
//---------------------------------------------------------------------------
void MapViewOptDialog::BtnOkClick()
{
    QLineEdit *titles[]={
		MapTitle1,MapTitle2,MapTitle3,MapTitle4,MapTitle5,MapTitle6
	};
    QLineEdit *tiles[]={
		MapTile1,MapTile2,MapTile3,MapTile4,MapTile5,MapTile6
	};
	for (int i=0;i<6;i++) {
        MapStrs[i][0]=titles[i]->text();
        MapStrs[i][1]=tiles [i]->text();
	}
    ApiKey=EditApiKey->text();
    accept();
}
//---------------------------------------------------------------------------
void MapViewOptDialog::BtnNotesClick()
{
    QString file, dir;
    TextViewer *viewer;
    
    dir = qApp->applicationDirPath(); // exe directory
    file=dir+"/gmview_notes.txt";
    viewer=new TextViewer(this);
    viewer->setWindowTitle(file);
    viewer->Option=0;
    viewer->exec();
    viewer->Read(file);	
}
//---------------------------------------------------------------------------


