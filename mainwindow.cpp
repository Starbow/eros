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
#include "erostitlebar.h"
#include "util.h"

MainWindow::MainWindow(Eros *eros, QWidget *parent )
	: QMainWindow(parent)
{

	QFontDatabase::addApplicationFont(":/font/NotoSans-Regular");
	QFontDatabase::addApplicationFont(":/font/NotoSans-Bold");
	QFontDatabase::addApplicationFont(":/font/NotoSans-Italic");
	QFontDatabase::addApplicationFont(":/font/NotoSans-BoldItalic");
	QFontDatabase::addApplicationFont(":/font/Gobold");
	QFontDatabase::addApplicationFont(":/font/Gobold-bold");
	QFontDatabase::addApplicationFont(":/font/Gobold-thin");


	QFile version(":/data/version_info");
	if(!version.open(QIODevice::ReadOnly))
	{
		this->local_version_ = 1;
	}
	else
	{
		QByteArray data = version.readAll();
		QStringList tokens = QString::fromUtf8(data).split('|');
		this->local_version_ = tokens[0].toInt();
		version.close();
	}
	
	ui.setupUi(this);
	ui.centralWidget->setMouseTracking(true);
	this->setWindowTitle(tr("Alpha Version %1").arg(this->local_version_));

	setUiEnabled(false);
	this->eros_ = eros;
	this->config_ = new Config(this);
	this->connection_timer_ = new QTimer(this);
	this->matchmaking_timer_ = new QTimer(this);
	this->matchmaking_timer_->setInterval(500);
	this->matchmaking_start_ = new QTime();
	this->matchmaking_result_time_ = new QTime();
	this->long_process_start_time_ = new QTime();
	this->watcher_ = new DirectoryWatcher(this);
	this->watches_ = QList<DirectoryWatch*>();
	this->update_checker_nam_ = new QNetworkAccessManager(this);
	this->update_timer_ = new QTimer(this);
	this->long_process_timer_ = new QTimer(this);
	this->tray_icon_ = new QSystemTrayIcon(this);
	this->tray_icon_menu_ = new QMenu(this);
	this->preview_loader_nam_ = new QNetworkAccessManager(this);
	this->preview_cache_ = QMap<QString, QPixmap*>();
	this->upload_queue_ = QMap<QString, QTime*>();
	this->upload_queue_timer_ = new QTimer(this);

	tray_icon_action_show_ = new QAction("Hide Eros", this);
	tray_icon_action_close_ = new QAction("Close Eros", this);


	QObject::connect(tray_icon_action_show_, SIGNAL(triggered()), this, SLOT(toggleWindow()));
    QObject::connect(tray_icon_action_close_, SIGNAL(triggered()), qApp, SLOT(quit()));
	QObject::connect(this->tray_icon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconClicked(QSystemTrayIcon::ActivationReason)));


	this->tray_icon_->setIcon(QIcon(":/img/client/icons/icon_32x32"));
	this->tray_icon_menu_->addAction(tray_icon_action_show_);
	this->tray_icon_menu_->addAction(tray_icon_action_close_);
	this->tray_icon_->setContextMenu(this->tray_icon_menu_);

#if !defined(Q_OS_MAC)
    title_bar_ = ErosTitleBar::addToLayout(this, ui.verticalLayout);
    title_bar_->setMenu(this->tray_icon_menu_);
#endif



	// File watcher
	QObject::connect(this->watcher_, SIGNAL(added(const QString &, const QString &)), this, SLOT(fileAdded(const QString &, const QString&)));

	// Update checker
	QObject::connect(update_checker_nam_, SIGNAL(finished(QNetworkReply*)), this, SLOT(updateCheckerFinished(QNetworkReply*)));
	QObject::connect(this->update_timer_, SIGNAL(timeout()), this, SLOT(updateCheckerTimerWorker()));

	//Preview loader
	QObject::connect(preview_loader_nam_, SIGNAL(finished(QNetworkReply*)), this, SLOT(previewDownloadFinished(QNetworkReply*)));

	// Set up Eros signals
	/// Eros Signals
	QObject::connect(eros_, SIGNAL(stateChanged(ErosState)), this, SLOT(erosStateChanged(ErosState)));
	QObject::connect(eros_, SIGNAL(connectionError(QAbstractSocket::SocketError, const QString)), this, SLOT(erosConnectionError(QAbstractSocket::SocketError, const QString)));
	QObject::connect(eros_, SIGNAL(connected()), this, SLOT(erosConnected()));
	QObject::connect(eros_, SIGNAL(disconnected()), this, SLOT(erosDisconnected()));
	QObject::connect(eros_, SIGNAL(authenticationFailed()), this, SLOT(erosAuthenticationFailed()));
	QObject::connect(eros_, SIGNAL(alreadyLoggedIn()), this, SLOT(erosAlreadyLoggedIn()));
	QObject::connect(eros_, SIGNAL(broadcastAlert(const QString, int)), this, SLOT(erosBroadcastAlert(const QString, int)));
	QObject::connect(eros_, SIGNAL(chatRoomAdded(ChatRoom*)), this, SLOT(erosChatRoomAdded(ChatRoom*)));
	QObject::connect(eros_, SIGNAL(chatRoomRemoved(ChatRoom*)), this, SLOT(erosChatRoomRemoved(ChatRoom*)));
	QObject::connect(eros_, SIGNAL(chatRoomJoined(ChatRoom*)), this, SLOT(erosChatRoomJoined(ChatRoom*)));
	QObject::connect(eros_, SIGNAL(chatRoomLeft(ChatRoom*)), this, SLOT(erosChatRoomLeft(ChatRoom*)));
	QObject::connect(eros_, SIGNAL(chatMessageReceieved(User*, const QString)), this, SLOT(erosChatMessageReceieved(User*, const QString)));
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

	QObject::connect(eros_, SIGNAL(vetoesUpdated()), this, SLOT(erosVetoesUpdated()));
	QObject::connect(eros_, SIGNAL(toggleVetoFailed(Map*,ErosError)), this, SLOT(erosToggleVetoFailed(Map*,ErosError)));


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

	QObject::connect(this, SIGNAL(toggleVeto(Map*)), eros_, SLOT(toggleVeto(Map*)));



	// timers
	QObject::connect(this->connection_timer_, SIGNAL(timeout()), this, SLOT(connectionTimerWorker()));
	QObject::connect(this->matchmaking_timer_, SIGNAL(timeout()), this, SLOT(matchmakingTimerWorker()));
	QObject::connect(this->long_process_timer_, SIGNAL(timeout()), this, SLOT(longProcessTimerWorker()));
	QObject::connect(this->upload_queue_timer_, SIGNAL(timeout()), this, SLOT(uploadTimerWorker()));

	this->long_process_timer_->setInterval(250);

	// UI Stuff
	settings_window_ = nullptr;
	bnetsettings_window_ = nullptr;

	// Remove the close box from the first 2 tabs.
    // On mac it's LeftSide. Assuming RightSide causes a nullptr.

	for (int i = 0; i < 3; i++)
	{
		QWidget *tab = ui.tabContainer->tabBar()->tabButton(i, QTabBar::RightSide);
		if (tab != nullptr)
			tab->resize(0, 0);

		tab = ui.tabContainer->tabBar()->tabButton(i, QTabBar::LeftSide);
		if (tab != nullptr)
			tab->resize(0, 0);
	}
    
	ui.tabContainer->tabBar()->setUsesScrollButtons(true);
	
	QObject::connect(ui.tabContainer, SIGNAL(tabCloseRequested(int)), this, SLOT(tabContainer_tabCloseRequested(int)));
	QObject::connect(ui.tabContainer, SIGNAL(currentChanged(int)), this, SLOT(tabContainer_currentChanged(int)));
	QObject::connect(ui.lblBottomMenu, SIGNAL(linkActivated(const QString &)), this, SLOT(label_linkActivated(const QString&)));
	QObject::connect(ui.btnJoinRoom, SIGNAL(clicked()), this, SLOT(btnJoinRoom_pressed()));
	QObject::connect(ui.lstChats, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(lstChats_currentItemChanged(QListWidgetItem *, QListWidgetItem *)));
	QObject::connect(ui.lstChats, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(lstChats_itemDoubleClicked(QListWidgetItem*)));
	QObject::connect(ui.cmbRegion, SIGNAL(currentIndexChanged(int)), this, SLOT(cmbRegion_currentIndexChanged(int)));
	QObject::connect(ui.cmbMapRegion, SIGNAL(currentIndexChanged(int)), this, SLOT(cmbMapRegion_currentIndexChanged(int)));
	QObject::connect(ui.btnQueue, SIGNAL(clicked()), this, SLOT(btnQueue_pressed()));
	QObject::connect(ui.btnDraw, SIGNAL(clicked()), this, SLOT(btnDraw_pressed()));
	QObject::connect(ui.btnNoShow, SIGNAL(clicked()), this, SLOT(btnNoShow_pressed()));
	QObject::connect(ui.lstMaps, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(lstMaps_currentItemChanged(QListWidgetItem *, QListWidgetItem *)));
	QObject::connect(ui.btnToggleVeto, SIGNAL(clicked()), this, SLOT(btnToggleVeto_clicked()));

#if !defined(Q_OS_MAC)
	this->tray_icon_->show();
#endif

		// The user should be prevented from emptying invalid values in the settings dialog.
	if (this->config_->profiles().count() == 0)
	{
		QMessageBox::information(this, "Eros", tr("Welcome to Eros! You need to configure some settings in order to continue. The settings window will now open."));
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

void MainWindow::changeEvent(QEvent* e)
{
	if (e != nullptr)
	{
		switch (e->type())
		{
		case QEvent::LanguageChange:
			ui.retranslateUi(this);
			this->setWindowTitle(tr("Alpha Version %1").arg(this->local_version_));
			erosStateChanged(this->eros_->state());
			if (this->eros_->state() == ErosState::ConnectedState)
				erosConnected();
			break;
		}
	}
}


void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
#if !defined(Q_OS_MAC)
	this->title_bar_->mouseMoveEvent(e);
#endif
}
void MainWindow::mousePressEvent(QMouseEvent *e)
{
#if !defined(Q_OS_MAC)
	this->title_bar_->mousePressEvent(e);
#endif
}
void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
#if !defined(Q_OS_MAC)
	this->title_bar_->mouseReleaseEvent(e);
#endif
}

void MainWindow::uploadTimerWorker()
{
	
	QMutableMapIterator<QString, QTime*> i(this->upload_queue_);
	while (i.hasNext())
	{
		i.next();
		if (i.value()->elapsed() > 5000)
		{
			emit uploadReplay(i.key());
			i.remove();
		}
	}
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

void MainWindow::closeEvent(QCloseEvent *event)
{
#if !defined(Q_OS_MAC)
	if (this->tray_icon_->isVisible()) {
		toggleWindow();
		event->ignore();

		this->config_->setTrayNotificationShown(true);
		this->tray_icon_->showMessage("Eros", tr("Eros is still running in the notification tray. Right click the icon if you want to exit."));
	}
#else
    QApplication::exit();
#endif
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
		QStringList tokens = QString::fromUtf8(data).split('|');
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
		ui.lblRegionStats->setText(tr("%n (person/people) currently queueing on this region.", "", count));
	}
}

void MainWindow::erosStatsUpdated(int online, int searching)
{

	ui.lblRegionStats->setText(tr("%n (person/people) currently online.", "", online));
	
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
	QString map = tr("1v1 on <a href=\"starcraft://map/%1/%2\">%3</a>", "Please keep the link intact.").arg(QString::number((int)region), QString::number(match->mapId()), match->mapName());
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
	if (index < 0)
		return;

	if (eros_->state() != ErosState::ConnectedState)
		return;

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

void MainWindow::cmbMapRegion_currentIndexChanged (int index)
{
	if (this->eros_->state() != ErosState::ConnectedState)
		return;

	ui.lstMaps->clear();

	if (index < 0)
		return;

	LocalUser *user = eros_->localUser();
	int regionIndex = ui.cmbMapRegion->currentData().toInt();
	ErosRegion region = eros_->activeRegions()[regionIndex];

	for (int i = 0; i < eros_->mapPool().size(); i++)
	{
		Map *map = eros_->mapPool()[i];
		if (map->region() == region)
		{
			QListWidgetItem *item = new QListWidgetItem(map->name());
			item->setData(101, (int)region);
			item->setData(102, (int)map->battleNetId());

			if (eros_->localUser()->vetoes().contains(map))
			{
				item->setIcon(QIcon(":/img/client/icons/cross"));
			}
			ui.lstMaps->addItem(item);
		}
	}
	ui.lstMaps->sortItems();

	if (this->selected_map_ != nullptr)
	{
		for (int i = 0; i < ui.lstMaps->count(); i++)
		{
			QListWidgetItem *item = ui.lstMaps->item(i);

			if (item->data(101).toInt() == this->selected_map_->region() && item->data(102).toInt() == this->selected_map_->battleNetId())
			{
				ui.lstMaps->setCurrentItem(item);
				break;
			}
		}
	}
}
void MainWindow::erosConnectionError(QAbstractSocket::SocketError error, const QString error_string)
{
	ui.lblInformation->setText(tr("Connection error (%1): %2").arg(QString::number(error), error_string));
}

void MainWindow::erosConnected()
{
	setupWatches();
	ui.cmbRegion->clear();
	ui.cmbMapRegion->clear();
	ui.lstMaps->clear();
	ui.lstChats->clear();
	this->selected_map_ = nullptr;
	int set_index = 0;
	int pref_region = this->config_->preferredRegion();
	for (int i =0; i < this->eros_->activeRegions().count(); i++)
	{
		ui.cmbRegion->addItem(QIcon(QString(":/img/client/icons/flags/%1").arg(Eros::regionToString(this->eros_->activeRegions()[i]))), Eros::regionToLongString(this->eros_->activeRegions()[i]), i);

		int vetoes = 0;
		for (int j = 0; j < eros_->localUser()->vetoes().size(); j++)
		{
			if (eros_->localUser()->vetoes()[j]->region() == this->eros_->activeRegions()[i])
				vetoes++;
		}
		

		ui.cmbMapRegion->addItem(QIcon(QString(":/img/client/icons/flags/%1").arg(Eros::regionToString(this->eros_->activeRegions()[i]))), QString("%1 (%2)").arg(Eros::regionToLongString(this->eros_->activeRegions()[i]), tr("%1/%n veto(es) used", "", eros_->maxVetoes()).arg(vetoes)), i);

		if (this->eros_->activeRegions()[i] == pref_region)
			set_index = i;
	}
	if (pref_region < 1)
		set_index = 0;

	if (ui.cmbRegion->count() > 0)
		ui.cmbRegion->setCurrentIndex(set_index);
	if (ui.cmbMapRegion->count() > 0)
		ui.cmbMapRegion->setCurrentIndex(set_index);
	erosLocalUserUpdated(this->eros_->localUser());
	if (this->config_->autoJoin())
	{
		ChatRoom *room = this->eros_->getChatRoom("Starbow");
		emit joinChatRoom(room);
	}
	setUiEnabled(true);
	QTimer::singleShot(0, this->eros_, SLOT(refreshChatRooms()));
	this->upload_queue_timer_->start(250);
}

void MainWindow::erosDisconnected()
{
	setUiEnabled(false);
	setQueueState(false);
	ui.lstChats->clear();
	ui.lstMaps->clear();
	this->selected_map_ = nullptr;
	this->upload_queue_timer_->stop();
}

void MainWindow::erosAuthenticationFailed()
{
	ui.lblInformation->setText(tr("Authentication failed. Server error if you previously connected fine."));
}


void MainWindow::erosAlreadyLoggedIn()
{
	ui.lblInformation->setText(tr("Already logged in. Check that you're not running another client."));
}

void MainWindow::erosLocalUserUpdated(LocalUser *user)
{
	cmbRegion_currentIndexChanged(ui.cmbRegion->currentIndex());
	cmbMapRegion_currentIndexChanged(ui.cmbMapRegion->currentIndex());
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
	if (link == "#settings")
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

void MainWindow::tabContainer_currentChanged(int index)
{
	if (ChatWidget* widget = dynamic_cast<ChatWidget*>(ui.tabContainer->widget(index)))
	{
		widget->resetEventCount();
	}
}

void MainWindow::tabContainer_tabCloseRequested(int index)
{
	if (this->settings_window_ != nullptr && ui.tabContainer->widget(index) == this->settings_window_)
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
	else if (ChatWidget* widget = dynamic_cast<ChatWidget*>(ui.tabContainer->widget(index)))
	{
		if (widget->chatroom() != nullptr)
		{
			emit leaveChatRoom(widget->chatroom());
		}
		
		delete widget;
	}
	else if(bnetsettings_window_ != nullptr && ui.tabContainer->widget(index) == bnetsettings_window_)
	{
		ui.tabContainer->removeTab(index);
		delete bnetsettings_window_;
		bnetsettings_window_ = nullptr;
	}
}

void MainWindow::setUiEnabled(bool enabled)
{
	if (enabled)
	{
		ui.tabMatchmaking->setEnabled(true);
		ui.tabChat->setEnabled(true);
		ui.tabMaps->setEnabled(true);
	}
	else
	{
		ui.tabMatchmaking->setDisabled(true);
		ui.tabChat->setDisabled(true);
		ui.tabMaps->setDisabled(true);
	}
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

void MainWindow::chatEventCountUpdated(ChatWidget *widget)
{
	for (int i = 0; i < ui.tabContainer->count(); i++)
	{
		if (ChatWidget* tabWidget = dynamic_cast<ChatWidget*>(ui.tabContainer->widget(i)))
		{
			if (tabWidget == widget)
			{
				if (ui.tabContainer->currentIndex() == i && widget->eventCount() > 0)
				{
					widget->resetEventCount();
				}

				if (widget->eventCount() == 0)
				{
					ui.tabContainer->setTabText(i, Util::truncateText(widget->name()));
				}
				else
				{
					ui.tabContainer->setTabText(i, QString("(%1) %2").arg(QString::number(widget->eventCount()), Util::truncateText(widget->name())));
				}

				break;
			}
		}
	}
}

void MainWindow::privateChatRequested(User *user)
{
	openPrivateChat(user, true);
}

ChatWidget *MainWindow::openPrivateChat(User *user, bool activate, const QString &initial_message)
{
	for (int i = 0; i < ui.tabContainer->count(); i++)
	{
		if (ChatWidget* widget = dynamic_cast<ChatWidget*>(ui.tabContainer->widget(i)))
		{
			if (widget->user() != nullptr && widget->user()->username() == user->username())
			{
				if (widget->user() != user)
				{
					widget->setUser(user);
					widget->chatMessageReceieved(user, initial_message);
				}

				if (activate)
					this->ui.tabContainer->setCurrentIndex(i);

				return widget;
			}
		}
	}

	ChatWidget *widget = new ChatWidget(this->eros_, user, initial_message);
	QObject::connect(widget, SIGNAL(eventCountUpdated(ChatWidget*)), this, SLOT(chatEventCountUpdated(ChatWidget*)));
	
	int id = ui.tabContainer->addTab(widget, QIcon(":/img/client/icons/private_chat"), Util::truncateText(user->username()));
	if (activate)
		this->ui.tabContainer->setCurrentIndex(id);

	return widget;
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
	QObject::connect(widget, SIGNAL(eventCountUpdated(ChatWidget*)), this, SLOT(chatEventCountUpdated(ChatWidget*)));
	QObject::connect(widget, SIGNAL(privateChatRequested(User *)), this, SLOT(privateChatRequested(User *)));
	QString icon = ":/img/client/icons/public_chat";
	if (room->passworded())
	{
		icon = ":/img/client/icons/private_chat";
	}
	else if (!room->joinable() || room->fixed())
	{
		icon = ":/img/client/icons/match_chat";
	}
	int id = ui.tabContainer->addTab(widget, QIcon(icon), Util::truncateText(room->name()));
	
	const MatchmakingMatch *match = eros_->matchmakingMatch();
	if (match != nullptr)
	{
		if (room->key() == match->chatRoom()->key())
		{
			int regionIndex = ui.cmbRegion->currentData().toInt();
			ErosRegion region = eros_->activeRegions()[regionIndex];
			//Split these up to minimise HTML in translation files.
			QString map = QString("<a href=\"starcraft://map/%1/%2\">%3</a>").arg(QString::number((int)region), QString::number(match->mapId()), match->mapName());
			QString opponent = QString("<strong>%1</strong> <a href=\"http://www.starbowmod.com/ladder/player/%2\" title=\"%3\"><img src=\":/img/client/icons/profile\" /></a>").arg(match->opponent()->username(), QString::number(match->opponent()->id()), tr("View player profile"));
			QString speed = QString("<strong>%1</strong>").arg(tr("Faster"));
			QString channel = QString("<strong>%1</strong> <a href=\"clipboard:%1\" title=\"%2\"><img src=\":/img/client/icons/clipboard\" /></a>").arg(match->battleNetChannel(), tr("Copy to clipboard"));

			widget->writeLog(tr("You have been automatically joined to this chat room for your match against %1 on %2. Don't forget to set the game speed to %3 when clicking the map link. We suggest joining the channel %4 on Battle.net. GLHF!").arg(opponent, map, speed, channel)); 
		}
	}

	if (!room->forced())
	{
		ui.tabContainer->setCurrentIndex(id);
	}

}
void MainWindow::erosChatRoomLeft(ChatRoom *room)
{

}

void MainWindow::erosChatMessageReceieved(User *user, const QString message)
{
	openPrivateChat(user, false, message);
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
		ui.tabContainer->insertTab(3, bnetsettings_window_, tr("Battle.net Accounts"));
		ui.tabContainer->setCurrentIndex(3);		
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
		ui.tabContainer->insertTab(3, this->settings_window_, tr("Settings"));
		ui.tabContainer->setCurrentIndex(3);
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
		this->watcher_->removeWatch(this->watches_.at(i)->directory());
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

void MainWindow::fileAdded(const QString &dir, const QString &filename)
{
	QStringList pieces = filename.split('.');
	QString extension = pieces.value(pieces.length()-1);

	if(extension.toLower() == "sc2replay")
	{
		ui.lblInformation->setText(tr("Uploading %1").arg(filename));
        QFileInfo info(filename);
        QString path;
        if (info.isAbsolute())
        {
            path = filename;
        }
        else
        {
            path = dir + "/" + filename;
        }


		if (!this->upload_queue_.contains(path))
		{
			QTime *now = new QTime();
			now->start();
			this->upload_queue_.insert(path, now);
		}
	}
}

void MainWindow::activeProfileChanged()
{
	emit disconnectFromEros();
	QTimer::singleShot(0, this, SLOT(connectionTimerWorker()));
}

void MainWindow::previewDownloadFinished(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError)
	{
		QByteArray data = reply->readAll();
		QPixmap *pixmap = new QPixmap();
		pixmap->loadFromData(data);

		ui.lblMapPreview->setPixmap(*pixmap);
		preview_cache_.insert(reply->request().url().toString(), pixmap);
	}
}

void MainWindow::lstMaps_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
	if (current != nullptr)
	{
		ErosRegion region = (ErosRegion)current->data(101).toInt();
		int id = current->data(102).toInt();
		
		selected_map_ = eros_->findMap(region, id);

		if (selected_map_ != nullptr)
		{
			bool vetoed = eros_->localUser()->vetoes().contains(selected_map_);

			if (vetoed)
				ui.btnToggleVeto->setText(tr("Unveto"));
			else
				ui.btnToggleVeto->setText(tr("Veto"));
		
			QString infoLink = tr("<em>No info link</em>");
			if (!selected_map_->infoUrl().isEmpty())
				infoLink = tr("<a href=\"%1\">Info link</a>").arg(selected_map_->infoUrl());

			ui.lblMapInfoLinks->setText(QString("<img src=\":/img/client/icons/flags/%1\"> %2<br />%3 | <a href=\"starcraft://map/%4/%5\">%6</a>").arg(Eros::regionToString(region), selected_map_->name(), infoLink, QString::number(region), QString::number(id), tr("View in game")));

			if (selected_map_->description().isEmpty())
			{
				ui.lblMapDescription->setText(tr("<em>No Description</em>"));
			}
			else
			{
				ui.lblMapDescription->setText(selected_map_->description());
			}

			if (selected_map_->previewUrl().isEmpty())
			{
				ui.lblMapPreview->setPixmap(QPixmap(":/img/maps/nopreview"));
			} 
			else
			{
				ui.lblMapPreview->setPixmap(QPixmap(":/img/maps/loading"));
				if (preview_cache_.contains(selected_map_->previewUrl()))
				{
					ui.lblMapPreview->setPixmap(*preview_cache_[selected_map_->previewUrl()]);
				}
				else
				{
					QNetworkReply* reply = this->preview_loader_nam_->get(QNetworkRequest(selected_map_->previewUrl()));
				}
			}
		}
	}

}

void MainWindow::btnToggleVeto_clicked()
{
	if (this->selected_map_ == nullptr)
		return;
	ui.btnToggleVeto->setDisabled(true);
	emit toggleVeto(this->selected_map_);
}

void MainWindow::erosToggleVetoFailed(Map* map, ErosError error)
{
	QMessageBox::information(this, tr("Veto Update Failed"), tr("Failed to update veto for \"%1\". %2", "%1 is the map name. %2 is an error message (found in liberos).").arg(map->name(), Eros::errorString(error)));
	ui.btnToggleVeto->setEnabled(true);
}
void MainWindow::erosVetoesUpdated()
{
	ui.btnToggleVeto->setEnabled(true);
	for (int i = 0; i < ui.cmbMapRegion->count(); i++)
	{
		int vetoes = 0;
		ErosRegion region = eros_->activeRegions()[ui.cmbMapRegion->itemData(i).toInt()];

		for (int j = 0; j < eros_->localUser()->vetoes().size(); j++)
		{
			if (eros_->localUser()->vetoes()[j]->region() == region)
				vetoes++;
		}

		ui.cmbMapRegion->setItemText(i, QString("%1 (%2)").arg(Eros::regionToLongString(this->eros_->activeRegions()[i]), tr("%1/%n veto(es) used", "", eros_->maxVetoes()).arg(vetoes)));
	}
}
