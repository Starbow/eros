#ifndef MATCHMAKINGPLAYERINFO_H
#define MATCHMAKINGPLAYERINFO_H

#include "ui_matchmakingplayerinfo.h"
#include <QWidget>
#include "../liberos/eros.h"

class MatchmakingPlayerInfo : public QWidget
{
	Q_OBJECT

public:
	MatchmakingPlayerInfo(const QString &username, const Division *division, const UserLadderStats *stats, bool flipped = false, QWidget *parent = 0);
	~MatchmakingPlayerInfo();

private:
	Ui::MatchmakingPlayerInfo ui;
};

#endif // MATCHMAKINGPLAYERINFO_H
