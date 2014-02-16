#include "mainwindow.h"
#include "settingswindow.h"

#include <QMessageBox>
#include "chatwidget.h"

MainWindow::MainWindow(Eros *eros, QWidget *parent )
	: QMainWindow(parent)
{
	ui.setupUi(this);
	this->config_ = new Config(this);

	// The user should be prevented from emptying invalid values in the settings dialog.
	if (this->config_->profiles().count() == 0)
	{
		QMessageBox::information(this, "Eros", tr("Welcome to Eros! You need to configure some settings in order to continue. The options window will now open."));
		this->settings_window_ = new SettingsWindow(this, this->config_);
		int result = settings_window_->exec();
		delete settings_window_;
	}
	else
	{		
		ui.lblInformation->setText(config_->activeProfile()->username());
	}
	settings_window_ = nullptr;
	ui.tabContainer->tabBar()->tabButton(0, QTabBar::RightSide)->resize(0, 0);
	ui.tabContainer->tabBar()->tabButton(1, QTabBar::RightSide)->resize(0, 0);

	
	QObject::connect(ui.tabContainer, SIGNAL(tabCloseRequested(int)), this, SLOT(tabContainer_tabCloseRequested(int)));
	QObject::connect(ui.lblBottomMenu, SIGNAL(linkActivated(const QString &)), this, SLOT(label_linkActivated(const QString&)));
//	ChatWidget *c1 = new ChatWidget(false, ui.tabChats);
//	ChatWidget *c2 = new ChatWidget(true, ui.tabChats);

//	ui.tabChats->addTab(c1, "Veritas");
//	ui.tabChats->addTab(c2, "Chat room name lolol");
}


MainWindow::~MainWindow()
{

}

void MainWindow::label_linkActivated(const QString &link)
{
	if (link == "#options")
	{
		if (this->settings_window_ == nullptr)
		{
			this->settings_window_ = new SettingsWindow(this, this->config_);
		
			ui.tabContainer->insertTab(2, this->settings_window_, "Settings");
			ui.tabContainer->setCurrentIndex(2);
		} 
		else
		{
			for (int i = 0; i < ui.tabContainer->count(); i++)
			{
				if (ui.tabContainer->widget(i) == this->settings_window_)
				{
					ui.tabContainer->setCurrentIndex(i);
				}
			}
		}

	}
}

void MainWindow::tabContainer_tabCloseRequested(int index)
{
	if (this->settings_window_ != nullptr)
	{
		if (ui.tabContainer->widget(index) == this->settings_window_)
		{
			ui.tabContainer->removeTab(index);
			delete this->settings_window_;
			this->settings_window_ = nullptr;
		}
	}
	
}