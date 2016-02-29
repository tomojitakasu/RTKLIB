//---------------------------------------------------------------------------

#include <QCloseEvent>
#include <QShowEvent>
#include <QSettings>
#include <QFileDialog>
#include <QTimer>
#include <QCommandLineParser>

#include "rtklib.h"
#include "aboutdlg.h"
#include "gmview.h"
#include "browsmain.h"
#include "staoptdlg.h"
//---------------------------------------------------------------------------

#define PRGNAME			"Ntrip Browser Qt"
#define NTRIP_HOME		"rtcm-ntrip.org:2101" // caster list home
#define NTRIP_TIMEOUT	10000				// response timeout (ms)
#define NTRIP_CYCLE		50					// processing cycle (ms)
#define MAXSRCTBL		512000				// max source table size (bytes)
#define ENDSRCTBL		"ENDSOURCETABLE"	// end marker of table
#define MAXLINE			1024				// max line size (byte)

static char buff[MAXSRCTBL];				// source table buffer

MainForm *mainForm;

/* get source table -------------------------------------------------------*/
static char *getsrctbl(const char *path)
{
	static int lock=0;
	stream_t str;
	char *p=buff,msg[MAXSTRMSG];
	int ns,stat,len=strlen(ENDSRCTBL);
	unsigned int tick=tickget();
	
	if (lock) return NULL; else lock=1;
	
	strinit(&str);
	if (!stropen(&str,STR_NTRIPCLI,STR_MODE_R,path)) {
		lock=0; 
        mainForm->ShowMsg(QT_TR_NOOP("stream open error"));
		return NULL;
	}
    mainForm->ShowMsg(QT_TR_NOOP("connecting..."));
	
	while(p<buff+MAXSRCTBL-1) {
        ns=strread(&str,(unsigned char*)p,(buff+MAXSRCTBL-p-1)); *(p+ns)='\0';
		if (p-len-3>buff&&strstr(p-len-3,ENDSRCTBL)) break;
		p+=ns;
        //Sleep(NTRIP_CYCLE);
		stat=strstat(&str,msg);
        mainForm->ShowMsg(msg);
		if (stat<0) break;
		if ((int)(tickget()-tick)>NTRIP_TIMEOUT) {
            mainForm->ShowMsg(QT_TR_NOOP("response timeout"));
			break;
		}
	}
	strclose(&str);
	lock=0;
	return buff;
}
//---------------------------------------------------------------------------
MainForm::MainForm(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);
    setMenuBar(MainMenu);
    mainForm=this;

    QCoreApplication::setApplicationName(PRGNAME);
    QCoreApplication::setApplicationVersion("1.0");

    setWindowIcon(QIcon(":/icons/srctblbrows_Icon"));

    QString file=QApplication::applicationFilePath();

    IniFile=QFileInfo(file).absoluteFilePath()+".ini";
    
    googleMapView = new GoogleMapView(this);
    staListDialog = new StaListDialog(this);

    connect(Address,SIGNAL(editTextChanged(QString)),this,SLOT(AddressChange()));
    connect(BtnUpdate,SIGNAL(clicked(bool)),this,SLOT(BtnUpdateClick()));
    connect(TypeCas,SIGNAL(clicked(bool)),this,SLOT(TypeCasClick()));
    connect(TypeStr,SIGNAL(clicked(bool)),this,SLOT(TypeStrClick()));
    connect(TypeNet,SIGNAL(clicked(bool)),this,SLOT(TypeNetClick()));
    connect(TypeSrc,SIGNAL(clicked(bool)),this,SLOT(TypeSrcClick()));
    connect(BtnMap,SIGNAL(clicked(bool)),this,SLOT(BtnMapClick()));
    connect(BtnList,SIGNAL(clicked(bool)),this,SLOT(BtnListClick()));
    connect(BtnSta,SIGNAL(clicked(bool)),this,SLOT(BtnStaClick()));
    connect(StaMask,SIGNAL(clicked(bool)),this,SLOT(StaMaskClick()));
    connect(MenuOpen,SIGNAL(triggered(bool)),this,SLOT(MenuOpenClick()));
    connect(MenuSave,SIGNAL(triggered(bool)),this,SLOT(MenuSaveClick()));
    connect(MenuQuit,SIGNAL(triggered(bool)),this,SLOT(MenuQuitClick()));
    connect(MenuUpdateCaster,SIGNAL(triggered(bool)),this,SLOT(MenuUpdateCasterClick()));
    connect(MenuUpdateTable,SIGNAL(triggered(bool)),this,SLOT(MenuUpdateTableClick()));
    connect(MenuViewCas,SIGNAL(triggered(bool)),this,SLOT(MenuViewCasClick()));
    connect(MenuViewNet,SIGNAL(triggered(bool)),this,SLOT(MenuViewNetClick()));
    connect(MenuViewSrc,SIGNAL(triggered(bool)),this,SLOT(MenuViewSrcClick()));
    connect(MenuViewStr,SIGNAL(triggered(bool)),this,SLOT(MenuViewStrClick()));
    connect(MenuAbout,SIGNAL(triggered(bool)),this,SLOT(MenuAboutClick()));
    connect(Timer,SIGNAL(timeout()),this,SLOT(TimerTimer()));
    connect(Table0,SIGNAL(cellActivated(int,int)),this,SLOT(Table0SelectCell(int,int)));

    Table0->setSortingEnabled(true);
    Table1->setSortingEnabled(true);
    Table2->setSortingEnabled(true);
    void TypeChange();
    void AddressKeyPress(char &Key);

    strinitcom();

    Timer=new QTimer;
    Timer->setInterval(100);
}
//---------------------------------------------------------------------------
void MainForm::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    QString colw0="74,116,56,244,18,52,62,28,50,50,18,18,120,28,18,18,40,600,";
    QString colw1="112,40,96,126,18,28,50,50,160,40,600,0,0,0,0,0,0,0,";
    QString colw2="80,126,18,18,300,300,300,600,0,0,0,0,0,0,0,0,0,0,";
    QSettings setting(IniFile,QSettings::IniFormat);
    QString list,url="";
    QStringList colw,stas;
    int i;
	
	
    QCommandLineParser parser;
    parser.setApplicationDescription(PRGNAME);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("URL", QCoreApplication::translate("main", "file with list of Ntrip sources"));

    parser.process(*QApplication::instance());

    const QStringList args = parser.positionalArguments();

    if (args.count()>=1) url=args.at(0);
	
    setWindowTitle(QString("%1 ver.%2 %3").arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL));
	
    list=setting.value("srctbl/addrlist","").toString();
    QStringList items=list.split("@");
    foreach (QString item, items)
    {
        Address->addItem(item);
    }

    if (url!="") {
        Address->setCurrentText(url);
        UpdateTable();
    }
	else {
        Address->setCurrentText(setting.value("srctbl/address","").toString());
	}

    FontScale=physicalDpiX();

    colw=setting.value("srctbl/colwidth0",colw0).toString().split(",");
    i=0;
    foreach (QString width,colw)
    {
        Table0->setColumnWidth(i++,width.toDouble()*FontScale/96);
	}
    colw=setting.value("srctbl/colwidth1",colw1).toString().split(",");
    i=0;
    foreach (QString width,colw)
    {
        Table1->setColumnWidth(i++,width.toDouble()*FontScale/96);
    }
    colw=setting.value("srctbl/colwidth2",colw2).toString().split(",");
    i=0;
    foreach (QString width,colw)
    {
        Table2->setColumnWidth(i++,width.toDouble()*FontScale/96);
    }

    StaList.clear();
    for (int i=0;i<10;i++) {
        stas=setting.value(QString("sta/station%1").arg(i),"").toString().split(",");
        foreach(QString sta, stas)
        {
            StaList.append(sta);
        }
    }
	
	ShowTable();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void MainForm::closeEvent(QCloseEvent *event)
{
    if (event->spontaneous()) return;

    QSettings setting(IniFile,QSettings::IniFormat);
    QString list,colw;
	
    setting.setValue("srctbl/address",Address->currentText());
    for (int i=0;i<Address->count();i++) {
        list=list+Address->itemText(i)+"@";
	}
    setting.setValue("srctbl/addrlist",list);
	colw="";
    for (int i=0;i<Table0->columnCount();i++) {
        colw=colw+QString::number(Table0->columnWidth(i)*96/FontScale);
	}
    setting.setValue("srctbl/colwidth0",colw);
	colw="";
    for (int i=0;i<Table1->columnCount();i++) {
        colw=colw+QString::number(Table1->columnWidth(i)*96/FontScale);
	}
    setting.setValue("srctbl/colwidth1",colw);
	colw="";
    for (int i=0;i<Table2->columnCount();i++) {
        colw=colw+QString::number(Table2->columnWidth(i)*96/FontScale);
	}
    setting.setValue("srctbl/colwidth2",colw);
    
    for (int i=0;i<10;i++) {
        setting.setValue (QString("sta/station%1").arg(i),StaList.join(","));
    }
}
//---------------------------------------------------------------------------
void MainForm::MenuOpenClick()
{
    QString OpenDialog_FileName=QFileDialog::getOpenFileName(this,tr("Open"),QString(),tr("All File (*.*)"));
    QFile fp(OpenDialog_FileName);

    SrcTable="";

    if (!fp.open(QIODevice::ReadOnly)) return;

    SrcTable=fp.readAll();

    AddrCaster=Address->currentText();

    ShowTable();
    ShowMsg(tr("source table loaded"));
}
//---------------------------------------------------------------------------
void MainForm::MenuSaveClick()
{
    QString SaveDialog_FileName=QFileDialog::getSaveFileName(this,tr("Save File"),QString(),tr("All File (*.*)"));
    QFile fp(SaveDialog_FileName);

    if (!fp.open(QIODevice::WriteOnly)) return;

    fp.write(SrcTable.toLatin1());

    ShowMsg(tr("source table saved"));
}
//---------------------------------------------------------------------------
void MainForm::MenuQuitClick()
{
    close();
}
//---------------------------------------------------------------------------
void MainForm::MenuUpdateCasterClick()
{
	UpdateCaster();
}
//---------------------------------------------------------------------------
void MainForm::MenuUpdateTableClick()
{
	UpdateTable();
}
//---------------------------------------------------------------------------
void MainForm::MenuViewStrClick()
{
    TypeStr->setChecked(true);
	ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::MenuViewCasClick()
{
    TypeCas->setChecked(true);
	ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::MenuViewNetClick()
{
    TypeNet->setChecked(true);
	ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::MenuViewSrcClick()
{
    TypeSrc->setChecked(true);
	ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::MenuAboutClick()
{
    AboutDialog *aboutDialog=new AboutDialog(this);
    aboutDialog->About=PRGNAME;
    aboutDialog->IconIndex=7;
    aboutDialog->exec();

    delete aboutDialog;
}
//---------------------------------------------------------------------------
void MainForm::BtnMapClick()
{
    Timer->start();
    googleMapView->exec();
}
//---------------------------------------------------------------------------
void MainForm::Table0SelectCell(int ACol, int ARow)
{
    Q_UNUSED(ACol);
    QString title;
    if (0<ARow&&ARow<Table0->rowCount()) {
        title=Table0->item(ARow,0)->text();
        googleMapView->HighlightMark(title);
        googleMapView->setWindowTitle(QString(tr("NTRIP STR Map: %1/%2")).arg(Address->currentText()).arg(title));
	}
}
//---------------------------------------------------------------------------
void MainForm::BtnListClick()
{
	UpdateCaster();
}
//---------------------------------------------------------------------------
void MainForm::BtnUpdateClick()
{
	UpdateTable();
}
//---------------------------------------------------------------------------
void MainForm::TypeStrClick()
{
	ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::TypeCasClick()
{
	ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::TypeNetClick()
{
	ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::TypeSrcClick()
{
	ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::AddressKeyPress(QString)
{
    UpdateTable(); //FIXME
}
//---------------------------------------------------------------------------
void MainForm::AddressChange()
{
	ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::TypeChange()
{
	ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::TimerTimer()
{
    if (!googleMapView->GetState()) return;
	UpdateMap();
    Timer->stop();
}
//---------------------------------------------------------------------------
void MainForm::UpdateCaster(void)
{
    QString Address_Text=Address->currentText();
    QString text,item[3];
    char buff[MAXLINE],*p,*q,*r,*srctbl,addr[1024];
	int i,n;
    strcpy(addr,NTRIP_HOME);

    if (Address_Text!="") strcpy(addr,qPrintable(Address_Text));

	if (!(srctbl=getsrctbl(addr))) return;
	
    text=Address->currentText(); Address->clear(); Address->setCurrentText(text);
    Address->addItem("");
	for (p=srctbl;*p;p=q+1) {
		if (!(q=strchr(p,'\n'))) break;
		n=q-p<MAXLINE-1?q-p:MAXLINE-1;
		strncpy(buff,p,n); buff[n]='\0';
		if (strncmp(buff,"CAS",3)) continue;
		for (i=0,r=strtok(buff,";");i<3&&p;i++,r=strtok(NULL,";")) item[i]=r;
        Address->addItem(item[1]+":"+item[2]);
	}
    if (Address->count()>1) Address->setCurrentIndex(0);
	ShowMsg("update caster list");
}
//---------------------------------------------------------------------------
void MainForm::UpdateTable(void)
{
    QString Address_Text=Address->currentText();
    char *srctbl;
    char addr[1024];
    strcpy(addr,NTRIP_HOME);

    if (Address_Text!="") strcpy(addr,qPrintable(Address_Text));

	if ((srctbl=getsrctbl(addr))) {
		SrcTable=srctbl;
        AddrCaster=Address->currentText();
	}
	ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::ShowTable(void)
{
    const QString ti1[]={tr("Mountpoint"),tr("ID"),tr("Format"),tr("Format-Details"),tr("Carrier"),tr("Nav-System"),
        tr("Network"),tr("Country"),tr("Latitude"),tr("Longitude"),tr("NMEA"),tr("Solution"),
        tr("Generator"),"Compr-Encrp","Authentication","Fee","Bitrate"};
    const QString ti2[]={tr("Host"),tr("Port"),tr("ID"),tr("Operator"),tr("NMEA"),tr("Country"),tr("Latitude"),tr("Longitude"),
        tr("Fallback_Host"),tr("Fallback_Port")};
    const QString ti3[]={tr("ID"),tr("Operator"),tr("Authentication"),tr("Fee"),tr("Web-Net"),tr("Web-Str"),tr("Web-Reg")};
    const QString *ti[]={ti1,ti2,ti3};

    QTableWidget *table[]={Table0,Table1,Table2};
    QAction *action[]={MenuViewStr,MenuViewCas,MenuViewNet,MenuViewSrc};
    QString buff;
	int i,j,n,ns,type;

    Table3->setVisible(false);
    for (i=0;i<3;i++) table[i]->setVisible(false);
	
    type=TypeStr->isChecked()?0:(TypeCas->isChecked()?1:(TypeNet->isChecked()?2:3));
    for (i=0;i<4;i++) action[i]->setChecked(i==type);
	
	if (type==3) {
        Table3->setVisible(true);
        Table3->setPlainText("");
	}
	else {
        table[type]->setVisible(true);
        table[type]->setRowCount(2);
		for (i=0;i<18;i++) {
            table[type]->item(0,i)->setText(ti[type][i]);
            table[type]->item(1,i)->setText("");
		}
	}
    if (AddrCaster!=Address->currentText()) return;
	if (type==3) {
        Table3->setPlainText(SrcTable);
		return;
	}
    QStringList lines=SrcTable.split("\n");
    ns=0;
    foreach (QString line, lines) {
		switch (type) {
            case 0: if (line.contains("STR")) ns++; break;
            case 1: if (line.contains("CAS")) ns++; break;
            case 2: if (line.contains("NET")) ns++; break;
		}
	}
	if (ns<=0) return;
    table[type]->setRowCount(ns+1);
    j=1;
    foreach (QString line, lines) {
        n=line.length()<MAXLINE-1?line.length():MAXLINE-1;
        buff=line.left(n);
		switch (type) {
            case 0: if (line.contains("STR")) break; else continue;
            case 1: if (line.contains("CAS")) break; else continue;
            case 2: if (line.contains("NET")) break; else continue;
		}
        QStringList tokens=buff.split(";");
        for (int i=0;i<18&&i<tokens.size();i++) {
            table[type]->item(j,i-1)->setText(tokens.at(i));
		}
		j++;
	}
	UpdateMap();
}
//---------------------------------------------------------------------------
void MainForm::ShowMsg(const QString &msg)
{
    QString str=msg;
    Message->setText(str);
}
//---------------------------------------------------------------------------
void MainForm::UpdateMap(void)
{
    QString title,msg,LatText,LonText;
	double lat,lon;
	
    if (Address->currentText()=="") {
        googleMapView->setWindowTitle(tr("NTRIP STR Map"));
	}
	else {
        googleMapView->setWindowTitle(QString(tr("NTRIP STR Map: %1")).arg(Address->currentText()));
	}
    googleMapView->ClearMark();
	
    for (int i=1;i<Table0->rowCount();i++) {
        if (Table0->item(i,8)->text()=="") continue;
        LatText=Table0->item(i,8)->text();
        LonText=Table0->item(i,8)->text();
        lat=LatText.toDouble();
        lon=LonText.toDouble();
        title=Table0->item(i,0)->text();
        msg="<b>"+Table0->item(i,0)->text()+"</b>: "+
            Table0->item(i,1)->text()+" ("+Table0->item(i,7)->text()+"), POS: "+
            Table0->item(i,8)->text()+", "+Table0->item(i,9)->text()+"<br>"+
            Table0->item(i,2)->text()+": "+Table0->item(i,3)->text()+"<br>"+
            Table0->item(i,5)->text()+", "+Table0->item(i,6)->text()+", "+
            Table0->item(i,12)->text();
        googleMapView->AddMark(lat,lon,title,msg);
	}
}
//---------------------------------------------------------------------------
void MainForm::BtnStaClick()
{
    staListDialog->exec();
}
//---------------------------------------------------------------------------
void MainForm::UpdateEnable(void)
{
    BtnSta->setEnabled(StaMask->isChecked());
}
//---------------------------------------------------------------------------
void MainForm::StaMaskClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------

