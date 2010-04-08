#ifndef BANNERLAYOUT_H
#define BANNERLAYOUT_H

#include <QLayout>


/* A custom layout for Tads banner windows.
 */
class QTadsBannerLayout: public QLayout {
  public:
	enum Position { West, North, South, East, Center };

	QTadsBannerLayout( QWidget* parent, int margin = 0, int spacing = -1 );

	QTadsBannerLayout( int spacing = -1 )
	{ setSpacing(spacing); }

	~QTadsBannerLayout();

	virtual void
	addItem( QLayoutItem* item )
	{ add(item, West); }

	void
	addWidget( QWidget* widget, Position position )
	{ add(new QWidgetItem(widget), position); }

	virtual Qt::Orientations
	expandingDirections() const
	{ return Qt::Horizontal | Qt::Vertical; }

	virtual bool
	hasHeightForWidth() const
	{ return false; }

	virtual int
	count() const
	{ return list.size(); }

	virtual QLayoutItem*
	itemAt( int index ) const;

	virtual QSize
	minimumSize() const
	{ return calculateSize(MinimumSize); }

	virtual void
	setGeometry( const QRect& rect );

	virtual QSize
	sizeHint() const
	{ return calculateSize(SizeHint); }

	virtual QLayoutItem*
	takeAt( int index );

	void
	add( QLayoutItem* item, Position position )
	{ list.append(new ItemWrapper(item, position)); }

  private:
	struct ItemWrapper {
		ItemWrapper( QLayoutItem* i, Position p ) {
			item = i;
			position = p;
		}

		QLayoutItem* item;
		Position position;
	};

	enum SizeType { MinimumSize, SizeHint };

	QSize
	calculateSize( SizeType sizeType ) const;

	QList<ItemWrapper*> list;
};


#endif
