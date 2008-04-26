/***************************************************************************

    Copyright 2008 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LESSONMODEL_H
#define LESSONMODEL_H

#include "containermodel.h"

/**
  * Model for the tree of lessons.
  */
class LessonModel : public ContainerModel
{
    Q_OBJECT

public:
    /** When splitting a lesson into smaller ones - how to sort the entries into lessons.*/
    enum SplitLessonOrder {
        Sorted,    /**< The order of the entries in the document */
        Random /**< Randomized */
    };


    explicit LessonModel(QObject *parent = 0);

    /**
     * Divide a lesson into smaller ones.
     * Tip: If you create a lesson that is >= the original one and use random order, you get your lesson reshuffled. Maybe that is sometimes useful. For now the lessons are appended at the end.
     * @param lessonIndex lesson to split
     * @param entriesPerLesson number of entries in each new lesson
     * @param order one of SplitLessonOrder
        */
    void splitLesson(const QModelIndex& containerIndex, int entriesPerLesson, SplitLessonOrder order);
};


#endif