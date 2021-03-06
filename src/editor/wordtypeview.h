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

#ifndef WORDTYPEVIEW_H
#define WORDTYPEVIEW_H

#include "wordtypeview.h"
#include "containerview.h"

class QAction;
class KSelectAction;
class KEduVocWordType;

namespace Editor
{
class EditorWindow;

class WordTypeView : public ContainerView
{
    Q_OBJECT

public:
    WordTypeView(EditorWindow *parent);

public slots:
    /** Append a word type to the model and automatically set an edit up so the user can change "New lesson" into something meaningfull.*/
    void slotCreateNewWordType();

    /** Remove a word type. */
    void slotDeleteWordType();

    void setTranslation(KEduVocExpression* entry, int translationId);

signals:
    void selectedWordTypeChanged(KEduVocWordType* wordType);

protected:
    void contextMenuEvent(QContextMenuEvent * event);

private slots:
    void setWordTypeNone();
    void setWordTypeNoun();
    void setWordTypeNounMale();
    void setWordTypeNounFemale();
    void setWordTypeNounNeutral();
    void setWordTypeAdjective();
    void setWordTypeAdverb();
    void setWordTypeVerb();
    void setWordTypeConjunction();

private:
    QAction *m_actionNewWordType;
    QAction *m_actionDeleteWordType;
    QAction *m_actionRenameWordType;
    KSelectAction *m_actionSpecialTypeMenu;

    QAction *m_noneAction;
    QAction *m_separator;
    QAction *m_nounAction;
    QAction *m_nounMaleAction;
    QAction *m_nounFemaleAction;
    QAction *m_nounNeutralAction;
    QAction *m_adjectiveAction;
    QAction *m_adverbAction;
    QAction *m_verbAction;
    QAction *m_conjunctionAction;
};
}

#endif
