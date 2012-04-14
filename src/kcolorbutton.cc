/*  This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
    Copyright (C) 1999 Cristian Tibirna (ctibirna@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kcolorbutton.h"

#include <QPainter>
#include <qdrawutil.h>
#include <QApplication>
#include <QClipboard>
#include <QStyle>
#include <QMouseEvent>
#include <QStyleOptionButton>
#include <QColorDialog>
#include <QDrag>
#include <QMimeData>


KColorButton::KColorButton( QWidget *parent )
  : QPushButton( parent )
  , d( new KColorButtonPrivate(this) )
{
  d->m_bdefaultColor = false;
  d->m_defaultColor = QColor();
  setAcceptDrops( true);

  // 2000-10-15 (putzer): fixes broken keyboard usage
  connect (this, SIGNAL(clicked()), this, SLOT(_k_chooseColor()));
}

KColorButton::KColorButton( const QColor &c, QWidget *parent )
  : QPushButton( parent )
  , d( new KColorButtonPrivate(this) )
{
  d->col = c;
  d->m_bdefaultColor = false;
  d->m_defaultColor = QColor();
  setAcceptDrops( true);

  // 2000-10-15 (putzer): fixes broken keyboard usage
  connect (this, SIGNAL(clicked()), this, SLOT(_k_chooseColor()));
}

KColorButton::KColorButton( const QColor &c, const QColor &defaultColor, QWidget *parent )
  : QPushButton( parent )
  , d( new KColorButtonPrivate(this) )
{
  d->col = c;
  d->m_bdefaultColor = true;
  d->m_defaultColor = defaultColor;
  setAcceptDrops( true);

  // 2000-10-15 (putzer): fixes broken keyboard usage
  connect (this, SIGNAL(clicked()), this, SLOT(_k_chooseColor()));
}

KColorButton::~KColorButton()
{
  delete d;
}

QColor KColorButton::color() const
{
  return d->col;
}

void KColorButton::setColor( const QColor &c )
{
  if ( d->col != c ) {
    d->col = c;
    repaint();
    emit changed( d->col );
  }
}

QColor KColorButton::defaultColor() const
{
  return d->m_defaultColor;
}

void KColorButton::setDefaultColor( const QColor &c )
{
  d->m_bdefaultColor = c.isValid();
  d->m_defaultColor = c;
}

void KColorButton::KColorButtonPrivate::initStyleOption(QStyleOptionButton* opt) const
{
    opt->initFrom(q);
    opt->state |= q->isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
    opt->features = QStyleOptionButton::None;
    if (q->isDefault())
      opt->features |= QStyleOptionButton::DefaultButton;
    opt->text.clear();
    opt->icon = QIcon();
}

void KColorButton::paintEvent( QPaintEvent* )
{
  QPainter painter(this);

  //First, we need to draw the bevel.
  QStyleOptionButton butOpt;
  d->initStyleOption(&butOpt);
  style()->drawControl( QStyle::CE_PushButtonBevel, &butOpt, &painter, this );

  //OK, now we can muck around with drawing out pretty little color box
  //First, sort out where it goes
  QRect labelRect = style()->subElementRect( QStyle::SE_PushButtonContents,
      &butOpt, this );
  int shift = style()->pixelMetric( QStyle::PM_ButtonMargin ) / 2;
  labelRect.adjust(shift, shift, -shift, -shift);
  int x, y, w, h;
  labelRect.getRect(&x, &y, &w, &h);

  if (isChecked() || isDown()) {
    x += style()->pixelMetric( QStyle::PM_ButtonShiftHorizontal );
    y += style()->pixelMetric( QStyle::PM_ButtonShiftVertical   );
  }

  QColor fillCol = isEnabled() ? d->col : palette().color(backgroundRole());
  qDrawShadePanel( &painter, x, y, w, h, palette(), true, 1, NULL);
  if ( fillCol.isValid() )
    painter.fillRect( x+1, y+1, w-2, h-2, fillCol );

  if ( hasFocus() ) {
    QRect focusRect = style()->subElementRect( QStyle::SE_PushButtonFocusRect, &butOpt, this );
    QStyleOptionFocusRect focusOpt;
    focusOpt.init(this);
    focusOpt.rect            = focusRect;
    focusOpt.backgroundColor = palette().background().color();
    style()->drawPrimitive( QStyle::PE_FrameFocusRect, &focusOpt, &painter, this );
  }
}

QSize KColorButton::sizeHint() const
{
    QStyleOptionButton opt;
    d->initStyleOption(&opt);
    return style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(40, 15), this).
        expandedTo(QApplication::globalStrut());
}

QSize KColorButton::minimumSizeHint() const
{
    QStyleOptionButton opt;
    d->initStyleOption(&opt);
    return style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(3, 3), this).
        expandedTo(QApplication::globalStrut());
}

void KColorButton::dragEnterEvent( QDragEnterEvent *event)
{
  event->setAccepted(qvariant_cast<QColor>(event->mimeData()->colorData()).isValid() && isEnabled());
}

void KColorButton::dropEvent( QDropEvent *event)
{
  const QColor& c = qvariant_cast<QColor>(event->mimeData()->colorData());
  if (c.isValid()) {
    setColor(c);
  }
}

void KColorButton::keyPressEvent( QKeyEvent *e )
{
  if (e->matches(QKeySequence::Copy)) {
    QMimeData *mime=new QMimeData;
    mime->setColorData(color());
    QApplication::clipboard()->setMimeData( mime, QClipboard::Clipboard );
  }
  else if (e->matches(QKeySequence::Paste)) {
      const QColor& color =
              qvariant_cast<QColor>(QApplication::clipboard()->mimeData(QClipboard::Clipboard)->colorData());
    setColor( color );
  }
  else
    QPushButton::keyPressEvent( e );
}

void KColorButton::mousePressEvent( QMouseEvent *e)
{
  d->mPos = e->pos();
  QPushButton::mousePressEvent(e);
}

void KColorButton::mouseMoveEvent( QMouseEvent *e)
{
    QPushButton::mouseMoveEvent(e);
  if( (e->buttons() & Qt::LeftButton) &&
      (e->pos()-d->mPos).manhattanLength() > QApplication::startDragDistance() )
  {
    QDrag* drag = new QDrag(this);
    QMimeData* mime = new QMimeData;
    mime->setColorData(color());
    drag->setMimeData(mime);
    drag->exec(Qt::CopyAction);
    setDown(false);
  }
}

void KColorButton::KColorButtonPrivate::_k_chooseColor()
{
  QColor c = q->color();
  if ( m_bdefaultColor )
  {
      c = QColorDialog::getColor(m_defaultColor, q);
      if( c.isValid() ) {
          q->setColor( c );
      } else {
          q->setColor( m_defaultColor );
      }
  }
  else
  {
      c = QColorDialog::getColor(c, q);
      if(c.isValid()) {
          q->setColor( c );
      }
  }
}
