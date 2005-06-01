//
// C++ Implementation: kvtnewstuff
//
// Description:
//
//
// Author: Peter Hedlund <peter@peterandlinda.com>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <qdir.h>

#include <kprocess.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <knewstuff/entry.h>

#include "kvtnewstuff.h"
#include "prefs.h"

KVTNewStuff::KVTNewStuff(QWidget *parent, const char *name) : QObject(), KNewStuff("kdeedu/vocabulary", parent)
{
  //m_app = (kvoctrainApp*) parent;
}


bool KVTNewStuff::install(const QString & fileName)
{
  //m_app->loadfileFromPath(fileName, true);

  //kdDebug(5850) << "KNewStuffGeneric::install(): " << fileName << endl;
  QStringList list, list2;

//   mConfig->setGroup("KNewStuff");
//
//   QString uncompress = mConfig->readEntry( "Uncompress" );
//   if ( !uncompress.isEmpty() ) {
//     kdDebug(5850) << "Uncompression method: " << uncompress << endl;
//     KTar tar(fileName, uncompress);
//     tar.open(IO_ReadOnly);
//     const KArchiveDirectory *dir = tar.directory();
//     dir->copyTo(destinationPath(0));
//     tar.close();
//     QFile::remove(fileName);
//   }

  QString cmd = Prefs::installationCommand();
  if (!cmd.isEmpty())
  {
    //kdDebug(5850) << "InstallationCommand: " << cmd << endl;
    list = QStringList::split(" ", cmd);
    for (QStringList::iterator it = list.begin(); it != list.end(); it++ )
    {
      list2 << (*it).replace("%f", fileName);
    }
    KProcess proc;
    proc << list2;
    proc.start(KProcess::DontCare);
  }

  return true;
}


bool KVTNewStuff::createUploadFile(const QString & fileName)
{
  return true;
}


QString KVTNewStuff::destinationPath(KNS::Entry * entry)
{
  if (entry)
  {
    KURL url = entry->payload();
    QString fileName = url.fileName();

    QString path = Prefs::installPath(); //default is Vocabularies which will be created in the user's home directory
    QString file;

    if (path.isEmpty())
      file = KNewStuff::downloadDestination(entry); //fall back on a temp file, should never happen
    else
    {
      file = QDir::home().path() + "/" + path + "/";
      KStandardDirs::makeDir(file); //ensure the directory exists
      file += fileName;
    }
    return file;
  }
  else
    return QString::null;
}


QString KVTNewStuff::downloadDestination(KNS::Entry * entry)
{
  QString file = destinationPath(entry);

  if (KStandardDirs::exists(file))
  {
    int result = KMessageBox::questionYesNo(parentWidget(),
        i18n("The file '%1' already exists. Do you want to overwrite it?")
        .arg(file),
        QString::null, i18n("Overwrite") );
    if (result == KMessageBox::No)
      return QString::null;
  }
  return file;
}


#include "kvtnewstuff.moc"
