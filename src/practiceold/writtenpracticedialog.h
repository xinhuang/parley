/***************************************************************************

                   dialog when in random query mode

    -----------------------------------------------------------------------

    begin          : Thu Mar 11 20:50:53 MET 1999

    copyright      : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                     (C) 2005, 2007 Peter Hedlund <peter.hedlund@kdemail.net>
    Copyright 2007-2009 Frederik Gladhorn <gladhorn@kde.org>

    -----------------------------------------------------------------------

 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WRITTENPRACTICEDIALOG_H
#define WRITTENPRACTICEDIALOG_H

#include <QKeyEvent>
#include <QList>

#include "ui_writtenpracticedialog.h"
#include "practicedialog.h"

class KEduVocDocument;

class KComboBox;
class KLineEdit;

class WrittenPracticeDialog : public PracticeDialog
{
    Q_OBJECT

public:
    WrittenPracticeDialog(KEduVocDocument *doc, QWidget *parent);

    ~WrittenPracticeDialog();

    void setEntry( TestEntry* entry );
    void setProgressCounter(int current, int total);

public slots:
    void verifyClicked();
    void showMoreClicked();
    void showSolution();
    void slotAnswerChanged(const QString&);
//     void slotTransChanged(const QString&);
//     void slotTransLostFocus();
//     void slotFalseFriendClicked(bool show);
    void slotTypeClicked(bool show);
    void slotRemClicked(bool show);

private:
    void setStatusText(const QString &statusText) { mw->status->setText(statusText); }
    QProgressBar* timebar() { return mw->timebar; }

    void keyPressEvent(QKeyEvent *e);

    void setHintFields();
//     QStringList extractTranslations(const QString &);

    void showContinueButton(bool show);

//     QList<KComboBox*> transCombos;
//     QList<KLineEdit*> transFields;

    // the line edit which awaits the users input
//     KLineEdit * m_answerLineEdit;

//     QStringList translations;
//     QStringList vocabulary;

    /// who or what am I?
    bool suggestion_hint;

    // number of solutions (???)
//     int  fields;

    Ui::WrittenPracticeDialog * mw;
};

#endif