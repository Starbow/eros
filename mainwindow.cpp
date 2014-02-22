#include "mainwindow.h"
#include <QLocale>
#include <QFontDatabase>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QStandardPaths>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "chatwidget.h"
#include "matchmakingplayerinfo.h"
#include "matchmakingsearchprogresswidget.h"

#define EROS_CURRENT_VERSION = 1

MainWindow::MainWindow(Eros *eros, QWidget *parent )
	: QMainWindow(parent)
{

	QFontDatabase::addApplicationFont(":/font/NotoSans-Regular");
	QFontDatabase::addApplicationFont(":/font/NotoSans-Bold");
	QFontDatabase::addApplicationFont(":/font/NotoSans-Italic");
	QFontDatabase::addApplicationFont(":/font/NotoSans-BoldItalic");
	QFontDatabase::addApplicationFont(":/font/Gobold");
	QFontDatabase::addApplicationFont(":/font/Gobold-bold");


	QFile version(":/data/version_info");
	if(!version.open(QIODevice::ReadOnly))
	{
		this->local_version_ = 1;
	}
	else
	{
		QByteArray data = version.readAll();
		QStringList tokens = QString::fromLocal8Bit(data).split('|');
		this->local_version_ = tokens[0].toInt();
		version.close();
	}

	ui.setupUi(this);
	this->setWindowTitle(tr("Alpha version %1").arg(this->local_version_));
	delete ui.lblLocalPlaceholder;
	delete ui.lblRemotePlaceholder;

	setUiEnabled(false);
	this->eros_ = eros;
	this->config_ = new Config(this);
	this->connection_timer_ = new QTimer(this);
	this->matchmaking_timer_ = new QTimer(this);
	this->matchmaking_timer_->setInterval(500);
	this->matchmaking_start_ = new QTime();
	this->matchmaking_result_time_ = new QTime();
	this->long_process_start_time_ = new QTime();
	this->watcher_ = new QSimpleFileWatcher(this);
	this->watches_ = QList<WatchID>();
	this->update_checker_nam_ = new QNetworkAccessManager(this);
	this->update_timer_ = new QTimer(this);
	this->long_process_timer_ = new QTimer(this);
	this->tray_icon_ = new QSystemTrayIcon(this);
	this->tray_icon_menu_ = new QMenu(this);
	
	tray_icon_action_show_ = new QAction("Hide Eros", this);
	tray_icon_action_close_ = new QAction("Close Eros", this);


	QObject::connect(tray_icon_action_show_, SIGNAL(triggered()), this, SLOT(toggleWindow()));
    QObject::connect(tray_icon_action_close_, SIGNAL(triggered()), qApp, SLOT(quit()));
	QObject::connect(this->tray_icon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconClicked(QSystemTrayIcon::ActivationReason)));


	this->tray_icon_->setIcon(QIcon(":/img/client/icons/bow"));
	this->tray_icon_menu_->addAction(tray_icon_action_show_);
	this->tray_icon_menu_->addAction(tray_icon_action_close_);
	this->tray_icon_->setContextMenu(this->tray_icon_menu_);



	// File watcher
	QObject::connect(this->watcher_, SIGNAL(fileAction(WatchID, const QString &, const QString &, Action )), this, SLOT(fileAction(WatchID, const QString &, const QString, Action)));

	// Update checker
	QObject::connect(update_checker_nam_, SIGNAL(finished(QNetworkReply*)), this, SLOT(updateCheckerFinished(QNetworkReply*)));
	QObject::connect(this->update_timer_, SIGNAL(timeout()), this, SLOT(updateCheckerTimerWorker()));

	// Set up Eros signals
	/// Eros Signals
	QObject::connect(eros_, SIGNAL(stateChanged(ErosState)), this, SLOT(erosStateChanged(ErosState)));
	QObject::connect(eros_, SIGNAL(connectionError(QAbstractSocket::SocketError, const QString)), this, SLOT(erosConnectionError(QAbstractSocket::SocketError, const QString)));
	QObject::connect(eros_, SIGNAL(connected()), this, SLOT(erosConnected()));
	QObject::connect(eros_, SIGNAL(disconnected()), this, SLOT(erosDisconnected()));
	QObject::connect(eros_, SIGNAL(handshakeFailed()), this, SLOT(erosHandshakeFailed()));
	QObject::connect(eros_, SIGNAL(broadcastAlert(const QString, int)), this, SLOT(erosBroadcastAlert(const QString, int)));
	QObject::connect(eros_, SIGNAL(chatRoomAdded(ChatRoom*)), this, SLOT(erosChatRoomAdded(ChatRoom*)));
	QObject::connect(eros_, SIGNAL(chatRoomRemoved(ChatRoom*)), this, SLOT(erosChatRoomRemoved(ChatRoom*)));
	QObject::connect(eros_, SIGNAL(chatRoomJoined(ChatRoom*)), this, SLOT(erosChatRoomJoined(ChatRoom*)));
	QObject::connect(eros_, SIGNAL(chatRoomLeft(ChatRoom*)), this, SLOT(erosChatRoomLeft(ChatRoom*)));
	QObject::connect(eros_, SIGNAL(localUserUpdated(LocalUser*)), this, SLOT(erosLocalUserUpdated(LocalUser*)));
	QObject::connect(eros_, SIGNAL(matchmakingStateChanged(ErosMatchmakingState)), this, SLOT(erosMatchmakingStateChanged(ErosMatchmakingState)));
	QObject::connect(eros_, SIGNAL(matchmakingMatchFound(MatchmakingMatch *)), this, SLOT(erosMatchmakingMatchFound(MatchmakingMatch *)));
	//QObject::connect(eros_, SIGNAL(regionStatsUpdated(ErosRegion, int)), this, SLOT(erosRegionStatsUpdated(ErosRegion, int)));
	QObject::connect(eros_, SIGNAL(statsUpdated(int, int)), this, SLOT(erosStatsUpdated(int, int)));
	QObject::connect(eros_, SIGNAL(replayUploadError(ErosError)), this, SLOT(erosReplayUploadError(ErosError)));
	QObject::connect(eros_, SIGNAL(replayUploaded()), this, SLOT(erosReplayUploaded()));
	QObject::connect(eros_, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(erosUploadProgress(qint64, qint64)));

	QObject::connect(eros_, SIGNAL(longProcessStateChanged(ErosLongProcessState)), this, SLOT(erosLongProcessStateChanged(ErosLongProcessState)));
	QObject::connect(eros_, SIGNAL(drawRequested()), this, SLOT(erosDrawRequested()));
	QObject::connect(eros_, SIGNAL(drawRequestFailed()), this, SLOT(erosDrawRequestFailed()));
	QObject::connect(eros_, SIGNAL(noShowRequested()), this, SLOT(erosNoShowRequested()));
	QObject::connect(eros_, SIGNAL(noShowRequestFailed()), this, SLOT(erosNoShowRequestFailed()));
	QObject::connect(eros_, SIGNAL(acknowledgeLongProcessFailed()), this, SLOT(erosAcknowledgeLongProcessFailed()));
	QObject::connect(eros_, SIGNAL(acknowledgedLongProcess()), this, SLOT(erosAcknowledgedLongProcess()));


	/// Eros Slots
	QObject::connect(this, SIGNAL(connectToEros(const QString, const QString, const QString)), eros_, SLOT(connectToEros(const QString, const QString, const QString)));
	QObject::connect(this, SIGNAL(disconnectFromEros()), eros_, SLOT(disconnectFromEros()));
	QObject::connect(ui.btnRefreshChats, SIGNAL(clicked()), eros_, SLOT(refreshChatRooms()));
	QObject::connect(this, SIGNAL(joinChatRoom(ChatRoom *, const QString)), eros_, SLOT(joinChatRoom(ChatRoom *, const QString)));
	QObject::connect(this, SIGNAL(leaveChatRoom(ChatRoom *)), eros_, SLOT(leaveChatRoom(ChatRoom *)));

	QObject::connect(this, SIGNAL(queueMatchmaking(ErosRegion, int)), eros_, SLOT(queueMatchmaking(ErosRegion, int)));
	QObject::connect(this, SIGNAL(dequeueMatchmaking()), eros_, SLOT(dequeueMatchmaking()));
    QObject::connect(this, SIGNAL(forfeitMatchmaking()), eros_, SLOT(forfeitMatchmaking()));

	QObject::connect(this, SIGNAL(uploadReplay(QIODevice*)), eros_, SLOT(uploadReplay(QIODevice*)));
	QObject::connect(this, SIGNAL(uploadReplay(const QString)), eros_, SLOT(uploadReplay(const QString)));

	QObject::connect(this, SIGNAL(requestDraw()), eros_, SLOT(requestDraw()));
	QObject::connect(this, SIGNAL(requestNoShow()), eros_, SLOT(requestNoShow()));
	QObject::connect(this, SIGNAL(acknowledgeLongProcess(bool)), eros_, SLOT(acknowledgeLongProcess(bool)));



	// timers
	QObject::connect(this->connection_timer_, SIGNAL(timeout()), this, SLOT(connectionTimerWorker()));
	QObject::connect(this->matchmaking_timer_, SIGNAL(timeout()), this, SLOT(matchmakingTimerWorker()));
	QObject::connect(this->long_process_timer_, SIGNAL(timeout()), this, SLOT(longProcessTimerWorker()));

	this->long_process_timer_->setInterval(250);

	// UI Stuff
	settings_window_ = nullptr;
	bnetsettings_window_ = nullptr;

	// Remove the close box from the first 2 tabs.
    // On mac it's LeftSide. Assuming RightSide causes a nullptr.
    QWidget *tab = ui.tabContainer->tabBar()->tabButton(0, QTabBar::RightSide);
    if (tab != nullptr)
        tab->resize(0, 0);

    tab = ui.tabContainer->tabBar()->tabButton(1, QTabBar::RightSide);
    if (tab != nullptr)
        tab->resize(0, 0);

    tab = ui.tabContainer->tabBar()->tabButton(0, QTabBar::LeftSide);
    if (tab != nullptr)
        tab->resize(0, 0);

    tab = ui.tabContainer->tabBar()->tabButton(1, QTabBar::LeftSide);
    if (tab != nullptr)
        tab->resize(0, 0);

	ui.tabContainer->tabBar()->setUsesScrollButtons(true);
	
	QObject::connect(ui.tabContainer, SIGNAL(tabCloseRequested(int)), this, SLOT(tabContainer_tabCloseRequested(int)));
	QObject::connect(ui.lblBottomMenu, SIGNAL(linkActivated(const QString &)), this, SLOT(label_linkActivated(const QString&)));
	QObject::connect(ui.btnJoinRoom, SIGNAL(clicked()), this, SLOT(btnJoinRoom_pressed()));
	QObject::connect(ui.lstChats, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(lstChats_currentItemChanged(QListWidgetItem *, QListWidgetItem *)));
	QObject::connect(ui.lstChats, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(lstChats_itemDoubleClicked(QListWidgetItem*)));
	QObject::connect(ui.cmbRegion, SIGNAL(currentIndexChanged(int)), this, SLOT(cmbRegion_currentIndexChanged(int)));
	QObject::connect(ui.btnQueue, SIGNAL(clicked()), this, SLOT(btnQueue_pressed()));
	QObject::connect(ui.btnDraw, SIGNAL(clicked()), this, SLOT(btnDraw_pressed()));
	QObject::connect(ui.btnNoShow, SIGNAL(clicked()), this, SLOT(btnNoShow_pressed()));
	

	this->tray_icon_->show();
		// The user should be prevented from emptying invalid values in the settings dialog.
	if (this->config_->profiles().count() == 0)
	{
		QMessageBox::information(this, "Eros", tr("Welcome to Eros! You need to configure some settings in order to continue. The options window will now open."));
		openSettings();
	}
	else
	{
		this->connection_timer_->setInterval(500);
		this->connection_timer_->start();
	}

	QTimer::singleShot(0, this, SLOT(updateCheckerTimerWorker()));
	this->update_timer_->setInterval(1000 * 60 * 10);
	this->update_timer_->start();

	notification_sound_ = new QSound(":/sound/notification", this);
	
}



MainWindow::~MainWindow()
{

}

void MainWindow::longProcessTimerWorker()
{
	if (this->eros_->matchmakingMatch() != nullptr)
	{
		const MatchmakingMatch *match = this->eros_->matchmakingMatch();
		int secs;
		int mins;
		ErosLongProcessState state = this->eros_->longProcessState();
		switch (state)
		{
			case ErosLongProcessState::LongProcessIdle:
				secs = match->longProcessUnlockTime() - (this->matchmaking_result_time_->elapsed()  / 1000);
				if (secs <= 0)
				{
					this->ui.frmLongProcessInterface->setEnabled(true);
					this->ui.frmLongProcessButtons->setEnabled(true);
					this->long_process_timer_->stop();
					this->ui.lblLongProcess->setText(tr("Abusing these options will result in a ban."));
				}
				else
				{
					mins = (secs / 60) % 60;
					secs = secs - (mins * 60);
					this->ui.lblLongProcess->setText(QString("These options will unlock in %1:%2.").arg(mins,2, 10, QLatin1Char('0')).arg(secs,2,10,QLatin1Char('0')));
				}
				break;

			case ErosLongProcessState::DrawRequest:
			case ErosLongProcessState::FlaggedNoShow:
			case ErosLongProcessState::OpponentFlaggedDrawRequest:
			case ErosLongProcessState::OpponentFlaggedNoShow:
				secs = match->longProcessResponseTime() - (this->long_process_start_time_->elapsed()  / 1000);
				if (secs <= 0)
				{
					this->ui.frmLongProcessInterface->setEnabled(true);
					this->long_process_timer_->stop();
					this->ui.lblLongProcess->setText(tr("Response timeout."));
				}
				else
				{
					mins = (secs / 60) % 60;
					secs = secs - (mins * 60);
					if (state == ErosLongProcessState::DrawRequest || state == ErosLongProcessState::FlaggedNoShow)
					{
						this->ui.lblLongProcess->setText(QString("Request pending. Auto-yes in %1:%2.").arg(mins,2, 10, QLatin1Char('0')).arg(secs,2,10,QLatin1Char('0')));
					}
					else
					{
						this->ui.lblLongProcess->setText(QString("Opponent request pending. Auto-yes in %1:%2.").arg(mins,2, 10, QLatin1Char('0')).arg(secs,2,10,QLatin1Char('0')));
					}
				}
		}

		

	}
	int elapsed = this->matchmaking_result_time_->elapsed() / 1000;

}

void MainWindow::btnDraw_pressed()
{
	emit requestDraw();
}
void MainWindow::btnNoShow_pressed()
{
	emit requestNoShow();
}

void MainWindow::erosLongProcessStateChanged(ErosLongProcessState state)
{
	switch (state)
	{
	case ErosLongProcessState::DrawRequest:
	case ErosLongProcessState::FlaggedNoShow:
	case ErosLongProcessState::OpponentFlaggedDrawRequest:
	case ErosLongProcessState::OpponentFlaggedNoShow:
		this->long_process_start_time_->restart();
		this->long_process_timer_->start();
		ui.frmLongProcessButtons->setDisabled(true);
		break;
	case ErosLongProcessState::LongProcessIdle:
		this->ui.lblLongProcess->setText(tr("Abusing these options will result in a ban."));
		break;
	}
}
void MainWindow::erosDrawRequested()
{
	this->showNormal();
	this->raise();
	this->activateWindow();
	this->notification_sound_->setLoops(3);
	this->notification_sound_->play();
	QMessageBox::StandardButton result = QMessageBox::critical(this, tr("Draw Request"), tr("Your opponent has requested this game be marked as a draw. Do you wish to accept this request? Clicking yes or not responding will result in this game ending in a draw."), QMessageBox::StandardButton::Yes|QMessageBox::StandardButton::No, QMessageBox::StandardButton::No);
	bool response = (result == QMessageBox::StandardButton::Yes);
	emit acknowledgeLongProcess(response);
}
void MainWindow::erosDrawRequestFailed()
{
	ui.lblInformation->setText("Draw request failed.");
}
void MainWindow::erosNoShowRequested()
{
	this->showNormal();
	this->raise();
	this->activateWindow();
	this->notification_sound_->setLoops(3);
	this->notification_sound_->play();
	QMessageBox::StandardButton result = QMessageBox::critical(this, tr("Forfeit Request"), tr("Your opponent has flagged you as a no-show. Do you wish to accept this request? Clicking yes or not responding will result in you forfeiting this game."), QMessageBox::StandardButton::Yes|QMessageBox::StandardButton::No, QMessageBox::StandardButton::No);
	bool response = (result == QMessageBox::StandardButton::Yes);
	emit acknowledgeLongProcess(response);
}
void MainWindow::erosNoShowRequestFailed()
{

}

void MainWindow::erosAcknowledgeLongProcessFailed()
{

}

void MainWindow::erosAcknowledgedLongProcess()
{

}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if(event->type() == QEvent::WindowStateChange) {
        if(isMinimized())
            this->hide();
    }
}
 
void MainWindow::closeEvent(QCloseEvent *event)
{
	if (this->tray_icon_->isVisible()) {
		toggleWindow();
		event->ignore();
		if (!this->config_->trayNotificationShown())
		{
			this->config_->setTrayNotificationShown(true);
			this->tray_icon_->showMessage("Eros", tr("Eros is still running in the notification tray. Right click the icon if you want to exit."));
		}
	}
}

void MainWindow::toggleWindow()
{
	if(this->isVisible())
	{
		this->hide();
		this->tray_icon_action_show_->setText("Show Eros");
	}
	else
	{
		this->showNormal();
		this->raise();
		this->activateWindow();
		this->tray_icon_action_show_->setText("Hide Eros");
	}
}
 
void MainWindow::trayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::DoubleClick)
        toggleWindow();
}
 

void MainWindow::erosBroadcastAlert(const QString message, int hint)
{
	if (hint == 1)
	{
		QMessageBox::information(this, tr("Server Broadcast Message"), tr("The server is shutting down. %1").arg(message));
	}
	else if (hint == 2)
	{
		QMessageBox::information(this, tr("Server Broadcast Message"), tr("You were forfeited from your matchmaking match by an admin. %1").arg(message));
	}
	else if (hint == 3)
	{
		QMessageBox::information(this, tr("Server Broadcast Message"), tr("Your opponent was forfeited from your matchmaking match by an admin. %1").arg(message));
	}
	else if (hint == 4)
	{
		QMessageBox::information(this, tr("Server Broadcast Message"), tr("Your matchmaking match was ended by an admin. %1").arg(message));
	}
	else
	{
		QMessageBox::information(this, tr("Server Broadcast Message"), message);
	}
}

void MainWindow::updateCheckerTimerWorker()
{
	QUrl url("http://www.starbowmod.com/static/version.txt");
	QNetworkReply* reply = this->update_checker_nam_->get(QNetworkRequest(url));
}

void MainWindow::updateCheckerFinished(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError)
	{
		QByteArray data = reply->readAll();
		QStringList tokens = QString::fromLocal8Bit(data).split('|');
		if (tokens.count() == 3)
		{
			int version = tokens[0].toInt();
			if (version > this->local_version_)
			{
				ui.lblUpdateInfo->setText(tr("<strong>Update:</strong> <a href=\"%1\">%2</a>").arg(tokens[2], tokens[1]));
				ui.lblUpdateInfo->setMaximumHeight(8888);
			}
		}
	}
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

void MainWindow::erosStatsUpdated(int online, int searching)
{

	ui.lblRegionStats->setText(tr("%1 people currently online.").arg(online));
	
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
	
	this->matchmaking_result_time_->restart();
	this->matchmaking_timer_->stop();
	this->long_process_timer_->start();
	this->ui.frmLongProcessInterface->setMaximumHeight(400);
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
	ui.btnQueue->setText(tr("Forfeit Match"));

	this->showNormal();
	this->raise();
	this->activateWindow();
	this->notification_sound_->setLoops(0);
	this->notification_sound_->play();

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


		MatchmakingSearchProgressWidget *search = new MatchmakingSearchProgressWidget(this->eros_->activeUserCount(), this->eros_->searchingUserCount(), this);
		QObject::connect(this->eros_, SIGNAL(statsUpdated(int, int)), search, SLOT(statsUpdated(int, int)));
		ui.frmMatchmakingOpponent->layout()->addWidget(search);
		ui.lblVS->setText("00:00");
		ui.lblVS->setMaximumHeight(16777215);

	}
	else
	{
		this->long_process_timer_->stop();
		this->matchmaking_timer_->stop();
		this->ui.frmLongProcessInterface->setMaximumHeight(0);
		this->ui.frmLongProcessInterface->setDisabled(true);
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
			this->config_->setPreferredRegion(region);
			emit queueMatchmaking(region, this->config_->activeProfile()->searchRange());
			ui.btnQueue->setEnabled(false);
		}
		else if (eros_->matchmakingState() == ErosMatchmakingState::Matched)
		{
			QMessageBox::StandardButton reply = QMessageBox::warning(this, tr("Confirm Forfeit"), tr("Are you sure you want to forfeit? This will be recorded as a loss for you and a win for your opponent."), QMessageBox::Yes | QMessageBox::No);	
			if (reply == QMessageBox::StandardButton::Yes)
			{
				emit forfeitMatchmaking();
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

	//ui.lblRegionStats->setText(tr("%1 people currently queueing on this region.").arg(eros_->regionSearchingUserCount(region)));
}

void MainWindow::erosConnectionError(QAbstractSocket::SocketError error, const QString error_string)
{
	ui.lblInformation->setText(tr("Connection error (%1): %2").arg(QString::number(error), error_string));
}

void MainWindow::erosConnected()
{
	setupWatches();
	ui.cmbRegion->clear();
	ui.lstChats->clear();
	int set_index = 0;
	int pref_region = this->config_->preferredRegion();
	for (int i =0; i < this->eros_->activeRegions().count(); i++)
	{
		ui.cmbRegion->addItem(QIcon(QString(":/img/client/icons/flags/%1").arg(Eros::regionToString(this->eros_->activeRegions()[i]))), Eros::regionToLongString(this->eros_->activeRegions()[i]), i);
		if (this->eros_->activeRegions()[i] == pref_region)
			set_index = i;
	}
	if (pref_region < 1)
		set_index = 0;

	if (ui.cmbRegion->count() > 0)
		ui.cmbRegion->setCurrentIndex(set_index);

	erosLocalUserUpdated(this->eros_->localUser());
	if (this->config_->autoJoin())
	{
		ChatRoom *room = this->eros_->getChatRoom("Starbow");
		emit joinChatRoom(room);
	}
	setUiEnabled(true);
}

void MainWindow::erosDisconnected()
{
	setUiEnabled(false);
	setQueueState(false);
	ui.lstChats->clear();
}

void MainWindow::erosHandshakeFailed()
{
	ui.lblInformation->setText(tr("Authentication failed. Server error if you previously connected fine."));
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
		openSettings();
	}
	else if (link == "#upload")
	{
		QString filename = QFileDialog::getOpenFileName(this, tr("Select Replay"), this->config_->activeProfile()->replayFolder(), tr("Replay Files (*.SC2Replay);;All Files (*.*)"));
		if (!filename.isEmpty())
		{
			emit uploadReplay(filename);
		}
	}
	else if (link == "#battlenet")
	{
		if (this->eros_->state() != ErosState::ConnectedState)
		{
			QMessageBox::information(this, tr("Unable to open Battle.Net Settings"), tr("You must be logged in before you can manage your Battle.net settings."));
		}
		else
		{
			openBnetSettings();
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

void MainWindow::lstChats_itemDoubleClicked(QListWidgetItem *item)
{
	if (item != nullptr)
	{
		ui.txtRoomName->setText(item->text());
		btnJoinRoom_pressed();
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
			widget->writeLog(tr("You have been automatically joined to this chat room for your match against <strong>%1</strong> on <a href=\"starcraft://map/%2/%3\">%4</a>. Don't forget to set the game speed to <strong>Faster</strong> when clicking the map link. We suggest joining the channel <strong>%5</strong> on Battle.net. GLHF!").arg(match->opponent()->username(), QString::number((int)region), QString::number(match->mapId()), match->mapName(), match->battleNetChannel()), false); 
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


void MainWindow::openBnetSettings()
{
	
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

void MainWindow::openSettings()
{
	if (this->settings_window_ == nullptr)
	{
		this->settings_window_ = new SettingsWindow(this, this->config_);
		QObject::connect(this->settings_window_, SIGNAL(profileChanged()), this, SLOT(activeProfileChanged()));
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
	clearWatches();

	try {
		if (this->config_->activeProfile() != nullptr)
		{
			const QString &path = this->config_->activeProfile()->replayFolder();
			if (!path.isEmpty())
			{
				addWatch(path);
			}
		}
	} catch (...)
	{
		QMessageBox::critical(this, tr("Error"), tr("An error occured while trying to monitor your specified SC2 user folder. Please make sure this application has permission to read it."));
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

void MainWindow::activeProfileChanged()
{
	emit disconnectFromEros();
	QTimer::singleShot(0, this, SLOT(connectionTimerWorker()));
}
