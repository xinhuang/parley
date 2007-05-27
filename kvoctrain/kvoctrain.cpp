/***************************************************************************

                      main part of kvoctrain

    -----------------------------------------------------------------------

    begin         : Thu Mar 11 20:50:53 MET 1999

    copyright     : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                    (C) 2001 The KDE-EDU team
                    (C) 2004-2007 Peter Hedlund <peter.hedlund@kdemail.net>

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

#include "kvoctrain.h"

#include <QFile>
#include <QTimer>
#include <QPixmap>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <QProgressBar>
#include <QSplitter>

#include <kstatusbar.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kselectaction.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kprinter.h>
#include <krecentfilesaction.h>
#include <kinputdialog.h>
#include <kapplication.h>

#include "entry-dialogs/EntryDlg.h"
#include "docprop-dialogs/DocPropDlg.h"
#include "docprop-dialogs/DocPropLangDlg.h"
#include "statistics-dialogs/StatisticsDialog.h"
#include "common-dialogs/kvoctrainprefs.h"
#include "prefs.h"

#define MAX_LESSON       25
#define THRESH_LESSON    KV_MIN_GRADE


void KVocTrainApp::saveOptions()
{
    fileOpenRecent->saveEntries(KGlobal::config()->group("Recent Files"));

    if (m_tableView) {
        Prefs::setCurrentRow(m_tableView->currentIndex().row());
        Prefs::setCurrentCol(m_tableView->currentIndex().column());
    }

    if (m_lessonSelectionCombo)
        Prefs::setLessonEditingSelection(m_lessonSelectionCombo->currentIndex());

    if (m_mainSplitter)
        Prefs::setMainWindowSplitter(m_mainSplitter->sizes());

    m_languages.write();
    Prefs::self()->writeConfig();
}


void KVocTrainApp::readOptions()
{
    fileOpenRecent->loadEntries(KGlobal::config()->group("Recent Files"));
    m_languages.read();
}


void KVocTrainApp::saveProperties(KConfigGroup &config)
{
    saveOptions();
    if (m_doc) {
        config.writeEntry("Filename", m_doc->url().path());
        config.writeEntry("Title", m_doc->title());
        config.writeEntry("Modified", m_doc->isModified());

        QString filename=m_doc->url().path();
        QString tempname = kapp->tempSaveName(filename);
        saveDocProps(m_doc);
        m_doc->saveAs(KUrl(tempname), KEduVocDocument::automatic, "KVocTrain");
    }
}


void KVocTrainApp::readProperties(const KConfigGroup &config)
{
    QString filename = config.readEntry("Filename");
    QString title = config.readEntry("Title");
    bool modified = config.readEntry("Modified", false);
    if (modified) {
        bool b_canRecover;
        QString tempname = kapp->checkRecoverFile(filename,b_canRecover);

        if (b_canRecover) {
            m_doc = new KEduVocDocument(this);
            m_doc->setUrl(KUrl(tempname));
            removeProgressBar();
            m_doc->setModified();
            m_doc->setTitle(title);
            m_doc->setUrl(KUrl(filename));
            setCaption(m_doc->title(), m_doc->isModified());
            QFile::remove(tempname);
        }
    } else if (!filename.isEmpty()) {
        m_doc = new KEduVocDocument(this);
        m_doc->setUrl(KUrl(filename));
        removeProgressBar();
        setCaption(m_doc->title(), m_doc->isModified());
    }

    show();
}

/*void KVocTrainApp::slotSaveOptions()
{
   saveOptions(true);
}
*/

KVocTrainApp::~KVocTrainApp()
{
    removeEntryDlg();
    delete m_doc;
}


void KVocTrainApp::slotCancelSelection()
{
    m_tableView->clearSelection();
}


void KVocTrainApp::slotSelectAll()
{
    m_tableView->selectAll();
}


void KVocTrainApp::slotEditRow()
{
    slotEditEntry2(m_tableView->currentIndex());
}


void KVocTrainApp::slotEditCallBack(int res)
{
    switch (res) {
    case EntryDlg::EditCancel:
        removeEntryDlg();
        break;

    case EntryDlg::EditApply:
        commitEntryDlg(true);
        if (Prefs::smartAppend()) {
            int row = m_tableView->currentIndex().row();
            if (row == m_tableModel->rowCount(QModelIndex()) - 1) {
                int col = m_tableView->currentIndex().column();
                if (col < m_tableModel->columnCount(QModelIndex()) - 1 && col >= KV_COL_ORG) {
                    int lesson = m_doc->entry(row)->lesson();
                    //if (lesson >= m_lessonsComboBox->count())
                    //lesson = qMax (0, m_lessonsComboBox->count()-1);
                    m_lessonView->slotSelectLesson(lesson);

                    QString exp;
                    exp = m_doc->entry(row)->translation(col+1-KV_COL_ORG);
                    if (exp.isEmpty())
                        m_tableView->setCurrentIndex(m_tableModel->index(row, col + 1));
                } else
                    slotAppendRow();
            }
        }
        break;

    case EntryDlg::EditUndo:
        int row, col;
        QModelIndexList tabsel;
        entryDlg->getCell(row, col, tabsel);
        setDataEntryDlg(row, col);
        break;
    }
}


void KVocTrainApp::commitEntryDlg(bool force)
{
    if (entryDlg == 0) {
        kError() << "KVocTrainApp::commitEntryDlg: entryDlg == 0\n";
        return;
    }

    if (!force && entryDlg->isModified() && !Prefs::autoEntryApply()) {
        if (KMessageBox::No == KMessageBox::warningYesNo(this,
                i18n("The entry dialog contains unsaved changes.\n"
                     "Do you want to apply or discard your changes?"),
                i18n("Unsaved Changes"),
                KStandardGuiItem::apply(), KStandardGuiItem::discard())) {
            return;
        }
    }

    int row, col;

    QModelIndexList tabsel;
    entryDlg->getCell(row, col, tabsel);
    int hasSel = tabsel.count() > 1;

//  fillLessonBox();

    if (!hasSel) {
        KEduVocExpression *expr = m_doc->entry(row);
        if (col >= KV_COL_ORG) {
            m_tableModel->setData(m_tableModel->index(row, col), entryDlg->getExpr(), Qt::EditRole);
            col -= KV_EXTRA_COLS;

            expr->setRemark(col, entryDlg->getRemark());
            expr->setPronunciation(col, entryDlg->getPronounce());
            expr->setSynonym(col, entryDlg->getSynonym());
            expr->setAntonym(col, entryDlg->getAntonym());
            expr->setExample(col, entryDlg->getExample());
            expr->setUsageLabel(col, entryDlg->getUsageLabel());
            expr->setParaphrase(col, entryDlg->getParaphrase());
            expr->setConjugation(col, entryDlg->getConjugation());
            expr->setComparison(col, entryDlg->getComparison());
            expr->setMultipleChoice(col, entryDlg->getMultipleChoice());
            expr->setFauxAmi(col, entryDlg->getFromFauxAmi(), false);
            expr->setFauxAmi(col, entryDlg->getToFauxAmi(), true);
            expr->setGrade(col, entryDlg->getFromGrade(), false);
            expr->setGrade(col, entryDlg->getToGrade(), true);
            expr->setQueryCount(col, entryDlg->getFromQCount(), false);
            expr->setQueryCount(col, entryDlg->getToQCount(), true);
            expr->setBadCount(col, entryDlg->getFromBCount(), false);
            expr->setBadCount(col, entryDlg->getToBCount(), true);
            expr->setQueryDate(col, entryDlg->getFromDate(), false);
            expr->setQueryDate(col, entryDlg->getToDate(), true);
            expr->setType(col, entryDlg->getType());

            for (int j = 0; j <= expr->translationCount(); j++)
                if (expr->type(j).isEmpty())
                    expr->setType(j, entryDlg->getType());

            for (int j = 0; j <= expr->translationCount(); j++)
                if (KVTQuery::getMainType(expr->type(j))
                        !=
                        KVTQuery::getMainType(entryDlg->getType()))
                    expr->setType(j, entryDlg->getType());
        }
        m_tableModel->setData(m_tableModel->index(row, 0), entryDlg->getLesson(), Qt::EditRole);
        expr->setActive(entryDlg->getActive());

        entryDlg->setModified(false);
        m_doc->setModified(true);

    } else {
        col -= KV_EXTRA_COLS;
        foreach(QModelIndex selIndex, tabsel) {
            QModelIndex index = m_sortFilterModel->mapToSource(selIndex);
            KEduVocExpression *expr = m_doc->entry(index.row());

            if (col >= 0) {
                // only updated "common" props in multimode
                if (entryDlg->fromGradeIsModified())
                    expr->setGrade(col, entryDlg->getFromGrade(), false);
                if (entryDlg->toGradeIsModified())
                    expr->setGrade(col, entryDlg->getToGrade(), true);
                if (entryDlg->fromQueryCountIsModified())
                    expr->setQueryCount(col, entryDlg->getFromQCount(), false);
                if (entryDlg->toQueryCountIsModified())
                    expr->setQueryCount(col, entryDlg->getToQCount(), true);
                if (entryDlg->fromBadCountIsModified())
                    expr->setBadCount(col, entryDlg->getFromBCount(), false);
                if (entryDlg->toBadCountIsModified())
                    expr->setBadCount(col, entryDlg->getToBCount(), true);
                if (entryDlg->fromDateIsModified())
                    expr->setQueryDate(col, entryDlg->getFromDate(), false);
                if (entryDlg->toDateIsModified())
                    expr->setQueryDate(col, entryDlg->getToDate(), true);
                if (entryDlg->usageIsModified())
                    for (int j = 0; j <= expr->translationCount(); j++)
                        expr->setUsageLabel(j, entryDlg->getUsageLabel());
                if (entryDlg->typeIsModified())
                    for (int j = 0; j <= expr->translationCount(); j++)
                        expr->setType(j, entryDlg->getType());
            }

            if (entryDlg->activeIsModified())
                expr->setActive(entryDlg->getActive());
            if (entryDlg->lessonIsModified())
                m_tableModel->setData(m_tableModel->index(index.row(), 0), entryDlg->getLesson(), Qt::EditRole);
        }
        entryDlg->setModified(false);
        m_doc->setModified(true);
    }
}


void KVocTrainApp::removeEntryDlg()
{
    if (entryDlg != 0) {
        commitEntryDlg(false);
        disconnect(entryDlg, SIGNAL(sigEditChoice(int)), this, SLOT(slotEditCallBack(int)));
        entryDlg->deleteLater();
        entryDlg = 0;
    }

    m_tableView->setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::EditKeyPressed | QAbstractItemView::DoubleClicked);
}


void KVocTrainApp::slotEditEntry(int row, int col)
{
    if (entryDlg == 0) {
        entryDlg = new EntryDlg(this, m_doc, m_queryManager->getKVTQuery());
        connect(entryDlg, SIGNAL(sigEditChoice(int)), this, SLOT(slotEditCallBack(int)));

    }

    if (entryDlg->isModified()) {
        commitEntryDlg(false);
    }
    entryDlg->show();
    setDataEntryDlg(row, col);
}


void KVocTrainApp::slotEditEntry2(const QModelIndex & index)
{
    if (index.isValid()) {
        QModelIndex docIndex = m_sortFilterModel->mapToSource(index);
        slotEditEntry(docIndex.row(), docIndex.column());
    }
}


void KVocTrainApp::setDataEntryDlg(int row, int col)
{
    if (entryDlg == 0) {
        kError() << "KVocTrainApp::setDataEntryDlg: entryDlg == 0\n";
        return;
    }

    if ((row < 0) || (col < 0) || (m_tableModel->rowCount(QModelIndex()) <= 0))
        return;

    QString text, title;

    KEduVocExpression *expr = m_doc->entry(row);

    if (expr == 0)
        return; // entry delete in the meantime

    int lesson = expr->lesson();
    if (lesson > m_lessonModel->rowCount())
        lesson = qMax(0, m_lessonModel->rowCount());

    bool hasSelection = (m_tableView->selectionModel()->selectedRows().count() > 1);
    kDebug() << hasSelection << endl;

    col -= KV_EXTRA_COLS;

    EntryDlg::EnableType et;

    if (col < 0)
        et = EntryDlg::EnableOnlyCommon;
    else if (col == 0)
        et = EntryDlg::EnableOnlyOriginal;
    else
        et = EntryDlg::EnableAll;

    if (col < 0) {
        title = i18n("Edit General Properties");

        entryDlg->setData(et,
                          hasSelection,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          QDateTime(),
                          QDateTime(),
                          QString(),
                          QString(),
                          QString(),
                          lesson,
                          QString(),
                          m_doc->entry(row)->type(0),
                          QString(),
                          QString(),
                          QString(),
                          QString(),
                          QString(),
                          QString(),
                          m_doc->conjugation(0),
                          KEduVocConjugation(),
                          KEduVocComparison(),
                          KEduVocMultipleChoice(),
                          title,
                          m_doc->entry(row)->isActive());
    } else {
        text = m_tableModel->data(m_tableModel->index(row, col + KV_EXTRA_COLS), Qt::DisplayRole).toString();
        //col -= KV_EXTRA_COLS;

        if (col == 0)
            title = i18n("Edit Properties for Original");
        else
            title = i18n("Edit Properties of a Translation");

        entryDlg->setData(et,
                          hasSelection,
                          m_doc->entry(row)->grade(col, false),
                          m_doc->entry(row)->grade(col, true),
                          m_doc->entry(row)->queryCount(col, false),
                          m_doc->entry(row)->queryCount(col, true),
                          m_doc->entry(row)->badCount(col, false),
                          m_doc->entry(row)->badCount(col, true),
                          m_doc->entry(row)->queryDate(col, false),
                          m_doc->entry(row)->queryDate(col, true),
                          m_doc->entry(row)->fauxAmi(col, false),
                          m_doc->entry(row)->fauxAmi(col, true),
                          text,
                          lesson,
                          m_doc->entry(row)->remark(col),
                          m_doc->entry(row)->type(col),
                          m_doc->entry(row)->pronunciation(col),
                          m_doc->entry(row)->synonym(col),
                          m_doc->entry(row)->antonym(col),
                          m_doc->entry(row)->example(col),
                          m_doc->entry(row)->usageLabel(col),
                          m_doc->entry(row)->paraphrase(col),
                          m_doc->conjugation(col),
                          m_doc->entry(row)->conjugation(col),
                          m_doc->entry(row)->comparison(col),
                          m_doc->entry(row)->multipleChoice(col),
                          title,
                          m_doc->entry(row)->isActive());
    }

    entryDlg->setCell(row, col + KV_EXTRA_COLS, m_tableView->selectionModel()->selectedRows());
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}


void KVocTrainApp::slotDocumentProperties()
{
    int old_types = (int) m_doc->typeDescriptions().count();
    int old_tenses = (int) m_doc->tenseDescriptions().count();
    int old_usages = (int) m_doc->usageDescriptions().count();

    DocPropsDlg ddlg(m_doc, this);

    if (ddlg.exec() == QDialog::Accepted) {
        QList<int> typeIndex;
        QList<int> tenseIndex;
        QList<int> usageIndex;
        QList<int> lessonIndex;
        QStringList new_typeStr;
        QStringList new_tenseStr;
        QStringList new_usageStr;

        m_doc->setTitle(ddlg.getTitle());
        m_doc->setAuthor(ddlg.getAuthor());
        m_doc->setLicense(ddlg.getLicense());
        m_doc->setDocumentRemark(ddlg.getDocRemark());

        slotStatusMsg(i18n("Updating lesson indices..."));
        QApplication::setOverrideCursor(Qt::WaitCursor);

        ddlg.getTypeNames(new_typeStr, typeIndex);
        ddlg.getTenseNames(new_tenseStr, tenseIndex);
        ddlg.getUsageLabels(new_usageStr, usageIndex);

        slotStatusMsg(i18n("Updating type indices..."));
        TypeOptPage::cleanUnused(m_doc, typeIndex, old_types);
        KVTQuery::setTypeNames(new_typeStr);

        slotStatusMsg(i18n("Updating tense indices..."));
        TenseOptPage::cleanUnused(m_doc, tenseIndex, old_tenses);
        KEduVocConjugation::setTenseNames(new_tenseStr);

        slotStatusMsg(i18nc("usage (area) of an expression", "Updating usage label indices..."));
        UsageOptPage::cleanUnused(m_doc, usageIndex, old_usages);
        KVTUsage::setUsageNames(new_usageStr);

        m_doc->setSortingEnabled(ddlg.getSorting());
        m_tableView->setSortingEnabled(m_doc->isSortingEnabled());
        m_doc->setTypeDescriptions(new_typeStr);
        m_doc->setTenseDescriptions(new_tenseStr);
        m_doc->setUsageDescriptions(new_usageStr);
        m_doc->setModified();

        m_tableModel->reset();
        setCaption(m_doc->title(), m_doc->isModified());
        QApplication::restoreOverrideCursor();
        slotStatusMsg(IDS_DEFAULT);
    }
}


void KVocTrainApp::slotDocPropsLang()
{
    DocPropsLangDlg ldlg(m_doc, m_languages, this);

    if (ldlg.exec() == QDialog::Accepted) {
        for (int i = 0; i < m_doc->identifierCount(); i++) {
            m_doc->setArticle(i, ldlg.getArticle(i));
            m_doc->setConjugation(i, ldlg.getConjugation(i));
        }

        m_doc->setModified();
        m_tableModel->reset();
        setCaption(m_doc->title(), m_doc->isModified());
        slotStatusMsg(IDS_DEFAULT);
    }
}


void KVocTrainApp::slotModifiedDoc(bool /*mod*/)
{
    setCaption(m_doc->title(), m_doc->isModified());
    slotStatusMsg(IDS_DEFAULT);
}


void KVocTrainApp::slotRemoveRow()
{
    if (m_tableView->selectionModel()->selectedRows().count() == 1) {
        if (KMessageBox::Continue == KMessageBox::warningContinueCancel(this, i18n("Do you really want to delete the selected entry?"), "", KStandardGuiItem::del())) {
            int currentRow = m_tableView->currentIndex().row();
            int currentColumn = m_tableView->currentIndex().column();
            m_tableModel->removeRows(m_tableView->currentIndex().row(), 1, QModelIndex());
            m_tableView->selectionModel()->setCurrentIndex(m_tableModel->index(currentRow, currentColumn), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
    } else {
        if (KMessageBox::Continue == KMessageBox::warningContinueCancel(this, i18n("Do you really want to delete the selected entries?"), "", KStandardGuiItem::del())) {
            int currentRow = m_tableView->currentIndex().row();
            int currentColumn = m_tableView->currentIndex().column();
            int rowCount = m_tableModel->rowCount(QModelIndex());
            // Must count backwards otherwise entry-numbering goes wrong when
            // deleting.
            for (int i = rowCount - 1; i >= 0; i--)
                if (m_tableView->selectionModel()->isRowSelected(i, QModelIndex()))
                    m_tableModel->removeRows(i, 1, QModelIndex());
            m_tableView->selectionModel()->setCurrentIndex(m_tableModel->index(currentRow, currentColumn), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
    }
    editDelete->setEnabled(m_tableModel->rowCount(QModelIndex()) > 0);
}


void KVocTrainApp::slotAppendRow()
{
    m_tableModel->insertRows(m_tableModel->rowCount(QModelIndex()), 1, QModelIndex());
    m_tableView->selectionModel()->setCurrentIndex(m_tableModel->index(m_tableModel->rowCount(QModelIndex()) - 1, KV_COL_ORG), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    m_tableModel->setData(m_tableView->currentIndex(), m_doc->currentLesson(), KVTTableModel::LessonRole);
    editDelete->setEnabled(m_tableModel->rowCount(QModelIndex()) > 0);
    makeLessonVisibleInTable(m_doc->currentLesson());
}

void KVocTrainApp::makeLessonVisibleInTable(int lessonIndex)
{
    switch (m_lessonSelectionCombo->currentIndex()) {
    case Prefs::EnumLessonEditingSelection::CurrentLesson:
        m_doc->setCurrentLesson(lessonIndex);
        m_sortFilterModel->clear();
        break;
    case Prefs::EnumLessonEditingSelection::LessonsInQuery:
        m_doc->setCurrentLesson(lessonIndex);
        if (!m_doc->lessonInQuery(lessonIndex)) {
            m_lessonSelectionCombo->setCurrentIndex(Prefs::EnumLessonEditingSelection::CurrentLesson);
        }
        m_sortFilterModel->clear();
        break;
    case Prefs::EnumLessonEditingSelection::AllLessons:
        break;
    }
    updateTableFilter();
}

void KVocTrainApp::keyPressEvent(QKeyEvent *e)
{
    controlActive = (e->modifiers() & Qt::ControlModifier) !=0;

    switch (e->key()) {
        /*
        case Qt::Key_Plus:
          if (controlActive) {
            int less = m_lessonsComboBox->currentIndex();
            if (less == m_lessonsComboBox->count() - 1)
              m_lessonsComboBox->setCurrentIndex(0);
            else
              m_lessonsComboBox->setCurrentIndex(less+1);
            slotChooseLesson(m_lessonsComboBox->currentIndex());
          }
        break;

        case Qt::Key_Minus:
          if (controlActive) {
            int less = m_lessonsComboBox->currentIndex();
            if (less == 0)
              m_lessonsComboBox->setCurrentIndex(m_lessonsComboBox->count() - 1);
            else
              m_lessonsComboBox->setCurrentIndex(less - 1);
            slotChooseLesson(m_lessonsComboBox->currentIndex());
          }
        break;
        */
    case Qt::Key_Control:
        controlActive = true;
        break;

    case Qt::Key_Tab:
        if (m_tableView->hasFocus())  {
            m_searchLine->setFocus();
            m_searchLine->selectAll();
        } else
            m_tableView->setFocus();
        break;

    case Qt::Key_Backtab:
        if (m_searchLine->hasFocus())
            m_tableView->setFocus();
        else {
            m_searchLine->setFocus();
            m_searchLine->selectAll();
        }
        break;

    default:
        bool found = false;
        if (!found)
            e->ignore();
    }
    slotStatusMsg(IDS_DEFAULT);
}


void KVocTrainApp::slotShowStatistics()
{
    KVTStatisticsDialog sdlg(m_languages, m_tableModel, this);
    sdlg.exec();
}


void KVocTrainApp::slotCleanVocabulary()
{
    prepareProgressBar();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    int num = m_doc->cleanUp();
    QApplication::restoreOverrideCursor();
    removeProgressBar();

    slotStatusMsg(IDS_DEFAULT);

    if (num != 0) {
        m_tableModel->reset();
        QString s = i18np("1 entry with the same content was found and removed.",
                          "%1 entries with the same content were found and removed.", num);

        KMessageBox::information(this, s, i18n("Remove Duplicates"));
    }
}


void KVocTrainApp::slotGeneralOptions()
{
    slotGeneralOptionsPage(-1);
}


void KVocTrainApp::slotGeneralOptionsPage(int index)
{
    KVocTrainPrefs* dialog = new KVocTrainPrefs(m_languages, &m_queryManager->getKVTQuery(), this, "settings",  Prefs::self());
    connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(slotApplyPreferences()));
    if (index >= 0)
        dialog->selectLanguagePage();
    dialog->show();
}


void KVocTrainApp::slotApplyPreferences()
{
    if (Prefs::autoBackup())
        QTimer::singleShot(Prefs::backupTime() * 60 * 1000, this, SLOT(slotTimeOutBackup()));

    if (m_pronunciationStatusBarLabel)
        m_pronunciationStatusBarLabel->setFont(Prefs::iPAFont());

    m_tableView->setFont(Prefs::tableFont());
    m_tableView->reset();

    m_languages.read();
    m_tableModel->setLanguages(m_languages);
    m_tableModel->reset();
}


void KVocTrainApp::slotAppendLanguage(int index)
{
    if (index >= m_languages.count()) {

        QString msg = i18n("To add a language which is not listed in the menu, you must first define its properties in the "
                           "general options dialog.\n\nWould you like to add a new language?");
        if (KMessageBox::Yes == KMessageBox::questionYesNo(this, msg)) {
            slotGeneralOptionsPage(1);
        }
        return;
    }

    m_tableModel->insertColumns(m_tableModel->columnCount(QModelIndex()), 1, QModelIndex());
    m_tableModel->setHeaderData(m_tableModel->columnCount(QModelIndex()) - 1, Qt::Horizontal, m_languages[index].shortId(), Qt::EditRole);
}


void KVocTrainApp::slotAssignLanguage(QAction * action)
{
    int column = qvariant_cast<QPoint>(action->data()).x() + KV_EXTRA_COLS;
    int index  = qvariant_cast<QPoint>(action->data()).y();

    if (index >= (int) m_languages.count())
        return;

    m_tableModel->setHeaderData(column, Qt::Horizontal, m_languages[index].shortId(), Qt::EditRole);
}


void KVocTrainApp::slotAssignLanguage2(int column, int languageIndex)
{
    column += KV_EXTRA_COLS;

    if (languageIndex >= (int) m_languages.count())
        return;

    m_tableModel->setHeaderData(column, Qt::Horizontal, m_languages[languageIndex].shortId(), Qt::EditRole);
}

void KVocTrainApp::slotRemoveLanguage(int index)
{
    int column = index + KV_EXTRA_COLS + 1;
    QString name = m_tableModel->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();

    QString msg = i18n("You are about to delete a language permanently.\nDo you really want to delete '%1'?", name);

    int result = KMessageBox::warningContinueCancel(this, msg, "", KStandardGuiItem::del());
    if (result == KMessageBox::Continue)
        m_tableModel->removeColumns(column, 1, QModelIndex());
}

void KVocTrainApp::slotSearch(const QString& s)
{
    if (s.length() == 0) {
        m_sortFilterModel->setSearchRegExp(QRegExp());
        return;
    }
    // this can probably be done a little cleaner
    // now "hello world" becomes "(hello|world)" so it basically works,
    // but there are bound to be exceptions
    QString searchterm = s.simplified();
    searchterm.replace(QString(" "), QString("|"));
    QRegExp searchRegExp = QRegExp('(' + searchterm + ')', Qt::CaseInsensitive);
    m_sortFilterModel->setSearchRegExp(searchRegExp);
}


void KVocTrainApp::slotStatusMsg(const QString &/*text*/)
{}

/** create the learning action menu
 * use a qsignalmapper to connect all subitems to one slot.
 */
void KVocTrainApp::aboutToShowLearn()
{
    QMenu *identifierSubMenu;
    QMenu *identifierSubSubMenu;
    QAction *typeAction;

    QString mapString = QString("%1%2%3");
    QChar fillChar = QLatin1Char('0');

    learningMenu->clear();

    if (m_learningMapper != 0)
        delete m_learningMapper;
    m_learningMapper = 0;
    m_learningMapper = new QSignalMapper(this);
    connect(m_learningMapper, SIGNAL(mapped(const QString &)), this, SLOT(slotLearningMapperTriggered(const QString &)));

    int columns = m_tableModel->columnCount(QModelIndex()) - KV_EXTRA_COLS;

    //collect needed data

    /// @todo this should come from m_doc
    QStringList titles;
    QList<QPixmap> icons;
    for (int i = 0; i < columns; i++) {
        titles.append(m_tableModel->headerData(i + KV_EXTRA_COLS, Qt::Horizontal, Qt::DisplayRole).toString());
        icons.append(m_tableModel->headerData(i + KV_EXTRA_COLS, Qt::Horizontal, Qt::DecorationRole).value<QPixmap>());
    }

    for (int j = 0; j < columns; j++) {

        //top level item for each identifier
        identifierSubMenu = learningMenu->addMenu(icons[j], titles[j]);

        //Random and multiple choice items
        if (j == 0) {
            identifierSubSubMenu = identifierSubMenu->addMenu(KIcon("run_query"), i18n("Create Random &Query"));
            for (int k = 1; k < columns; k++) {
                typeAction = identifierSubSubMenu->addAction(icons[k], i18n("From %1", titles[k]), m_learningMapper, SLOT(map()));
                typeAction->setWhatsThis(i18n("Creates and starts query from %1 to %2", titles[k], titles[0]));
                typeAction->setToolTip(typeAction->whatsThis());
                typeAction->setStatusTip(typeAction->whatsThis());
                m_learningMapper->setMapping(typeAction, mapString.arg(START_QUERY, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(k, 3, 10, fillChar));
            }

            identifierSubSubMenu = identifierSubMenu->addMenu(KIcon("run_multi"), i18n("Create &Multiple Choice"));
            for (int k = 1; k < columns; k++) {
                typeAction = identifierSubSubMenu->addAction(icons[k], i18n("From %1", titles[k]), m_learningMapper, SLOT(map()));
                typeAction->setWhatsThis(i18n("Creates and starts multiple choice from %1 to %2", titles[k], titles[0]));
                typeAction->setToolTip(typeAction->whatsThis());
                typeAction->setStatusTip(typeAction->whatsThis());
                m_learningMapper->setMapping(typeAction, mapString.arg(START_MULTIPLE, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(k, 3, 10, fillChar));
            }
        } else {
            typeAction = identifierSubMenu->addAction(KIcon("run_query"), i18n("Create Random &Query"), m_learningMapper, SLOT(map()));
            typeAction->setWhatsThis(i18n("Creates and starts query from %1 to %2", titles[0], titles[j]));
            typeAction->setToolTip(typeAction->whatsThis());
            typeAction->setStatusTip(typeAction->whatsThis());
            m_learningMapper->setMapping(typeAction, mapString.arg(START_QUERY, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));

            typeAction = identifierSubMenu->addAction(KIcon("run_multi"), i18n("Create &Multiple Choice"), m_learningMapper, SLOT(map()));
            typeAction->setWhatsThis(i18n("Creates and starts multiple choice from %1 to %2", titles[0], titles[j]));
            typeAction->setToolTip(typeAction->whatsThis());
            typeAction->setStatusTip(typeAction->whatsThis());
            m_learningMapper->setMapping(typeAction, mapString.arg(START_MULTIPLE, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));
        }

        identifierSubMenu->addSeparator();

        //Special queries items for each identifier
        typeAction = identifierSubMenu->addAction(i18n("&Verbs"), m_learningMapper, SLOT(map()));
        typeAction->setWhatsThis(i18n("Starts training with verbs"));
        typeAction->setToolTip(typeAction->whatsThis());
        typeAction->setStatusTip(typeAction->whatsThis());
        m_learningMapper->setMapping(typeAction, mapString.arg(START_VERB, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));

        typeAction = identifierSubMenu->addAction(i18n("&Articles"), m_learningMapper, SLOT(map()));
        typeAction->setWhatsThis(i18n("Starts training with articles"));
        typeAction->setToolTip(typeAction->whatsThis());
        typeAction->setStatusTip(typeAction->whatsThis());
        m_learningMapper->setMapping(typeAction, mapString.arg(START_ARTICLE, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));

        typeAction = identifierSubMenu->addAction(i18n("&Comparisons"), m_learningMapper, SLOT(map()));
        typeAction->setWhatsThis(i18n("Starts training with adjectives"));
        typeAction->setToolTip(typeAction->whatsThis());
        typeAction->setStatusTip(typeAction->whatsThis());
        m_learningMapper->setMapping(typeAction, mapString.arg(START_ADJECTIVE, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));

        identifierSubMenu->addSeparator();

        typeAction = identifierSubMenu->addAction(i18n("S&ynonyms"), m_learningMapper, SLOT(map()));
        typeAction->setWhatsThis(i18n("Starts training with synonyms"));
        typeAction->setToolTip(typeAction->whatsThis());
        typeAction->setStatusTip(typeAction->whatsThis());
        m_learningMapper->setMapping(typeAction, mapString.arg(START_SYNONYM, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));

        typeAction = identifierSubMenu->addAction(i18n("A&ntonyms"), m_learningMapper, SLOT(map()));
        typeAction->setWhatsThis(i18n("Starts training with antonyms"));
        typeAction->setToolTip(typeAction->whatsThis());
        typeAction->setStatusTip(typeAction->whatsThis());
        m_learningMapper->setMapping(typeAction, mapString.arg(START_ANTONYM, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));

        typeAction = identifierSubMenu->addAction(i18n("E&xamples"), m_learningMapper, SLOT(map()));
        typeAction->setWhatsThis(i18n("Starts training with examples"));
        typeAction->setToolTip(typeAction->whatsThis());
        typeAction->setStatusTip(typeAction->whatsThis());
        m_learningMapper->setMapping(typeAction, mapString.arg(START_EXAMPLE, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));

        typeAction = identifierSubMenu->addAction(i18n("&Paraphrase"), m_learningMapper, SLOT(map()));
        typeAction->setWhatsThis(i18n("Starts training with paraphrases"));
        typeAction->setToolTip(typeAction->whatsThis());
        typeAction->setStatusTip(typeAction->whatsThis());
        m_learningMapper->setMapping(typeAction, mapString.arg(START_PARAPHRASE, 3, 10, fillChar).arg(j, 3, 10, fillChar).arg(0, 3, 10, fillChar));
    }

    learningMenu->addSeparator();
    learningMenu->addAction(learningResumeQuery);
    learningMenu->addAction(learningResumeMultipleChoice);
}


void KVocTrainApp::aboutToShowVocabAppendLanguage()
{
    if (m_doc != 0) {
        vocabAppendLanguage->removeAllActions();

        QStringList names;
        for (int i = 0; i < m_languages.count(); i++) {
            if (m_languages[i].longId().isEmpty())
                names.append(m_languages[i].shortId());
            else
                names.append(m_languages[i].longId());
        }

        QAction *action = 0;

        for (int i = 0; i < m_languages.count(); i++) {
            if (!m_languages[i].pixmapFile().isEmpty() && !m_languages[i].longId().isEmpty())
                action = new QAction(QIcon(QPixmap(m_languages[i].pixmapFile())), names[i], vocabAppendLanguage->selectableActionGroup());
            else
                action = new QAction(names[i], vocabAppendLanguage->selectableActionGroup());
            action->setWhatsThis(i18n("Add the language '%1' to the vocabulary", names[i]));
            action->setToolTip(action->whatsThis());
            action->setStatusTip(action->whatsThis());
            vocabAppendLanguage->addAction(action);
        }

        action = new QAction("", vocabAppendLanguage->selectableActionGroup());
        action->setSeparator(true);
        vocabAppendLanguage->addAction(action);

        action = new QAction(i18n("&New Language..."), vocabAppendLanguage->selectableActionGroup());
        action->setWhatsThis(i18n("Add a new language to the vocabulary"));
        action->setToolTip(action->whatsThis());
        action->setStatusTip(action->whatsThis());
        vocabAppendLanguage->addAction(action);
    }
}


void KVocTrainApp::aboutToShowVocabSetLanguage()
{
    if (m_doc != 0) {
        vocabSetLanguage->removeAllActions();
        //QMenu * set_m = vocabSetLanguage->menu();

        QStringList names;
        for (int i = 0; i < m_languages.count(); i++) {
            if (m_languages[i].longId().isEmpty())
                names.append(m_languages[i].shortId());
            else
                names.append(m_languages[i].longId());
        }

        QAction *action = 0;
        KSelectAction *selAction = 0;

        for (int column = 0; column < m_doc->identifierCount(); ++column) {
            if (column == 0)
                selAction = new KSelectAction(i18n("&Original"), vocabSetLanguage->selectableActionGroup());
            else {
                if (m_doc->identifierCount() <= 2)
                    selAction = new KSelectAction(i18n("&Translation"), vocabSetLanguage->selectableActionGroup());
                else
                    selAction = new KSelectAction(i18n("&%1. Translation", column), vocabSetLanguage->selectableActionGroup());
            }
            connect(selAction, SIGNAL(triggered(QAction *)), this, SLOT(slotAssignLanguage(QAction *)));
            vocabSetLanguage->addAction(selAction);

            for (int i = 0; i < m_languages.count(); i++) {
                if (!m_languages[i].pixmapFile().isEmpty() && !m_languages[i].longId().isEmpty())
                    action = new QAction(QIcon(QPixmap(m_languages[i].pixmapFile())), names[i], selAction->selectableActionGroup());
                else
                    action = new QAction(names[i], selAction->selectableActionGroup());
                action->setData(QVariant(QPoint(column, i))); //QPair doesn't work with QVariant
                action->setWhatsThis(i18n("Assign the language '%1' to the selected column", names[i]));
                action->setToolTip(action->whatsThis());
                action->setStatusTip(action->whatsThis());
                selAction->addAction(action);
            }
        }
    }
}


void KVocTrainApp::aboutToShowVocabRemoveLanguage()
{
    if (m_doc != 0) {
        vocabRemoveLanguage->removeAllActions();

        QStringList names;
        for (int j = 1; j < (int) m_doc->identifierCount(); j++) {
            int i;
            if ((i = m_languages.indexShortId(m_doc->identifier(j))) >= 0)
                names.append(m_languages[i].longId());
            else
                names.append(m_doc->identifier(j));
        }

        QAction *action = 0;

        for (int i = 1; i < (int) m_doc->identifierCount(); i++) {
            int j;
            if ((j = m_languages.indexShortId(m_doc->identifier(i))) >= 0 && !m_languages[j].pixmapFile().isEmpty() && !m_languages[j].longId().isEmpty())
                action = new QAction(QIcon(QPixmap(m_languages[j].pixmapFile())), names[i - 1], vocabRemoveLanguage->selectableActionGroup());
            else
                action = new QAction(names[i - 1], vocabRemoveLanguage->selectableActionGroup());
            action->setWhatsThis(i18n("Permanently remove the language '%1' from the vocabulary", names[i - 1]));
            action->setToolTip(action->whatsThis());
            action->setStatusTip(action->whatsThis());
            vocabRemoveLanguage->addAction(action);
        }
    }
}


void KVocTrainApp::slotStatusHelpMsg(const QString &text)
{
    ///////////////////////////////////////////////////////////////////
    // change status message of whole statusbar temporary (text, msec)
    if (pbar == 0 || !pbar->isVisible())
        statusBar()->showMessage(text, 3000);
}

void KVocTrainApp::slotFilePrint()
{
    slotStatusMsg(i18n("Printing..."));
    KPrinter printer;
    printer.setFullPage(true);
    if (printer.setup(this))
        m_tableView->print(&printer);
    slotStatusMsg(i18n("Ready"));
}


void KVocTrainApp::slotEditCopy()
{
    slotStatusMsg(i18n("Copying selection to clipboard..."));

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString textToCopy;
    QModelIndexList selectedRows = m_tableView->selectionModel()->selectedRows(0);

    foreach(const QModelIndex &idx, selectedRows) {
        bool sep = false;
        for (int i = KV_EXTRA_COLS; i < m_tableModel->columnCount(QModelIndex()); i++) {
            if (!sep)
                sep = true;
            else
                textToCopy += '\t';

            QModelIndex mappedIndex = m_sortFilterModel->mapToSource(m_sortFilterModel->index(idx.row(), i));
            textToCopy += m_tableModel->data(mappedIndex, Qt::DisplayRole).toString();
        }
        if (!textToCopy.isEmpty())
            textToCopy += '\n';
    }

    if (!textToCopy.isEmpty())
        QApplication::clipboard()->setText(textToCopy);

    QApplication::restoreOverrideCursor();
    slotStatusMsg(IDS_DEFAULT);
}


void KVocTrainApp::slotEditPaste()
{
    /// @todo make the pasted stuff visible by making the corresponding lesson visible, if it is not (?)
    slotStatusMsg(i18n("Inserting clipboard contents..."));

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString s;
    QString textToPaste = QApplication::clipboard()->text();

    QTextStream ts;
    ts.setString(&textToPaste, QIODevice::Text);

    QString num;

    while (!ts.atEnd()) {
        s = ts.readLine();
        if (!s.isEmpty()) {
            m_tableModel->insertRows(m_tableModel->rowCount(QModelIndex()), 1, QModelIndex());
            QStringList sl = s.split('\t', QString::KeepEmptyParts);

            for (int i = 0; i < sl.count(); ++i) {
                m_tableModel->setData(m_tableModel->index(m_tableModel->rowCount(QModelIndex()) - 1, i + KV_COL_ORG), sl[i], Qt::EditRole);
                m_tableModel->setData(m_tableModel->index(m_tableModel->rowCount(QModelIndex()) - 1, i + KV_COL_ORG), m_doc->currentLesson(), KVTTableModel::LessonRole);
            }
        }
    }

    QApplication::restoreOverrideCursor();
    slotStatusMsg(IDS_DEFAULT);
}

void KVocTrainApp::slotCurrentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    Q_UNUSED(previous);
    if (!current.isValid())
        return;

    int column = current.column() - KV_EXTRA_COLS;
    int row = current.row();
    bool noData = false;
    KEduVocExpression *expr = 0;

    if (m_doc->entryCount() <= row || m_doc->identifierCount() <= column || row < 0 || column < 0)
        noData = true;
    else
        expr = m_doc->entry(row);

    if (m_remarkStatusBarLabel != 0)
        m_remarkStatusBarLabel->setText(i18nc("Abbreviation for R)emark","R: %1", noData ? QString() : expr->remark(column)));
    if (m_pronunciationStatusBarLabel != 0)
        m_pronunciationStatusBarLabel->setText(i18nc("Abbreviation for P)ronouncation","P: %1", noData ? QString() : expr->pronunciation(column)));
    if (m_typeStatusBarLabel != 0)
        m_typeStatusBarLabel->setText(i18nc("Abbreviation for T)ype of word", "T: %1", noData ? QString() : KVTQuery::typeStr(expr->type(column))));

    if (entryDlg != 0) {
        slotEditEntry2(current);
    }
}


void KVocTrainApp::updateTableFilter()
{
    QModelIndexList indexes = m_lessonView->selectionModel()->selectedIndexes();
    // oops - this crashes if there is no selection - there should always be a current lesson!!!
    if (indexes.empty()) {
        kDebug() << "WARNING - NO SELECTION FOR ACTIVE LESSON! THIS SHOULD NOT HAPPEN!" << endl;
        return;
    }
    QModelIndex current = indexes.at(0); // should be one item selected anyway...

    m_doc->setCurrentLesson(current.row()+1);
    int comboState = m_lessonSelectionCombo->currentIndex();

    // do the columns play a role here??? I mean... I also wanto to use this for search - to filter out results... so this should only apply for the lesson column. How do I do something with the search at the same time?

    //QRegExp myReg("(Lektion 09)|(Lektion 04)");
    QString lessonStrings;
    QStringList description;

    switch (comboState) {
    case Prefs::EnumLessonEditingSelection::CurrentLesson:
        m_sortFilterModel->setLessonRegExp(QRegExp(m_lessonModel->data(current, Qt::DisplayRole).toString(), Qt::CaseInsensitive, QRegExp::FixedString));
        break;
    case Prefs::EnumLessonEditingSelection::LessonsInQuery:
        description = m_doc->lessonDescriptions();
        //kDebug << lessonStrings << endl;
        lessonStrings.append("(");
        foreach(int lesson, m_doc->lessonsInQuery()) {
            lessonStrings.append(m_doc->lessonDescriptions().at(lesson-1));
            lessonStrings.append(")|(");
        }
        lessonStrings.remove(lessonStrings.length()-2, 2); // remove the last "|("
        if (lessonStrings.compare("")==0) // in this case select none to be consistent!
        {
            lessonStrings = "$^"; // hoping no-one hase a lesson called "" for now. It's your own fault, if you call them like this ;) this is generally unfortunate... maybe I should forbid it and default back to "New lesson 1".
        }
        //m_sortFilterModel->setFilterRegExp(lessonStrings);

        m_sortFilterModel->setLessonRegExp(QRegExp(lessonStrings));
        break;
    case Prefs::EnumLessonEditingSelection::AllLessons:
        m_sortFilterModel->setLessonRegExp(QRegExp());
        break;
    }
    m_doc->setModified();
}

void KVocTrainApp::slotLessonSelectionComboChanged(int index)
{
    Q_UNUSED(index);
    updateTableFilter();
}

void KVocTrainApp::slotCurrentLessonChanged(int currentLesson)
{
    Q_UNUSED(currentLesson);
    updateTableFilter();
}

void KVocTrainApp::slotLessonCheckboxesChanged(const QModelIndex &, const QModelIndex &)
{
    updateTableFilter();
    // I'd rather have this access m_doc->lessonsInQuery() when a query is started.
    // this could make resume query act weird.
    /// @todo look into this
    m_queryManager->getKVTQuery().setLessonItems(m_doc->lessonsInQuery());
}


void KVocTrainApp::slotLearningMapperTriggered(const QString & mapString)
{
    int cmd     = mapString.mid(0, 3).toInt(); // type of query
    int fromTranslation = mapString.mid(3, 3).toInt(); // from language
    int toTranslation = mapString.mid(6, 3).toInt(); // to language

    m_queryManager->query(cmd, toTranslation, fromTranslation);

    slotStatusMsg(IDS_DEFAULT);
}

void KVocTrainApp::slotConfigShowSearch()
{
    if (m_searchWidget) {
        m_searchWidget->setVisible(m_searchWidget->isHidden());
        Prefs::setShowSearch(m_searchWidget->isVisible());
    }
}

#include "kvoctrain.moc"
