/*+
________________________________________________________________________

 CopyRight:     (C) dGB Beheer B.V.
 Author:        A.H. Bril
 Date:          Feb 2002
 RCS:           $Id: uiodmain.cc,v 1.4 2003-12-28 16:10:23 bert Exp $
________________________________________________________________________

-*/

#include "uiodmain.h"
#include "uicmain.h"
#include "uiodapplmgr.h"
#include "uiodscenemgr.h"
#include "uiodmenumgr.h"
#include "uiviscoltabed.h"
#include "uivispartserv.h"
#include "uinlapartserv.h"
#include "uiattribpartserv.h"
#include "uidockwin.h"
#include "uisurvey.h"
#include "uiioobjsel.h"
#include "uifiledlg.h"
#include "uimsg.h"
#include "ioman.h"
#include "iodir.h"
#include "ptrman.h"
#include "ctxtioobj.h"
#include "filegen.h"
#include "settings.h"
#include "plugins.h"
#include "odsessionfact.h"
#include "dtect.xpm"

static const int cCTHeight = 200;


static uiODMain* manODMainWin( uiODMain* i )
{
    static uiODMain* theinst = 0;
    if ( i ) theinst = i;
    return theinst;
}


uiODMain* ODMainWin()
{
    return manODMainWin(0);
}


int ODMain( int argc, char** argv )
{
    PIM().setArgs( argc, argv );
    PIM().loadAuto( false );
    uiODMain* odmain = new uiODMain( *new uicMain(argc,argv) );
    manODMainWin( odmain );
    PIM().loadAuto( true );
    odmain->go();
    delete odmain;
    return 0;
}



uiODMain::uiODMain( uicMain& a )
	: uiMainWin(0,"OpendTect Main Window",3,true,true)
    	, uiapp(a)
	, failed(true)
    	, ctabed(0)
    	, ctabwin(0)
    	, lastsession(*new ODSession)
    	, cursession(0)
    	, sessionSave(this)
    	, sessionRestore(this)
{
    setIcon( dtect_xpm_data, "OpendTect" );
    uiMSG().setMainWin( this );
    uiapp.setTopLevel( this );

    if ( !ensureGoodDataDir() )
	::exit( 0 );

    applmgr = new uiODApplMgr( *this );

    if ( !ensureGoodSurveySetup() )
	::exit( 0 );

    if ( buildUI() )
	failed = false;
}


uiODMain::~uiODMain()
{
    delete ctabed;
    delete &lastsession;
}


bool uiODMain::ensureGoodDataDir()
{
    if ( !GetDataDir() )
    {
	uiMSG().message(
	"OpendTect needs a place to store its data.\n"
	"You have not yet specified a location for this datastore,\n"
	"and there is no 'DTECT_DATA or dGB_DATA' set in your environment.\n\n"
	"Please specify where the OpendTect Data Directory should\n"
	"be created or select an existing OpendTect data directory."
#ifndef __win__
	"\n\nNote that you can still put surveys and "
	"individual cubes on other disks;\nbut this is where the "
	"'base' data store will be."
#endif
	);

	BufferString dirnm = GetPersonalDir();
	while ( true )
	{
	    uiFileDialog dlg( this, uiFileDialog::DirectoryOnly, dirnm, "*;*.*",
			      "Specify the directory for the OpendTect data" );
	    if ( !dlg.go() )
		return false;
	    dirnm = dlg.fileName();

	    if ( !File_exists(dirnm) )
	    {
		uiMSG().error( "Selected directory does not exist" );
		continue;
	    }
	    else if ( !File_isDirectory(dirnm) )
	    {
		uiMSG().error( "Please select a directory" );
		continue;
	    }

	    BufferString omfnm = File_getFullPath( dirnm, ".omf" );
	    if ( File_exists(omfnm) ) break;

	    BufferString oddatdir = File_getFullPath( dirnm, "OpendTectData" );
	    if ( File_exists(oddatdir) )
	    {
		if ( !File_isDirectory( oddatdir ) )
		{
		    BufferString msg( "A file \"" );
		    msg += oddatdir; 
		    msg += "\" exists, but it is not a directory.";
		    uiMSG().error( "" );
		    continue;
		}
	    }
	    else if ( !File_createDir( oddatdir, 0 ) )
	    {
		BufferString msg = "Could not create directory \"";
		msg += oddatdir;
		msg += "\"";
		uiMSG().error( msg );
		return false;
	    }

	    dirnm = oddatdir; 

	    omfnm = File_getFullPath( dirnm, ".omf" );
	    if ( File_exists(omfnm) ) break;

	    BufferString datomf( GetDataFileName(".omf") );
	    if ( !File_exists(datomf) )
	    {
		BufferString msg ( "Source .omf file \"" ); 
		msg += datomf;
		msg += "\" does not exist.";
		msg += "\nCannot create OpendTect Data directory without it.";
		uiMSG().error( msg );
		continue;
	    }

	    if ( !File_exists(omfnm) )
		File_copy( datomf, omfnm, NO );
	    if ( !File_exists(omfnm) )
	    {
		uiMSG().error( "Couldn't write on selected directory" );
		continue;
	    }
	    if ( !getenv( "DTECT_DEMO_SURVEY") ) break;

	    const BufferString surveynm( getenv( "DTECT_DEMO_SURVEY" ) );
	    const BufferString todir( File_getFullPath( dirnm,
				      File_getFileName(surveynm) ) );

	    if ( !File_isDirectory(surveynm) || File_exists(todir) ) break;

	    File_copy( surveynm, todir, YES );

	    break;
	}

	Settings::common().set( "Default DATA directory", dirnm );
    }

    return true;
}


bool uiODMain::ensureGoodSurveySetup()
{
    BufferString errmsg;
    if ( !IOMan::validSurveySetup(errmsg) )
    {
	cerr << errmsg << endl;
	uiMSG().error( errmsg );
	return false;
    }
    else if ( IOM().dirPtr()->key() == MultiID("-1") )
    {
	while ( !applmgr->manageSurvey() )
	{
	    if ( uiMSG().askGoOn( "No survey selected. Do you wish to quit?" ) )
		return false;
	}
    }

    return true;
}


bool uiODMain::buildUI()
{
    menumgr = new uiODMenuMgr( this );
    scenemgr = new uiODSceneMgr( this );

    ctabwin = new uiDockWin( this, "Color Table" );
    moveDockWindow( *ctabwin, uiMainWin::Left );
    ctabwin->setResizeEnabled( true );
							    
    ctabed = new uiVisColTabEd( ctabwin );
    ctabed->setPrefHeight( cCTHeight );
    ctabed->attach(hCentered);

    return true;
}


IOPar& uiODMain::sessionPars()
{
    return cursession->pluginpars();
}


CtxtIOObj* uiODMain::getUserSessionIOData( bool restore )
{
    CtxtIOObj* ctio = mMkCtxtIOObj(ODSession);
    ctio->ctxt.forread = restore;
    uiIOObjSelDlg dlg( this, *ctio );
    if ( !dlg.go() )
	{ delete ctio->ioobj; delete ctio; ctio = 0; }
    return ctio;
}


bool uiODMain::hasSessionChanged()
{
    ODSession sess;
    cursession = &sess;
    updateSession();
    cursession = &lastsession;
    return !( sess == lastsession );
}


void uiODMain::saveSession()
{
    PtrMan<CtxtIOObj> ctio = getUserSessionIOData( false );
    if ( !ctio ) return;
    ODSession sess; cursession = &sess;
    if ( !updateSession() ) return;
    BufferString bs;
    if ( !ODSessionTranslator::store(sess,ctio->ioobj,bs) )
	{ uiMSG().error( bs ); return; }

    lastsession = sess; cursession = &lastsession;
}


void uiODMain::restoreSession()
{
    PtrMan<CtxtIOObj> ctio = getUserSessionIOData( true );
    if ( !ctio ) return;
    ODSession sess; BufferString bs;
    if ( !ODSessionTranslator::retrieve(sess,ctio->ioobj,bs) )
	{ uiMSG().error( bs ); return; }

    cursession = &sess;
    doRestoreSession();
    cursession = &lastsession; lastsession.clear();
    updateSession();
}


bool uiODMain::updateSession()
{
    cursession->clear();
    applMgr().visServer()->fillPar( cursession->vispars() );
    applMgr().attrServer()->fillPar( cursession->attrpars() );
    sceneMgr().getScenePars( cursession->scenepars() );
    if ( applMgr().nlaServer()
      && !applMgr().nlaServer()->fillPar( cursession->nlapars() ) ) 
	return false;

    sessionSave.trigger();
    return true;
}


void uiODMain::doRestoreSession()
{
    sceneMgr().cleanUp( false );
    applMgr().resetServers();

    if ( applMgr().nlaServer() )
	applMgr().nlaServer()->usePar( cursession->nlapars() );
    applMgr().attrServer()->usePar( cursession->attrpars() );
    bool visok = applMgr().visServer()->usePar( cursession->vispars() );

    if ( visok ) 
	sceneMgr().useScenePars( cursession->scenepars() );
    else
    {
	uiMSG().error( "An error occurred while reading session file.\n"
		       "A new scene will be launched" );	
	sceneMgr().cleanUp( true );
    }
    sessionRestore.trigger();
}


bool uiODMain::go()
{
    if ( failed ) return false;

    show();
    uiSurvey::updateViewsGlobal();
    int rv = uiapp.exec();
    delete applmgr; applmgr = 0;
    return rv ? false : true;
}


bool uiODMain::closeOk( CallBacker* )
{
    if ( failed ) return true;

    menumgr->storePositions();
    scenemgr->storePositions();
    ctabwin->storePosition();

    int res = hasSessionChanged()
	    ? uiMSG().askGoOnAfter( "Do you want to save this session?" )
            : (int)!uiMSG().askGoOn( "Do you want to quit?" ) + 1;

    if ( res == 0 )
	saveSession();
    else if ( res == 2 )
	return false;

    return true;
}


void uiODMain::exit()
{
    if ( !closeOk(0) ) return;

    uiapp.exit(0);
}
