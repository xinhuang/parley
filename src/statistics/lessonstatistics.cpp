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

// Own
#include "lessonstatistics.h"

// Qt
#include <QApplication>
#include <QHeaderView>
#include <QItemDelegate>
#include <QPainter>

// KDE
#include <KLocalizedString>
#include <KMessageBox>
#include <QInputDialog>
#include <QAction>
#include <KActionCollection>

// Parley
#include "statisticslegendwidget.h"
#include "statisticsmodel.h"
#include "keduvoclesson.h"
#include "prefs.h"


class GradeDelegate: public QItemDelegate
{
public:
    GradeDelegate(QObject *parent = 0)
        : QItemDelegate(parent) {
    }

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
		       const QModelIndex &index) const
    {
        QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);

        // empty lesson
        if (!index.data(StatisticsModel::TotalCount).toInt()) {
            return;
        }
        drawBackground(painter, option, index);
        painter->drawText(option.rect, Qt::AlignCenter,
			  QString("%1%").arg(index.data(StatisticsModel::TotalPercent).toInt()));
    }

protected:
    void drawBackground(QPainter *painter, const QStyleOptionViewItem &option,
			const QModelIndex &index) const
    {
        QList<QVariant> fractions = index.data(StatisticsModel::LegendFractions).toList();
        StatisticsLegendWidget::paintStatisticsBar(*painter, option.rect, fractions);
    }
};


// ----------------------------------------------------------------


LessonStatisticsView::LessonStatisticsView(QWidget *parent)
    : ContainerView(parent)
{
    header()->setVisible(true);
    header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBottom);
    header()->setSectionsMovable( true );

    // inherits context menu policy - so action will show up in right click menu
    QAction *removeGradesAction = new QAction(this);
    removeGradesAction->setText(i18n("Remove confidence levels from this unit"));
    removeGradesAction->setIcon(QIcon::fromTheme("edit-clear"));
    removeGradesAction->setWhatsThis(i18n("Remove confidence levels from this unit"));
    removeGradesAction->setToolTip(removeGradesAction->whatsThis());
    removeGradesAction->setStatusTip(removeGradesAction->whatsThis());

    connect(removeGradesAction, SIGNAL(triggered()), SLOT(removeGrades()));
    addAction(removeGradesAction);

    QAction *removeGradesChildrenAction = new QAction(this);
    removeGradesChildrenAction->setText(i18n("Remove confidence levels from this unit and all sub-units"));
    removeGradesChildrenAction->setIcon(QIcon::fromTheme("edit-clear"));
    removeGradesChildrenAction->setWhatsThis(i18n("Remove confidence level from this unit and all sub-units"));
    removeGradesChildrenAction->setToolTip(removeGradesChildrenAction->whatsThis());
    removeGradesChildrenAction->setStatusTip(removeGradesChildrenAction->whatsThis());

    connect(removeGradesChildrenAction, SIGNAL(triggered()), SLOT(removeGradesChildren()));
    addAction(removeGradesChildrenAction);

    connect(header(), SIGNAL(geometriesChanged()),
	    this,     SLOT(adjustColumnWidths()));
    connect(header(), SIGNAL(sectionResized(int,int,int)),
	    this,     SLOT(sectionResized(int,int,int)));
}

void LessonStatisticsView::setModel(ContainerModel *model)
{
    ContainerView::setModel(model);

    GradeDelegate *delegate = new GradeDelegate(this);
    for (int i = ContainerModel::FirstDataColumn; i < model->columnCount(QModelIndex()); i++) {
        setItemDelegateForColumn(i, delegate);
    }

    adjustColumnWidths();
}

void LessonStatisticsView::resizeEvent(QResizeEvent *event)
{
    adjustColumnWidths();
    ContainerView::resizeEvent(event);
}

void LessonStatisticsView::sectionResized(int index,
					  int /*oldSize*/, int /*newSize*/)
{
    if (index < ContainerModel::FirstDataColumn) {
	adjustColumnWidths();
    }
}

void LessonStatisticsView::adjustColumnWidths()
{
    qDebug() << "adjusting column widths";
    int firstWidth = columnWidth(0) + columnWidth(1);
    // Subtract 5 here otherwise we get a horizontal scrollbar.
    int totalWidth = width() - firstWidth - 5;
    int columnCount = model()->columnCount(QModelIndex());
    int visibleColumns = 0;
    for (int i = ContainerModel::FirstDataColumn; i < columnCount; ++i) {
        if (!isColumnHidden(i))
            visibleColumns++;
    }
    int columnWidth = visibleColumns > 0 ? totalWidth / visibleColumns : 150;
    for (int i = ContainerModel::FirstDataColumn; i < model()->columnCount(QModelIndex()); i++) {
        setColumnWidth(i, columnWidth);
    }
//    header()->resizeSections(QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(QHeaderView::Interactive);
    header()->setStretchLastSection(true);
}

void LessonStatisticsView::removeGrades()
{
    QModelIndex selectedIndex = selectionModel()->currentIndex();
    KEduVocLesson *lesson = static_cast<KEduVocLesson*>(selectedIndex.internalPointer());
    lesson->resetGrades(-1, KEduVocContainer::NotRecursive);
}

void LessonStatisticsView::removeGradesChildren()
{
    QModelIndex selectedIndex = selectionModel()->currentIndex();
    KEduVocLesson *lesson = static_cast<KEduVocLesson*>(selectedIndex.internalPointer());
    lesson->resetGrades(-1, KEduVocContainer::Recursive);
}
