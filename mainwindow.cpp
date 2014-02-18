#include "mainwindow.h"
#include "settingswindow.h"

#include <QLocale>
#include <QFontDatabase>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include "chatwidget.h"
#include "matchmakingplayerinfo.h"
#include "matchmakingsearchprogresswidget.h"


MainWindow::MainWindow(Eros *eros, QWidget *parent )
	: QMainWindow(parent)
{

	QFontDatabase::addApplicationFont(":/font/NotoSans-Regular");
	QFontDatabase::addApplicationFont(":/font/NotoSans-Bold");
	QFontDatabase::addApplicationFont(":/font/NotoSans-Italic");
	QFontDatabase::addApplicationFont(":/font/NotoSans-BoldItalic");
	QFontDatabase::addApplicationFont(":/font/Gobold");
	QFontDatabase::addApplicationFont(":/font/Gobold-bold");

	ui.setupUi(this);
	delete ui.lblLocalPlaceholder;
	delete ui.lblRemotePlaceholder;

	setUiEnabled(false);
	this->eros_ = eros;
	this->config_ = new Config(this);
	this->connection_timer_ = new QTimer(this);
	this->matchmaking_timer_ = new QTimer(this);
	this->matchmaking_timer_->setInterval(500);
	this->matchmaking_start_ = new QTime();
	this->watcher_ = new QSimpleFileWatcher(this);
	this->watches_ = QList<WatchID>();

	// The user should be prevented from emptying invalid values in the settings dialog.
	if (this->config_->profiles().count() == 0)
	{
		QMessageBox::information(this, "Eros", tr("Welcome to Eros! You need to configure some settings in order to continue. The options window will now open."));
		this->settings_window_ = new SettingsWindow(this, this->config_);
		int result = settings_window_->exec();
		delete settings_window_;
	}

	// timers
	QObject::connect(this->connection_timer_, SIGNAL(timeout()), this, SLOT(connectionTimerWorker()));
	QObject::connect(this->matchmaking_timer_, SIGNAL(timeout()), this, SLOT(matchmakingTimerWorker()));

	// File watcher
	QObject::connect(this->watcher_, SIGNAL(fileAction(WatchID, const QString &, const QString &, Action )), this, SLOT(fileAction(WatchID, const QString &, const QString, Action)));

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
	QObject::connect(eros_, SIGNAL(regionStatsUpdated(ErosRegion, int)), this, SLOT(erosRegionStatsUpdated(ErosRegion, int)));
	QObject::connect(eros_, SIGNAL(replayUploadError(ErosError)), this, SLOT(erosReplayUploadError(ErosError)));
	QObject::connect(eros_, SIGNAL(replayUploaded()), this, SLOT(erosReplayUploaded()));
	QObject::connect(eros_, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(erosUploadProgress(qint64, qint64)));

	/// Eros Slots
	QObject::connect(this, SIGNAL(connectToEros(const QString, const QString, const QString)), eros_, SLOT(connectToEros(const QString, const QString, const QString)));
	QObject::connect(this, SIGNAL(disconnectFromEros()), eros_, SLOT(disconnectFromEros()));
	QObject::connect(ui.btnRefreshChats, SIGNAL(clicked()), eros_, SLOT(refreshChatRooms()));
	QObject::connect(this, SIGNAL(joinChatRoom(ChatRoom *, const QString)), eros_, SLOT(joinChatRoom(ChatRoom *, const QString)));
	QObject::connect(this, SIGNAL(leaveChatRoom(ChatRoom *)), eros_, SLOT(leaveChatRoom(ChatRoom *)));

	QObject::connect(this, SIGNAL(queueMatchmaking(ErosRegion, int)), eros_, SLOT(queueMatchmaking(ErosRegion, int)));
	QObject::connect(this, SIGNAL(dequeueMatchmaking()), eros_, SLOT(dequeueMatchmaking()));
	QObject::connect(this, SIGNAL(forefeitMatchmaking()), eros_, SLOT(forefeitMatchmaking()));
	QObject::connect(this, SIGNAL(uploadReplay(QIODevice*)), eros_, SLOT(uploadReplay(QIODevice*)));
	QObject::connect(this, SIGNAL(uploadReplay(const QString)), eros_, SLOT(uploadReplay(const QString)));


	


	// UI Stuff
	settings_window_ = nullptr;
	bnetsettings_window_ = nullptr;

	// Remove the close box from the first 2 tabs.
	ui.tabContainer->tabBar()->tabButton(0, QTabBar::RightSide)->resize(0, 0);
	ui.tabContainer->tabBar()->tabButton(1, QTabBar::RightSide)->resize(0, 0);

	
	QObject::connect(ui.tabContainer, SIGNAL(tabCloseRequested(int)), this, SLOT(tabContainer_tabCloseRequested(int)));
	QObject::connect(ui.lblBottomMenu, SIGNAL(linkActivated(const QString &)), this, SLOT(label_linkActivated(const QString&)));
	QObject::connect(ui.btnJoinRoom, SIGNAL(clicked()), this, SLOT(btnJoinRoom_pressed()));
	QObject::connect(ui.lstChats, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(lstChats_currentItemChanged(QListWidgetItem *, QListWidgetItem *)));
	QObject::connect(ui.cmbRegion, SIGNAL(currentIndexChanged(int)), this, SLOT(cmbRegion_currentIndexChanged(int)));
	QObject::connect(ui.btnQueue, SIGNAL(clicked()), this, SLOT(btnQueue_pressed()));
	

	this->connection_timer_->setInterval(500);
	this->connection_timer_->start();
}


MainWindow::~MainWindow()
{

}

void MainWindow::refreshChatRoomList()
{
	if (this->eros_->state() == ErosState::ConnectedState)
	{
		ui.lstChats->clear();
		for (int i =0; i < this->eros_->chatRooms().count(); i++)
		{
			ui.lstChats->addItem(this->eros_->chatRooms()[i]->name());
		}

		ui.lstChats->sortItems();
	}
}

void MainWindow::erosRegionStatsUpdated(ErosRegion region, int count)
{
	int regionIndex = ui.cmbRegion->currentData().toInt();
	ErosRegion current_region = eros_->activeRegions()[regionIndex];

	if (current_region == region)
	{
		ui.lblRegionStats->setText(tr("%1 people currently queueing on this region.").arg(count));
	}
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
		break;
	case ErosMatchmakingState::InvalidRegion:
		setQueueState(false);
		QMessageBox::warning(this, tr("Eros Matchmaking"), tr("Your queue was aborted because you do not have any verified StarCraft 2 profiles for the selected region."));
		break;
	}
}
void MainWindow::erosMatchmakingMatchFound(MatchmakingMatch *match)
{
	setQueueState(true);
	
	this->matchmaking_timer_->stop();
	int regionIndex = ui.cmbRegion->currentData().toInt();
	ErosRegion region = eros_->activeRegions()[regionIndex];

	QLayoutItem* item;
	while ( ( item = ui.frmMatchmakingOpponent->layout()->takeAt(0)) != NULL )
	{
		delete item->widget();
		delete item;
	}

	UserLadderStats *stats = match->opponent()->ladderStats()[region];
	const QPair<int, QString> &region_division = eros_->divisions()->division(stats->points());

	MatchmakingPlayerInfo *region_info = new MatchmakingPlayerInfo(match->opponent()->username(), region_division.second, stats, true, this);
	ui.frmMatchmakingOpponent->layout()->addWidget(region_info);

	ui.lblVS->setText("VS");
	ui.lblVS->setMaximumHeight(9999);
	QString map = tr("1v1 on <a href=\"starcraft://map/%1/%2\">%3</a>").arg(QString::number((int)region), QString::number(match->mapId()), match->mapName());
	ui.lblMapInfo->setText(map);
	ui.lblMapInfo->setMaximumHeight(9999);
	ui.btnQueue->setText(tr("Forefeit Match"));
}

void MainWindow::matchmakingTimerWorker()
{

	int secs = this->matchmaking_start_->elapsed()  / 1000;
	int mins = (secs / 60) % 60;
	secs = secs - (mins * 60);
	ui.lblVS->setText(QString("%1:%2").arg(mins,2, 10, QLatin1Char('0')).arg(secs,2,10,QLatin1Char('0')));
	ui.btnQueue->setText(QString("Abort Queue"));
}

void MainWindow::setQueueState(bool queueing)
{
	if (queueing)
	{
		ui.btnQueue->setText(tr("Queued"));
		ui.cmbRegion->setEnabled(false);
		this->matchmaking_start_->restart();
		this->matchmaking_timer_->start();


		MatchmakingSearchProgressWidget *search = new MatchmakingSearchProgressWidget(this);
		ui.frmMatchmakingOpponent->layout()->addWidget(search);
		ui.lblVS->setText("00:00");
		ui.lblVS->setMaximumHeight(16777215);

	}
	else
	{
		this->matchmaking_timer_->stop();
		ui.cmbRegion->setEnabled(true);
		ui.btnQueue->setText(tr("Queue"));
		QLayoutItem* item;
		while ( ( item = ui.frmMatchmakingOpponent->layout()->takeAt(0)) != NULL )
		{
			delete item->widget();
			delete item;
		}
		while ( ( item = ui.frmMatchmakingOpponent->layout()->takeAt(0)) != NULL )
		{
			delete item->widget();
			delete item;
		}
		ui.lblVS->setMaximumHeight(0);
		
	}
	ui.btnQueue->setEnabled(true);
	ui.lblMapInfo->setText("");
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
			ui.btnQueue->setEnabled(false);
		}
		else if (eros_->matchmakingState() == ErosMatchmakingState::Matched)
		{
			QMessageBox::StandardButton reply = QMessageBox::warning(this, tr("Confirm Forefeit"), tr("Are you sure you want to forefeit?"), QMessageBox::Yes | QMessageBox::No);	
			if (reply == QMessageBox::StandardButton::Yes)
			{
				emit forefeitMatchmaking();
			}
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
		while ( ( item = ui.frmLocalInfo->layout()->takeAt(0)) != NULL )
		{
			delete item->widget();
			delete item;
		}
		MatchmakingPlayerInfo *region_info = new MatchmakingPlayerInfo(user->username(), region_division.second, stats, false, this);
		ui.frmLocalInfo->layout()->addWidget(region_info);

	}

	ui.lblRegionStats->setText(tr("%1 people currently queueing on this region.").arg(eros_->regionSearchingUserCount(region)));
}

void MainWindow::erosConnected()
{
	setupWatches();
	ui.cmbRegion->clear();
	ui.lstChats->clear();
	for (int i =0; i < this->eros_->activeRegions().count(); i++)
	{
		ui.cmbRegion->addItem(QIcon(QString(":/img/client/icons/flags/%1").arg(Eros::regionToString(this->eros_->activeRegions()[i]))), Eros::regionToLongString(this->eros_->activeRegions()[i]), i);
	}
	ui.cmbRegion->setCurrentIndex(0);

	erosLocalUserUpdated(this->eros_->localUser());

	setUiEnabled(true);
	
}

void MainWindow::erosDisconnected()
{
	setUiEnabled(false);
	setQueueState(false);
	ui.lstChats->clear();
}

void MainWindow::erosLocalUserUpdated(LocalUser *user)
{
	cmbRegion_currentIndexChanged(ui.cmbRegion->currentIndex());
	
}
// Handle updating the status label and toggling the connection timer.
void MainWindow::erosStateChanged(ErosState state)
{
	setQueueState(false);
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
			const QString &server = this->config_->server();
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
	else if (link == "#upload")
	{
		QString filename = QFileDialog::getOpenFileName(this, tr("Select Replay"), this->config_->activeProfile()->replayFolder(), tr("Replay Files (*.SC2Replay);;All Files (*.*)"));
		if (!filename.isEmpty())
		{
			emit uploadReplay(filename);
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

			if (this->eros_->state() == ErosState::ConnectedState)
			{
				if (this->eros_->localUser()->username().toLower().trimmed() != this->config_->activeProfile()->username().toLower().trimmed())
				{
					emit disconnectFromEros();
					QTimer::singleShot(0, this, SLOT(connectionTimerWorker()));
				}
			}

			setupWatches();
		}
	}	
	
	if (ChatWidget* widget = dynamic_cast<ChatWidget*>(ui.tabContainer->widget(index)))
	{
		if (widget->chatroom() != nullptr)
		{
			emit leaveChatRoom(widget->chatroom());
			delete widget;
		}
	}

	if(bnetsettings_window_ != nullptr)
	{
		if (ui.tabContainer->widget(index) == bnetsettings_window_)
		{
			ui.tabContainer->removeTab(index);
			delete bnetsettings_window_;
			bnetsettings_window_ = nullptr;
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
	QString icon = ":/img/client/icons/public_chat";
	if (room->passworded())
	{
		icon = ":/img/client/icons/private_chat";
	}
	else if (!room->joinable() || room->fixed())
	{
		icon = ":/img/client/icons/match_chat";
	}
	int id = ui.tabContainer->addTab(widget, QIcon(icon), room->name());
	
	const MatchmakingMatch *match = eros_->matchmakingMatch();
	if (match != nullptr)
	{
		if (room->key() == match->chatRoom()->key())
		{
			int regionIndex = ui.cmbRegion->currentData().toInt();
			ErosRegion region = eros_->activeRegions()[regionIndex];
			widget->writeLog(tr("You have been automatically joined to this chat room for your match against <strong>%1</strong> on <a href=\"starcraft://map/%2/%3\">%4</a>. We suggest joining the channel <strong>%5</strong> on Battle.net. GLHF!").arg(match->opponent()->username(), QString::number((int)region), QString::number(match->mapId()), match->mapName(), match->battleNetChannel()), false); 
		}
	}
	ui.tabContainer->setCurrentIndex(id);

}
void MainWindow::erosChatRoomLeft(ChatRoom *room)
{

}
void MainWindow::erosChatRoomAdded(ChatRoom *room)
{
	QListWidgetItem *item = new QListWidgetItem(room->name());
	if (room->passworded())
	{
		item->setIcon(QIcon(":/img/client/icons/padlock"));
	}
	else if (room->fixed() || !room->joinable())
	{
		item->setIcon(QIcon(":/img/client/icons/bow"));
	}
	ui.lstChats->addItem(item);
	ui.lstChats->sortItems();
}
void MainWindow::erosChatRoomRemoved(ChatRoom *room)
{
	qDeleteAll(ui.lstChats->findItems(room->name(), Qt::MatchFlag::MatchExactly));
}

//////// END CHAT


//bnetsettings test
void MainWindow::openBnetSettings()
{
	
	MainWindow *main = (MainWindow*) this->parent();
	if(bnetsettings_window_ == nullptr)
	{
		bnetsettings_window_ = new BnetSettingsWindow(this, this->eros_);
		ui.tabContainer->insertTab(2, bnetsettings_window_, "Battle.net Accounts");
		ui.tabContainer->setCurrentIndex(2);		
	}
	else
	{
		for (int i = 0; i < ui.tabContainer->count(); i++)
		{
			if (ui.tabContainer->widget(i) == bnetsettings_window_)
			{
				ui.tabContainer->setCurrentIndex(i);
			}
		}
	}
}

void MainWindow::erosReplayUploaded()
{
	//QMessageBox::information(this, tr("Replay Uploaded Successfully."), tr("Your replay was successfully uploaded. In future more match info will be available to the client."));
	setQueueState(false);
}

void MainWindow::erosReplayUploadError(ErosError error)
{
	if (error != 301)
	{
		QMessageBox::warning(this, tr("Replay Upload Error"), tr("Error uploading the replay.\nError %1: %2").arg(QString::number(error), Eros::errorString(error)));
	}
}

void MainWindow::erosUploadProgress(qint64 written, qint64 total)
{
	int percent = ((written / total) * 100);
	ui.lblInformation->setText(tr("Upload %1% complete").arg(percent));
}

void MainWindow::clearWatches()
{
	for (int i = 0; i < this->watches_.count(); i++)
	{
		this->watcher_->removeWatch(this->watches_.at(i));
	}

	this->watches_.clear();
}
void MainWindow::setupWatches()
{
	if (this->config_->activeProfile() != nullptr)
	{
		const QString &path = this->config_->activeProfile()->replayFolder();
		if (!path.isEmpty())
		{
			addWatch(path);
		}
	}
}

void MainWindow::addWatch(const QString &path)
{
	QDir dir(path);
	if (dir.exists())
	{
		dir.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoSymLinks);
		QFileInfoList list = dir.entryInfoList();

		for (int i = 0; i < list.count(); i++)
		{
			if (list[i].fileName() == "." || list[i].fileName() == "..")
				continue;

			if (list[i].isDir())
			{
				addWatch(list[i].absoluteFilePath());
			}
		}

		this->watches_ << this->watcher_->addWatch(path);
	}
}

void MainWindow::fileAction(WatchID watchId, const QString &dir, const QString &filename, Action action)
{
	if (action == Action::Add)
	{
		QStringList pieces = filename.split('.');
		QString extension = pieces.value(pieces.length()-1);

		if(extension.toLower() == "sc2replay")
		{
			ui.lblInformation->setText(tr("Uploading %1").arg(filename));
			QString path = dir + "/" + filename;

			QTime time;
			time.start();

			while (time.elapsed() < 10000)
			{
				// Ugly delay hack.
				QCoreApplication::processEvents();
			}

			uploadReplay(path);		
		}
	}
}