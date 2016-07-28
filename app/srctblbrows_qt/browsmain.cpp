//---------------------------------------------------------------------------
// Ported to Qt by Jens Reimann

#include <QCloseEvent>
#include <QShowEvent>
#include <QSettings>
#include <QFileDialog>
#include <QTimer>
#include <QCommandLineParser>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QMetaObject>

#include "rtklib.h"
#include "aboutdlg.h"
#include "gmview.h"
#include "browsmain.h"
#include "staoptdlg.h"
//---------------------------------------------------------------------------

#define PRGNAME			"Ntrip Browser Qt"
#define NTRIP_HOME		"rtcm-ntrip.org:2101" // caster list home
#define NTRIP_TIMEOUT	10000				// response timeout (ms)
#define MAXSRCTBL		512000				// max source table size (bytes)
#define ENDSRCTBL		"ENDSOURCETABLE"	// end marker of table

static char buff[MAXSRCTBL];				// source table buffer

MainForm *mainForm;

/* get source table -------------------------------------------------------*/
static char * getsrctbl(const QString addr)
{
    static int lock=0;
	stream_t str;
	char *p=buff,msg[MAXSTRMSG];
    int len=strlen(ENDSRCTBL);
	unsigned int tick=tickget();

	if (lock) return NULL; else lock=1;
	
    strinit(&str);

    if (!stropen(&str,STR_NTRIPCLI,STR_MODE_R,qPrintable(addr))) {
		lock=0; 
        QMetaObject::invokeMethod(mainForm,"ShowMsg",Qt::QueuedConnection,Q_ARG(QString,QT_TR_NOOP("stream open error")));
		return NULL;
	}
    QMetaObject::invokeMethod(mainForm,"ShowMsg",Qt::QueuedConnection,Q_ARG(QString,QT_TR_NOOP("connecting...")));

	while(p<buff+MAXSRCTBL-1) {
        int ns=strread(&str,(unsigned char*)p,(buff+MAXSRCTBL-p-1)); *(p+ns)='\0';
		if (p-len-3>buff&&strstr(p-len-3,ENDSRCTBL)) break;
		p+=ns;
        qApp->processEvents();
        int stat=strstat(&str,msg);

        QMetaObject::invokeMethod(mainForm,"ShowMsg",Qt::QueuedConnection,Q_ARG(QString,msg));

		if (stat<0) break;
		if ((int)(tickget()-tick)>NTRIP_TIMEOUT) {
            QMetaObject::invokeMethod(mainForm,"ShowMsg",Qt::QueuedConnection,Q_ARG(QString,QT_TR_NOOP("response timeout")));
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
    FiltFmt->setVisible(false);
    mainForm=this;

    QCoreApplication::setApplicationName(PRGNAME);
    QCoreApplication::setApplicationVersion("1.0");

    setWindowIcon(QIcon(":/icons/srctblbrows_Icon"));

    QString file=QApplication::applicationFilePath();
    QFileInfo fi(file);
    IniFile=fi.absolutePath()+"/"+fi.baseName()+".ini";
    
    googleMapView = new GoogleMapView(this);
    staListDialog = new StaListDialog(this);
    Timer = new QTimer();

    TypeCas->setDefaultAction(MenuViewCas);
    TypeNet->setDefaultAction(MenuViewNet);
    TypeSrc->setDefaultAction(MenuViewSrc);
    TypeStr->setDefaultAction(MenuViewStr);

    connect(Address,SIGNAL(currentTextChanged(QString)),this,SLOT(AddressChange()));
    connect(BtnUpdate,SIGNAL(clicked(bool)),this,SLOT(BtnUpdateClick()));
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
    connect(Table0,SIGNAL(cellClicked(int,int)),this,SLOT(Table0SelectCell(int,int)));

#ifdef QWEBKIT
    BtnMap->setEnabled(true);
#else
  #ifdef QWEBENGINE
    BtnMap->setEnabled(true);
  #else
    BtnMap->setEnabled(false);
  #endif
#endif

    Table0->setSortingEnabled(true);
    Table1->setSortingEnabled(true);
    Table2->setSortingEnabled(true);

    statusbar->addWidget(Panel2);

    strinitcom();

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
        GetTable();
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
    QString OpenDialog_FileName=QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Open"),QString(),tr("All File (*.*)")));
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
    QString SaveDialog_FileName=QDir::toNativeSeparators(QFileDialog::getSaveFileName(this,tr("Save File"),QString(),tr("All File (*.*)")));
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
    GetCaster();
}
//---------------------------------------------------------------------------
void MainForm::MenuUpdateTableClick()
{
    GetTable();
}
//---------------------------------------------------------------------------
void MainForm::MenuViewStrClick()
{
    MenuViewCas->setChecked(false);
    MenuViewNet->setChecked(false);
    MenuViewSrc->setChecked(false);
    ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::MenuViewCasClick()
{
    MenuViewStr->setChecked(false);
    MenuViewNet->setChecked(false);
    MenuViewSrc->setChecked(false);
	ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::MenuViewNetClick()
{
    MenuViewStr->setChecked(false);
    MenuViewCas->setChecked(false);
    MenuViewSrc->setChecked(false);
	ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::MenuViewSrcClick()
{
    MenuViewStr->setChecked(false);
    MenuViewCas->setChecked(false);
    MenuViewNet->setChecked(false);
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
    googleMapView->show();
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
    GetCaster();
}
//---------------------------------------------------------------------------
void MainForm::BtnUpdateClick()
{
    GetTable();
}
//---------------------------------------------------------------------------
void MainForm::AddressChange()
{
    GetTable();
}
//---------------------------------------------------------------------------
void MainForm::TimerTimer()
{
    if (!googleMapView->GetState()) return;
	UpdateMap();
    Timer->stop();
}
//---------------------------------------------------------------------------
void MainForm::GetCaster(void)
{
    if (CasterWatcher.isRunning()) return;
    QString Address_Text=Address->currentText();
    QString addr=NTRIP_HOME;

    if (Address_Text!="") addr=Address_Text;

    BtnList->setEnabled(false);
    MenuUpdateCaster->setEnabled(false);

    QFuture<char*> tblFuture= QtConcurrent::run(getsrctbl,addr);
    connect(&CasterWatcher,SIGNAL(finished()),this,SLOT(UpdateCaster()));
    CasterWatcher.setFuture(tblFuture);
}
//---------------------------------------------------------------------------
void MainForm::UpdateCaster()
{
    QString text;
    QString srctbl;

    BtnList->setEnabled(true);
    MenuUpdateCaster->setEnabled(true);

    if ((srctbl=CasterWatcher.result()).isEmpty())  return;

    text=Address->currentText(); Address->clear(); Address->setCurrentText(text);
    Address->addItem("");

    QStringList tokens,lines=srctbl.split('\n');
    foreach (const QString & line, lines) {
        if (!line.contains("CAS")) continue;
        tokens=line.split(";");
        if (tokens.size()<3) continue;
        Address->addItem(tokens.at(1)+":"+tokens.at(2));
	}
    if (Address->count()>1) Address->setCurrentIndex(0);
	ShowMsg("update caster list");
}
//---------------------------------------------------------------------------
void MainForm::GetTable(void)
{
    if (TableWatcher.isRunning()) return;

    QString Address_Text=Address->currentText();
    QString addr=NTRIP_HOME;

    if (Address_Text!="") addr=Address_Text;

    BtnUpdate->setEnabled(false);
    Address->setEnabled(false);
    MenuUpdateTable->setEnabled(false);

    QFuture<char*> tblFuture= QtConcurrent::run(getsrctbl,addr);
    connect(&TableWatcher,SIGNAL(finished()),this,SLOT(UpdateTable()));
    TableWatcher.setFuture(tblFuture);
}
//---------------------------------------------------------------------------
void MainForm::UpdateTable(void)
{
    QString srctbl;

    BtnUpdate->setEnabled(true);
    Address->setEnabled(true);
    MenuUpdateTable->setEnabled(true);

    if (!(srctbl=TableWatcher.result()).isEmpty()) {
		SrcTable=srctbl;
        AddrCaster=Address->currentText();
	}
	ShowTable();
}
//---------------------------------------------------------------------------
void MainForm::ShowTable(void)
{
    const QString ti[3][18]={{tr("Mountpoint"),tr("ID"),tr("Format"),tr("Format-Details"),tr("Carrier"),tr("Nav-System"),
        tr("Network"),tr("Country"),tr("Latitude"),tr("Longitude"),tr("NMEA"),tr("Solution"),
        tr("Generator"),"Compr-Encrp","Authentication","Fee","Bitrate",""},
        {tr("Host"),tr("Port"),tr("ID"),tr("Operator"),tr("NMEA"),tr("Country"),tr("Latitude"),tr("Longitude"),
        tr("Fallback_Host"),tr("Fallback_Port"),"","","","","","",""},
        {tr("ID"),tr("Operator"),tr("Authentication"),tr("Fee"),tr("Web-Net"),tr("Web-Str"),tr("Web-Reg"),"","","","",
        "","","","","",""}};


    QTableWidget *table[]={Table0,Table1,Table2};
    QAction *action[]={MenuViewStr,MenuViewCas,MenuViewNet,MenuViewSrc};
    int i,j,ns,type;

    Table3->setVisible(false);
    for (i=0;i<3;i++) table[i]->setVisible(false);
	
    type=MenuViewStr->isChecked()?0:(MenuViewCas->isChecked()?1:(MenuViewNet->isChecked()?2:3));
    for (i=0;i<4;i++) action[i]->setChecked(i==type);
	
	if (type==3) {
        Table3->setVisible(true);
        Table3->setPlainText("");
	}
	else {
        table[type]->setVisible(true);
        table[type]->setRowCount(1);
        table[type]->setColumnCount(18);
		for (i=0;i<18;i++) {
            table[type]->setHorizontalHeaderItem(i, new QTableWidgetItem(ti[type][i]));
            table[type]->setItem(0,i, new QTableWidgetItem(""));
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
    table[type]->setRowCount(ns);
    j=0;
    foreach (QString line, lines) {
		switch (type) {
            case 0: if (line.contains("STR")) break; else continue;
            case 1: if (line.contains("CAS")) break; else continue;
            case 2: if (line.contains("NET")) break; else continue;
		}
        QStringList tokens=line.split(";");
        for (int i=0;i<18&&i<tokens.size();i++) {
            table[type]->setItem(j,i-1, new QTableWidgetItem(tokens.at(i)));
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
    bool okay;
	
    if (Address->currentText()=="") {
        googleMapView->setWindowTitle(tr("NTRIP STR Map"));
	}
	else {
        googleMapView->setWindowTitle(QString(tr("NTRIP STR Map: %1")).arg(Address->currentText()));
	}
    googleMapView->ClearMark();
	
    for (int i=0;i<Table0->rowCount();i++) {
        if (Table0->item(i,8)->text()=="") continue;
        LatText=Table0->item(i,8)->text();
        LonText=Table0->item(i,9)->text();
        lat=LatText.toDouble(&okay); if (!okay) continue;
        lon=LonText.toDouble(&okay); if (!okay) continue;
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

