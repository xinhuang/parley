/***************************************************************************
    Copyright 2010 Frederik Gladhorn <gladhorn@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef COMPARISONBACKENDMODE_H
#define COMPARISONBACKENDMODE_H

#include "abstractbackendmode.h"
#include "practice/sessionmanagerbase.h"

namespace Practice
{

class ComparisonBackendMode : public AbstractBackendMode
{
    Q_OBJECT

public:
    ComparisonBackendMode(AbstractFrontend *frontend,
                          QObject *parent, Practice::SessionManagerBase* sessionManager, KEduVocDocument* doc);

    virtual bool setTestEntry(TestEntry* current);
    virtual grade_t currentPreGradeForEntry() const;
    virtual grade_t currentGradeForEntry() const;

    void checkAnswer();

public Q_SLOTS:
    virtual void hintAction();

protected:
    virtual void updateGrades();

private:

    SessionManagerBase* m_sessionManager;
    KEduVocDocument* m_doc;
    QStringList m_lastAnswers;
};

}

#endif
