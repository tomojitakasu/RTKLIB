//---------------------------------------------------------------------------

#include "plotmain.h"
#include "fileseldlg.h"

#include <QShowEvent>
#include <QDir>
#include <QFileInfoList>
#include <QTreeView>
#include <QFileSystemModel>
#include <QRegularExpression>

extern Plot *plot;

//---------------------------------------------------------------------------
FileSelDialog::FileSelDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    dirModel=new QFileSystemModel(this);
    dirModel->setFilter(QDir::Dirs|QDir::NoDotAndDotDot);

#ifdef FLOATING_DIRSELECTOR
    DirSelector= new QTreeView(0);
    DirSelector->setWindowFlags(Qt::Window|Qt::X11BypassWindowManagerHint|Qt::FramelessWindowHint);
#else
    DirSelector= new QTreeView(this);
#endif
    Panel2->layout()->addWidget(DirSelector);
    DirSelector->setModel(dirModel);
    DirSelector->hideColumn(1); DirSelector->hideColumn(2);DirSelector->hideColumn(3); //only show names

    fileModel = new QFileSystemModel(this);
    fileModel->setFilter((fileModel->filter()& ~QDir::Dirs & ~QDir::AllDirs));
    fileModel->setNameFilterDisables(false);
    FileList->setModel(fileModel);

    connect(DriveSel,SIGNAL(currentIndexChanged(QString)),this,SLOT(DriveSelChanged()));
    connect(DirSelector,SIGNAL(clicked(QModelIndex)),this,SLOT(DirSelChange(QModelIndex)));
    connect(DirSelector,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(DirSelSelected(QModelIndex)));
    connect(BtnDirSel,SIGNAL(clicked(bool)),this,SLOT(BtnDirSelClick()));
    connect(FileList,SIGNAL(clicked(QModelIndex)),this,SLOT(FileListClick(QModelIndex)));
    connect(Filter,SIGNAL(currentIndexChanged(QString)),this,SLOT(FilterClick()));

}
FileSelDialog::~FileSelDialog() {
    delete DirSelector;
}

//---------------------------------------------------------------------------
void  FileSelDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    QFileInfoList drives=QDir::drives();
    if (drives.size()>1&&drives.at(0).filePath()!="/"){
        Panel1->setVisible(true);
        DriveSel->clear();

        foreach (const QFileInfo & drive,drives) {
            DriveSel->addItem(drive.filePath());
        }
    } else Panel1->setVisible(false); // do not show drive selection on unix

    if (Dir=="") Dir=drives.at(0).filePath();

    DriveSel->setCurrentText(Dir.mid(0,Dir.indexOf(":")+2));
    dirModel->setRootPath(Dir);
    DirSelector->setVisible(false);
    DirSelected->setText(Dir);
    fileModel->setRootPath(Dir);
    FileList->setRootIndex(fileModel->index(Dir));
}
//---------------------------------------------------------------------------
void  FileSelDialog::DriveSelChanged()
{
    DirSelector->setVisible(false);

    DirSelector->setRootIndex(dirModel->index(DriveSel->currentText()));
    DirSelected->setText(DriveSel->currentText());
}
//---------------------------------------------------------------------------
void  FileSelDialog::BtnDirSelClick()
{
#ifdef FLOATING_DIRSELECTOR
    QPoint pos=Panel5->mapToGlobal(BtnDirSel->pos());
    pos.rx()+=BtnDirSel->width()-DirSelector->width();
    pos.ry()+=BtnDirSel->height();

    DirSelector->move(pos);
#endif
    DirSelector->setVisible(!DirSelector->isVisible());
}
//---------------------------------------------------------------------------
void  FileSelDialog::DirSelChange(QModelIndex index)
{
    DirSelector->expand(index);

    Dir=dirModel->filePath(index);
    DirSelected->setText(Dir);
    fileModel->setRootPath(Dir);
    FileList->setRootIndex(fileModel->index(Dir));
}
//---------------------------------------------------------------------------
void  FileSelDialog::DirSelSelected(QModelIndex)
{
    DirSelector->setVisible(false);
}
//---------------------------------------------------------------------------
void  FileSelDialog::FileListClick(QModelIndex index)
{
    QStringList file;
    file.append(fileModel->filePath(index));
    plot->ReadSol(file,0);

    DirSelector->setVisible(false);
}
//---------------------------------------------------------------------------
void  FileSelDialog::FilterClick()
{
    QString filter=Filter->currentText();

    // only keep data between brackets
    filter=filter.mid(filter.indexOf("(")+1);
    filter.remove(")");

    fileModel->setNameFilters(filter.split(" "));
    DirSelector->setVisible(false);
}
//---------------------------------------------------------------------------

