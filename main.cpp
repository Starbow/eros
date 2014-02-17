#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setOrganizationDomain("starbowmod.com");
	a.setOrganizationName("Starbow");
	a.setApplicationName("Eros");
	a.setApplicationDisplayName("Eros");
	
	Eros *eros = new Eros(0);
	QThread *erosThread = new QThread();
	eros->moveToThread(erosThread);
	erosThread->start();

	MainWindow w(eros, 0);
	w.show();
	return a.exec();
}
