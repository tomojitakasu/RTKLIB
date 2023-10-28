//---------------------------------------------------------------------------
// Ported to Qt by Jens Reimann
#include <clocale>

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
#ifdef QWEBENGINE
    #include "gmview.h"
#endif
#include "browsmain.h"
#include "staoptdlg.h"
//---------------------------------------------------------------------------

#define PRGNAME                 "NTRIP Browser Qt"
#define PRGVERSION              "1.0"
#define NTRIP_HOME              "rtcm-ntrip.org:2101"   // caster list home
#define NTRIP_TIMEOUT           10000                   // response timeout (ms)
#define MAXSRCTBL               512000                  // max source table size (bytes)
#define ENDSRCTBL               "ENDSOURCETABLE"        // end marker of table
#define ADDRESS_WIDTH           184                     // width of Address (px)

static char buff[MAXSRCTBL];                            // source table buffer

MainForm *mainForm;

extern "C" {
    extern int showmsg(const char *, ...)  {return 0;}
    extern void settime(gtime_t) {}
    extern void settspan(gtime_t, gtime_t) {}
}

/* get source table -------------------------------------------------------*/
static char *getsrctbl(const QString addr)
{
    static int lock = 0;
	stream_t str;
    char *p = buff, msg[MAXSTRMSG];
    int len = strlen(ENDSRCTBL);
    unsigned int tick = tickget();

    if (lock) return NULL; else lock = 1;

    strinit(&str);

    if (!stropen(&str, STR_NTRIPCLI, STR_MODE_R, qPrintable(addr))) {
        lock = 0;
        QMetaObject::invokeMethod(mainForm, "showMsg", Qt::QueuedConnection, Q_ARG(QString, QT_TR_NOOP("stream open error")));
		return NULL;
	}
    QMetaObject::invokeMethod(mainForm, "showMsg", Qt::QueuedConnection, Q_ARG(QString, QT_TR_NOOP("connecting...")));

    while (p < buff + MAXSRCTBL - 1) {
        int ns = strread(&str, (uint8_t *)p, (buff + MAXSRCTBL - p - 1));
        p += ns; *p='\0';
        qApp->processEvents();
        int stat = strstat(&str, msg);

        QMetaObject::invokeMethod(mainForm, "showMsg", Qt::QueuedConnection, Q_ARG(QString, msg));

        if (stat <= 0) break;
        if (strstr(buff, ENDSRCTBL)) break;
        if ((int)(tickget() - tick) > NTRIP_TIMEOUT) {
            QMetaObject::invokeMethod(mainForm, "showMsg", Qt::QueuedConnection, Q_ARG(QString, QT_TR_NOOP("response timeout")));
			break;
		}
	}
	strclose(&str);
    lock = 0;
	return buff;
}
//---------------------------------------------------------------------------
MainForm::MainForm(QWidget *parent)
    : QMainWindow(parent)
{
    mainForm = this;
    setupUi(this);
    FiltFmt->setVisible(false);

    QCoreApplication::setApplicationName(PRGNAME);
    QCoreApplication::setApplicationVersion(PRGVERSION);

    setWindowIcon(QIcon(":/icons/srctblbrows_Icon"));

    // retrieve config file name
    QString prg_filename = QApplication::applicationFilePath();
    QFileInfo prg_fileinfo(prg_filename);
    iniFile = prg_fileinfo.absolutePath() + "/" + prg_fileinfo.baseName() + ".ini";

 #ifdef QWEBENGINE
    mapView = new GoogleMapView(this);
#endif
    staListDialog = new StaListDialog(this);
    loadTimer = new QTimer();

    TypeCas->setDefaultAction(MenuViewCas);
    TypeNet->setDefaultAction(MenuViewNet);
    TypeSrc->setDefaultAction(MenuViewSrc);
    TypeStr->setDefaultAction(MenuViewStr);

    connect(Address, SIGNAL(textActivated(QString)), this, SLOT(addressChanged()));
    connect(BtnUpdate, SIGNAL(clicked(bool)), this, SLOT(btnUpdateClicked()));
    connect(BtnMap, SIGNAL(clicked(bool)), this, SLOT(btnMapClicked()));
    connect(BtnList, SIGNAL(clicked(bool)), this, SLOT(btnListClicked()));
    connect(BtnSta, SIGNAL(clicked(bool)), this, SLOT(btnStatsionClicked()));
    connect(StaMask, SIGNAL(clicked(bool)), this, SLOT(stationMaskClicked()));
    connect(MenuOpen, SIGNAL(triggered(bool)), this, SLOT(menuOpenClicked()));
    connect(MenuSave, SIGNAL(triggered(bool)), this, SLOT(menuSaveClicked()));
    connect(MenuQuit, SIGNAL(triggered(bool)), this, SLOT(menuQuitClicked()));
    connect(MenuUpdateCaster, SIGNAL(triggered(bool)), this, SLOT(menuUpdateCasterClicked()));
    connect(MenuUpdateTable, SIGNAL(triggered(bool)), this, SLOT(menuUpdateTableClicked()));
    connect(MenuViewCas, SIGNAL(triggered(bool)), this, SLOT(menuViewCasterClicked()));
    connect(MenuViewNet, SIGNAL(triggered(bool)), this, SLOT(menuViewNetClicked()));
    connect(MenuViewSrc, SIGNAL(triggered(bool)), this, SLOT(menuViewSourceClicked()));
    connect(MenuViewStr, SIGNAL(triggered(bool)), this, SLOT(menuViewStrClicked()));
    connect(MenuAbout, SIGNAL(triggered(bool)), this, SLOT(menuAboutClicked()));
    connect(loadTimer, SIGNAL(timeout()), this, SLOT(loadTimerExpired()));
    connect(Table0, SIGNAL(cellClicked(int,int)), this, SLOT(Table0SelectCell(int,int)));

    BtnMap->setEnabled(false);
#ifdef QWEBENGINE
    BtnMap->setEnabled(true);
#endif

    Table0->setSortingEnabled(true);
    Table1->setSortingEnabled(true);
    Table2->setSortingEnabled(true);

    statusbar->addWidget(Panel2);

    strinitcom();

    loadTimer->setInterval(100);
}
//---------------------------------------------------------------------------
void MainForm::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    const QString colw0 = "30,74,116,56,244,18,52,62,28,50,50,18,18,120,28,18,18,40,600,";
    const QString colw1 = "30,112,40,96,126,18,28,50,50,160,40,600,0,0,0,0,0,0,0,";
    const QString colw2 = "30,80,126,18,18,300,300,300,600,0,0,0,0,0,0,0,0,0,0,";
    QSettings setting(iniFile, QSettings::IniFormat);
    QString list, url = "";
    QStringList colw, stas;
    int i;

    QCommandLineParser parser;
    parser.setApplicationDescription(PRGNAME);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("URL", QCoreApplication::translate("main", "file with list of Ntrip sources"));

    parser.process(*QApplication::instance());

    const QStringList args = parser.positionalArguments();

    if (args.count() >= 1) url = args.at(0);

    setWindowTitle(QString("%1 ver.%2 %3").arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL));

    list = setting.value("srctbl/addrlist", "").toString();
    QStringList items = list.split("@");
    foreach(QString item, items){
        Address->addItem(item);
    }

    if (url != "") {
        Address->setCurrentText(url);
        getTable();
    } else {
        Address->setCurrentText(setting.value("srctbl/address", "").toString());
	}

    fontScale = physicalDpiX();

    colw = setting.value("srctbl/colwidth0", colw0).toString().split(",");
    i = 0;
    foreach(QString width, colw){
        Table0->setColumnWidth(i++, width.toDouble() * fontScale / 96);
    }
    colw = setting.value("srctbl/colwidth1", colw1).toString().split(",");
    i = 0;
    foreach(QString width, colw){
        Table1->setColumnWidth(i++, width.toDouble() * fontScale / 96);
    }
    colw = setting.value("srctbl/colwidth2", colw2).toString().split(",");
    i = 0;
    foreach(QString width, colw){
        Table2->setColumnWidth(i++, width.toDouble() * fontScale / 96);
	}

    stationList.clear();
    for (int i = 0; i < 10; i++) {
        stas = setting.value(QString("sta/station%1").arg(i), "").toString().split(",");
        foreach(QString sta, stas){
            stationList.append(sta);
        }
    }

    showTable();
    updateEnable();
    getTable();

    QTimer::singleShot(0, this, SLOT(updateTable()));
}
//---------------------------------------------------------------------------
void MainForm::closeEvent(QCloseEvent *)
{
    QSettings setting(iniFile, QSettings::IniFormat);
    QString list, colw;

    // save table layout
    setting.setValue("srctbl/address", Address->currentText());
    for (int i = 0; i < Address->count(); i++)
        list = list + Address->itemText(i) + "@";
    setting.setValue("srctbl/addrlist", list);

    colw = "";
    for (int i = 0; i < Table0->columnCount(); i++)
        colw = colw + QString::number(Table0->columnWidth(i) * 96 / fontScale);
    setting.setValue("srctbl/colwidth0", colw);

    colw = "";
    for (int i = 0; i < Table1->columnCount(); i++)
        colw = colw + QString::number(Table1->columnWidth(i) * 96 / fontScale);
    setting.setValue("srctbl/colwidth1", colw);

    colw = "";
    for (int i = 0; i < Table2->columnCount(); i++)
        colw = colw + QString::number(Table2->columnWidth(i) * 96 / fontScale);
    setting.setValue("srctbl/colwidth2", colw);

    for (int i = 0; i < 10; i++)
        setting.setValue(QString("sta/station%1").arg(i), stationList.join(","));
}
//---------------------------------------------------------------------------
void MainForm::menuOpenClicked()
{
    QString fileName = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open"), QString(), tr("All File (*.*)")));
    QFile file(fileName);

    sourceTable = "";

    if (!file.open(QIODevice::ReadOnly)) return;

    sourceTable = file.readAll();

    addressCaster = Address->currentText();

    showTable();
    showMsg(tr("source table loaded"));
}
//---------------------------------------------------------------------------
void MainForm::menuSaveClicked()
{
    QString fileName = QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Save File"), QString(), tr("All File (*.*)")));
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) return;

    file.write(sourceTable.toLatin1());

    showMsg(tr("source table saved"));
}
//---------------------------------------------------------------------------
void MainForm::menuQuitClicked()
{
    close();
}
//---------------------------------------------------------------------------
void MainForm::menuUpdateCasterClicked()
{
    getCaster();
}
//---------------------------------------------------------------------------
void MainForm::menuUpdateTableClicked()
{
    getTable();
}
//---------------------------------------------------------------------------
void MainForm::menuViewStrClicked()
{
    MenuViewCas->setChecked(false);
    MenuViewNet->setChecked(false);
    MenuViewSrc->setChecked(false);

    showTable();
}
//---------------------------------------------------------------------------
void MainForm::menuViewCasterClicked()
{
    MenuViewStr->setChecked(false);
    MenuViewNet->setChecked(false);
    MenuViewSrc->setChecked(false);

    showTable();
}
//---------------------------------------------------------------------------
void MainForm::menuViewNetClicked()
{
    MenuViewStr->setChecked(false);
    MenuViewCas->setChecked(false);
    MenuViewSrc->setChecked(false);

    showTable();
}
//---------------------------------------------------------------------------
void MainForm::menuViewSourceClicked()
{
    MenuViewStr->setChecked(false);
    MenuViewCas->setChecked(false);
    MenuViewNet->setChecked(false);

    showTable();
}
//---------------------------------------------------------------------------
void MainForm::menuAboutClicked()
{
    AboutDialog *aboutDialog = new AboutDialog(this);

    aboutDialog->aboutString = PRGNAME;
    aboutDialog->iconIndex = 7;
    aboutDialog->exec();

    delete aboutDialog;
}
//---------------------------------------------------------------------------
void MainForm::btnMapClicked()
{
    loadTimer->start();
    mapView->show();
}
//---------------------------------------------------------------------------
void MainForm::Table0SelectCell(int ARow, int ACol)
{
    Q_UNUSED(ACol);
    QString title;

    if (0 <= ARow && ARow < Table0->rowCount()) {
        title = Table0->item(ARow,1)->text();
        mapView->highlightMark(title);
        mapView->setWindowTitle(QString(tr("NTRIP Data Stream Map: %1/%2")).arg(Address->currentText()).arg(title));
	}
}
//---------------------------------------------------------------------------
void MainForm::btnListClicked()
{
    getCaster();
}
//---------------------------------------------------------------------------
void MainForm::btnUpdateClicked()
{
    getTable();
}
//---------------------------------------------------------------------------
void MainForm::addressChanged()
{
    getTable();
}
//---------------------------------------------------------------------------
void MainForm::loadTimerExpired()
{
    if (!mapView->getState()) return;

    updateMap();

    loadTimer->stop();
}
//---------------------------------------------------------------------------
void MainForm::getCaster(void)
{
    if (casterWatcher.isRunning()) return;

    QString addressText = Address->currentText();
    QString addr = NTRIP_HOME;

    if (addressText != "") addr = addressText;

    BtnList->setEnabled(false);
    MenuUpdateCaster->setEnabled(false);

    QFuture<char *> tblFuture = QtConcurrent::run(getsrctbl, addr);
    connect(&casterWatcher, SIGNAL(finished()), this, SLOT(updateCaster()));
    casterWatcher.setFuture(tblFuture);
}
//---------------------------------------------------------------------------
void MainForm::updateCaster()
{
    QString text;
    QString srctbl;

    BtnList->setEnabled(true);
    MenuUpdateCaster->setEnabled(true);

    if ((srctbl = casterWatcher.result()).isEmpty()) return;

    text = Address->currentText();
    Address->clear();
    Address->setCurrentText(text);
    Address->addItem("");

    QStringList tokens, lines = srctbl.split('\n');
    foreach(const QString &line, lines) {
        if (!line.contains("CAS")) continue;

        tokens = line.split(";");
        if (tokens.size() != 3) continue;
        Address->addItem(tokens.at(1) + ":" + tokens.at(2));
	}
    if (Address->count() > 1) Address->setCurrentIndex(0);
}
//---------------------------------------------------------------------------
void MainForm::getTable()
{
    if (tableWatcher.isRunning()) return;

    QString Address_Text = Address->currentText();
    QString addr = NTRIP_HOME;

    if (Address_Text != "") addr = Address_Text;

    BtnUpdate->setEnabled(false);
    Address->setEnabled(false);
    MenuUpdateTable->setEnabled(false);

    QFuture<char *> tblFuture = QtConcurrent::run(getsrctbl, addr);
    connect(&tableWatcher, SIGNAL(finished()), this, SLOT(updateTable()));
    tableWatcher.setFuture(tblFuture);
}
//---------------------------------------------------------------------------
void MainForm::updateTable(void)
{
    QString srctbl;

    BtnUpdate->setEnabled(true);
    Address->setEnabled(true);
    MenuUpdateTable->setEnabled(true);

    srctbl = tableWatcher.result();

    if (!srctbl.isEmpty()) {
        sourceTable = srctbl;
        addressCaster = Address->currentText();
	}

    showTable();
}
//---------------------------------------------------------------------------
void MainForm::showTable(void)
{
    const QString ti[3][19] = { {tr("No"), tr("Mountpoint"), tr("ID"),	tr("Format"),	      tr("Format-Details"), tr("Carrier"), tr("Nav-System"),
                      tr("Network"), tr("Country"), tr("Latitude"), tr("Longitude"), tr("NMEA"), tr("Solution"),
                      tr("Generator"), "Compr-Encrp", "Authentication", "Fee", "Bitrate", "" },
                    { tr("No"), tr("Host"),	tr("Port"),	tr("ID"),	      tr("Operator"),	    tr("NMEA"),	   tr("Country"),   tr("Latitude"), tr("Longitude"),
                      tr("Fallback_Host"), tr("Fallback_Port"), "", "", "", "", "", "", "" },
                    { tr("No"), tr("ID"),		tr("Operator"), tr("Authentication"), tr("Fee"),	    tr("Web-Net"), tr("Web-Str"),   tr("Web-Reg"),  "",		    "","", "",
                      "", "", "", "", "", "" } };


    QTableWidget *table[] = { Table0, Table1, Table2 };
    QAction *action[] = { MenuViewStr, MenuViewCas, MenuViewNet, MenuViewSrc };
    int i, j, ns, type;

    Table3->setVisible(false);
    for (i = 0; i < 3; i++) table[i]->setVisible(false);

    type = MenuViewStr->isChecked() ? 0 : (MenuViewCas->isChecked() ? 1 : (MenuViewNet->isChecked() ? 2 : 3));
    for (i = 0; i < 4; i++) action[i]->setChecked(i == type);

    if (type == 3) {
        Table3->setVisible(true);
        Table3->setPlainText("");
    } else {
        table[type]->setVisible(true);
        table[type]->setRowCount(1);
        table[type]->setColumnCount(18);
        for (i = 0; i < 18; i++) {
            table[type]->setHorizontalHeaderItem(i, new QTableWidgetItem(ti[type][i]));
            table[type]->setItem(0, i, new QTableWidgetItem(""));
		}
	}
    if (addressCaster != Address->currentText()) return;
    if (type == 3) {
        Table3->setPlainText(sourceTable);
		return;
	}
    QStringList lines = sourceTable.split("\n");
    ns = 0;
    foreach(QString line, lines) {
		switch (type) {
        case 0: if (line.startsWith("STR")) ns++; break;
        case 1: if (line.startsWith("CAS")) ns++; break;
        case 2: if (line.startsWith("NET")) ns++; break;
		}
	}
    if (ns <= 0) return;
    table[type]->setRowCount(ns);
    j = 0;
    foreach(QString line, lines) {
		switch (type) {
            case 0: if (line.startsWith("STR")) break; else continue;
            case 1: if (line.startsWith("CAS")) break; else continue;
            case 2: if (line.startsWith("NET")) break; else continue;
		}
        table[type]->setItem(j, 0, new QTableWidgetItem(QString::number(j)));
        QStringList tokens = line.split(";");
        for (int i = 0; i < 18 && i < tokens.size(); i++)
            table[type]->setItem(j, i, new QTableWidgetItem(tokens.at(i)));
		j++;
	}
    updateMap();
}
//---------------------------------------------------------------------------
void MainForm::showMsg(const QString &msg)
{
    QString str = msg;

    Message->setText(str);
}
//---------------------------------------------------------------------------
void MainForm::updateMap(void)
{
    QString title, msg, latitudeText, longitudeText;
    double latitude, longitude;
    bool okay;

    if (Address->currentText() == "")
        mapView->setWindowTitle(tr("NTRIP Data Stream Map"));
    else
        mapView->setWindowTitle(QString(tr("NTRIP Data Stream Map: %1")).arg(Address->currentText()));
    mapView->clearMark();

    for (int i = 0; i < Table0->rowCount(); i++) {
        if (Table0->item(i, 8)->text() == "") continue;

        latitudeText = Table0->item(i, 9)->text();
        longitudeText = Table0->item(i, 10)->text();
        latitude = latitudeText.toDouble(&okay); if (!okay) continue;
        longitude = longitudeText.toDouble(&okay); if (!okay) continue;

        title = Table0->item(i, 1)->text();
        msg = "<b>" + Table0->item(i, 1)->text() + "</b>: " + Table0->item(i, 2)->text() + " (" + Table0->item(i,8)->text()+")<br>"+
              "Format: "+ Table0->item(i, 3)->text() + ", " + Table0->item(i, 4)->text() + ", <br> " +
              "Nav-Sys: "+Table0->item(i, 6)->text() + "<br>" +
              "Network: "+Table0->item(i, 7)->text() + "<br>" +
              "Latitude/Longitude: "+Table0->item(i, 9)->text()+", "+Table0->item(i, 10)->text()+"<br>"+
              "Generator: "+Table0->item(i, 13)->text();

        mapView->addMark(latitude, longitude, title, msg);
	}
}
//---------------------------------------------------------------------------
void MainForm::btnStatsionClicked()
{
    staListDialog->exec();
}
//---------------------------------------------------------------------------
void MainForm::updateEnable(void)
{
    BtnSta->setEnabled(StaMask->isChecked());
}
//---------------------------------------------------------------------------
void MainForm::stationMaskClicked()
{
    updateEnable();
}
//---------------------------------------------------------------------------
