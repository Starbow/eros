#include "settingswindow.h"
#include <QFileDialog>
#include <QDir>
#include <QLocale>
SettingsWindow::SettingsWindow(QWidget *parent, Config *cfg)
	: QDialog(parent)
{
	ui.setupUi(this);

	QString language_path = QString("%1/languages").arg(QApplication::applicationDirPath());
	QDir language_dir(language_path);

	QStringList language_files = language_dir.entryList(QStringList("*.qm"), QDir::Filter::Files);
	for (int i = 0; i < language_files.size(); i++)
	{
		QString locale_name = language_files[i];
		locale_name.truncate(locale_name.lastIndexOf('.'));
		QLocale locale(locale_name);
		QString country = locale.name();
		country.remove(0, country.indexOf('_') + 1);
		ui.cmbLanguage->addItem(QIcon(QString(":/img/client/icons/flags/%1").arg(country.toLower())), locale.nativeLanguageName(), locale_name);
	}

	this->config_ = cfg;
	
	//no profiles so only the add profile button is avaiable
	if(config_->profiles().size() == 0)
	{
		disableProfileInterface();
		reconnectGUI();
	}
	else
	{  
		//add existing profiles to combobox
		for(int i=0; i< config_->profiles().size(); ++i)
		{
			ui.cmbProfiles->addItem(config_->profiles()[i]->username());	
		}
		
		enableProfileInterface();
		refreshProfileInterface();
	}	

}

SettingsWindow::~SettingsWindow()
{
	config_->save();
}

void SettingsWindow::changeEvent(QEvent* e)
{
	if (e != nullptr)
	{
		switch (e->type())
		{
		case QEvent::LanguageChange:
			ui.retranslateUi(this);
			break;
		}
	}
}

void SettingsWindow::cmbProfiles_changed()
{	
	if (ui.cmbProfiles->currentIndex() >= 0)
	{
		config_->setActiveProfile(config_->profiles()[ui.cmbProfiles->currentIndex()]);
		refreshProfileInterface();
		emit profileChanged();
	}
	else 
	{
		config_->setActiveProfile(nullptr);
		enableProfileInterface();
		disableProfileInterface();
	}
	
	
}

void SettingsWindow::btnNewProfile_click()
{
	bool ok;
	QString new_username = QInputDialog::getText(this, tr("Create Profile"), tr("Enter your starbowmod.com username"), QLineEdit::Normal, "", &ok).trimmed();
	
	if(ok && !new_username.isEmpty() && profileExists(new_username) == false)
	{
		
		Profile *profile = this->config_->createProfile(new_username);
		ui.cmbProfiles->addItem(new_username);
		ui.cmbProfiles->setCurrentIndex(ui.cmbProfiles->count() - 1);
		
		if(this->config_->profiles().size() == 1)
		{
			this->config_->setActiveProfile(profile);
		}

		enableProfileInterface();
		refreshProfileInterface();
		
	}
	else if(profileExists(new_username) == true)
	{
		QMessageBox::critical(this, tr("Profile creation error"), tr("The username provided already exists. Please try again."), QMessageBox::Ok);
	}
}

void SettingsWindow::btnDeleteProfile_click()
{ 
	if (ui.cmbProfiles->currentIndex() < 0)
		return;

	QMessageBox::StandardButton reply = QMessageBox::warning(this, tr("Confirm Deletion"), tr("Are you sure you want to delete the selected user?"), QMessageBox::Yes | QMessageBox::No);	
	
	if(reply == QMessageBox::Yes)
	{

		Profile *profile = this->config_->profiles()[ui.cmbProfiles->currentIndex()];
		this->config_->setActiveProfile(nullptr);
		this->config_->removeProfile(profile);


		ui.cmbProfiles->removeItem(ui.cmbProfiles->currentIndex());

		if(ui.cmbProfiles->count() == 0)
		{
			disableProfileInterface();
		}
		else
		{
			refreshProfileInterface();
		}
		
		emit profileChanged();
	}
}

void SettingsWindow::cmbSearchRange_changed()
{
	if (config_->activeProfile() == nullptr)
		return;

	config_->activeProfile()->setSearchRange(ui.cmbSearchRange->currentData().toInt());
}

void SettingsWindow::cmbAutostart_changed()
{

	bool autostart = false;
	if(ui.cmbAutostart->currentText() == "Yes")
	{
		autostart = true;
	}

	config_->setStartOnLogin(autostart);//ui.cmbAutostart->currentData().toBool());
}

void SettingsWindow::cmbChatLinks_changed()
{
	if (config_->activeProfile() == nullptr)
		return;

	bool links = false;
	if(ui.cmbChatLinks->currentText() == "Yes")
	{
		links = true;
	}

	config_->activeProfile()->setChatLinks(links);
}

void SettingsWindow::cmbLanguage_changed()
{
	if (config_->activeProfile() == nullptr)
		return;
	config_->activeProfile()->setLanguage(ui.cmbLanguage->currentData().toString());
	refreshProfileInterface();
}

void SettingsWindow::btnOK_click()
{
	this->close();
}

void SettingsWindow::btnSetToken_click()
{
	bool ok;
	QString token = QInputDialog::getText(this, tr("Eros authentication token"), tr("Please enter your authentication token. This can be found on your starbowmod.com profile."), QLineEdit::Normal,"",&ok);	
	
	if(!token.isEmpty() && ok)
	{
		if (token.trimmed().length() == 30) 
		{
			this->config_->activeProfile()->setToken(token.trimmed());
			emit profileChanged();
		}
		else
		{
			QMessageBox::critical(this, tr("Bad token"), tr("Authentication tokens are 30 characters long. Please ensure you have copied all of the token in to the input box."), QMessageBox::Ok);
		}
	}
	
}

void SettingsWindow::reconnectGUI()
{
	//just make sure they are disconnected first so we dont get double connection anywhere
	this->disconnect(ui.cmbProfiles,       SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbProfiles_changed()));
	this->disconnect(ui.btnNewProfile,     SIGNAL(clicked()), this, SLOT(btnNewProfile_click()));
	this->disconnect(ui.btnDeleteProfile,  SIGNAL(clicked()), this, SLOT(btnDeleteProfile_click()));
	this->disconnect(ui.btnSetToken,       SIGNAL(clicked()), this, SLOT(btnSetToken_click()));
	this->disconnect(ui.cmbSearchRange,    SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbSearchRange_changed()));
	this->disconnect(ui.cmbAutostart,      SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbAutostart_changed()));
	this->disconnect(ui.cmbChatLinks,      SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbChatLinks_changed()));
	this->disconnect(ui.cmbLanguage,       SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbLanguage_changed()));
//	this->disconnect(ui.chkAutoJoin,       SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));

	this->connect(ui.cmbProfiles,       SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbProfiles_changed()));
	this->connect(ui.btnNewProfile,     SIGNAL(clicked()), this, SLOT(btnNewProfile_click()));
	this->connect(ui.btnDeleteProfile,  SIGNAL(clicked()), this, SLOT(btnDeleteProfile_click()));
	this->connect(ui.btnSetToken,       SIGNAL(clicked()), this, SLOT(btnSetToken_click()));
	this->connect(ui.btnSelectWatchFolder,       SIGNAL(clicked()), this, SLOT(btnSelectWatchFolder_click()));
	this->connect(ui.cmbSearchRange,    SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbSearchRange_changed()));
	this->connect(ui.cmbAutostart,      SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbAutostart_changed()));
	this->connect(ui.cmbChatLinks,      SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbChatLinks_changed()));
	this->connect(ui.cmbLanguage,       SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbLanguage_changed()));
//	this->connect(ui.chkAutoJoin,       SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
}

bool SettingsWindow::profileExists(QString profileName)
{
	for (int i = 0; i < config_->profiles().size(); ++i)
	{
		if(profileName == config_->profiles()[i]->username())
		{
			return true;
		}
	}
	return false;
}

void SettingsWindow::disableProfileInterface()
{
	
	ui.lblProfile->setDisabled(true);
	ui.btnDeleteProfile->setDisabled(true);
	ui.btnSetToken->setDisabled(true);

	
	ui.cmbProfiles->setDisabled(true);
	ui.cmbSearchRange->setDisabled(true);
	ui.cmbAutostart->setDisabled(true);
	ui.cmbChatLinks->setDisabled(true);
	ui.cmbLanguage->setDisabled(true);	
	ui.btnSelectWatchFolder->setDisabled(true);

	
	ui.lblSearchRange->setDisabled(true);
	ui.lblAutostart->setDisabled(true);
	ui.lblChatLinks->setDisabled(true);
	ui.lblLanguage->setDisabled(true);	
	ui.lblUserFolder->setDisabled(true);


	ui.cmbProfiles->setCurrentIndex(-1);
	ui.cmbSearchRange->setCurrentIndex(-1);
	ui.cmbAutostart->setCurrentIndex(-1);	
	ui.cmbChatLinks->setCurrentIndex(-1);	
	ui.cmbLanguage->setCurrentIndex(-1);

	ui.cmbProfiles->disconnect();
	ui.cmbSearchRange->disconnect();
	ui.cmbAutostart->disconnect();
	ui.cmbChatLinks->disconnect();
	ui.cmbLanguage->disconnect();
	
	ui.cmbProfiles->clear();

}

void SettingsWindow::enableProfileInterface()
{
	reconnectGUI();
	ui.lblProfile->setEnabled(true);
	ui.btnDeleteProfile->setEnabled(true);
	ui.btnSetToken->setEnabled(true);

	
	//enable comboboxes
	ui.cmbProfiles->setEnabled(true);
	ui.cmbSearchRange->setEnabled(true);
	ui.cmbAutostart->setEnabled(true);
	ui.cmbChatLinks->setEnabled(true);
	ui.cmbLanguage->setEnabled(true);
	ui.btnSelectWatchFolder->setEnabled(true);

	//enable option lables
	ui.lblSearchRange->setEnabled(true);
	ui.lblAutostart->setEnabled(true);
	ui.lblChatLinks->setEnabled(true);
	ui.lblLanguage->setEnabled(true);
	ui.lblUserFolder->setEnabled(true);
}

void SettingsWindow::refreshProfileInterface()
{
	//set the current profile
	if(ui.cmbProfiles->count() > 0)
	{
		ui.cmbProfiles->setCurrentIndex(this->config_->profiles().indexOf(this->config_->activeProfile())); //is this the best way?
	}

	//set options
	int range = config_->activeProfile()->searchRange();
	ui.cmbSearchRange->clear();
	for (int i = 1; i <= 5; i++)
	{
		ui.cmbSearchRange->addItem(tr("Within %n division(s)", "", i), i);
	}

	for (int i = 0; i < ui.cmbSearchRange->count(); i++)
	{
		if (range == ui.cmbSearchRange->itemData(i).toInt())
		{
			ui.cmbSearchRange->setCurrentIndex(i);		
			break;
		}
	}
	if (ui.cmbSearchRange->currentIndex() < 0)
		ui.cmbSearchRange->setCurrentIndex(0);
	ui.cmbAutostart->setCurrentIndex(ui.cmbAutostart->findText(boolToYesNo(config_->startOnLogin())));
	ui.cmbChatLinks->setCurrentIndex(ui.cmbChatLinks->findText(boolToYesNo(this->config_->activeProfile()->chatLinks())));
	int pos = ui.cmbLanguage->findData(this->config_->activeProfile()->language());
	if (pos < 0)
		pos = ui.cmbLanguage->findData("en_GB");
	ui.cmbLanguage->setCurrentIndex(pos);
	ui.txtWatchFolder->setText(this->config_->activeProfile()->replayFolder());
}

void SettingsWindow::chkAutoJoin_stateChanged(int state)
{
	this->config_->setAutoJoin(state == Qt::CheckState::Checked);
}

const QString SettingsWindow::boolToYesNo(bool yesno)
{
	if(yesno)
	{
		return "Yes";
	}
	else
	{
		return "No";
	}
}

void SettingsWindow::btnSelectWatchFolder_click()
{
	QString directory = QFileDialog::getExistingDirectory(this, "Select Folder", this->config_->activeProfile()->replayFolder());
	if (!directory.isEmpty() && !directory.isNull())
	{
		this->config_->activeProfile()->setReplayFolder(directory);
		ui.txtWatchFolder->setText(directory);
	}
}
