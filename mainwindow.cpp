#include "mainwindow.h"
#include "settingswindow.h"

#include <QLocale>
#include <QFontDatabase>
#include <QMessageBox>
#include "chatwidget.h"
#include "matchmakingplayerinfo.h"


MainWindow::MainWindow(Eros *eros, QWidget *parent )
	: QMainWindow(parent)
{

	QFontDatabase::addApplicationFont(":/font/NotoSans-Regular");
	QFontDatabase::addApplicationFont(":/font/NotoSans-Bold");
	QFontDatabase::addApplicationFont(":/font/NotoSans-Italic");
	QFontDatabase::addApplicationFont(":/font/NotoSans-BoldItalic");

	ui.setupUi(this);
	delete ui.lblLocalPlaceholder;
	delete ui.lblRemotePlaceholder;

	setUiEnabled(false);
	this->eros_ = eros;
	this->config_ = new Config(this);
	this->connection_timer_ = new QTimer(this);
	this->matchmaking_timer_ = new QTimer(this);
	this->matchmaking_timer_->setInterval(1000);
	this->matchmaking_start_ = new QTime();

	// The user should be prevented from emptying invalid values in the settings dialog.
	if (this->config_->profiles().count() == 0)
	{
		QMessageBox::information(this, "Eros", tr("Welcome to Eros! You need to configure some settings in order to continue. The options window will now open."));
		this->settings_window_ = new SettingsWindow(this, this->config_);
		int result = settings_window_->exec();
		delete settings_window_;
	}

	// Connection timer
	QObject::connect(this->connection_timer_, SIGNAL(timeout()), this, SLOT(connectionTimerWorker()));

	// Set up Eros signals
	/// Eros Signals
	QObject::connect(eros_, SIGNAL(stateChanged(ErosState)), this, SLOT(erosStateChanged(ErosState)));
	QObject::connect(eros_, SIGNAL(connected()), this, SLOT(erosConnected()));
	QObject::connect(eros_, SIGNAL(chatRoomAdded(ChatRoom*)), this, SLOT(erosChatRoomAdded(ChatRoom*)));
	QObject::connect(eros_, SIGNAL(chatRoomRemoved(ChatRoom*)), this, SLOT(erosChatRoomRemoved(ChatRoom*)));
	QObject::connect(eros_, SIGNAL(chatRoomJoined(ChatRoom*)), this, SLOT(erosChatRoomJoined(ChatRoom*)));
	QObject::connect(eros_, SIGNAL(chatRoomLeft(ChatRoom*)), this, SLOT(erosChatRoomLeft(ChatRoom*)));
	QObject::connect(eros_, SIGNAL(localUserUpdated(LocalUser*)), this, SLOT(erosLocalUserUpdated(LocalUser*)));
	QObject::connect(eros_, SIGNAL(matchmakingStateChanged(ErosMatchmakingState)), this, SLOT(erosMatchmakingStateChanged(ErosMatchmakingState)));
	QObject::connect(eros_, SIGNAL(matchmakingMatchFound(MatchmakingMatch *)), this, SLOT(erosMatchmakingMatchFound(MatchmakingMatch *)));

	/// Eros Slots
	QObject::connect(this, SIGNAL(connectToEros(const QString, const QString, const QString)), eros_, SLOT(connectToEros(const QString, const QString, const QString)));
	QObject::connect(ui.btnRefreshChats, SIGNAL(pressed()), eros_, SLOT(refreshChatRooms()));
	QObject::connect(this, SIGNAL(joinChatRoom(ChatRoom *, const QString)), eros_, SLOT(joinChatRoom(ChatRoom *, const QString)));
	QObject::connect(this, SIGNAL(leaveChatRoom(ChatRoom *)), eros_, SLOT(leaveChatRoom(ChatRoom *)));

	QObject::connect(this, SIGNAL(queueMatchmaking(ErosRegion, int)), eros_, SLOT(queueMatchmaking(ErosRegion, int)));
	QObject::connect(this, SIGNAL(dequeueMatchmaking()), eros_, SLOT(dequeueMatchmaking()));



	// UI Stuff
	settings_window_ = nullptr;

	// Remove the close box from the first 2 tabs.
	ui.tabContainer->tabBar()->tabButton(0, QTabBar::RightSide)->resize(0, 0);
	ui.tabContainer->tabBar()->tabButton(1, QTabBar::RightSide)->resize(0, 0);

	
	QObject::connect(ui.tabContainer, SIGNAL(tabCloseRequested(int)), this, SLOT(tabContainer_tabCloseRequested(int)));
	QObject::connect(ui.lblBottomMenu, SIGNAL(linkActivated(const QString &)), this, SLOT(label_linkActivated(const QString&)));
	QObject::connect(ui.btnJoinRoom, SIGNAL(pressed()), this, SLOT(btnJoinRoom_pressed()));
	QObject::connect(ui.lstChats, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(lstChats_currentItemChanged(QListWidgetItem *, QListWidgetItem *)));
	QObject::connect(ui.cmbRegion, SIGNAL(currentIndexChanged(int)), this, SLOT(cmbRegion_currentIndexChanged(int)));
	QObject::connect(ui.btnQueue, SIGNAL(pressed()), this, SLOT(btnQueue_pressed()));


	this->connection_timer_->setInterval(500);
	this->connection_timer_->start();
}


MainWindow::~MainWindow()
{

}

void MainWindow::erosMatchmakingStateChanged(ErosMatchmakingState status)
{
	switch (status)
	{
	case ErosMatchmakingState::Queued:
		setQueueState(true);
		break;
	case ErosMatchmakingState::Aborted:
	case ErosMatchmakingState::Idle:
		setQueueState(false);
	case ErosMatchmakingState::InvalidRegion:
		QMessageBox::warning(this, tr("Eros Matchmaking"), tr("Your queue was aborted because you do not have any verified StarCraft 2 profiles for the selected region."));
		break;
	}
}
void MainWindow::erosMatchmakingMatchFound(MatchmakingMatch *match)
{

}

void MainWindow::matchmakingTimerWorker()
{
	int time = this->matchmaking_start_->elapsed();
	ui.btnQueue->setText(QString(time));
}

void MainWindow::setQueueState(bool queueing)
{
	if (queueing)
	{
		ui.cmbRegion->setEnabled(false);
		this->matchmaking_start_->restart();
		this->matchmaking_timer_->start();
	}
	else
	{
		this->matchmaking_timer_->stop();
		ui.cmbRegion->setEnabled(true);
		ui.btnQueue->setText(tr("Queue"));
	}
}

void MainWindow::btnQueue_pressed()
{
	if (eros_->state() == ErosState::ConnectedState)
	{
		if (eros_->matchmakingState() == ErosMatchmakingState::Queued)
		{
			emit dequeueMatchmaking();
		}
		else if (eros_->matchmakingState() == ErosMatchmakingState::Idle || eros_->matchmakingState() == ErosMatchmakingState::Aborted || eros_->matchmakingState() == ErosMatchmakingState::InvalidRegion)
		{
			int regionIndex = ui.cmbRegion->currentData().toInt();
			ErosRegion region = eros_->activeRegions()[regionIndex];
			emit queueMatchmaking(region, this->config_->activeProfile()->searchRange());
		}
	}
}

void MainWindow::cmbRegion_currentIndexChanged (int index)
{
	LocalUser *user = eros_->localUser();
	int regionIndex = ui.cmbRegion->currentData().toInt();
	ErosRegion region = eros_->activeRegions()[regionIndex];

	if (user != nullptr)
	{

		UserLadderStats *stats = user->ladderStats()[region];
		const QPair<int, QString> &region_division = eros_->divisions()->division(stats->points());
		QLayoutItem* item;
		while ( ( item = ui.frmMatchmakingLocal->layout()->takeAt(0)) != NULL )
		{
			delete item->widget();
			delete item;
		}
		MatchmakingPlayerInfo *region_info = new MatchmakingPlayerInfo(user->username(), region_division.second, stats, this);
		ui.frmMatchmakingLocal->layout()->addWidget(region_info);

	}

	ui.lblRegionStats->setText(tr("%1 people are currently queueing on this region. Time to peace out some motherfuckers.").arg(eros_->regionSearchingUserCount(region)));
}

void MainWindow::erosConnected()
{
	ui.cmbRegion->clear();
	for (int i =0; i < this->eros_->activeRegions().count(); i++)
	{
		ui.cmbRegion->addItem(Eros::regionToLongString(this->eros_->activeRegions()[i]), i);
	}
	ui.cmbRegion->setCurrentIndex(0);

	erosLocalUserUpdated(this->eros_->localUser());

	setUiEnabled(true);
	
}

void MainWindow::erosDisconnected()
{
	setUiEnabled(false);
}

void MainWindow::erosLocalUserUpdated(LocalUser *user)
{
	cmbRegion_currentIndexChanged(ui.cmbRegion->currentIndex());
	
}
// Handle updating the status label and toggling the connection timer.
void MainWindow::erosStateChanged(ErosState state)
{
	switch (state)
	{
	case ErosState::ConnectingState:
		this->ui.lblInformation->setText(tr("Connecting"));
		this->connection_timer_->stop();
		break;
	case ErosState::DisconnectingState:
		this->ui.lblInformation->setText(tr("Disconnecting"));
		break;
	case ErosState::UnconnectedState:
		this->ui.lblInformation->setText(tr("Not connected"));
		this->connection_timer_->start();
		break;
	case ErosState::HandshakingState:
		this->ui.lblInformation->setText(tr("Logging in"));
		break;
	case ErosState::ConnectedState:
		this->ui.lblInformation->setText(tr("Connected as %1").arg(this->eros_->localUser()->username()));
		break;
	}
}

void MainWindow::connectionTimerWorker()
{
	this->connection_timer_->setInterval(10000);
	if (this->config_->activeProfile() != nullptr)
	{
		if (this->eros_->state() == ErosState::UnconnectedState)
		{
			const QString &server = this->config_->activeProfile()->server();
			const QString &username = this->config_->activeProfile()->username();
			const QString &token = this->config_->activeProfile()->token();

			if (server.isEmpty())
			{
				this->ui.lblInformation->setText(tr("Cannot connect. No server specified."));
			} 
			else if (token.isEmpty())
			{
				this->ui.lblInformation->setText(tr("Cannot connect. No authentication token specified."));
			}
			else
			{
				emit connectToEros(server, username, token);
				this->connection_timer_->stop();

			}
		}
	}
	else
	{
		this->ui.lblInformation->setText(tr("Cannot connect. No profile specified."));
	}
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
	else if (ChatWidget* widget = dynamic_cast<ChatWidget*>(ui.tabContainer->widget(index)))
	{
		if (widget->chatroom() != nullptr)
		{
			emit leaveChatRoom(widget->chatroom());
			delete widget;
		}
	}
	
}

void MainWindow::setUiEnabled(bool enabled)
{
	ui.tabMatchmaking->setEnabled(enabled);
	ui.tabChat->setEnabled(enabled);
}

//////// CHAT

void MainWindow::lstChats_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
	if (current != nullptr)
	{
		ui.txtRoomName->setText(current->text());
	}
}

void MainWindow::btnJoinRoom_pressed()
{
	if (this->eros_->state() == ErosState::ConnectedState)
	{
		const QString &name = this->ui.txtRoomName->text().trimmed();
		const QString &password = this->ui.txtRoomPassword->text();

		ChatRoom *room = this->eros_->getChatRoom(name);
		emit joinChatRoom(room, password);
	}
}

void MainWindow::erosChatRoomJoined(ChatRoom *room)
{
	for (int i = 0; i < ui.tabContainer->count(); i++)
	{
		if (ChatWidget* widget = dynamic_cast<ChatWidget*>(ui.tabContainer->widget(i)))
		{
			if (widget->chatroom() != nullptr && widget->chatroom()->name() == room->name())
			{
				widget->setChatroom(room);
				return;
			}
		}
	}

	ChatWidget *widget = new ChatWidget(this->eros_, room);
	int id = ui.tabContainer->addTab(widget, room->name());
	ui.tabContainer->setCurrentIndex(id);
}
void MainWindow::erosChatRoomLeft(ChatRoom *room)
{

}
void MainWindow::erosChatRoomAdded(ChatRoom *room)
{
	ui.lstChats->addItem(room->name());
	ui.lstChats->sortItems();
}
void MainWindow::erosChatRoomRemoved(ChatRoom *room)
{
	qDeleteAll(ui.lstChats->findItems(room->name(), Qt::MatchFlag::MatchExactly));
}

//////// END CHAT