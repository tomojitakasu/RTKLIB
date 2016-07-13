//---------------------------------------------------------------------------

#ifndef markdlgH
#define markdlgH
//---------------------------------------------------------------------------
#include <QDialog>
#include "ui_markdlg.h"
//---------------------------------------------------------------------------
class QMarkDialog : public QDialog, private Ui::MarkDialog
{
    Q_OBJECT
public slots:
    void BtnCancelClick();
    void BtnOkClick();
    void ChkMarkerNameClick();

protected:
    void showEvent(QShowEvent *);

private:
    void UpdateEnable(void);

public:
    QString Marker,Comment;
	int PosMode;
	
    explicit QMarkDialog(QWidget *parent);
	
};
//---------------------------------------------------------------------------
#endif
