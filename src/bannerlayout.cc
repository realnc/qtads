#include "bannerlayout.h"


QTadsBannerLayout::QTadsBannerLayout( QWidget* parent, int margin, int spacing )
  : QLayout(parent)
{
	setMargin(margin);
	setSpacing(spacing);
}


QTadsBannerLayout::~QTadsBannerLayout()
{
	QLayoutItem* l;
	while ((l = takeAt(0)))
		delete l;
}


QLayoutItem*
QTadsBannerLayout::itemAt( int index ) const
{
	ItemWrapper *wrapper = list.value(index);
	if (wrapper)
		return wrapper->item;
	else
		return 0;
}


void
QTadsBannerLayout::setGeometry( const QRect& rect )
{
	ItemWrapper* center = 0;
	int eastWidth = 0;
	int westWidth = 0;
	int northHeight = 0;
	int southHeight = 0;
	int centerHeight = 0;

	QLayout::setGeometry(rect);

	for (size_t i = 0; i < list.size(); ++i) {
		ItemWrapper* wrapper = list.at(i);
		QLayoutItem* item = wrapper->item;
		Position position = wrapper->position;

		if (position == North) {
			item->setGeometry(QRect(rect.x(), northHeight, rect.width(), item->sizeHint().height()));
			northHeight += item->geometry().height() + spacing();
		} else if (position == South) {
			item->setGeometry(QRect(item->geometry().x(), item->geometry().y(), rect.width(),
									item->sizeHint().height()));
			southHeight += item->geometry().height() + spacing();
			item->setGeometry(QRect(rect.x(), rect.y() + rect.height() - southHeight + spacing(),
									item->geometry().width(), item->geometry().height()));
		} else if (position == Center) {
			center = wrapper;
		}
	}

	centerHeight = rect.height() - northHeight - southHeight;

	for (size_t i = 0; i < list.size(); ++i) {
		ItemWrapper* wrapper = list.at(i);
		QLayoutItem* item = wrapper->item;
		Position position = wrapper->position;

		if (position == West) {
			item->setGeometry(QRect(rect.x() + westWidth, northHeight, item->sizeHint().width(), centerHeight));
			westWidth += item->geometry().width() + spacing();
		} else if (position == East) {
			item->setGeometry(QRect(item->geometry().x(), item->geometry().y(), item->sizeHint().width(),
									centerHeight));
			eastWidth += item->geometry().width() + spacing();
			item->setGeometry(QRect(rect.x() + rect.width() - eastWidth + spacing(), northHeight,
									item->geometry().width(), item->geometry().height()));
		}
	}

	if (center) {
		center->item->setGeometry(QRect(westWidth, northHeight, rect.width() - eastWidth - westWidth, centerHeight));
	}
}


QLayoutItem*
QTadsBannerLayout::takeAt( int index )
{
	if (index >= 0 and index < list.size()) {
		ItemWrapper* layoutStruct = list.takeAt(index);
		return layoutStruct->item;
	}
	return 0;
}


QSize
QTadsBannerLayout::calculateSize( SizeType sizeType ) const
{
	QSize totalSize;

	for (size_t i = 0; i < list.size(); ++i) {
		ItemWrapper* wrapper = list.at(i);
		Position position = wrapper->position;
		QSize itemSize;

		if (sizeType == MinimumSize) {
			itemSize = wrapper->item->minimumSize();
		} else { // (sizeType == SizeHint)
			itemSize = wrapper->item->sizeHint();
		}
		if (position == North || position == South || position == Center) {
			totalSize.rheight() += itemSize.height();
		}
		if (position == West || position == East || position == Center) {
			totalSize.rwidth() += itemSize.width();
		}
	}
	return totalSize;
}
