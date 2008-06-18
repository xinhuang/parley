//
// C++ Implementation: scriptdialog
//
// Description:
//
//
// Author: Avgoustinos Kadis <avgoustinos.kadis@kdemail.net>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "scriptdialog.h"

#include "scriptmanager.h"

#include <KLocale>
#include <KPluginInfo>
#include <KSharedConfig>
#include <KConfigGroup>

#include <KDebug>

void test()
{
    ScriptManager sm;
    sm.loadPlugins();
}

/**
 * Constructor Function.
 */
ScriptDialog::ScriptDialog()
        : KDialog()
{
    test();

    //Configure window
    setCaption ( i18n ( "Script Dialog" ) );
    setButtons ( Ok|Cancel );

    //Add KPluginSelector as the main widget of the dialog
    m_kps = new KPluginSelector ( 0 );
    setMainWidget ( m_kps );

    //Load available plugins
    pluginsInfoList = KPluginInfo::fromFiles ( ScriptManager::listAvailablePlugins() );

    m_kps->addPlugins ( pluginsInfoList,KPluginSelector::ReadConfigFile,i18n ( "Playlist" ),QString ( "playlist" ),KSharedConfig::openConfig ( "parleyrc" ) );
}

/**
 * Destructor. Releasing any dynamically allocated memory
 */
ScriptDialog::~ScriptDialog()
{
    delete m_kps;
}


/** Executed when user clicks OK button.
 * Saves the state of the plugins (which ones are loaded) in the config file
 * and make the necessary loads/unloads of plugins
 */
void ScriptDialog::accept()
{
    //Update KPluginInfo object changes
    m_kps->updatePluginsState();   //necessary for KPluginInfo::isPluginEnabled to work

//     foreach(KPluginInfo inf, pluginsInfoList) {
//         kDebug() << QString("Is enabled? : ") << inf.pluginName() << inf.isPluginEnabled();
//     }

    //Save changes in config file (parleyrc)
    m_kps->save();

    //Close dialog
    done ( 0 ); //not sure if I put the right return code
}