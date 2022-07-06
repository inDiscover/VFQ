#include "BillFormatterApp.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //BillFormatterApp w;
    BillFormatterApp::instance()->show();
    return a.exec();
}
