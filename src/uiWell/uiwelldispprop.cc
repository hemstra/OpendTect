/*+
________________________________________________________________________

 CopyRight:	(C) dGB Beheer B.V.
 Author:	Bruno
 Date:		Dec 2008
________________________________________________________________________

-*/
static const char* rcsID = "$Id: uiwelldispprop.cc,v 1.5 2008-12-17 13:08:34 cvsbruno Exp $";

#include "uiwelldispprop.h"

#include "uibutton.h"
#include "uicolor.h"
#include "uispinbox.h"
#include "uigeninput.h"
#include "uilabel.h"
#include "uicoltabman.h"
#include "uicoltabtools.h"

#include "welldata.h"
#include "multiid.h"
#include "wellman.h"
#include "welllog.h"
#include "welllogset.h"
#include "multiid.h"


uiWellDispProperties::uiWellDispProperties( uiParent* p,
				const uiWellDispProperties::Setup& su,
				Well::DisplayProperties::BasicProps& pr )
    : uiGroup(p,"Well display properties group")
    , props_(pr)
    , propChanged(this)

{
    szfld_ = new uiSpinBox( this, 0, "Size" );
    szfld_->setInterval( StepInterval<int>(1,mUdf(int),1) );
    szfld_->valueChanging.notify( mCB(this,uiWellDispProperties,propChg) );
    new uiLabel( this, su.mysztxt_ , szfld_ );

    uiColorInput::Setup csu( props_.color_ );
    csu.lbltxt( su.mycoltxt_ ).withalpha( false );
    BufferString dlgtxt( "Select " );
    dlgtxt += su.mycoltxt_; dlgtxt += " for "; dlgtxt += props_.subjectName();
    colfld_ = new uiColorInput( this, csu, su.mycoltxt_ );
    colfld_->attach( alignedBelow, szfld_ );
    colfld_->colorchanged.notify( mCB(this,uiWellDispProperties,propChg) );

    setHAlignObj( colfld_ );
    
}

void uiWellDispProperties::propChg( CallBacker* )
{
    getFromScreen();
    propChanged.trigger();
}


void uiWellDispProperties::putToScreen()
{
    szfld_->setValue( props_.size_ );
    colfld_->setColor( props_.color_ );
    doPutToScreen();
}


void uiWellDispProperties::getFromScreen()
{
    props_.size_ = szfld_->getValue();
    props_.color_ = colfld_->color();
    doGetFromScreen();
}


uiWellTrackDispProperties::uiWellTrackDispProperties( uiParent* p,
				const uiWellDispProperties::Setup& su,
				Well::DisplayProperties::Track& tp )
    : uiWellDispProperties(p,su,tp)
{
    dispabovefld_ = new uiCheckBox( this, "Above" );
    dispabovefld_->attach( alignedBelow, colfld_ );
    dispabovefld_->activated.notify( mCB(this,uiWellTrackDispProperties,propChg) );
    dispbelowfld_ = new uiCheckBox( this, "Below" );
    dispbelowfld_->attach( rightOf, dispabovefld_ );
    dispbelowfld_->activated.notify( mCB(this,uiWellTrackDispProperties,propChg) );
    uiLabel* lbl = new uiLabel( this, "Display well name" , dispabovefld_ );
    lbl = new uiLabel( this, "track" );
    lbl->attach( rightOf, dispbelowfld_ );
}


void uiWellTrackDispProperties::doPutToScreen()
{
    dispabovefld_->setChecked( trackprops().dispabove_ );
    dispbelowfld_->setChecked( trackprops().dispbelow_ );
}


void uiWellTrackDispProperties::doGetFromScreen()
{
    trackprops().dispabove_ = dispabovefld_->isChecked();
    trackprops().dispbelow_ = dispbelowfld_->isChecked();
}


uiWellMarkersDispProperties::uiWellMarkersDispProperties( uiParent* p,
				const uiWellDispProperties::Setup& su,
				Well::DisplayProperties::Markers& mp )
    : uiWellDispProperties(p,su,mp)
{
    circfld_ = new uiGenInput( this, "Shape",
			       BoolInpSpec(true,"Circular","Square") );
    circfld_->attach( alignedBelow, colfld_ );
    circfld_->valuechanged.notify( mCB(this,uiWellMarkersDispProperties,propChg) );
}


void uiWellMarkersDispProperties::doPutToScreen()
{
    circfld_->setValue( mrkprops().circular_ );
}


void uiWellMarkersDispProperties::doGetFromScreen()
{
    mrkprops().circular_ = circfld_->getBoolValue();
}


uiWellLogDispProperties::uiWellLogDispProperties( uiParent* p,
				const uiWellDispProperties::Setup& su,
				Well::DisplayProperties::Log& lp, 
				Well::Data* d)
    : uiWellDispProperties(p,su,lp)
{
    stylefld_ = new uiGenInput( this, "Style", 
			       BoolInpSpec( true,"Well log","Seismic" ) );
    stylefld_->attach( alignedAbove, szfld_ );
    stylefld_->valuechanged.notify( mCB(this,uiWellLogDispProperties,
	                                     isSeismicSel) );
    stylefld_->valuechanged.notify( mCB(this,uiWellLogDispProperties,propChg) );

    rangefld_ = new uiGenInput( this, "data range",
			     FloatInpIntervalSpec()
			     .setName(BufferString(" range start"),0)
			     .setName(BufferString(" range stop"),1) );
    rangefld_->attach( alignedAbove, stylefld_ );
    rangefld_->valuechanged.notify( mCB(this,uiWellLogDispProperties,
	       						choiceSel) );
    rangefld_->valuechanged.notify( mCB(this,uiWellLogDispProperties,propChg) );
   
    const char* choice[] = { "cliprate", "data range", 0 };
    cliprangefld_ = new uiGenInput( this, "Specify",
				StringListInpSpec(choice) );
    cliprangefld_->attach( alignedAbove, rangefld_ );
    //cliprangefld_->setValue(true);
    cliprangefld_->valuechanged.notify( mCB(this,uiWellLogDispProperties,
							choiceSel));
   
    clipratefld_ = new uiGenInput( this, "Clip rate", StringInpSpec() );
    clipratefld_->setElemSzPol( uiObject::Small );
    clipratefld_->attach( alignedBelow, cliprangefld_ );
    clipratefld_->valuechanged.notify( mCB(this,uiWellLogDispProperties,
	       						choiceSel) );
    clipratefld_->valuechanged.notify( mCB(this,uiWellLogDispProperties,
							   propChg) );
    clipratefld_->display(false);

    logfillfld_ = new uiCheckBox( this, "log filled" );
    logfillfld_->attach( rightOf, szfld_ );   
    logfillfld_->activated.notify( mCB(this,uiWellLogDispProperties,
    							isFilledSel));
    logfillfld_->activated.notify( mCB(this,uiWellLogDispProperties,
							   propChg) );

    coltablistfld_ = new uiComboBox( this, "Table selection" );
    coltablistfld_->attach( rightOf, colfld_ );
    coltablistfld_->display( false );
    BufferStringSet allctnms;
    ColTab::SM().getSequenceNames( allctnms );
    allctnms.sort();
    for ( int idx=0; idx<allctnms.size(); idx++ )
    {
	const int seqidx = ColTab::SM().indexOf( allctnms.get(idx) );
	if ( seqidx<0 ) continue;
	const ColTab::Sequence* seq = ColTab::SM().get( seqidx );
	coltablistfld_->addItem( seq->name() );
	coltablistfld_->setPixmap( ioPixmap(*seq,16,10), idx );
    }
    coltablistfld_->selectionChanged.notify( mCB(this,uiWellLogDispProperties,
								propChg) );

    wd_ = d;
    BufferStringSet lognames;
    lognames.setIsOwner(false);
    Well::LogSet& welllog = wd_->logs();
    for ( int idx=0; idx< welllog.size(); idx++ )
    lognames.addIfNew( welllog.getLog(idx).name() );
    lognames.sort();
    BufferString sellbl( "Select log" );
    logsfld_ = new uiLabeledComboBox( this, sellbl );
    logsfld_->box()->addItem("None");
    logsfld_->box()->addItems( lognames );
    logsfld_->attach( alignedAbove, cliprangefld_ );
    logsfld_->box()->selectionChanged.notify( mCB(this, uiWellLogDispProperties,
				 		logSel));
    logsfld_->box()->selectionChanged.notify( mCB(this, uiWellLogDispProperties,
				 		updateRange));

    repeatfld_ = new uiGenInput( this, "logs number", FloatInpSpec() );
    repeatfld_->setElemSzPol( uiObject::Small );
    repeatfld_->valuechanged.notify( mCB(this,uiWellLogDispProperties,
							isRepeatSel) );
    repeatfld_->attach(alignedBelow, colfld_);
    repeatfld_->display(false);
    repeatfld_->valuechanged.notify( mCB(this,uiWellLogDispProperties,propChg));

    ovlapfld_ = new uiGenInput( this, "Overlapping",
						FloatInpSpec() );
    ovlapfld_->setElemSzPol( uiObject::Small );
    ovlapfld_->attach( rightOf, repeatfld_ );
    ovlapfld_->display(false);
    ovlapfld_->valuechanged.notify( mCB(this,uiWellLogDispProperties,propChg) );

    seiscolorfld_ = new uiColorInput( this,
		                 uiColorInput::Setup(logprops().seiscolor_)
			        .lbltxt("Filling Color") );
    seiscolorfld_->attach( rightOf, colfld_);
    seiscolorfld_->display(false);
    seiscolorfld_->colorchanged.notify( mCB(this,uiWellLogDispProperties,
								propChg) );
 
    if ( logprops().name_ = "None" ) selNone();
}


void uiWellLogDispProperties::doPutToScreen()
{
    stylefld_->setValue( logprops().iswelllog_ ); 
    rangefld_->setValue( logprops().range_ ); 
    coltablistfld_->setText( logprops().seqname_ ); 
    logfillfld_->setChecked( logprops().islogfill_ );
    clipratefld_->setValue( logprops().cliprate_ );
    repeatfld_->setValue( logprops().repeat_ );
    ovlapfld_->setValue( logprops().repeatovlap_);
    seiscolorfld_->setColor( logprops().seiscolor_);
    logsfld_-> box() -> setText( logprops().name_ );
}


void uiWellLogDispProperties::doGetFromScreen()
{
    logprops().iswelllog_ = stylefld_->getBoolValue();
    logprops().range_ = rangefld_->getFInterval();
    logprops().isdatarange_ = cliprangefld_->getBoolValue();
    logprops().islogfill_ = logfillfld_->isChecked();
    logprops().cliprate_ = clipratefld_->getfValue();
    logprops().seqname_ = coltablistfld_-> text();
    if ( stylefld_->getBoolValue() == true )
	logprops().repeat_ = 1;
    else
	logprops().repeat_ = repeatfld_->getIntValue();
    logprops().repeatovlap_ = ovlapfld_->getfValue();
    logprops().seiscolor_ = seiscolorfld_->color();
    logprops().name_ = logsfld_->box()->text();
}


void uiWellLogDispProperties::isFilledSel( CallBacker* )
{
    const bool iswelllog = stylefld_->getBoolValue();
    const bool isfilled = logfillfld_->isChecked();
    coltablistfld_->display( iswelllog && isfilled );
    seiscolorfld_->display( !iswelllog );
}


void uiWellLogDispProperties::isRepeatSel( CallBacker* )
{
    const bool isrepeat = repeatfld_->isChecked()
				&& repeatfld_->getIntValue() > 0;
    const bool noseismic = stylefld_->getBoolValue();
    if (noseismic)
	repeatfld_->setValue(1);
}


void uiWellLogDispProperties::isSeismicSel( CallBacker* )
{
    const bool iswelllog = stylefld_->getBoolValue();
    repeatfld_->display( !iswelllog );
    ovlapfld_->display( !iswelllog );
    logfillfld_->display( iswelllog );
    if (iswelllog)
	repeatfld_->setValue(1);
    BufferString str = "Select ";
    str += "filling color";
    isFilledSel(0);
}


void uiWellLogDispProperties::setRangeFields( Interval<float>& range )
{
    rangefld_->setValue( range );
    valuerange_ = range;
}

void uiWellLogDispProperties::logSel( CallBacker* )
{
    setFieldVals( false );
}


void uiWellLogDispProperties::selNone()
{
    rangefld_->setValue( Interval<float>(0,0) );
    colfld_->setColor( Color::White );
    seiscolorfld_->setColor( Color::White );
    stylefld_->setValue( true );
    setFldSensitive( false );
    cliprangefld_->setValue( true );
    clipratefld_->setValue( 0.0 );
    repeatfld_->setValue( 5 );
    ovlapfld_->setValue( 50 );
    logfillfld_->setChecked( false );
    //singlfillcolfld_->setChecked( false );
    //logscfld_->setChecked( false );
}



void uiWellLogDispProperties::setFldSensitive( bool yn )
{
    rangefld_->setSensitive( yn );
    cliprangefld_->setSensitive( yn );
    colfld_->setSensitive( yn );
    seiscolorfld_->setSensitive( yn );
    stylefld_->setSensitive( yn );
    clipratefld_->setSensitive( yn );
    repeatfld_->setSensitive( yn );
    logfillfld_->setSensitive( yn );
    coltablistfld_->setSensitive( yn );
    logfillfld_->setSensitive( yn );
    szfld_->setSensitive( yn );
}


void uiWellLogDispProperties::choiceSel( CallBacker* )
{
    const int choiceclp = cliprangefld_->getIntValue();
    rangefld_->display( choiceclp );
    clipratefld_->display( !choiceclp );
}


void uiWellLogDispProperties::setFieldVals( bool def )
{
    BufferString sel = logsfld_->box()->text();
    if ( sel == "None")
    {
	selNone();
	return;
    }

    setFldSensitive( true );
}



void uiWellLogDispProperties::updateRange( CallBacker* )
{
	Well::LogSet& welllog = wd_->logs();
	const char* lognm = logsfld_->box()->textOfItem(
			    logsfld_->box()->currentItem() );;
	const int logno = welllog.indexOf( lognm );
	if ( logno < 0 )
	return;

	Interval<float> range; 
	range = welllog.getLog(logno).valueRange();
	setRangeFields( range );
	propChanged.trigger();
	return;

    calcLogValueRange();

    if ( mIsUdf(valuerange_.start) || mIsUdf(valuerange_.stop) )
	valuerange_.set(0,0);
    else
	setRangeFields( valuerange_ );

    propChanged.trigger();
}


void uiWellLogDispProperties::calcLogValueRange( )
{
    valuerange_.set( mUdf(float), -mUdf(float) );
    Well::LogSet& welllog = wd_->logs();
    const char* lognm =  logsfld_->box()->textOfItem(
	                    logsfld_->box()->currentItem() ); 
    for ( int idy=0; idy<welllog.size(); idy++ )
    {
	if ( !strcmp(lognm,welllog.getLog(idy).name()) )
	{
	    const int logno = welllog.indexOf( lognm );
	    Interval<float> range = welllog.getLog(logno).valueRange();
	    if ( valuerange_.start > range.start )
	    if ( valuerange_.stop < range.stop )
	    valuerange_.stop = range.stop;
	}
    }
}
