#ifndef CHATROOMWIDGET_H
#define CHATROOMWIDGET_H

#include <QWidget>
#include "ui_chatwidget.h"
#include "../liberos/eros.h"

class ChatWidget : public QWidget
{
	Q_OBJECT

public:
	ChatWidget(Eros *eros, User *user, QWidget *parent = 0);
	ChatWidget(Eros *eros, ChatRoom *chatroom, QWidget *parent = 0);
	~ChatWidget();

private:
	Ui::ChatWidget ui;
	ChatRoom *chatroom;
	User *user;

	void addUser(User *user);
	void removeUser(User *user);
	void writeLog(const QString &data);

private slots:
	
	void chatMessageReceieved(ChatRoom *room, User *user, const QString message);
	void chatMessageReceieved(User *user, const QString message);

	void chatMessageSent(ChatRoom *room, const QString message);
	void chatMessageSent(User *user, const QString message);
	void chatMessageFailed(ChatRoom *room, const QString message, ErosError code);
	void chatMessageFailed(User *user, const QString message, ErosError code);

	void chatRoomUserJoined(ChatRoom *room, User *user);
	void chatRoomUserLeft(ChatRoom *room, User *user);

	void sendMessagePressed();

signals:
	void sendMessage(ChatRoom *room, const QString message);
	void sendMessage(User *user, const QString message);
};

#endif // CHATROOMWIDGET_H
