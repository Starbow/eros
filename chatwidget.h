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

	void setChatroom(ChatRoom *chatroom);
	void setUser(User *user);

	User *user() const;
	ChatRoom *chatroom() const;
	void writeLog(const QString &data, bool sanitize = true);

private:
	Ui::ChatWidget ui;
	ChatRoom *chatroom_;
	User *user_;

	void addUser(User *user);
	void removeUser(User *user);
	

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
