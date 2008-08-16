/***************************************************************************

    Copyright 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>
    Copyright 2008 David Capel <wot.narg@gmail.com>

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

#include <prefs.h>

class QString;
class ActiveArea;
class KUrl;

class ParleyPracticeMainWindow : public KXmlGuiWindow
{
        Q_OBJECT

    public:
        ParleyPracticeMainWindow(QWidget *parent = 0);
        ParleyPracticeMainWindow(class KEduVocDocument * doc, QWidget * parent = 0);
        ~ParleyPracticeMainWindow();

    // States that the actions can be in.
    static const int CheckAnswer = 1;
    static const int Continue = 2;

    protected:
        class KSvgRenderer* m_renderer;

        class PracticeView* m_view;

        // for questions without images (or if no image theme exists)
        class ActiveArea* m_normalArea;

        // for questions with images (uses the normal ones if no image theme exists)
        class ActiveArea * m_imageArea;

        class PracticeEntryManager * m_manager;
        class Statistics* m_stats;
        class AnswerValidator* m_validator;
        class Timer * m_timer;

        int m_state;
        Prefs::EnumTestType::type m_mode;


        void setupBase(const QString& desktopFileFilename, class KEduVocDocument * doc = 0);
        void setupActiveArea();
        void setupActions();
        void setupModeSpecifics();
        void setupModeIndependent(ActiveArea * area);

        // Mode specific setup functions
        void setupWrittenTemplate(ActiveArea * area);
    	void setupMultipleChoiceTemplate(ActiveArea * area);
        void setupMixedLettersTemplate(ActiveArea * area);
        void setupFlashCardTemplate(ActiveArea * area);
        void setupComparisonTemplate(ActiveArea * area);
        void setupConjugationTemplate(ActiveArea * area);
    public slots:
        void slotCheckAnswer(const QString& input);
        void slotCheckAnswer(const QStringList& input);
        void slotShowSolution();
        void slotShowHint();
        void slotToggleCheckAnswerContinueActions();
        void slotCreatePreferencesDialog();
        /// Forces the entry to be marked as correctly answered. Used for when correction isn't neccessary. (eg  Flashcard mode)
        void slotForceCorrect();
        /// Forces the entry to be marked as incorrectly answered. Used for when correction isn't neccessary. (eg Flashcard mode)
        void slotForceIncorrect();

        // if a valid url exists, we show the image view.
        void slotShowImageView(const KUrl&, bool backsideOfFlashcard);

        void slotClose() { queryClose(); };
        bool queryClose();
    signals:
        void signalCheckAnswer(const QString&, const QString&);
        void signalShowSolution(const QString&, int);
        void signalShowSolution(const QStringList&, int);
        void signalShowHint(const QString&);
        void signalCheckAnswerContinueActionsToggled(int);
        void signalPracticeFinished();
};


#endif
