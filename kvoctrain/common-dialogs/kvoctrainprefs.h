/***************************************************************************
                kvoctrainprefs.h  -  KVocTrain configuration dialog

                             -------------------
    begin         : Fri Mar 25 2005

    copyright     : (C) 2005, 2007 Peter Hedlund <peter.hedlund@kdemail.net>
                    (C) 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVOCTRAINPREFS_H
#define KVOCTRAINPREFS_H

#include <KConfigDialog>

#include "kvtlanguages.h"

/**
  *@author Peter Hedlund
*/

class GeneralOptions;
class ViewOptions;
class PasteOptions;
class QueryOptions;
class ThresholdOptions;
class BlockOptions;

//class KVTLanguageList;
class KEduVocDocument;
class KVTQuery;
class KComboBox;

class KVocTrainPrefs : public KConfigDialog
{
    Q_OBJECT

public:
    /**
     * Creates a configuration dialog.
     */
    KVocTrainPrefs(KEduVocDocument *doc, const KVTLanguageList &ls, KVTQuery *m, QWidget *parent, const QString &name, KConfigSkeleton *config);

    /**
     * Displays the dialog with the language page selected.
     */
    void selectLanguagePage();

protected slots:
    /**
     * Called when the user clicks Apply or OK.
     */
    void updateSettings();
    /**
     * Updates dialog widgets. Here only used after loading a profile.
     * Profiles only store the settings of the last three pages in the dialog.
     */
    void updateWidgets();
    /**
     * Called when the user clicks Default
     */
    void updateWidgetsDefault();
    /**
     * Called when the user clicks Profiles...
     */
    void slotUser1();

protected:
    /**
     * Returns true if the current state of the dialog is different from the saved settings
     */
    bool hasChanged();
    /**
     * Returns true if the current state of the dialog represents the default settings.
     */
    bool isDefault();

private:
    GeneralOptions * m_generalOptions;
    ViewOptions * m_viewOptions;
    PasteOptions * m_pasteOptions;
    QueryOptions * m_queryOptions;
    ThresholdOptions * m_thresholdOptions;
    BlockOptions * m_blockOptions;

    KConfigSkeleton *m_config;
    KVTLanguageList  m_langSet;
    KEduVocDocument *m_doc;
    KComboBox       *m_lessons;
    KVTQuery        *m_queryManager;
    KPageWidgetItem *m_languagePage;
};

#endif
