/*+
________________________________________________________________________

 CopyRight:     (C) de Groot-Bril Earth Sciences B.V.
 Author:        Nanne Hemstra
 Date:          June 2001
 RCS:           $Id: uisurvey.cc,v 1.30 2002-11-15 13:51:48 nanne Exp $
________________________________________________________________________

-*/

#include "uisurvey.h"
#include "dirlist.h"
#include "filegen.h"
#include "genc.h"
#include "ioman.h"
#include "survinfoimpl.h"
#include "tuthandle.h"
#include "uibutton.h"
#include "uicanvas.h"
#include "uiconvpos.h"
#include "uigroup.h"
#include "uilabel.h"
#include "uilistbox.h"
#include "uimsg.h"
#include "uiseparator.h"
#include "uisurvinfoed.h"
#include "uisurvmap.h"
#include "uitextedit.h"
#include "uimain.h"
#include "strmprov.h"

#include <math.h>

extern "C" const char* GetSurveyName();
extern "C" const char* GetSurveyFileName();
extern "C" const char* GetBaseDataDir();
extern "C" void SetSurveyName(const char*);

uiSurvey::uiSurvey( uiParent* p )
	: uiDialog(p,uiDialog::Setup("Survey selection",
		   "Select and setup survey","0.3.1"))
{
    SurveyInfo::produceWarnings( false );
    const int lbwidth = 250;
    const int mapwdth = 300;
    const int maphght = 300;
    const int noteshght = 5;
    const int totwdth = lbwidth + mapwdth + 10;

    const char* ptr = GetBaseDataDir();
    if ( !ptr ) return;

    dirlist = new DirList( ptr, 1 );
    dirlist->sort();
    ptr = GetSurveyName();
    if ( ptr )
    {
        UserIDObject* uidobj = (*dirlist)[ptr];
        if ( uidobj ) dirlist->setCurrent( uidobj );
    }

    uiGroup* selgrp = new uiGroup( this, "Survey selection" );
    mapcanvas = new uiCanvas( selgrp, "Survey map" );
    mapcanvas->setPrefHeight( maphght );
    mapcanvas->setPrefWidth( mapwdth );
    mapcanvas->setStretch( 0, 0 );
    mapcanvas->preDraw.notify( mCB(this,uiSurvey,doCanvas) );
    listbox = new uiListBox( selgrp, dirlist );
    listbox->selectionChanged.notify( mCB(this,uiSurvey,selChange) );
    listbox->doubleClicked.notify( mCB(this,uiSurvey,accept) );
    listbox->attach( leftOf, mapcanvas );
    listbox->setPrefWidth( lbwidth );
    listbox->attach( heightSameAs, mapcanvas );
    listbox->setStretch( 1, 0 );

    newbut = new uiPushButton( selgrp, "New ..." );
    newbut->activated.notify( mCB(this,uiSurvey,newButPushed) );
    newbut->attach( alignedBelow, listbox );
    rmbut = new uiPushButton( selgrp, "Remove ..." );
    rmbut->activated.notify( mCB(this,uiSurvey,rmButPushed) );
    rmbut->attach( rightAlignedBelow, listbox );
    editbut = new uiPushButton( selgrp, "Edit ..." );
    editbut->activated.notify( mCB(this,uiSurvey,editButPushed) );
    editbut->attach( alignedBelow, mapcanvas );
    convbut = new uiPushButton( selgrp, "X/Y <-> I/C ..." );
    convbut->activated.notify( mCB(this,uiSurvey,convButPushed) );
    convbut->attach( rightAlignedBelow, mapcanvas );
    uiButton* tutbut = 0;
    if ( GetDgbApplicationCode() == mDgbApplCodeGDI )
    {
	tutbut = new uiPushButton( selgrp, "Get Tutorial" );
	tutbut->attach( centeredBelow, listbox );
	tutbut->activated.notify( mCB(this,uiSurvey,tutButPushed) );
    }

    uiSeparator* horsep1 = new uiSeparator( this );
    horsep1->setPrefWidth( totwdth );
    horsep1->attach( stretchedBelow, selgrp, -2 );

    uiGroup* infogrp = new uiGroup( this, "Survey information" );
    infogrp->setFont( uiFontList::get(FontData::key(FontData::ControlSmall)) ); 
    infogrp->attach( alignedBelow, selgrp );
    infogrp->attach( ensureBelow, horsep1 );
    infogrp->setPrefWidth( totwdth );
    uiLabel* irange1 = new uiLabel( infogrp, "In-line range:" );
    uiLabel* xrange1 = new uiLabel( infogrp, "Cross-line range:" );
    uiLabel* zrange1 = new uiLabel( infogrp, "Time range (s):" );
    uiLabel* binsize1 = new uiLabel( infogrp, "Bin size (m/line):" );
    xrange1->attach( alignedBelow, irange1 );
    zrange1->attach( rightOf, irange1, 225 );
    binsize1->attach( alignedBelow, zrange1 );

    irange2 = new uiLabel( infogrp, "" );
    irange2->setHSzPol( uiObject::medium );
    xrange2 = new uiLabel( infogrp, "" );
    xrange2->setHSzPol( uiObject::medium );
    zrange2 = new uiLabel( infogrp, "" );
    zrange2->setHSzPol( uiObject::medium );
    binsize2 = new uiLabel( infogrp, "" );
    binsize2->setHSzPol( uiObject::medium );
    irange2->attach( rightOf, irange1 );
    xrange2->attach( rightOf, xrange1 );
    zrange2->attach( rightOf, zrange1 );
    binsize2->attach( rightOf, binsize1 );
   
    uiSeparator* horsep2 = new uiSeparator( this );
    horsep2->attach( stretchedBelow, infogrp, -2 );
    horsep2->setPrefWidth( totwdth );

    uiLabel* notelbl = new uiLabel( this, "Notes:" );
    notelbl->attach( alignedBelow, horsep2 );
    notes = new uiTextEdit( this, "Notes" );
    notes->attach( alignedBelow, notelbl);
    notes->setPrefHeightInChar( noteshght );
    notes->setPrefWidth( totwdth );
   
    getSurvInfo(); 
    mkInfo();
    setOkText( "Select" );
}


uiSurvey::~uiSurvey()
{
    delete dirlist;
    delete mapcanvas;
    delete survinfo;
    delete survmap;
}


void uiSurvey::newButPushed( CallBacker* )
{
    ioDrawTool& dt = *mapcanvas->drawTool();
    dt.beginDraw(); dt.clear(); dt.endDraw();

    BufferString oldnm = listbox->getText();
  
    BufferString from( GetSoftwareDir() );
    from = File_getFullPath( GetSoftwareDir(), "data" );
    from = File_getFullPath( from, "BasicSurvey" );
    delete survinfo;
    survinfo = SurveyInfo::read( from );
    survinfo->dirname = "";
    mkInfo();
    if ( !survInfoDialog() )
	updateInfo(0);

    rmbut->setSensitive(true);
    editbut->setSensitive(true);
    convbut->setSensitive(true);
}


void uiSurvey::editButPushed( CallBacker* )
{
    if ( !survInfoDialog() )
	updateInfo(0);
}


bool uiSurvey::survInfoDialog()
{
    BufferString selnm( listbox->getText() );
    BufferString dgbdata( GetBaseDataDir() );

    uiSurveyInfoEditor dlg( this, survinfo, mCB(this,uiSurvey,updateInfo) );
    if ( !dlg.go() ) return false;

    bool doupd = true;
    if ( dlg.dirnmChanged() )
    {
        const char* newnm = dlg.dirName();
        if ( *newnm )
        {
            BufferString newfname(File_getFullPath( dgbdata, newnm ));
            if ( File_exists(newfname) )
            {
                BufferString errmsg( "Cannot rename directory:\n" );
                errmsg += newfname;
                errmsg += "\nexists";
                ErrMsg( errmsg );
            }
            else
            {
                BufferString fname(File_getFullPath( dgbdata, selnm ));
                File_rename( fname, newfname );
                updateSvyList(); doupd = false;
                updateSvyFile();
                selnm = newnm;
            }
        }
    }
    else
	selnm = dlg.dirName();

    if ( doupd ) updateSvyList();
    listbox->setCurrentItem( selnm );

    return true;
}


void uiSurvey::rmButPushed( CallBacker* )
{
    BufferString selnm( listbox->getText() );
    BufferString msg( "This will remove the entire survey:\n\t" );
    msg += selnm;
    msg += "\nAre you sure you wish to continue?";
    if ( !uiMSG().askGoOn( msg ) ) return;

    msg = File_getFullPath( GetBaseDataDir(), selnm );
    if ( !File_remove( msg, YES, YES ) )
    {
        msg += "\nnot removed properly";
        return;
    }

    updateSvyList();
    const char* ptr = GetSurveyName();
    if ( ptr && selnm == ptr )
    {
        BufferString newsel( listbox->getText() );
        writeSurveyName( newsel );
    }

}


void uiSurvey::convButPushed( CallBacker* )
{
    uiConvertPos dlg( this, survinfo );
    dlg.go();
}


void uiSurvey::tutButPushed( CallBacker* )
{
    TutHandling tuthl;
    if ( !tuthl.copyTut() ) return;

    updateSvyList();
    tuthl.fillTut();

    rmbut->setSensitive(true);
    editbut->setSensitive(true);
    convbut->setSensitive(true);
}


void uiSurvey::updateSvyList()
{
    dirlist->update();
    dirlist->sort();
    if ( !dirlist->size() ) updateInfo(0);
    listbox->empty();
    listbox->addItems( dirlist );
}


bool uiSurvey::updateSvyFile()
{
    const char* ptr = GetSurveyName();
    if ( ptr ) ptr = File_getFileName( ptr );
    BufferString seltxt( listbox->getText() );
    if ( seltxt == "" ) return true;

    if ( (!ptr || seltxt != ptr ) && !writeSurveyName( seltxt ) )
    {
        ErrMsg( "Cannot update the .dgbSurvey file in your $HOME" );
        return false;
    }
    FileNameString fname( File_getFullPath(GetBaseDataDir(),seltxt) );
    if ( !File_exists(fname) )
    {
        ErrMsg( "Survey directory does not exist anymore" );
        return false;
    }
    delete SurveyInfo::theinst_;
    SurveyInfo::theinst_ = SurveyInfo::read( fname );
    SetSurveyName( seltxt );

    return true;
}


bool uiSurvey::writeSurveyName( const char* nm )
{
    const char* ptr = GetSurveyFileName();
    if ( !ptr )
    {
        ErrMsg( "Error in survey system. Please check $HOME." );
        return false;
    }

    StreamData sd = StreamProvider( ptr ).makeOStream();
    if ( !sd.usable() )
    {
        BufferString errmsg = "Cannot write to ";
        errmsg += ptr;
        ErrMsg( errmsg );
        return false;
    }

    *sd.ostrm << nm;

    sd.close();
    return true;
}


void uiSurvey::mkInfo()
{
    if ( survinfo->rangeUsable() )
    {
	BufferString inlinfo = survinfo->range().start.inl;
	inlinfo += " - "; inlinfo += survinfo->range().stop.inl;
	inlinfo += "  step: "; inlinfo += survinfo->inlStep();
	
	BufferString crlinfo = survinfo->range().start.crl;
	crlinfo += " - "; crlinfo += survinfo->range().stop.crl;
	crlinfo += "  step: "; crlinfo += survinfo->crlStep();

	if ( survinfo->is3D() )
	{
	    const SurveyInfo3D& si = *(SurveyInfo3D*)survinfo;
	    double xinl = si.b2c_.getTransform(true).b;
	    double yinl = si.b2c_.getTransform(false).b;
	    double xcrl = si.b2c_.getTransform(true).c;
	    double ycrl = si.b2c_.getTransform(false).c;

	    double ibsz = double( int( 100*sqrt(xinl*xinl + yinl*yinl)+.5 ) )
			/ 100;
	    double cbsz = double( int( 100*sqrt(xcrl*xcrl + ycrl*ycrl)+.5 ) )
			/ 100;
	    BufferString bininfo = "inline: "; bininfo += ibsz;
	    bininfo += "  crossline: "; bininfo += cbsz;

	    irange2->setText( inlinfo );
	    xrange2->setText( crlinfo );
	    binsize2->setText( bininfo );
	}
    }
    else
    {
	irange2->setText( "" );
        xrange2->setText( "" );
        binsize2->setText( "" );
    }

    if ( survinfo->zRangeUsable() )
    {
	BufferString zinfo = survinfo->zRange().start;
	zinfo += " - "; zinfo += survinfo->zRange().stop;
	zinfo += "  step: "; zinfo += survinfo->zRange().step;
	zrange2->setText( zinfo );
    }
    else
	zrange2->setText( "" );

    notes->setText( survinfo->comment() );

    bool anysvy = dirlist->size();
    rmbut->setSensitive( anysvy );
    editbut->setSensitive( anysvy );
    convbut->setSensitive( anysvy );
}


void uiSurvey::selChange()
{
    writeComments();
    updateInfo(0);
}


void uiSurvey::updateInfo( CallBacker* cb )
{
    mDynamicCastGet(uiSurveyInfoEditor*,dlg,cb);
    if ( dlg )
	survinfo = dlg->getSurvInfo();
    else
	getSurvInfo();

    mkInfo();
    survmap->drawMap( survinfo );
}


void uiSurvey::writeComments()
{
    BufferString txt = notes->text();
    if ( txt == survinfo->comment() ) return;

    survinfo->setComment( txt );
    if ( !survinfo->write( GetBaseDataDir() ) )
        ErrMsg( "Failed to write survey info.\nNo changes committed." );
}


void uiSurvey::doCanvas( CallBacker* c )
{
    mDynamicCastGet(uiCanvas*,mapcanvas,c)
    if (!mapcanvas) return;
    survmap = new uiSurveyMap( mapcanvas, survinfo );
    survmap->drawMap( survinfo );
}


bool uiSurvey::rejectOK( CallBacker* )
{
    SurveyInfo::produceWarnings( true );
    return true;
}


bool uiSurvey::acceptOK( CallBacker* )
{
    writeComments();
    SurveyInfo::produceWarnings( true );
    if ( !updateSvyFile() || !IOMan::newSurvey() )
    {
	SurveyInfo::produceWarnings( false );
	return false;
    }

    updateViewsGlobal();
    return true;
}


void uiSurvey::updateViewsGlobal()
{
    BufferString capt( GetProjectVersionName() );
    const char* usr = GetSoftwareUser();
    if ( usr && *usr )
	{ capt += " ["; capt += usr; capt += "]"; }

    if ( SI().name() != "" )
    {
	capt += ": ";
	capt += SI().name();
    }

    uiMain::setTopLevelCaption( capt );
}


void uiSurvey::getSurvInfo()
{
    BufferString fname( File_getFullPath(GetBaseDataDir(), listbox->getText()));
    survinfo = SurveyInfo::read( fname );
}
