#include "getmain.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainForm w(NULL);
    w.show();

    return a.exec();
}
