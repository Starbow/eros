#include "chatwidget.h"

#include <QScrollBar>

ChatWidget::ChatWidget(Eros *eros, User *user, QWidget *parent)
	:QWidget(parent)
{
	ui.setupUi(this);
	this->user = user;
	this->chatroom = nullptr;

	// We don't need the user list
	ui.listUsers->hide();

	QObject::connect(eros, SIGNAL(chatMessageSent(User*, const QString)), this, SLOT( chatMessageSent(User*, const QString)));
	QObject::connect(eros, SIGNAL(chatMessageReceieved(User*, const QString)), this, SLOT( chatMessageReceieved(User*, const QString)));
	QObject::connect(eros, SIGNAL(chatMessageFailed(User*, const QString, ErosError)), this, SLOT( chatMessageFailed(User*, const QString, ErosError)));

	QObject::connect(this, SIGNAL(sendMessage(User *, const QString)), eros, SLOT(sendMessage(User *, const QString)));
	
	QObject::connect(ui.btnSend, SIGNAL(pressed()), this, SLOT(sendMessagePressed()));
	QObject::connect(ui.txtInput, SIGNAL(returnPressed()), this, SLOT(sendMessagePressed()));
}
ChatWidget::ChatWidget(Eros *eros, ChatRoom *chatroom, QWidget *parent)
	:QWidget(parent)
{
	ui.setupUi(this);
	this->user = nullptr;
	this->chatroom = chatroom;

	QObject::connect(eros, SIGNAL(chatMessageSent(ChatRoom*, const QString)), this, SLOT( chatMessageSent(ChatRoom*, const QString)));
	QObject::connect(eros, SIGNAL(chatMessageReceieved(ChatRoom*, User*, const QString)), this, SLOT( chatMessageReceieved(ChatRoom*, User*, const QString)));
	QObject::connect(eros, SIGNAL(chatMessageFailed(ChatRoom*, const QString, ErosError)), this, SLOT( chatMessageFailed(ChatRoom*, const QString, ErosError)));

	QObject::connect(eros, SIGNAL(chatRoomUserJoined(ChatRoom*, User*)), this, SLOT(chatRoomUserJoined(ChatRoom*, User*)));
	QObject::connect(eros, SIGNAL(chatRoomUserLeft(ChatRoom*, User*)), this, SLOT(chatRoomUserLeft(ChatRoom*, User*)));

	QObject::connect(this, SIGNAL(sendMessage(ChatRoom *, const QString)), eros, SLOT(sendMessage(ChatRoom *, const QString)));

	QObject::connect(ui.btnSend, SIGNAL(pressed()), this, SLOT(sendMessagePressed()));
	QObject::connect(ui.txtInput, SIGNAL(returnPressed()), this, SLOT(sendMessagePressed()));
	const QList<User *> &users = chatroom->participants();
	for (int i = 0; i < users.size(); i++)
	{
		this->ui.listUsers->addItem(users[i]->username());
	}
}

ChatWidget::~ChatWidget()
{

}

void ChatWidget::addUser(User *user)
{
	writeLog(QString(tr("%1 has joined the chat.")).arg(user->username()));
	this->ui.listUsers->addItem(user->username());
}

void ChatWidget::removeUser(User *user)
{
	writeLog(QString(tr("%1 has left the chat.")).arg(user->username()));
	qDeleteAll(this->ui.listUsers->findItems(user->username(), Qt::MatchFlag::MatchExactly));
}

void ChatWidget::writeLog(const QString &data) 
{	
    ui.txtMessages->append(data);
    ui.txtMessages->verticalScrollBar()->setValue(ui.txtMessages->verticalScrollBar()->maximum());
}

void ChatWidget::sendMessagePressed()
{
	QString message = ui.txtInput->text().trimmed();

	if (!message.isEmpty())
	{
		if (this->user != nullptr)
			emit sendMessage(this->user, message);
		
		if (this->chatroom != nullptr)
			emit sendMessage(this->chatroom, message);

		//writeLog(QString("%1: %2").arg( "Me" , message ));
	}

	ui.txtInput->clear();
}

void ChatWidget::chatRoomUserJoined(ChatRoom *room, User *user)
{
	if (room == this->chatroom)
		addUser(user);
}
void ChatWidget::chatRoomUserLeft(ChatRoom *room, User *user)
{
	if (room == this->chatroom)
		removeUser(user);
}


void ChatWidget::chatMessageSent(ChatRoom *room, const QString message)
{
	if (room == this->chatroom)
		return;
	//printf("Your message \"%s\" in chatroom: %s was sent successfully\n", message.toStdString().c_str(), room->name().toStdString().c_str());
}

void ChatWidget::chatMessageReceieved(ChatRoom *room, User *user, const QString message)
{
	if (room == this->chatroom)
		writeLog(QString("%1: %2").arg( user->username() , message ));
}

void ChatWidget::chatMessageReceieved(User *user, const QString message)
{
	if (this->user == user)
		writeLog(QString("%1: %2").arg( user->username() , message ));
}


void ChatWidget::chatMessageSent(User *user, const QString message)
{
	if (this->user == user)
		writeLog(QString("%1: %2").arg( user->username() , message ));
}



void ChatWidget::chatMessageFailed(ChatRoom *room, const QString message, ErosError code)
{
	if (this->chatroom == room)
		writeLog(QString("Chat Message \"%1\" failed.").arg(message));

}

void ChatWidget::chatMessageFailed(User *user, const QString message, ErosError code)
{
	if (this->user == user)
		writeLog(QString("Chat message \"%1\" failed.").arg(user->username()));
}
