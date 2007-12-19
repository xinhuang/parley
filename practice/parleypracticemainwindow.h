/***************************************************************************

    Copyright 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef PARLEYPRACTICEMAINWINDOW_H
#define PARLEYPRACTICEMAINWINDOW_H

#include <KXmlGuiWindow>

#include <QGraphicsView>

class ParleyPracticeMainWindow : public KXmlGuiWindow
{
Q_OBJECT
public:
    ParleyPracticeMainWindow(QWidget *parent = 0);

    ~ParleyPracticeMainWindow();

private:
    QGraphicsView* m_view;

};

#endif
