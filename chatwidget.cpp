#include "chatwidget.h"

#include <QScrollBar>
#include <QTime>
#include <QDesktopServices>
#include <QClipboard>
#include <QMessageBox>
#include "util.h"

ChatWidget::ChatWidget(Eros *eros, User *user, const QString &initial_message, QWidget *parent)
	:QWidget(parent)
{
	this->eros_ = eros;
	this->events_ = 0;
	ui.setupUi(this);
	setUser(user);

	// We don't need the user list
	ui.listUsers->hide();

	QObject::connect(eros, SIGNAL(chatMessageSent(User*, const QString)), this, SLOT( chatMessageSent(User*, const QString)));
	QObject::connect(eros, SIGNAL(chatMessageReceieved(User*, const QString)), this, SLOT( chatMessageReceieved(User*, const QString)));
	QObject::connect(eros, SIGNAL(chatMessageFailed(User*, const QString, ErosError)), this, SLOT( chatMessageFailed(User*, const QString, ErosError)));

	QObject::connect(eros, SIGNAL(disconnected()), this, SLOT(disconnected()));
	QObject::connect(eros, SIGNAL(connected()), this, SLOT(connected()));

	QObject::connect(this, SIGNAL(sendMessage(User *, const QString)), eros, SLOT(sendMessage(User *, const QString)));

	
	QObject::connect(ui.btnSend, SIGNAL(pressed()), this, SLOT(sendMessagePressed()));
	QObject::connect(ui.txtInput, SIGNAL(returnPressed()), this, SLOT(sendMessagePressed()));

	if (!initial_message.isEmpty())
		chatMessageReceieved(user, initial_message);
	
}
ChatWidget::ChatWidget(Eros *eros, ChatRoom *chatroom, QWidget *parent)
	:QWidget(parent)
{
	this->events_ = 0;
	this->eros_ = eros;
	ui.setupUi(this);
	setChatroom(chatroom);

	QObject::connect(eros, SIGNAL(chatMessageSent(ChatRoom*, const QString)), this, SLOT( chatMessageSent(ChatRoom*, const QString)));
	QObject::connect(eros, SIGNAL(chatMessageReceieved(ChatRoom*, User*, const QString)), this, SLOT( chatMessageReceieved(ChatRoom*, User*, const QString)));
	QObject::connect(eros, SIGNAL(chatMessageFailed(ChatRoom*, const QString, ErosError)), this, SLOT( chatMessageFailed(ChatRoom*, const QString, ErosError)));

	QObject::connect(eros, SIGNAL(chatRoomUserJoined(ChatRoom*, User*)), this, SLOT(chatRoomUserJoined(ChatRoom*, User*)));
	QObject::connect(eros, SIGNAL(chatRoomUserLeft(ChatRoom*, User*)), this, SLOT(chatRoomUserLeft(ChatRoom*, User*)));

	QObject::connect(eros, SIGNAL(disconnected()), this, SLOT(disconnected()));
	QObject::connect(eros, SIGNAL(connected()), this, SLOT(connected()));

	QObject::connect(this, SIGNAL(sendMessage(ChatRoom *, const QString)), eros, SLOT(sendMessage(ChatRoom *, const QString)));
	QObject::connect(this, SIGNAL(sendMessage(User *, const QString)), eros, SLOT(sendMessage(User *, const QString)));

	QObject::connect(ui.btnSend, SIGNAL(pressed()), this, SLOT(sendMessagePressed()));
	QObject::connect(ui.txtInput, SIGNAL(returnPressed()), this, SLOT(sendMessagePressed()));
	QObject::connect(ui.listUsers, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(userDoubleClicked(QListWidgetItem*)));
	
	populateUserList();
}

ChatWidget::~ChatWidget()
{

}

void ChatWidget::populateUserList()
{
	const QList<User *> &users = this->chatroom_->participants();
	for (int i = 0; i < users.size(); i++)
	{
		User *user = users.at(i);
		const QPair <int, QString> &division = this->eros_->divisions()->division(user->ladderStatsGlobal()->points());
		QListWidgetItem *item = new QListWidgetItem(QIcon(Util::getIcon(user->username(), division.second, true)), user->username());
		this->ui.listUsers->addItem(item);
	}
	this->ui.listUsers->sortItems();
}

void ChatWidget::userDoubleClicked(QListWidgetItem *item)
{
	if (item != nullptr)
	{
		User *user = eros_->getUser(item->text());
		emit privateChatRequested(user);
	}
}

int ChatWidget::eventCount() const
{
	return this->events_;
}

void ChatWidget::resetEventCount()
{
	this->events_ = 0;
	emit eventCountUpdated(this);
}

void ChatWidget::addEvent()
{
	this->events_ += 1;
	emit eventCountUpdated(this);
}


const QString &ChatWidget::name() const {
	if (this->chatroom_ != nullptr)
	{
		return this->chatroom_->name();
	}
	else if (this->user_ != nullptr)
	{
		return this->user_->username();
	}
}

void ChatWidget::connected()
{
	if (this->chatroom_ != nullptr)
	{
		ui.listUsers->clear();
		populateUserList();
		writeLog(tr("You have reconnected to the server. You may need to re-join the channel."));
	}
	else
	{
		writeLog(tr("You have reconnected to the server."));
	}
	ui.btnSend->setEnabled(true);
	ui.txtInput->setEnabled(true);
}


void ChatWidget::disconnected()
{
	writeLog(tr("You have been disconnected from the server."));
	ui.btnSend->setDisabled(true);
	ui.txtInput->setDisabled(true);
}


QString ChatWidget::getColour(User *user)
{
	switch (user->username().length())
	{
	case 1:
	case 6:
	case 11:
		return "#FF0000"; // Red
		break;
	case 2:
	case 7:
	case 12:
		return "#4F64FF"; // Blue
		break;
	case 3:
	case 8:
	case 13:
		return "#23A136"; // Green
		break;
	case 4:
	case 9:
	case 14:
		return "#A75EAD"; // Purple
		break;
	default:
		return "#CC8E4B"; // Brown
		break;
	}
}

QString ChatWidget::getUsername(User *user)
{
	const UserLadderStats *stats = user->ladderStatsGlobal();
	if (stats == nullptr)
	{
		return QString("<strong>%2</strong>").arg(Util::sanitizeHtml(user->username()));
	}
	else
	{
		const QPair <int, QString> &division = this->eros_->divisions()->division(stats->points());
		// Add special icons.
		return QString("<a title=\"%1\"><strong>%2</strong></a>").arg(division.second, Util::sanitizeHtml(user->username()));
	}
}

QString ChatWidget::colourise(const QString &data, User *user, bool sending)
{
	QString colour = "";
	if (this->chatroom_ != nullptr)
	{
		if (user == eros_->localUser())
			colour = "#000000";
		else
			colour = getColour(user);
	}
	else
	{
		if (sending)
			colour = "#000000";
		else
			colour = getColour(user);
	}

	return QString("<span style=\"color: %1\">%2</span>").arg(colour, data);
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
	{
		QString username = getUsername(user);
		writeLog(colourise(QString(tr("%1 has joined the chat.")).arg(username), user));
	}
	const QPair <int, QString> &division = this->eros_->divisions()->division(user->ladderStatsGlobal()->points());
	QListWidgetItem *item = new QListWidgetItem(QIcon(Util::getIcon(user->username(), division.second, true)), user->username());
	this->ui.listUsers->addItem(item);
	this->ui.listUsers->sortItems();
}

void ChatWidget::removeUser(User *user)
{
	if (!this->chatroom_->forced())
	{
		QString username = getUsername(user);
		writeLog(colourise(QString(tr("%1 has left the chat.")).arg(username), user));
	}
	qDeleteAll(this->ui.listUsers->findItems(user->username(), Qt::MatchFlag::MatchExactly));
}



void ChatWidget::writeLog(const QString &data)
{	
	//When appending text in Qt, the new text will take on the same format as the current selection.
	//Remove the selection cursor and preserve the previous selection for later.
	QTextCursor cursor = ui.txtMessages->textCursor();
	QScrollBar *bar = ui.txtMessages->verticalScrollBar();
	bool shouldScroll = (bar->value() == bar->maximum());
	int barValue = bar->value();
	ui.txtMessages->setTextCursor(QTextCursor());

	//Remove formatting for the append.
	QTextCharFormat format;
	ui.txtMessages->setCurrentCharFormat(format);


    ui.txtMessages->append(QString("<span style=\"color: #cccccc\">[%1]</span> %2").arg(QTime::currentTime().toString("HH:mm:ss"), data));
	//Apply the previous selection cursor and move the scrollbar to the bottom.

	ui.txtMessages->setTextCursor(cursor);
	if (shouldScroll)
	{
		bar->setValue(bar->maximum());
	}
	else
	{
		bar->setValue(barValue);
	}
	addEvent();
}

void ChatWidget::sendMessagePressed()
{
	QString message = ui.txtInput->text().trimmed();

	if (!message.isEmpty())
	{

		if (message[0] == '/')
		{
			QStringList commands = message.split(" ");
			QString command = commands[0].toLower();
			command.remove(0, 1);
			
			if (command == "help")
			{
				writeLog(tr("/w <name> <message> - Send a private message."));
				writeLog(tr("/msg <name> <message> - Send a private message."));
			}
			else if (command == "w" || command == "msg")
			{
				if (commands.length() < 3)
				{
					writeLog(tr("Cannot perform command. Not enough arguments."));
					return;
				}

				QString username = commands[1];
				commands.removeFirst();
				commands.removeFirst();

				message = commands.join(" ").trimmed();
				if (message == "")
				{
					writeLog(tr("Cannot perform command. Not enough arguments."));
					return;
				}
				else
				{
					User *user = eros_->getUser(username);
					emit sendMessage(user, message);
					if (this->user_ != user)
					{
						writeLog(colourise(QString("-> %1: %2").arg(getUsername(user), Util::sanitizeHtml(message)), this->eros_->localUser(), true));
					}
				}
			}		
		}
		else
		{
			if (this->user_ != nullptr)
				emit sendMessage(this->user_, message);
		
			if (this->chatroom_ != nullptr)
				emit sendMessage(this->chatroom_, message);
		}
		
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
	//We don't need to process this. Our message will be echoed back to us through chatMessageReceived.
}

void ChatWidget::chatMessageReceieved(ChatRoom *room, User *user, const QString message)
{
	if (room == this->chatroom_)
	{
		writeLog(colourise(QString("%1: %2").arg(getUsername(user), Util::sanitizeHtml(message)), user));
	}
}

void ChatWidget::chatMessageReceieved(User *user, const QString message)
{
	if (this->user_ == user)
	{
		writeLog(colourise(QString("%1: %2").arg(getUsername(user), Util::sanitizeHtml(message)), user));
	}
}


void ChatWidget::chatMessageSent(User *user, const QString message)
{
	if (this->user_ == user)
	{
		writeLog(colourise(QString("%1: %2").arg(getUsername(this->eros_->localUser()), Util::sanitizeHtml(message)), user, true));
	}
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

void ChatWidget::anchorClicked(QUrl url)
{

	if (url.scheme() == "clipboard")
	{
		QApplication::clipboard()->setText(url.path());
#if defined(Q_OS_MAC)
		QString button = tr("Command+V", "Mac OS X paste command");
#else
		QString button = tr("Ctrl+V", "Windows and Linux paste command");
#endif

		QString text = QString("<strong>%1</strong>").arg(url.path());
		writeLog(tr("The text \"%1\" has been copied to your clipboard. Select the join channel textbox in StarCraft II and press %2 to paste the channel name.").arg(text, button));
	}
	else
	{
		QDesktopServices::openUrl(url);
	}
}