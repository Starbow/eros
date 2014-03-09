#include "matchmakingplayerinfo.h"
#include "util.h"
#include <QLayoutItem>
MatchmakingPlayerInfo::MatchmakingPlayerInfo(const QString &username, const QString &division, const UserLadderStats *stats, bool flipped, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);


	ui.lblPoints->setText(tr("%1 points").arg(QLocale().toString(stats->points())));
	ui.lblDivision->setText(division);
	ui.lblLocalPlayerLeagueImage->setPixmap(QPixmap(Util::getIcon(username, division)));
	ui.lblPlayerName->setText(username);

	if (flipped) 
	{
		QLayoutItem* item = ui.gridLayout->takeAt(0);
		ui.gridLayout->addWidget(item->widget(), 3, 0, Qt::AlignHCenter|Qt::AlignTop);
	}
}

MatchmakingPlayerInfo::~MatchmakingPlayerInfo()
{

}
