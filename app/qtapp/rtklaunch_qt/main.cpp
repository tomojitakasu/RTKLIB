#include "launchmain.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainForm w;
    w.show();

    return a.exec();
}
