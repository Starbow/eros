#ifndef CHATROOMWIDGET_H
#define CHATROOMWIDGET_H

#include <QWidget>
#include "ui_chatwidget.h"
#include "../liberos/eros.h"

class ChatWidget : public QWidget
{
	Q_OBJECT

public:
	ChatWidget(Eros *eros, User *user, const QString &initial_message = "", QWidget *parent = 0);
	ChatWidget(Eros *eros, ChatRoom *chatroom, QWidget *parent = 0);
	~ChatWidget();

	void setChatroom(ChatRoom *chatroom);
	void setUser(User *user);

	void resetEventCount();
	User *user() const;
	ChatRoom *chatroom() const;
    void writeLog(const QString &data);

	int eventCount() const;
	const QString &name() const;
private:
	Ui::ChatWidget ui;
	Eros *eros_;
	ChatRoom *chatroom_;
	User *user_;

	void addUser(User *user);
	void removeUser(User *user);
	QString getUsername(User *user);
	QString getPrivateUsername(User *user);
	QString getColour(User *user);
	QString colourise(const QString &text, User *user, bool sending = false);
	void addEvent();
	void populateUserList();
	int events_;
public slots:
	
	void chatMessageReceieved(ChatRoom *room, User *user, const QString message);
	void chatMessageReceieved(User *user, const QString message);
private slots:
	void chatMessageSent(ChatRoom *room, const QString message);
	void chatMessageSent(User *user, const QString message);
	void chatMessageFailed(ChatRoom *room, const QString message, ErosError code);
	void chatMessageFailed(User *user, const QString message, ErosError code);

	void chatRoomUserJoined(ChatRoom *room, User *user);
	void chatRoomUserLeft(ChatRoom *room, User *user);

	void disconnected();
	void connected();

	void sendMessagePressed();

	void anchorClicked(QUrl);
	void userDoubleClicked(QListWidgetItem * item);

signals:
	void sendMessage(ChatRoom *room, const QString message);
	void sendMessage(User *user, const QString message);
	void eventCountUpdated(ChatWidget *widget);
	void privateChatRequested(User *user);
};

#endif // CHATROOMWIDGET_H
