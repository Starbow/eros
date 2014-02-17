#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QSettings>
#include <QTimer>
#include <QTime>
#include <QTimer>
#include <qsimplefilewatcher.h>

#include "config.h"
#include "settingswindow.h"
#include "ui_mainwindow.h"
#include "../liberos/eros.h"


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(Eros *eros, QWidget *parent = 0);
	~MainWindow();

private slots:

	// Config slots

	//void activeProfileChanged(Profile *profile);

	// UI Slots
	void label_linkActivated(const QString &link);
	void tabContainer_tabCloseRequested(int index);
	void setUiEnabled(bool);
	void setQueueState(bool);

	void btnJoinRoom_pressed();
	void btnQueue_pressed();
	void lstChats_currentItemChanged(QListWidgetItem *, QListWidgetItem *);
	void cmbRegion_currentIndexChanged (int index);

	void connectionTimerWorker();
	void matchmakingTimerWorker();

	void openBnetSettings();


	// Eros Slots
	void erosStateChanged(ErosState state);
	void erosConnected();
	void erosDisconnected();

	// Chat
	void erosChatRoomJoined(ChatRoom *room);
	void erosChatRoomLeft(ChatRoom *room);
	void erosChatRoomAdded(ChatRoom *room);
	void erosChatRoomRemoved(ChatRoom *room);

	void erosLocalUserUpdated(LocalUser *user);

	void refreshChatRoomList();

	// Matchmaking
	void erosMatchmakingStateChanged(ErosMatchmakingState status);
	void erosMatchmakingMatchFound(MatchmakingMatch *match);
	void erosRegionStatsUpdated(ErosRegion region, int searching);
	void erosReplayUploaded();
	void erosReplayUploadError(ErosError error);
	void erosUploadProgress(qint64 written, qint64 total);
	void fileAction(WatchID watchId, const QString &dir, const QString &filename, Action action);

signals:
	// Connectivity slots
	void connectToEros(const QString server, const QString username, const QString password);
	void disconnectFromEros();

	// Matchmaking slots
	void queueMatchmaking(ErosRegion region, int search_radius);
	void dequeueMatchmaking();

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

private:
	Ui::MainWindow ui;

	QTimer *connection_timer_;
	QTimer *matchmaking_timer_;
	QTime *matchmaking_start_;
	QString configPath_;
	Eros *eros_;
	Config *config_;
	SettingsWindow *settings_window_;
	BnetSettingsWindow *bnetsettings_window_;
	QString username_;
	QString authtoken_;
	QString server_;
	QSimpleFileWatcher *watcher_;
	QList<WatchID> watches_;


	void clearWatches();
	void setupWatches();
	void addWatch(const QString &);

	
};

#endif // EROS_H
