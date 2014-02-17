#include "bnetsettingswindow.h"

BnetSettingsWindow::BnetSettingsWindow(QWidget *parent, Config *cfg)
	: QDialog(parent)
{
	ui.setupUi(this);
	cfg_ = cfg;

	QDialog::connect(ui.btnNewManualProfile, SIGNAL(pressed()), this, SLOT(clientNewManualProfile()));
	QDialog::connect(ui.btnDeleteProfile, SIGNAL(pressed()), this, SLOT(clientRemoveBnetProfile()));
	QDialog::connect(ui.cmbProfiles, SIGNAL(currentIndexChanged(const QString)), this, SLOT(clientChangeProfile()));

	if(cfg_->activeProfile()->getActiveBnetProfile().isEmpty() == false)
	{
		QString activebnet = cfg_->activeProfile()->getActiveBnetProfile();
		ui.cmbProfiles->addItems(cfg_->activeProfile()->getBnetProfiles());	
		ui.cmbProfiles->setCurrentIndex(ui.cmbProfiles->findText(activebnet));
	}
}

BnetSettingsWindow::~BnetSettingsWindow()
{

}

void BnetSettingsWindow::clientChangeProfile()
{
	cfg_->activeProfile()->setActiveBnetProfile(ui.cmbProfiles->currentText());
}

void BnetSettingsWindow::clientNewManualProfile()
{
	QString bnetUrl;
	bool ok;
	bnetUrl = QInputDialog::getText(this, tr("Input Battle.net Profile"), tr("Enter the full url of your Battle.net profile"), QLineEdit::Normal, "", &ok);

	if(!bnetProfileAlreadyExists(bnetUrl) && validateBnetUrl(bnetUrl) && ok == true)
	{
		//do stuff with cfg_
		cfg_->activeProfile()->addBnetProfile(bnetUrl);
		
		ui.cmbProfiles->addItem(bnetUrl);
	}
	else if(bnetProfileAlreadyExists(bnetUrl))
	{
		QMessageBox::critical(this, tr("Battle.net Url error"), tr("The specified url has already been added previously."), QMessageBox::Ok);
	}
	else if(!validateBnetUrl(bnetUrl))
	{
		QMessageBox::critical(this, tr("Battle.net Url error"), tr("The specified url is invalid. Please try again."), QMessageBox::Ok);
	}
}

bool BnetSettingsWindow::validateBnetUrl(QString bnetUrl)
{
	if (bnetUrl.isEmpty())
	{
		return false;
	}
	else
	{
		//TODO do real validation
		return true;
	}
}

bool BnetSettingsWindow::bnetProfileAlreadyExists(QString bnetUrl)
{
	foreach(QString url, cfg_->activeProfile()->getBnetProfiles())
	{
		if (url == bnetUrl)
		{
			return true;				
		}
	}
	return false;
}

void BnetSettingsWindow::clientRemoveBnetProfile()
{
	QMessageBox::StandardButton reply = QMessageBox::warning(this, tr("Confirm Deletion"), tr("Are you sure you want to delete the selected Battle.net account?"), QMessageBox::Yes | QMessageBox::No);

	if(reply == QMessageBox::Yes)
	{
		cfg_->activeProfile()->removeBnetProfile(ui.cmbProfiles->currentText());
		ui.cmbProfiles->removeItem(ui.cmbProfiles->currentIndex());
	}
}