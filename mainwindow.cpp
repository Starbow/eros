#include "mainwindow.h"
#include "settingswindow.h"

#include <QMessageBox>
MainWindow::MainWindow(Eros *eros, QWidget *parent )
	: QMainWindow(parent)
{
	ui.setupUi(this);
	this->settings_ = new QSettings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "Starbow", "Eros");
	loadSettings();


	// The user should be prevented from emptying invalid values in the settings dialog.
	while (this->username_.isEmpty() || this->authtoken_.isEmpty() || this->server_.isEmpty())
	{
		QMessageBox::information(this, "Eros", tr("Welcome to Eros! You need to configure some settings in order to continue. The options window will now open."));
		SettingsWindow *settingsWindow = new SettingsWindow(this, this->settings_);
		int result = settingsWindow->exec();
		delete settingsWindow;

		loadSettings();
	}
}

void MainWindow::loadSettings()
{
	this->username_ = this->settings_->value("profile/username", QString("")).toString();
	this->authtoken_ = this->settings_->value("profile/authtoken", QString("")).toString();
	this->server_ = this->settings_->value("connection/server", QString("")).toString();
}

MainWindow::~MainWindow()
{

}
