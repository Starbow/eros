#include "chatwidget.h"

#include <QScrollBar>
#include <QTime>

ChatWidget::ChatWidget(Eros *eros, User *user, QWidget *parent)
	:QWidget(parent)
{
	ui.setupUi(this);
	setUser(user);

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
	setChatroom(chatroom);

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
	this->ui.listUsers->sortItems();
}

ChatWidget::~ChatWidget()
{

}

ChatRoom *ChatWidget::chatroom() const
{
	return this->chatroom_;
}

User *ChatWidget::user() const
{
	return this->user_;
}
void ChatWidget::setChatroom(ChatRoom *chatroom)
{
	this->chatroom_ = chatroom;
	this->user_ = nullptr;
}
void ChatWidget::setUser(User *user)
{
	this->user_ = user;
	this->chatroom_ = nullptr;
}

void ChatWidget::addUser(User *user)
{
	if (!this->chatroom_->forced())
		writeLog(QString(tr("%1 has joined the chat.")).arg(user->username()));
	this->ui.listUsers->addItem(user->username());
	this->ui.listUsers->sortItems();
}

void ChatWidget::removeUser(User *user)
{
	if (!this->chatroom_->forced())
		writeLog(QString(tr("%1 has left the chat.")).arg(user->username()));
	qDeleteAll(this->ui.listUsers->findItems(user->username(), Qt::MatchFlag::MatchExactly));
}

void ChatWidget::writeLog(const QString &data, bool sanitize)
{		
	QTextCharFormat format;
	ui.txtMessages->setCurrentCharFormat(format);
    if (sanitize)
    {
        QString sanitized(data);
		sanitized = sanitized.replace("&", "&amp;", Qt::CaseSensitivity::CaseInsensitive);
        sanitized = sanitized.replace("<", "&lt;", Qt::CaseSensitivity::CaseInsensitive);
		sanitized = sanitized.replace(">", "&gt;", Qt::CaseSensitivity::CaseInsensitive);
		
		ui.txtMessages->append(QString("<span style=\"color: #cccccc\">[%1]</span> %2").arg(QTime::currentTime().toString("HH:mm:ss"), sanitized));
    }
    else
    {
        ui.txtMessages->append(QString("<span style=\"color: #cccccc\">[%1]</span> %2").arg(QTime::currentTime().toString("HH:mm:ss"), data));
    }
    ui.txtMessages->verticalScrollBar()->setValue(ui.txtMessages->verticalScrollBar()->maximum());
}

void ChatWidget::sendMessagePressed()
{
	QString message = ui.txtInput->text().trimmed();

	if (!message.isEmpty())
	{
		if (this->user_ != nullptr)
			emit sendMessage(this->user_, message);
		
		if (this->chatroom_ != nullptr)
			emit sendMessage(this->chatroom_, message);

		//writeLog(QString("%1: %2").arg( "Me" , message ));
	}

	ui.txtInput->clear();
}

void ChatWidget::chatRoomUserJoined(ChatRoom *room, User *user)
{
	if (room == this->chatroom_)
		addUser(user);
}
void ChatWidget::chatRoomUserLeft(ChatRoom *room, User *user)
{
	if (room == this->chatroom_)
		removeUser(user);
}


void ChatWidget::chatMessageSent(ChatRoom *room, const QString message)
{
	if (room == this->chatroom_)
		return;
	//printf("Your message \"%s\" in chatroom: %s was sent successfully\n", message.toStdString().c_str(), room->name().toStdString().c_str());
}

void ChatWidget::chatMessageReceieved(ChatRoom *room, User *user, const QString message)
{
	if (room == this->chatroom_)
		writeLog(QString("%1: %2").arg( user->username() , message ));
}

void ChatWidget::chatMessageReceieved(User *user, const QString message)
{
	if (this->user_ == user)
		writeLog(QString("%1: %2").arg( user->username() , message ));
}


void ChatWidget::chatMessageSent(User *user, const QString message)
{
	if (this->user_ == user)
		writeLog(QString("%1: %2").arg( user->username() , message ));
}



void ChatWidget::chatMessageFailed(ChatRoom *room, const QString message, ErosError code)
{
	if (this->chatroom_ == room)
		writeLog(QString("Chat Message \"%1\" failed.").arg(message));

}

void ChatWidget::chatMessageFailed(User *user, const QString message, ErosError code)
{
	if (this->user_ == user)
		writeLog(QString("Chat message \"%1\" failed.").arg(user->username()));
}
