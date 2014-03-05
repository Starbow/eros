#include "erostitlebar.h"
#include <QLayout>
#include <QMainWindow>
#include <QWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

// Adapted from CustomWindow by http://www.qt-coding.com/
// https://github.com/francescmm/qt-coding

ErosTitleBar::ErosTitleBar(QWidget *parent)
	: QWidget(parent)
{
	move_ = false;
	allow_resize_ = false;
	resize_diagonal_topleft_ = false;
	resize_diagonal_topright_ = false;
	resize_horizontal_ = false;
	resize_vertical_ = false;
	this->window_ = parent;
	ui.setupUi(this);
	ui.tbMenu->setIcon(parent->windowIcon());

	ui.titleBar->setMouseTracking(true);
	ui.lblTitle->setMouseTracking(true);
	ui.tbMenu->setMouseTracking(true);
	ui.btnMax->setMouseTracking(true);
	ui.btnMin->setMouseTracking(true);
	ui.btnClose->setMouseTracking(true);
	parent->setMouseTracking(true);


	ui.lblTitle->setText(parent->windowTitle());

	QObject::connect(parent, SIGNAL(windowTitleChanged(const QString &)), this, SLOT(setTitle(const QString &)));
	QObject::connect(parent, SIGNAL(windowIconChanged(const QIcon &)), this, SLOT(setIcon(const QIcon &)));

	parent->setStyleSheet(parent->styleSheet() + "#centralWidget { border: 8px solid #560d0d; }");

}

void ErosTitleBar::setMenu(QMenu *menu)
{
	ui.tbMenu->setMenu(menu);
}


ErosTitleBar::~ErosTitleBar()
{

}


ErosTitleBar *ErosTitleBar::addToLayout(QWidget *parent, QBoxLayout *widget)
{
	parent->setWindowFlags(Qt::FramelessWindowHint
		| Qt::WindowSystemMenuHint
		| Qt::WindowMinimizeButtonHint
		| Qt::WindowMaximizeButtonHint
		| Qt::Window);
	parent->setAttribute(Qt::WA_DeleteOnClose);
	parent->setMouseTracking(true);

	ErosTitleBar *title = new ErosTitleBar(parent);
	widget->insertWidget(0, title);


	return title;
}

void ErosTitleBar::setTitle(const QString & title)
{
	ui.lblTitle->setText(title);
}

void ErosTitleBar::setIcon(const QIcon & icon)
{
	ui.tbMenu->setIcon(icon);
}

void ErosTitleBar::mouseMoveEvent(QMouseEvent *e)
{
	int xMouse = e->pos().x();
	int yMouse = e->pos().y();
	int wWidth = window_->geometry().width();
	int wHeight = window_->geometry().height();

	if (move_)
	{
		in_resize_zone_ = false;
		moveWindow(e);
	}
	else if (allow_resize_)
		resizeWindow(e);
	//Cursor part dreta
	else if (xMouse >= wWidth - PIXELS_TO_ACT || allow_resize_)
	{
		in_resize_zone_ = true;

		if (yMouse >= wHeight - PIXELS_TO_ACT)
			window_->setCursor(Qt::SizeFDiagCursor);
		else if (yMouse <= PIXELS_TO_ACT)
			window_->setCursor(Qt::SizeBDiagCursor);
		else
			window_->setCursor(Qt::SizeHorCursor);

		resizeWindow(e);
	}
	//Cursor part esquerra
	else if (xMouse <= PIXELS_TO_ACT || allow_resize_)
	{
		in_resize_zone_ = true;

		if (yMouse >= wHeight - PIXELS_TO_ACT)
			window_->setCursor(Qt::SizeBDiagCursor);
		else if (yMouse <= PIXELS_TO_ACT)
			window_->setCursor(Qt::SizeFDiagCursor);
		else
			window_->setCursor(Qt::SizeHorCursor);

		resizeWindow(e);
	}
	//Cursor part inferior
	else if ((yMouse >= wHeight - PIXELS_TO_ACT) || allow_resize_)
	{
		in_resize_zone_ = true;
		window_->setCursor(Qt::SizeVerCursor);

		resizeWindow(e);
	}
	//Cursor part superior
	else if (yMouse <= PIXELS_TO_ACT || allow_resize_)
	{
		in_resize_zone_ = true;
		window_->setCursor(Qt::SizeVerCursor);

		resizeWindow(e);
	}
	else
	{
		in_resize_zone_ = false;
		window_->setCursor(Qt::ArrowCursor);
	}

	e->accept();
}

void ErosTitleBar::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton)
	{
		if (in_resize_zone_)
		{
			allow_resize_ = true;

			if (e->pos().y() <= PIXELS_TO_ACT)
			{
				if (e->pos().x() <= PIXELS_TO_ACT)
					resize_diagonal_topleft_ = true;
				else if (e->pos().x() >= geometry().width() - PIXELS_TO_ACT)
					resize_diagonal_topright_ = true;
				else
					resize_vertical_ = true;
			}
			else if (e->pos().x() <= PIXELS_TO_ACT)
				resize_horizontal_ = true;
		}
		else if (e->pos().x() >= PIXELS_TO_ACT && e->pos().x() < geometry().width()
			&& e->pos().y() >= PIXELS_TO_ACT && e->pos().y() < geometry().height())
		{
			move_ = true;
			drag_position_ = e->globalPos() - window_->frameGeometry().topLeft();
		}
	}

	e->accept();
}

void ErosTitleBar::mouseReleaseEvent(QMouseEvent *e)
{
	move_ = false;
	allow_resize_ = false;
	resize_diagonal_topleft_ = false;
	resize_diagonal_topright_ = false;
	resize_horizontal_ = false;
	resize_vertical_ = false;

	e->accept();
}

void ErosTitleBar::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (e->pos().x() < ui.tbMenu->geometry().right() && e->pos().y() < ui.tbMenu->geometry().bottom()
		&& e->pos().x() >=  ui.tbMenu->geometry().x() && e->pos().y() >= ui.tbMenu->geometry().y()
		&& ui.tbMenu->isVisible())
		close();
	else if (e->pos().x() < geometry().width()
		&& e->pos().y() < geometry().height()
		&& title_mode_ != FullScreenMode)
		maximizeBtnClicked();
	e->accept();
}

void ErosTitleBar::paintEvent (QPaintEvent *)
{
	QStyleOption opt;
	opt.init (this);
	QPainter p(this);
	style()->drawPrimitive (QStyle::PE_Widget, &opt, &p, this);
}

void ErosTitleBar::moveWindow(QMouseEvent *e)
{
	if (e->buttons() & Qt::LeftButton)
	{
		window_->move(e->globalPos() - drag_position_);
		e->accept();
	}
}

void ErosTitleBar::resizeWindow(QMouseEvent *e)
{
	if (allow_resize_)
	{
		int xMouse = e->pos().x();
		int yMouse = e->pos().y();
		int wWidth = window_->geometry().width();
		int wHeight = window_->geometry().height();

		if (window_->cursor().shape() == Qt::SizeVerCursor)
		{
			if (resize_vertical_)
			{
				int newY = window_->geometry().y() + yMouse;
				int newHeight = wHeight - yMouse;

				if (newHeight > window_->minimumSizeHint().height())
				{
					window_->resize(wWidth, newHeight);
					window_->move(window_->geometry().x(), newY);
				}
			}
			else
				window_->resize(wWidth, yMouse+1);
		}
		else if (window_->cursor().shape() == Qt::SizeHorCursor)
		{
			if (resize_horizontal_)
			{
				int newX = window_->geometry().x() + xMouse;
				int newWidth = wWidth - xMouse;

				if (newWidth > window_->minimumSizeHint().width())
				{
					window_->resize(newWidth, wHeight);
					window_->move(newX, window_->geometry().y());
				}
			}
			else
				window_->resize(xMouse, wHeight);
		}
		else if (window_->cursor().shape() == Qt::SizeBDiagCursor)
		{
			int newX = 0;
			int newWidth = 0;
			int newY = 0;
			int newHeight = 0;

			if (resize_diagonal_topright_)
			{
				newX = window_->geometry().x();
				newWidth = xMouse;
				newY = window_->geometry().y() + yMouse;
				newHeight = wHeight - yMouse;
			}
			else
			{
				newX = window_->geometry().x() + xMouse;
				newWidth = wWidth - xMouse;
				newY = window_->geometry().y();
				newHeight = yMouse;
			}

			if (newWidth >= window_->minimumSizeHint().width() && newHeight >= window_->minimumSizeHint().height())
			{
				window_->resize(newWidth, newHeight);
				window_->move(newX, newY);
			}
			else if (newWidth >= window_->minimumSizeHint().width())
			{
				window_->resize(newWidth, wHeight);
				window_->move(newX, window_->geometry().y());
			}
			else if (newHeight >= window_->minimumSizeHint().height())
			{
				window_->resize(wWidth, newHeight);
				window_->move(window_->geometry().x(), newY);
			}
		}
		else if (window_->cursor().shape() == Qt::SizeFDiagCursor)
		{
			if (this->resize_diagonal_topleft_)
			{
				int newX = window_->geometry().x() + xMouse;
				int newWidth = wWidth - xMouse;
				int newY = window_->geometry().y() + yMouse;
				int newHeight = wHeight - yMouse;

				if (newWidth >= window_->minimumSizeHint().width() && newHeight >= window_->minimumSizeHint().height())
				{
					window_->resize(newWidth, newHeight);
					window_->move(newX, newY);
				}
				else if (newWidth >= window_->minimumSizeHint().width())
				{
					window_->resize(newWidth, wHeight);
					window_->move(newX, window_->geometry().y());
				}
				else if (newHeight >= window_->minimumSizeHint().height())
				{
					window_->resize(wWidth, newHeight);
					window_->move(window_->geometry().x(), newY);
				}
			}
			else
				window_->resize(xMouse+1, yMouse+1);
		}

		e->accept();
	}
}

void ErosTitleBar::maximizeBtnClicked()
{
	if (window_->isFullScreen() || window_->isMaximized())
	{
		ui.btnMax->setIcon(QIcon(":/img/client/ui/max"));
		window_->setWindowState(window_->windowState() & ~Qt::WindowFullScreen & ~Qt::WindowMaximized);
	}
	else
	{
		ui.btnMax->setIcon(QIcon(":/img/client/ui/restore"));
		window_->setWindowState(window_->windowState() | Qt::WindowFullScreen | Qt::WindowMaximized);
	}
}

void ErosTitleBar::minimizeBtnClicked()
{
	if (window_->isMinimized())
	{
		window_->setWindowState(window_->windowState() & ~Qt::WindowMinimized);
	}
	else
	{
		window_-> setWindowState(window_->windowState() | Qt::WindowMinimized);
	}
}

void ErosTitleBar::closeBtnClicked()
{
	window_->close();
}