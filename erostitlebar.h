#ifndef EROSTITLEBAR_H
#define EROSTITLEBAR_H

#include "ui_erostitlebar.h"
#include <QWidget>
#include <QBoxLayout>

#define PIXELS_TO_ACT 4

class ErosTitleBar : public QWidget
{
	Q_OBJECT

public:
	enum TitleMode { CleanTitle = 0, OnlyCloseButton, MenuOff, MaxMinOff, FullScreenMode, MaximizeModeOff, MinimizeModeOff, FullTitle };


	ErosTitleBar(QWidget *parent);
	~ErosTitleBar();

	static ErosTitleBar *addToLayout(QWidget *parent, QBoxLayout* widget);
	void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

public slots:
	void setTitle(const QString & title);
	void setIcon(const QIcon & icon);
	void setMenu(QMenu *menu);
private slots:
	void moveWindow(QMouseEvent *e);
    void maximizeBtnClicked();
    void minimizeBtnClicked();
	void closeBtnClicked();

private:
	Ui::ErosTitleBar ui;
	QPoint drag_position_;
	TitleMode title_mode_;
	bool move_;
	bool in_resize_zone_;
	bool allow_resize_;
	bool resize_vertical_;
    bool resize_horizontal_;
    bool resize_diagonal_topleft_;
    bool resize_diagonal_topright_;

	void mouseDoubleClickEvent(QMouseEvent *e);
    void paintEvent (QPaintEvent *);
    void resizeWindow(QMouseEvent *e);

	QWidget *window_;

};

#endif // EROSTITLEBAR_H
