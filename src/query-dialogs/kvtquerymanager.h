/***************************************************************************

    C++ Interface: kvtquerymanager

    -----------------------------------------------------------------------

    begin         : So Mai 27 2007

    copyright     : (C) 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

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


#ifndef KVTQUERYMANAGER_H
#define KVTQUERYMANAGER_H

#include "kvtquery.h"
#include "QueryDlgBase.h"
#include <QString>

class KVocTrainApp;
class KEduVocDocument;

class SimpleQueryDlg;
class MCQueryDlg;
class VerbQueryDlg;
class RandomQueryDlg;
class AdjQueryDlg;
class ArtQueryDlg;

class QueryManager :public QObject
{
Q_OBJECT

public:
    QueryManager(KVocTrainApp *app, KEduVocDocument *doc);

    void startPractice();

public slots:
    void slotQueryExpressionResult(QueryDlgBase::Result res);
    void slotTimeOutType(QueryDlgBase::Result res);
    void slotTimeOutProperty(QueryDlgBase::Result res);
    void slotEditEntry(int row, int translation);

signals:
    void endPractice();

private:
    // merge these three:
    void startTypeQuery();
    void startPropertyQuery();
    void startQuery();

    bool queryIsEmpty();

    /** exit query mode */
    void stopQuery();

    TestEntryManager    *m_entryManager;
    KEduVocDocument     *m_doc;
    KVocTrainApp        *m_app;

    int     m_fromTranslation;
    int     m_toTranslation;

    QueryDlgBase        *m_testDialog;

    SimpleQueryDlg      *simpleQueryDlg;
    MCQueryDlg          *mcQueryDlg;
    VerbQueryDlg        *verbQueryDlg;
//     RandomQueryDlg      *randomQueryDlg;
    AdjQueryDlg         *adjQueryDlg;
    ArtQueryDlg         *artQueryDlg;
    int                  num_queryTimeout;
    int                  m_testType;
};

#endif
