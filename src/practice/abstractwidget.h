/***************************************************************************
    Copyright 2009 Daniel Laidig <d.laidig@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PRACTICE_ABSTRACTWIDGET_H
#define PRACTICE_ABSTRACTWIDGET_H

#include "abstractfrontend.h"

#include <QtGui/QWidget>

class QVariant;

namespace Practice
{
class GuiFrontend;

class AbstractModeWidget : public QWidget
{
    Q_OBJECT

public:
    AbstractModeWidget(GuiFrontend *frontend, QWidget* parent = 0);
    virtual ~AbstractModeWidget() {}

    virtual void setQuestion(const QVariant& question) = 0;
    virtual void setSolution(const QVariant& solution) = 0;
    virtual QVariant userInput() = 0;
    virtual void setHint(const QVariant& hint) = 0;
    virtual void setFeedback(const QVariant& feedback) = 0;
    virtual void setFeedbackState(AbstractFrontend::ResultState feedbackState) = 0;
    virtual void setResultState(AbstractFrontend::ResultState resultState) = 0;

    virtual void setQuestionSound(const KUrl& soundUrl) = 0;
    virtual void setSolutionSound(const KUrl& soundUrl) = 0;
    virtual void setSolutionPronunciation(const QString& pronunciationText) = 0;
    virtual void setQuestionPronunciation(const QString& pronunciationText) = 0;

    virtual void setQuestionFont(const QFont& font) = 0;
    virtual void setSolutionFont(const QFont& font) = 0;

    virtual void setSynonym(const QString& entry) = 0;

    virtual void setResultPalettes(const QPalette& correctPalette, const QPalette& wrongPalette);

public slots:
    virtual void showQuestion() = 0;
    virtual void showSolution() = 0;
    virtual void showSynonym() = 0;

    /** To be called prior to focus change in deletion to allow any cleanup.

     @todo When frameworks/kde5 is implemented remove this code, if the bug
     in Qt (returning a pointer to a deleted stylesheet) is fixed.
     */
    virtual void objectDestroyed(QObject *obj = 0);

signals:
    void continueAction();
    void stopAudio();
    void hintAction();
    void skipAction();

protected:
    GuiFrontend *m_frontend;

    QPalette m_correctPalette;
    QPalette m_wrongPalette;
};

}

#endif // PRACTICE_ABSTRACTWIDGET_H
