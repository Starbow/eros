#include "settingswindow.h"
#include <QFileDialog>
SettingsWindow::SettingsWindow(QWidget *parent, Config *cfg)
	: QDialog(parent)
{
	ui.setupUi(this);

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

void SettingsWindow::cmbProfiles_changed()
{	
	config_->setActiveProfile(config_->profiles()[ui.cmbProfiles->currentIndex()]);
	refreshProfileInterface();
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
		
	}
}

void SettingsWindow::cmbSearchRange_changed()
{

	QString searchRange = ui.cmbSearchRange->currentText();

	config_->activeProfile()->setSearchRange(searchRangeToInt(searchRange));//ui.cmbSearchRange->currentText().toInt());//->currentData().toInt());
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
	bool links = false;
	if(ui.cmbChatLinks->currentText() == "Yes")
	{
		links = true;
	}
	config_->activeProfile()->setChatLinks(links);
}

void SettingsWindow::cmbLanguage_changed()
{
	config_->activeProfile()->setLanguage(ui.cmbLanguage->currentText());
}

void SettingsWindow::btnOK_click()
{
	this->close();
}

void SettingsWindow::btnSetToken_click()
{
	bool ok;
	QString token = QInputDialog::getText(this, tr("Eros authentication token"), tr("Please enter your authentication token. This can be found on your starbowmod.com profile."), QLineEdit::Normal,"",&ok);	
	
	if(token.isEmpty() == false && ok == true)
	{
		this->config_->activeProfile()->setToken(token);
	}
	else
	{
		QMessageBox::critical(this, tr("Bad token"), tr("Please note that you will not be able to play ranked games if you do not provide a valid authentication token."), QMessageBox::Ok);
	}
}

void SettingsWindow::reconnectGUI()
{
	//just make sure they are disconnected first so we dont get double connection anywhere
	this->disconnect(ui.cmbProfiles,       SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbProfiles_changed()));
	this->disconnect(ui.btnNewProfile,     SIGNAL(pressed()), this, SLOT(btnNewProfile_click()));
	this->disconnect(ui.btnDeleteProfile,  SIGNAL(clicked()), this, SLOT(btnDeleteProfile_click()));
	//this->disconnect(ui.btnOK,             SIGNAL(clicked()), this, SLOT(btnOK_click()));
	this->disconnect(ui.btnSetToken,       SIGNAL(clicked()), this, SLOT(btnSetToken_click()));
	this->disconnect(ui.cmbSearchRange,    SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbSearchRange_changed()));
	this->disconnect(ui.cmbAutostart,      SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbAutostart_changed()));
	this->disconnect(ui.cmbChatLinks,      SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbChatLinks_changed()));
	this->disconnect(ui.cmbLanguage,       SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbLanguage_changed()));
	this->disconnect(ui.btnBnetAccounts,SIGNAL(pressed()), this->parent(), SLOT(openBnetSettings()));

	this->connect(ui.cmbProfiles,       SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbProfiles_changed()));
	this->connect(ui.btnNewProfile,     SIGNAL(pressed()), this, SLOT(btnNewProfile_click()));
	this->connect(ui.btnDeleteProfile,  SIGNAL(clicked()), this, SLOT(btnDeleteProfile_click()));
//	this->connect(ui.btnOK,             SIGNAL(clicked()), this, SLOT(btnOK_click()));
	this->connect(ui.btnSetToken,       SIGNAL(clicked()), this, SLOT(btnSetToken_click()));
	this->connect(ui.btnSelectWatchFolder,       SIGNAL(clicked()), this, SLOT(btnSelectWatchFolder_click()));
	this->connect(ui.cmbSearchRange,    SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbSearchRange_changed()));
	this->connect(ui.cmbAutostart,      SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbAutostart_changed()));
	this->connect(ui.cmbChatLinks,      SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbChatLinks_changed()));
	this->connect(ui.cmbLanguage,       SIGNAL(currentIndexChanged(const QString)), this, SLOT(cmbLanguage_changed()));
	this->connect(ui.btnBnetAccounts,	SIGNAL(pressed()), this->parent(), SLOT(openBnetSettings()));
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

	/*
	ui.cmbProfiles->setDisabled(true);
	ui.cmbSearchRange->setDisabled(true);
	ui.cmbAutostart->setDisabled(true);
	ui.cmbChatLinks->setDisabled(true);
	ui.cmbLanguage->setDisabled(true);	
	
	ui.lblSearchRange->setDisabled(true);
	ui.lblAutostart->setDisabled(true);
	ui.lblChatLinks->setDisabled(true);
	ui.lblLanguage->setDisabled(true);	
	*/
	ui.gbSettings->setEnabled(false);

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


	//stuff withbnet
	ui.btnBnetAccounts->setDisabled(true);
}

void SettingsWindow::enableProfileInterface()
{
	reconnectGUI();
	ui.lblProfile->setEnabled(true);
	ui.btnDeleteProfile->setEnabled(true);
	ui.btnSetToken->setEnabled(true);

	/*
	//enable comboboxes
	ui.cmbProfiles->setEnabled(true);
	ui.cmbSearchRange->setEnabled(true);
	ui.cmbAutostart->setEnabled(true);
	ui.cmbChatLinks->setEnabled(true);
	ui.cmbLanguage->setEnabled(true);

	//enable option lables
	ui.lblSearchRange->setEnabled(true);
	ui.lblAutostart->setEnabled(true);
	ui.lblChatLinks->setEnabled(true);
	ui.lblLanguage->setEnabled(true);*/
	ui.gbSettings->setEnabled(true);

	//stuff withbnet
	ui.btnBnetAccounts->setEnabled(true);
}

void SettingsWindow::refreshProfileInterface()
{
	//set the current profile
	if(ui.cmbProfiles->count() > 0)
	{
		ui.cmbProfiles->setCurrentIndex(this->config_->profiles().indexOf(this->config_->activeProfile())); //is this the best way?
	}

	//set options
	QString searchRange = intToSearchRange(config_->activeProfile()->searchRange());
	
	ui.cmbSearchRange->setCurrentIndex(ui.cmbSearchRange->findText(searchRange));
	ui.cmbAutostart->setCurrentIndex(ui.cmbAutostart->findText(boolToYesNo(config_->startOnLogin())));
	ui.cmbChatLinks->setCurrentIndex(ui.cmbChatLinks->findText(boolToYesNo(this->config_->activeProfile()->chatLinks())));
	ui.cmbLanguage->setCurrentIndex(ui.cmbLanguage->findText(this->config_->activeProfile()->language()));
	ui.txtWatchFolder->setText(this->config_->activeProfile()->replayFolder());
}

const short int SettingsWindow::searchRangeToInt(QString searchRange)
{
	if(searchRange == "Narrow")
	{
		return 1;
	}
	else if(searchRange == "Normal")
	{
		return 2;
	}
	else if(searchRange == "Wide")
	{
		return 0;
	}
	else
	{
		return -1; //default is empty
	}
}

const QString SettingsWindow::intToSearchRange(const short int searchRange)
{
	if(searchRange == 1)
	{
		return "Narrow";
	}
	else if(searchRange == 2)
	{
		return "Normal";
	}
	else if(searchRange == 0)		
	{
		return "Wide";
	}
	else
	{
		return "";//default is empty
	}
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
