#ifndef CHAT_H
#define CHAT_H

#include <qmessagebox.h>
#include <qinputdialog.h>

#include "ui_mainwindow.h"
#include "chatwidget.h"
#include <qmap.h>

class Chat : public QObject
{
	Q_OBJECT

public:
	Chat(QWidget *parent, Eros *client, Ui::MainWindow *ui, const char *username);
	~Chat();

	void sendPrivateMessage(QString message, User *user);
	void sendRoomMessage(QString message, ChatRoom *room);
	void joinProtectedRoom(ChatRoom *room, QString password);
	void joinRoom(ChatRoom *room);
	void leaveRoom(ChatRoom *room);
	void refreshRoomList();

//heaps of these probably should be private... not public
public slots:
	void clientChatRoomJoined(ChatRoom *room);
	void clientChatRoomLeft(ChatRoom *room);

	void clientRefreshRooms();
	void btnJoin_click();
	void btnLeave_click();
	void btnCreateRoom_click();

	void clientChatRoomAdded(ChatRoom *room);
	void testslot(QString testmsg);


private slots:
	void currentChatRoomTabChanged();
	void clientTabCloseRequested(int index);

private:
	Eros *client_;
	QWidget *parent_;
	QComboBox *cmbChatList_;

	//DEPRECATED
	void writeLog(QString data);


	Ui::MainWindow *mainUi_;
	ChatRoom *nameToRoom(QString name);

	QMap<QString, ChatWidget*> chatRoomMap_;

	int nameToTabIndex(QString name);


};
#endif