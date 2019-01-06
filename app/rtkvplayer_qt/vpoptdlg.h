#ifndef VPOPTDLG_H
#define VPOPTDLG_H

#include <QDialog>
#include "ui_vpoptdlg.h"

class QShowEvent;

class VideoPlayerOptDialog : public QDialog, private Ui::VideoPlayerOptionDialog
{
    Q_OBJECT
public:
    explicit VideoPlayerOptDialog(QWidget *parent = nullptr);

public slots:
    void BtnOkClick();

protected:
    void showEvent(QShowEvent*);

};

#endif // VPOPTDLG_H
