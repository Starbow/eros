#include "matchmakingplayerinfo.h"
#include "util.h"
#include <QLayoutItem>
MatchmakingPlayerInfo::MatchmakingPlayerInfo(const QString &username, const Division *division, const UserLadderStats *stats, bool flipped, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);


	
	if (stats->placements() > 0) {
		ui.lblDivision->setText(tr("%n placement(s) remaining", "", stats->placements()));
		ui.lblPoints->setText(tr("%n point(s)", "", stats->points()));
	} else {
		ui.lblDivision->setText("");
		ui.lblPoints->setText(QString("%1\n%2").arg(tr("%n point(s)", "", stats->points()), tr("%n MMR", "", stats->rating())));
	}
	
	ui.lblLocalPlayerLeagueImage->setText(division->name());
	ui.lblLocalPlayerLeagueImage->setPixmap(QPixmap(Util::getIcon(division->id())));
	
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
