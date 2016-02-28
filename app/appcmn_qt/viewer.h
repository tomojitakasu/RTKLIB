//---------------------------------------------------------------------------
#ifndef viewerH
#define viewerH
//---------------------------------------------------------------------------
#define MAXLINE		20000

#include <QDialog>
#include "ui_viewer.h"

class ViewerOptDialog;

//---------------------------------------------------------------------------
class TextViewer : public QDialog, private Ui::TextViewer
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent*);

    ViewerOptDialog *viewerOptDialog;
public slots:
    void BtnCloseClick();
    void BtnReadClick();
    void BtnOptClick();
    void BtnReloadClick();
    void BtnFindClick();

private:
    QString File;
    QString TextStr;
	
    void ReadText(QString file);
    void UpdateText(void);

public:
	int Option;
    static QColor Color1,Color2;
    static QFont FontD;

    void Read(QString file);
    void Save(QString file);

    TextViewer(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
