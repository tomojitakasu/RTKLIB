//---------------------------------------------------------------------------

#ifndef markdlgH
#define markdlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_markdlg.h"

class KeyDialog;

//---------------------------------------------------------------------------
class QMarkDialog : public QDialog, private Ui::MarkDialog
{
    Q_OBJECT
public slots:
    void BtnCancelClick();
    void BtnOkClick();
    void ChkMarkerNameClick();
    void BtnRepDlgClick();

protected:
    void showEvent(QShowEvent *);

private:
    void UpdateEnable(void);
    KeyDialog *keyDialog;

public:
    QString Marker,Comment;
    int PosMode,NMark;
	
    explicit QMarkDialog(QWidget *parent);
	
};
//---------------------------------------------------------------------------
#endif
