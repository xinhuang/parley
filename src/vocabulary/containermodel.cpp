/***************************************************************************

    Copyright 2007-2008 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "containermodel.h"

#include "containermimedata.h"
#include "vocabularymimedata.h"

#include <keduvocdocument.h>
#include <keduvoclesson.h>
#include <keduvocwordtype.h>
#include <keduvocexpression.h>

#include <KRandom>
#include <KIcon>
#include <kdebug.h>
#include <klocale.h>
#include <QItemSelection>

/** @file
  * Implementation of ContainerModel.
  * Functions to create the model from the lessons of the vocabulary document.
  */

ContainerModel::ContainerModel(KEduVocLesson::EnumContainerType type, QObject * parent) : QAbstractItemModel(parent)
{
    m_type = type;
    m_doc = 0;

    setSupportedDragActions(Qt::CopyAction | Qt::MoveAction);
}

void ContainerModel::setDocument(KEduVocDocument * doc)
{
    // cleanup old document
    if (rowCount(QModelIndex()) > 0) {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_doc = 0;
        endRemoveRows();
    }

    if (doc) {
        beginInsertRows(QModelIndex(), 0, 0);
        m_doc = doc;
        endInsertRows();
    }
}


QModelIndex ContainerModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!m_doc || !hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    KEduVocContainer *parentLesson;

    if (!parent.isValid()) {
        parentLesson = 0;
    } else {
        parentLesson = static_cast<KEduVocContainer*>(parent.internalPointer());
    }

    KEduVocContainer *childLesson;
    if (!parentLesson) {
        childLesson = rootContainer();
    } else {
        childLesson = parentLesson->childContainer(row);
    }
    return createIndex(row, column, childLesson);
}


QModelIndex ContainerModel::index(KEduVocContainer * container) const
{
    if(!container) {
        return QModelIndex();
    }

    QModelIndex currentIndex = index(container->row(), 0, index(container->parent()));
    Q_ASSERT(container == currentIndex.internalPointer());

    return currentIndex;
}


QModelIndex ContainerModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    KEduVocContainer *childItem = static_cast<KEduVocContainer*>(index.internalPointer());
    if (!childItem) {
        return QModelIndex();
    }

    KEduVocContainer *parentItem = childItem->parent();

    if (!parentItem) {
        return QModelIndex();
    }

    QModelIndex parentIndex = createIndex(parentItem->row(), 0, parentItem);
    return parentIndex;
}


int ContainerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    KEduVocContainer *parentItem;
    if (!parent.isValid()) {
        // root element
        if (!m_doc) {
            return 0;
        }
        return 1;
    } else {
        parentItem =  static_cast<KEduVocContainer*>(parent.internalPointer());
        return parentItem->childContainerCount();
    }
}


QModelIndex ContainerModel::appendContainer(const QModelIndex& parent, const QString & containerName)
{
    KEduVocContainer* parentContainer;
    if (parent.isValid()) {
        parentContainer = static_cast<KEduVocContainer*>(parent.internalPointer());
    } else {
        return QModelIndex();
    }

    beginInsertRows(parent, parentContainer->childContainerCount(),
                    parentContainer->childContainerCount() );
    switch (m_type){
    case (KEduVocContainer::Lesson):
        parentContainer->appendChildContainer(new KEduVocLesson(containerName, static_cast<KEduVocLesson*>(parentContainer)));
        break;
    case (KEduVocContainer::WordType):
        parentContainer->appendChildContainer(new KEduVocWordType(containerName, static_cast<KEduVocWordType*>(parentContainer)));
    break;
    }
    endInsertRows();

    return index(parentContainer->childContainerCount() - 1, 0, parent);
}


QVariant ContainerModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid()) {
         return QVariant();
    }

    KEduVocContainer *container = static_cast<KEduVocContainer*>(index.internalPointer());

    switch (index.column()){
    case 0: // Lesson name
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return container->name();
        }
        // checkboxes
        if (role == Qt::CheckStateRole) {
            if (container->inPractice())
                return Qt::Checked;
            else
                return Qt::Unchecked;
        }
//         if (role == Qt::DecorationRole) {
//             return KIcon("favorites");
//         }
        if (role == Qt::TextAlignmentRole) {
            return Qt::AlignLeft;
        }
        break;
    case 1: // Total count
        if (role == Qt::DisplayRole) {
            return container->entryCount(KEduVocLesson::Recursive);
        }
        if (role == Qt::TextAlignmentRole) {
            return Qt::AlignRight;
        }
        break;
    default: // Average grade
        if (role == Qt::DisplayRole) {
            return container->averageGrade(index.column()-2);
        }
        if (role == Qt::TextAlignmentRole) {
            return Qt::AlignRight;
        }
    }

    return QVariant();
}


bool ContainerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    if ( index.column() == 0 ) {
        KEduVocContainer *container = static_cast<KEduVocContainer*>(index.internalPointer());
        // rename a lesson
        if (role == Qt::EditRole) {
            container->setName(value.toString());
            emit dataChanged(index, index);
            emit documentModified();
            return true;
        }

        // checkboxes
        if (role == Qt::CheckStateRole) {
            bool newState = value.toBool();
            for (int i = 0; i < rowCount(index); i++) {
                setData(index.child(i, 0), newState, Qt::CheckStateRole);
            }
            container->setInPractice(newState);
            emit dataChanged(index, index);
            emit documentModified();
            return true;
        }
    }
    return false;
}


Qt::ItemFlags ContainerModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        // the root element, not editable for now
        if (index.parent() == QModelIndex()) {
            return (Qt::ItemIsEnabled | Qt::ItemIsSelectable
                        | Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled );
        }
        // the name column
        if ( index.column() == 0 ) {
            return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable
                    | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled );
        } else { // every other element
            return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled |  Qt::ItemIsDropEnabled );
        }
    }
    return  Qt::ItemIsDropEnabled;
}


QVariant ContainerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // statically two columns for now
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            if(role == Qt::DisplayRole) {
                return i18n("Lesson");
            }
            break;
        case 1:
            if(role == Qt::DisplayRole) {
                return QVariant();
            }
            if(role == Qt::ToolTipRole) {
                return i18n("Number of entries in this lesson.");
            }
            break;
        default:
            if(role == Qt::DisplayRole) {
                return i18nc("Grade in language, table header", "Grade (%1)", m_doc->identifier(section-2).name());
            }
        }
    }
    return QVariant();
}


int ContainerModel::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    if (!m_doc) {
        return 2;
    }

    // for now one grade per language
    return 2; // + m_doc->identifierCount();
}


void ContainerModel::deleteContainer(const QModelIndex & containerIndex)
{
    KEduVocContainer* container = static_cast<KEduVocContainer*>(containerIndex.internalPointer());
    KEduVocContainer* parent = container->parent();

    if (!parent) {
        // never delete the root container
        return;
    }

    beginRemoveRows(containerIndex.parent(), containerIndex.row(), containerIndex.row());
    parent->deleteChildContainer(container->row());
    endRemoveRows();
}


KEduVocContainer::EnumContainerType ContainerModel::containerType()
{
    return m_type;
}

Qt::DropActions ContainerModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

QStringList ContainerModel::mimeTypes() const
{
    return QStringList() << "text/plain";
}


QMimeData * ContainerModel::mimeData(const QModelIndexList &indexes) const
{
     ContainerMimeData *mimeData = new ContainerMimeData();
//      QByteArray encodedData;

    foreach (const QModelIndex &index, indexes) {
        mimeData->addContainer(static_cast<KEduVocContainer*>(index.internalPointer()));
    }
    mimeData->setText("Parley lesson");

//      QDataStream stream(&encodedData, QIODevice::WriteOnly);
// stream << "Parley lesson";
//      foreach (const QModelIndex &index, indexes) {
//          if (index.isValid()) {
//              QString text = data(index, Qt::DisplayRole).toString();
//              stream << text;
//          }
//      }
// // kDebug() << "mimeData:" << encodedData;
//      mimeData->setData("text/plain", encodedData);
    return mimeData;
}


bool ContainerModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
    Q_UNUSED(column)

    if (action == Qt::IgnoreAction) {
         return true;
    }

    // if it's internal, get the pointers
    const ContainerMimeData * containerData =
             qobject_cast<const ContainerMimeData *>(data);

    if (containerData) {
        foreach (KEduVocContainer* container, containerData->containerList()) {
            // no way to move a word type to a lesson for now
            if (container->containerType() != m_type) {
                return false;
            }

            if (action == Qt::MoveAction || action == Qt::CopyAction) {
                kDebug() << "Move container: " << container->name();
                KEduVocContainer* parentContainer = 0;

                if (parent.isValid()) {
                    parentContainer = static_cast<KEduVocContainer*>(parent.internalPointer());
                }
                if (!parentContainer) {
                    // drop into root
                    parentContainer = rootContainer();
                } else {
                    //make sure a container cannot be dropped into one of its child containers!
                    KEduVocContainer* childTest = parentContainer;
                    while (childTest != 0) {
                        if (childTest == container) {
                            kDebug() << "Cannot drop a container into one of its child containers!";
                            return false;
                        }
                        childTest = childTest->parent();
                    }
                }

                QModelIndex oldParent = index(container->parent());



                beginRemoveRows(oldParent, container->row(), container->row());
                container->parent()->removeChildContainer(container->row());
                endRemoveRows();

                // if we get to choose, append seems sane.
                if (row < 0) {
                    row = parentContainer->childContainerCount();
                }

                // use index because we sometimes reparent to the root container instead of dropping into nowhere
                beginInsertRows(index(parentContainer), row, row);
                parentContainer->insertChildContainer(row, container);
                endInsertRows();

                return true;
            }
        }
    }


    // if it's a translation, get the pointers
    const VocabularyMimeData * translationData =
             qobject_cast<const VocabularyMimeData *>(data);

    if (translationData) {
        if(!parent.isValid()) {
            return false;
        }
        if (containerType() == KEduVocContainer::Lesson) {
            // Create a list of the entries associated with the translations being copied. This prevents duplicates if they highlighted several columns.
            QList<KEduVocExpression*> entries;

            foreach (KEduVocTranslation* translation, translationData->translationList()) {
                if (!entries.contains(translation->entry())) {
                    entries << translation->entry();
                }
            }

            foreach (KEduVocExpression* entry, entries) {
                static_cast<KEduVocLesson*>(parent.internalPointer())->appendEntry(new KEduVocExpression(*entry));
            }
        }

        if (containerType() == KEduVocContainer::WordType) {
            foreach (KEduVocTranslation* translation, translationData->translationList()) {
                translation->setWordType(
                    static_cast<KEduVocWordType*>(parent.internalPointer()));
            }
        }
        return false;
    }


    kDebug() << data->formats();
/*
    if (data->hasText()) {
        if (action == Qt::CopyAction | action == Qt::MoveAction) {
            QString name;
            name = data->text();
            kDebug() << "Copy lesson " << name;

            appendLesson(parent, name);
            return true;
        }
    }*/

    return false;
}

/*
bool ContainerModel::removeRows(int row, int count, const QModelIndex & parent)
{
// BIG FAT WARNING this code works, but it gets called by the drag and drop implementation automatically, so either this gets used or the other (dropMimeData) to remove the containers. If both are active, containers get deleted without warning or visible signs.

    KEduVocContainer* parentContainer;
    if (!parent.internalPointer()) {
        parentContainer = m_container;
    } else {
        parentContainer = static_cast<KEduVocContainer*>(parent.internalPointer());
    }
    kDebug() << "removeRows from " << parentContainer->name() << " row " << row << "count" << count;

    beginRemoveRows ( parent, row, row+count );
    for (int i = 0; i<count; i++) {
        parentContainer->removeChildContainer(row);
    }
    endRemoveRows();

    return true;
}

*/

KEduVocContainer * ContainerModel::rootContainer() const
{
    if (!m_doc) {
        return 0;
    }

    switch (m_type) {
        case KEduVocContainer::Lesson:
            return m_doc->lesson();
        case KEduVocContainer::WordType:
            return m_doc->wordTypeContainer();
    }

    return 0;
}


#include "containermodel.moc"

