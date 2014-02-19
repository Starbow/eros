#include "mainwindow.h"
#include "crashhandler.h"
#include <QtWidgets/QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setOrganizationDomain("starbowmod.com");
	a.setOrganizationName("Starbow");
	a.setApplicationName("Eros");
	a.setApplicationDisplayName("Eros");
	
	#if defined(Q_OS_WIN32)
		CrashHandler::instance()->Init(QCoreApplication::applicationDirPath());
	#elif defined(Q_OS_LINUX)
		CrashHandler::instance()->Init(QCoreApplication::applicationDirPath());
	#elif defined(Q_OS_MAC)
		CrashHandler::instance()->Init(QCoreApplication::applicationDirPath());
	#endif
	Eros *eros = new Eros(0);
	QThread *erosThread = new QThread();
	eros->moveToThread(erosThread);
	erosThread->start();

	MainWindow w(eros, 0);
	w.show();
	return a.exec();
}
