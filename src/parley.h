/***************************************************************************

                         main part of Parley

    -----------------------------------------------------------------------

    begin         : Thu Mar 11 20:50:53 MET 1999

    copyright     : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                    (C) 2004-2007 Peter Hedlund <peter.hedlund@kdemail.net>
                    (C) 2007-2008 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

    -----------------------------------------------------------------------

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PARLEY_H
#define PARLEY_H

#include "parleydocument.h"

#include <KXmlGuiWindow>
#include <KUrl>
#include <QItemSelection>
#include <QModelIndex>

#define IDS_DEFAULT I18N_NOOP("Ready.")

class PracticeManager;
class EntryDlg;
class LessonDockWidget;
class WordTypeDockWidget;

class KEduVocDocument;
class KLineEdit;
class KComboBox;
class KMessageBox;
class KRecentFilesAction;
class KAction;
class KActionMenu;
class QProgressBar;
class QSplitter;
class QLabel;
class VocabularyView;
class VocabularyModel;
class VocabularyFilter;
class ContainerView;
class ContainerModel;

/**
  * This Class is the base class for your application. It sets up the main
  * window and reads the config file as well as providing a menubar, toolbar
  * and statusbar. For the main view, an instance of class kvoctrainView is
  * created which creates your view.
  */
class ParleyApp : public KXmlGuiWindow
{
    Q_OBJECT

public:
    /** construtor with appName (executable name) and filename to open */
    ParleyApp(const QString& appName, const KUrl &filename = KUrl());

    /** destructor */
    ~ParleyApp();

    /**
     * setup the action (menus etc)
     */
    void initActions();

    /** setup the statusbar */
    void initStatusBar();
    /** setup the main model*/
    void initModel();

    /** setup the main view*/
    void initView();

    void initDockWidgets();

    /** save the app-specific options on slotAppExit or by an Options dialog */
    void saveOptions();

    /** This will look at the lesson list and also the combo box to determine what should be displayed in the table. */
    void updateTableFilter();


public slots:
    void slotUpdateWindowCaption();

    /** append language to vocabulary - creates a new column */
    void slotEditLanguages();

    bool queryClose();
    /** overloaded for Message box on last window exit */
    bool queryExit();
    /** set up options */
    void slotGeneralOptions();
    void slotApplyPreferences();
    void slotLanguageProperties();
    void slotDocumentProperties();
    void slotShowStatistics();

    void startPractice();
    void configurePractice();

    /** put the marked text/object into the clipboard*/
    void slotEditCopy();

    /** Cut an entry */
    void slotCutEntry();

    /** Delete an entry */
    void slotDeleteEntry();

    /** paste the clipboard into the document*/
    void slotEditPaste();

    void slotConfigShowSearch();

    /**
     * Show the tip of the day (force it to be shown)
     */
    void tipOfDay();

    /**
     * Show the tip of the day - the startup version that can be disabled
     */
    void startupTipOfDay();

signals:
    void signalSetData( const QList<int>& entries, int currentTranslation);

private:

    /**
     * Set the current doc (after creating a new one or opening a file)
     */
    void updateDocument();

    // KAction pointers to enable/disable actions
    KRecentFilesAction* m_recentFilesAction;
    KAction* m_deleteEntriesAction;
    KAction* m_vocabShowSearchBarAction;
    KActionMenu* m_vocabularyColumnsActionMenu;

    QString lastPixName;

    VocabularyModel *m_vocabularyModel;
    VocabularyView *m_vocabularyView;
    VocabularyFilter *m_vocabularyFilter;

    QWidget *m_searchWidget;

    /** m_document is the current vocabulary document. */
    ParleyDocument   *m_document;

    /// dock widgets to display lessons, word types, ...
    ContainerView *m_lessonView;
    ContainerModel *m_lessonModel;

    ContainerView *m_wordTypeView;
    ContainerModel *m_wordTypeModel;

    KLineEdit           *m_searchLine;

    EntryDlg            *m_entryDlg;

    QLabel              *m_pronunciationStatusBarLabel;
    QLabel              *m_remarkStatusBarLabel;
    QLabel              *m_typeStatusBarLabel;

    friend class ParleyDocument;

    // the name of the executable
    QString m_appName;
};

#endif // PARLEY_H

