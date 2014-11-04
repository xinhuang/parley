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

#include "collection.h"

#include <QTimer>
//#include <QtGui/QPrinter>
//#include <QtGui/QPrintDialog>

// KDE
//#include <KFileDialog>
//#include <KRecentFilesAction>
//#include <KStandardDirs>
//#include <knewstuff3/downloaddialog.h>
//#include <knewstuff3/uploaddialog.h>
#include <KEMailSettings>
//#include <KMessageBox>
//#include <KProcess>
//#include <KTempDir>
#include <KGlobal>
#include <KLocale>

// KEduVocDocument library
#include <keduvoclesson.h>
#include <keduvocleitnerbox.h>
#include <keduvocexpression.h>
#include <keduvocwordtype.h>

#if 0
#ifdef HAVE_LIBXSLT
#include "exportdialog.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#endif
#endif

// Parley
#include "../config-parley.h"
//#include "parleymainwindow.h"
//#include "editor/editor.h"
//#include "version.h"
#include "prefs.h"

//#include "vocabularyview.h"
//#include "settings/documentproperties.h"
//#include "dashboard/dashboard.h"

//#include "settings/languageproperties.h"
//#include "settings/documentproperties.h"


void fetchGrammar(KEduVocDocument* doc, int languageIndex)
{
    QString locale = doc->identifier(languageIndex).locale();

    KUrl location(QString("http://edu.kde.org/parley/locale/") + locale + QString(".kvtml"));

    KEduVocDocument grammarDoc;
    if (grammarDoc.open(location) == KEduVocDocument::NoError) {
        doc->identifier(languageIndex).setArticle(grammarDoc.identifier(0).article());
        doc->identifier(languageIndex).setPersonalPronouns(grammarDoc.identifier(0).personalPronouns());
        // @todo        m_doc->identifier(index).setDeclension(grammarDoc.identifier(0).declension());
        doc->identifier(languageIndex).setTenseList(grammarDoc.identifier(0).tenseList());
    } else {
        kDebug() << "Download of " << location.url() << " failed.";
    }
}

Collection::Collection(KEduVocDocument *doc, QObject* parent)
    : QObject(parent)
    , m_doc(doc)
    , m_backupTimer(0)
{
}

Collection::Collection(KUrl *url, QObject* parent)
    : QObject(parent)
    , m_doc(new KEduVocDocument(this))
    , m_backupTimer(0)
{
    // We ignore file locks here because we open the file for readonly
    // purposes only.
    m_doc->open(*url, KEduVocDocument::FileIgnoreLock);
}

Collection::~Collection()
{
    close();

    delete m_backupTimer;
}


KEduVocDocument *Collection::eduVocDocument()
{
    // If there is no present vocabulary document, create an empty one.
    if (!m_doc) {
        m_doc = new KEduVocDocument();
    }

    return m_doc;
}

void Collection::setTitle(const QString& title)
{
    m_doc->setTitle(title);
}



void Collection::close()
{
    // NOTE: No saving here because at this point the Collection class is a
    //       read-only wrapper around KEduVocDocument.

    enableAutoBackup(false);
    if (m_doc) {
        emit documentChanged(0);
        m_doc->deleteLater();
        m_doc = 0;
    }
}


void Collection::setDefaultDocumentProperties(KEduVocDocument *doc)
{
    KEMailSettings emailSettings;
    emailSettings.setProfile(emailSettings.defaultProfileName());
    doc->setAuthor(emailSettings.getSetting(KEMailSettings::RealName));
    doc->setAuthorContact(emailSettings.getSetting(KEMailSettings::EmailAddress));

    doc->setLicense(i18n("Public Domain"));
    doc->setCategory(i18n("Languages"));

    QString locale = KGlobal::locale()->language();

    doc->appendIdentifier();
    doc->appendIdentifier();
    doc->identifier(0).setName(KGlobal::locale()->languageCodeToName(locale));
    doc->identifier(0).setLocale(locale);
    doc->identifier(1).setName(i18n("A Second Language"));
    doc->identifier(1).setLocale(locale);

    KEduVocLesson* lesson = new KEduVocLesson(i18n("Lesson 1"), doc->lesson());
    doc->lesson()->appendChildContainer(lesson);

    // add some entries
    for (int i = 0; i < 15 ; i++) {
        lesson->appendEntry(new KEduVocExpression());
    }

    doc->setModified(false);
}

void Collection::initializeDefaultGrammar(KEduVocDocument *doc)
{
    KEduVocWordType *root = doc->wordTypeContainer();
    KEduVocWordType *noun = new KEduVocWordType(i18n("Noun"), root);
    noun->setWordType(KEduVocWordFlag::Noun);
    root->appendChildContainer(noun);

    KEduVocWordType *nounChild = new KEduVocWordType(i18n("Masculine"), noun);
    nounChild->setWordType(KEduVocWordFlag::Noun | KEduVocWordFlag::Masculine);
    noun->appendChildContainer(nounChild);
    nounChild = new KEduVocWordType(i18n("Feminine"), noun);
    nounChild->setWordType(KEduVocWordFlag::Noun | KEduVocWordFlag::Feminine);
    noun->appendChildContainer(nounChild);
    nounChild = new KEduVocWordType(i18n("Neuter"), noun);
    nounChild->setWordType(KEduVocWordFlag::Noun | KEduVocWordFlag::Neuter);
    noun->appendChildContainer(nounChild);

    KEduVocWordType *verb = new KEduVocWordType(i18n("Verb"), root);
    verb->setWordType(KEduVocWordFlag::Verb);
    root->appendChildContainer(verb);

    KEduVocWordType *adjective = new KEduVocWordType(i18n("Adjective"), root);
    adjective->setWordType(KEduVocWordFlag::Adjective);
    root->appendChildContainer(adjective);

    KEduVocWordType *adverb = new KEduVocWordType(i18n("Adverb"), root);
    adverb->setWordType(KEduVocWordFlag::Adverb);
    root->appendChildContainer(adverb);

    KEduVocWordType *conjunction = new KEduVocWordType(i18n("Conjunction"), root);
    conjunction->setWordType(KEduVocWordFlag::Conjunction);
    root->appendChildContainer(conjunction);
}


void Collection::enableAutoBackup(bool enable)
{
    if (!enable) {
        if (m_backupTimer) {
            m_backupTimer->stop();
        }
    } else {
        if (!m_backupTimer) {
            m_backupTimer = new QTimer(this);
            connect(m_backupTimer, SIGNAL(timeout()), this, SLOT(save()));
        }
        m_backupTimer->start(Prefs::backupTime() * 60 * 1000);
    }
}
