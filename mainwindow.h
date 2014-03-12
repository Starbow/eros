#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QSettings>
#include <QTimer>
#include <QTime>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QSound>

#include "config.h"
#include "chatwidget.h"
#include "settingswindow.h"
#include "bnetsettingswindow.h"
#include "ui_mainwindow.h"
#include "erostitlebar.h"
#include "directorywatcher.h"

#include "../liberos/eros.h"


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(Eros *eros, QWidget *parent = 0);
	~MainWindow();

private slots:

	// Tray slots
	void trayIconClicked(QSystemTrayIcon::ActivationReason);
	void toggleWindow();

	// Config slots

	void activeProfileChanged();

	// UI Slots
	void label_linkActivated(const QString &link);
	void tabContainer_tabCloseRequested(int index);
	void tabContainer_currentChanged(int index);
	void setUiEnabled(bool);
	void setQueueState(bool);

	void btnJoinRoom_pressed();
	void btnQueue_pressed();
	void btnDraw_pressed();
	void btnNoShow_pressed();


	void lstChats_currentItemChanged(QListWidgetItem *, QListWidgetItem *);
	void lstChats_itemDoubleClicked(QListWidgetItem *);
	void cmbRegion_currentIndexChanged (int index);


	void connectionTimerWorker();
	void matchmakingTimerWorker();
	void longProcessTimerWorker();
	void uploadTimerWorker();

	void openBnetSettings();
	void openSettings();

	// Eros Slots
	void erosStateChanged(ErosState state);
	void erosConnectionError(QAbstractSocket::SocketError, const QString error_string);
	void erosConnected();
	void erosDisconnected();
	void erosAuthenticationFailed();
	void erosAlreadyLoggedIn();
	void erosBroadcastAlert(const QString message, int hint);

	// Chat
	void erosChatRoomJoined(ChatRoom *room);
	void erosChatRoomLeft(ChatRoom *room);
	void erosChatRoomAdded(ChatRoom *room);
	void erosChatRoomRemoved(ChatRoom *room);

	void erosLocalUserUpdated(LocalUser *user);

	void refreshChatRoomList();

	void chatEventCountUpdated(ChatWidget *widget);

	// Matchmaking
	void erosMatchmakingStateChanged(ErosMatchmakingState status);
	void erosMatchmakingMatchFound(MatchmakingMatch *match);
	void erosRegionStatsUpdated(ErosRegion region, int searching);
	void erosStatsUpdated(int active, int searching);
	void erosReplayUploaded();
	void erosReplayUploadError(ErosError error);
	void erosUploadProgress(qint64 written, qint64 total);
	void fileAdded(const QString &dir, const QString &filename);

	void erosLongProcessStateChanged(ErosLongProcessState);
	void erosDrawRequested();
	void erosDrawRequestFailed();
	void erosNoShowRequested();
	void erosNoShowRequestFailed();

	void erosAcknowledgeLongProcessFailed();
	void erosAcknowledgedLongProcess();

	void erosToggleVetoFailed(Map* map, ErosError error);
	void erosVetoesUpdated();

	// Update checker
	void updateCheckerFinished(QNetworkReply*);
	void updateCheckerTimerWorker();

	// Vetoes
	void cmbMapRegion_currentIndexChanged(int index);
	void lstMaps_currentItemChanged(QListWidgetItem *, QListWidgetItem *);
	void previewDownloadFinished(QNetworkReply*);
	void btnToggleVeto_clicked();
signals:
	// Connectivity slots
	void connectToEros(const QString server, const QString username, const QString password);
	void disconnectFromEros();

	// Matchmaking slots
	void queueMatchmaking(ErosRegion region, int search_radius);
	void dequeueMatchmaking();
	void forfeitMatchmaking();

	// Chat slots
	void sendMessage(ChatRoom *room, const QString message);
	void sendMessage(User *user, const QString message);
	void joinChatRoom(ChatRoom *room);
	void joinChatRoom(ChatRoom *room, const QString password);
	void leaveChatRoom(ChatRoom *room);
	void refreshChatRooms();

	// Character slots
	void addCharacter(const QString battle_net_url);
	void updateCharacter(Character *character, int new_character_code, const QString new_game_profile_link);
	void removeCharacter(Character *character);

	//Upload replay slots
	void uploadReplay(const QString path);
	void uploadReplay(QIODevice *device);

	// Opponent AFK stuff.
	void requestDraw();
	void requestNoShow();
	
	void acknowledgeLongProcess(bool response);

	void toggleVeto(Map *map);

private:
	Ui::MainWindow ui;

	QTimer *connection_timer_;
	QTimer *matchmaking_timer_;
	QTimer *update_timer_;
	QTimer *long_process_timer_;
	QTime *matchmaking_start_;
	QTime *matchmaking_result_time_;
	QTime *long_process_start_time_;
	QString configPath_;
	Eros *eros_;
	Config *config_;
	SettingsWindow *settings_window_;
	BnetSettingsWindow *bnetsettings_window_;
	QNetworkAccessManager *update_checker_nam_;
	QString username_;
	QString authtoken_;
	QString server_;
	DirectoryWatcher *watcher_;
	QList<DirectoryWatch*> watches_;
	QSystemTrayIcon *tray_icon_;
	QMenu *tray_icon_menu_;
    QAction *tray_icon_action_show_;
    QAction *tray_icon_action_close_;
	QSound *notification_sound_;
	ErosTitleBar *title_bar_;
	QNetworkAccessManager *preview_loader_nam_;
	QMap<QString, QPixmap*> preview_cache_;
	Map *selected_map_;
	QMap<QString, QTime*> upload_queue_;
	QTimer *upload_queue_timer_;

	int local_version_;

	void clearWatches();
	void setupWatches();
	void addWatch(const QString &);

	void closeEvent(QCloseEvent *);

	void mouseMoveEvent(QMouseEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);

	void changeEvent(QEvent* e);
};

#endif // EROS_H
