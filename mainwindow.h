#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QSettings>
#include "config.h"
#include "settingswindow.h"
#include "ui_mainwindow.h"
#include "../liberos/eros.h"
#include "Chat.h"

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

	void debugStateChange(ErosState state);

private:
	Ui::MainWindow ui;

	QString configPath_;
	Eros *eros_;
	Config *config_;
	SettingsWindow *settings_window_;
	QString username_;
	QString authtoken_;
	QString server_;

	Chat *chat_;

	
};

#endif // EROS_H
