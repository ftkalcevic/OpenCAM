#include "OpenCAM.h"
#include "stdafx.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Franksworkshop");
    QCoreApplication::setOrganizationDomain("www.franksworkshop.com");
    QCoreApplication::setApplicationName("OpenCAM");

    OpenCAM w;
    w.show();
    return a.exec();
}
