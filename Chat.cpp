#include "Chat.h"

Chat::Chat(QWidget *parent, Eros *client,  Ui::MainWindow *ui, const char *username)
{
	client_ = client;
	parent_ = parent;
	mainUi_ = ui;
	//cmbChatList_ = mainUi_->cmbChatList;

	//gui
	QObject::connect(mainUi_->btnRefreshChats, SIGNAL(pressed()), this, SLOT(clientRefreshRooms()));

	QObject::connect(mainUi_->btnJoinRoom, SIGNAL(pressed()), this, SLOT(btnJoin_click()));
	//QObject::connect(mainUi_->btnLeave, SIGNAL(pressed()), this, SLOT(btnLeave_click()));
	//QObject::connect(mainUi_->btnCreateRoom, SIGNAL(pressed()), this, SLOT(btnCreateRoom_click()));

	
	QObject::connect(mainUi_->tabContainer, SIGNAL(tabCloseRequested(int)), this, SLOT(clientTabCloseRequested(int)));

	//chat rooms
	QObject::connect(client_, SIGNAL(chatRoomJoined(ChatRoom*)), this, SLOT(clientChatRoomJoined(ChatRoom*)));
	QObject::connect(client_, SIGNAL(chatRoomLeft(ChatRoom*)), this, SLOT(clientChatRoomLeft(ChatRoom*)));

	QObject::connect(client_, SIGNAL(chatRoomAdded(ChatRoom*)), this, SLOT(clientChatRoomAdded(ChatRoom*)));

	

}

Chat::~Chat()
{
	//remove chatRoomMap_ stuff

}

void Chat::btnJoin_click()
{
	if(mainUi_->txtRoomName->text().isEmpty())
	{
		QList<QListWidgetItem *> chatRooms = mainUi_->chatList->selectedItems();
		foreach(QListWidgetItem *item, chatRooms)
		{
			QString currentRoom = item->text();

			if(!currentRoom.isEmpty())
			{
				joinRoom(nameToRoom(currentRoom));
			}
		}	
	}

	if(mainUi_->txtRoomPassword->text().isEmpty())
	{
		joinRoom(client_->getChatRoom(mainUi_->txtRoomName->text()));
	}
	else if(mainUi_->txtRoomPassword->text() == "Chat" || "Matchmaking" || "Settings")
	{
		printf("Channel name: %s restricted\n",mainUi_->txtRoomPassword->text());
	}
	else
	{
		joinProtectedRoom(client_->getChatRoom(mainUi_->txtRoomName->text()), mainUi_->txtRoomPassword->text());
	}
}

void Chat::btnLeave_click()
{
	//well apaprently there is no way to leave in the gui yet anyway
	//unless we actually just leave via the (x) for the tab is the only way?
	QList<QListWidgetItem *> chatRooms = mainUi_->chatList->selectedItems();
	foreach(QListWidgetItem *item, chatRooms)
	{
		QString currentRoom = item->text();

		if(!currentRoom.isEmpty())
		{
			leaveRoom(nameToRoom(currentRoom));
		}
		else
		{
			writeLog("No room selected");
		}
	}	
}

void Chat::btnCreateRoom_click()
{
	bool ok;
	QString roomName = QInputDialog::getText(parent_, tr("Room name"), tr("Enter a room name"), QLineEdit::Normal,tr("enter room name here"),&ok);	
	
	if(!roomName.isEmpty() && ok)
	{
		joinRoom(client_->getChatRoom(roomName));
	}
	else
	{
		QMessageBox::critical(parent_, tr("Invalid Room Name"), tr("Invalid Room Name, Room name must not be empty"), QMessageBox::Ok);
	}
}


ChatRoom* Chat::nameToRoom(QString name)
{
	foreach(ChatRoom *room, client_->chatRooms())
	{
		if(room->name() == name)
		{
			return room;
		}
	}
	return nullptr;
}

void Chat::sendRoomMessage(QString message, ChatRoom *room)
{
	if(room->joined())
	{
		client_->sendMessage(room, message);
	}
	else
	{
		writeLog("Not currently joined to the room");
	}
}

void Chat::sendPrivateMessage(QString message, User *user)
{
	client_->sendMessage(user, message);
}

void Chat::joinRoom(ChatRoom *room)
{
	client_->joinChatRoom(room);
}

void Chat::joinProtectedRoom(ChatRoom *room, QString password)
{
	//QString password = QInputDialog::getText(parent_, "Enter Password", "Enter chatroom password (Blank for no password)");
	client_->joinChatRoom(room, password);
	//joinedRooms_.push_back(room);
}

void Chat::leaveRoom(ChatRoom *room)
{
	client_->leaveChatRoom(room);
	//joinedRooms_.removeOne(room);
	//QString log = QString("??Left Room: %1").arg(room->name());
	//writeLog(log);
	printf("client left room\n");
}

void Chat::refreshRoomList()
{
	client_->refreshChatRooms();
}



void Chat::clientChatRoomJoined(ChatRoom *room)
{
	//writeLog(QString("User %1 joined room: %2").arg(user->username(), room->name()));
	ChatWidget *widget = new ChatWidget(client_, room, mainUi_->tabContainer);
	mainUi_->tabContainer->addTab(widget, room->name());
	chatRoomMap_.insert(room->name(), widget);
}
void Chat::clientChatRoomLeft(ChatRoom *room)
{
	mainUi_->tabContainer->removeTab(nameToTabIndex(room->name()));	
	chatRoomMap_.remove(room->name());
}

void Chat::writeLog(QString data) 
{	
	//this will have to do since this is basically deprecated anyway
	//mainUi_->statusBar->showMessage(data); 
	printf("Debug WriteLog: %s\n", data);
}

void Chat::clientRefreshRooms()
{
	client_->refreshChatRooms();
}

void Chat::clientChatRoomAdded(ChatRoom *room)
{
	mainUi_->chatList->addItem(room->name());
}

void Chat::testslot(QString message)
{
	//printf("no of chatrooms is =%i\n",client_->chatRooms().length());
	//this->sendRoomMessage(message, this->nameToRoom(currentChatBox_->getName()));
}

void Chat::currentChatRoomTabChanged()
{
	//currentChatBox_ = (QChatBox*) mainUi_->tabChatRooms->currentWidget();
}

void Chat::clientTabCloseRequested(int index)
{
	client_->leaveChatRoom(nameToRoom(mainUi_->tabContainer->tabText(index)));
}

int Chat::nameToTabIndex(QString name)
{
	//TODO: make sure we can detect if its a chat widget or not otherwise we run into problems with rooms: "Matchmaking" "Chat" "Settings"
	for(int i = 0; i < mainUi_->tabContainer->count(); ++i)
	{		
		if(mainUi_->tabContainer->tabText(i) == name)
		{
			return i;
		}
	}
}