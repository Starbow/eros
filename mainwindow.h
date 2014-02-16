#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QSettings>
#include "config.h"
#include "settingswindow.h"
#include "ui_mainwindow.h"
#include "../liberos/eros.h"


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(Eros *eros, QWidget *parent = 0);
	~MainWindow();

	void loadSettings();
	void saveSettings();
private slots:
	void label_linkActivated(const QString &link);
	void tabContainer_tabCloseRequested(int index);

private:
	Ui::MainWindow ui;
	QString configPath_;
	Eros *eros_;
	Config *config_;
	SettingsWindow *settings_window_;
	QString username_;
	QString authtoken_;
	QString server_;
};

#endif // EROS_H
