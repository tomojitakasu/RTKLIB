//---------------------------------------------------------------------------
#ifndef fileseldlgH
#define fileseldlgH

//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_fileseldlg.h"

class QShowEVent;
class QTreeView;
class QFileSystemModel;
class QModelIndex;

//---------------------------------------------------------------------------
class FileSelDialog : public QDialog, private Ui::FileSelDialog
{
    Q_OBJECT
public slots:
    void FileListClick(QModelIndex);
    void DirSelChange(QModelIndex);
    void DirSelSelected(QModelIndex);
    void DriveSelChanged();
    void FilterClick();
    void BtnDirSelClick();
protected:
    void showEvent(QShowEvent*);
    QTreeView *DirSelector;
    QFileSystemModel *fileModel,*dirModel;

public:
    QString Dir;
    explicit FileSelDialog(QWidget *parent=0);
    ~FileSelDialog();
};
//---------------------------------------------------------------------------
#endif
