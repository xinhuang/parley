//
// C++ Implementation: multipletextualprompt
//
// Description:
//
//
// Copyright 2008 David Capel <wot.narg@gmail.com>
//
/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#include "multipletextualprompt.h"


#include "../activearea.h"
#include "../practiceentry.h"


#include <QObject>
#include <QStringList>
#include <QString>
#include <KSvgRenderer>
#include <QMap>
#include <QList>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <KRandom>

MultipleTextualPrompt::MultipleTextualPrompt(KSvgRenderer * renderer, ActiveArea * area, QStringList& elementIds, QObject * parent)
    : QObject(parent), m_renderer(renderer),m_elementIds(elementIds), m_area(area)
{
    foreach(const QString &id, elementIds)
    {
        QString tId = area->translateElementId(id);
        if (tId.isEmpty()) continue; // we just move on if it doesn't exist.

        QGraphicsTextItem * t = new QGraphicsTextItem();
        m_kids.append(t);
        m_map[id] = t;
        area->scene()->addItem(t);

        QRectF backgroundRect = renderer->boundsOnElement ( tId );
        t->setPos (area->offset(tId).x() + backgroundRect.x(), area->offset(tId).y() + backgroundRect.y() + backgroundRect.height() / 4.0 );
        t->adjustSize();
        t->setZValue(5);

        t->setTextWidth(backgroundRect.width()*.8);

    }
}

MultipleTextualPrompt::~MultipleTextualPrompt()
{
    qDeleteAll(m_kids);
}


void MultipleTextualPrompt::slotSetText(const QStringList& texts)
{
    if (!m_area->active()) return;

    if (m_elementIds.size() != texts.size())
    {
        kDebug() << "Size mismatch; we got " << texts.size() << "and need" << m_elementIds.size();
        return;
    }

    if (texts.size() == 0)
    {
        kDebug() << "Text list of size 0 received, aborting";
        return;
    }

    for(int i = 0; i < texts.size(); ++i)
    {
        m_map[m_elementIds[i]]->setHtml(texts[i]);
    }

}

void MultipleTextualPrompt::slotClear()
{
    if (!m_area->active()) return;

    foreach(QGraphicsTextItem* k, m_kids)
    {
        k->setHtml("");
    }
}