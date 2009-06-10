/*+
________________________________________________________________________

 CopyRight:     (C) dGB Beheer B.V.
 Author:        Bert
 Date:          Mar 2008
________________________________________________________________________

-*/
static const char* rcsID = "$Id: uiwelllogdisplay.cc,v 1.2 2009-06-10 15:43:31 cvsbert Exp $";

#include "uiwelllogdisplay.h"
#include "welllog.h"

#include "uigraphicsscene.h"
#include "uigraphicsitemimpl.h"
#include "mouseevent.h"
#include "dataclipper.h"
#include "survinfo.h"
#include "unitofmeasure.h"
#include "linear.h"
#include <iostream>


uiWellLogDisplay::LogData::LogData( uiGraphicsScene& scn, bool isfirst )
    : wl_(0)
    , unitmeas_(0)
    , xax_(&scn,uiAxisHandler::Setup(isfirst?uiRect::Top:uiRect::Bottom))
    , yax_(&scn,uiAxisHandler::Setup(isfirst?uiRect::Left:uiRect::Right))
    , zrg_(mUdf(float),0)
    , valrg_(mUdf(float),0)
    , logarithmic_(false)
    , linestyle_(LineStyle::Solid)
    , clipratio_(0.001)
    , curvenmitm_(0)
{
    if ( !isfirst )
	yax_.setup().nogridline(true);

    const uiBorder b( 10 );
    xax_.setup().border( b );
    yax_.setup().border( b );
}


uiWellLogDisplay::uiWellLogDisplay( uiParent* p )
    : uiGraphicsView(p,"Well Log display viewer")
    , ld1_(scene(),true)
    , ld2_(scene(),false)
    , zrg_(mUdf(float),0)
    , dispzinft_(SI().zInFeet())
{
    setStretch( 2, 2 );
    getMouseEventHandler().buttonReleased.notify(
			    mCB(this,uiWellLogDisplay,mouseRelease) );

    reSize.notify( mCB(this,uiWellLogDisplay,reSized) );
    setScrollBarPolicy( true, uiGraphicsView::ScrollBarAlwaysOff );
    setScrollBarPolicy( false, uiGraphicsView::ScrollBarAlwaysOff );

    finaliseDone.notify( mCB(this,uiWellLogDisplay,init) );
}


uiWellLogDisplay::~uiWellLogDisplay()
{
}


void uiWellLogDisplay::reSized( CallBacker* )
{
    draw();
}


void uiWellLogDisplay::init( CallBacker* )
{
    dataChanged();
}


void uiWellLogDisplay::dataChanged()
{
    gatherInfo(); draw();
}


void uiWellLogDisplay::gatherInfo()
{
    setAxisRelations();
    gatherInfo( true );
    gatherInfo( false );

    if ( mIsUdf(zrg_.start) && ld1_.wl_ )
    {
	zrg_ = ld1_.zrg_;
	if ( ld2_.wl_ )
	    zrg_.include( ld2_.zrg_ );
    }
    setAxisRanges( true );
    setAxisRanges( false );

    ld1_.yax_.setup().islog( ld1_.logarithmic_ );
    ld2_.yax_.setup().islog( ld2_.logarithmic_ );

    if ( ld1_.wl_ ) ld1_.xax_.setup().name( ld1_.wl_->name() );
    if ( ld2_.wl_ ) ld2_.xax_.setup().name( ld1_.wl_->name() );
    BufferString znm( "MD ", dispzinft_ ? "(ft)" : "(m)" );
    ld1_.yax_.setup().name( znm ); ld2_.yax_.setup().name( znm );
}


void uiWellLogDisplay::setAxisRelations()
{
    ld1_.xax_.setBegin( &ld1_.yax_ );
    ld1_.yax_.setBegin( &ld2_.xax_ );
    ld2_.xax_.setBegin( &ld1_.yax_ );
    ld2_.yax_.setBegin( &ld2_.xax_ );
    ld1_.xax_.setEnd( &ld2_.yax_ );
    ld1_.yax_.setEnd( &ld1_.xax_ );
    ld2_.xax_.setEnd( &ld2_.yax_ );
    ld2_.yax_.setEnd( &ld1_.xax_ );

    ld1_.xax_.setNewDevSize( width(), height() );
    ld2_.xax_.setNewDevSize( width(), height() );
    ld1_.yax_.setNewDevSize( height(), width() );
    ld2_.yax_.setNewDevSize( height(), width() );
}


void uiWellLogDisplay::gatherInfo( bool first )
{
    uiWellLogDisplay::LogData& ld = first ? ld1_ : ld2_;

    const int sz = ld.wl_ ? ld.wl_->size() : 0;
    if ( sz < 2 )
    {
	if ( !first )
	{
	    ld2_.unitmeas_ = ld1_.unitmeas_;
	    ld2_.linestyle_ = ld1_.linestyle_;
	    ld2_.logarithmic_ = ld1_.logarithmic_;
	    ld2_.zrg_ = ld1_.zrg_;
	    ld2_.valrg_ = ld1_.valrg_;
	}
	return;
    }

    DataClipSampler dcs( sz );
    dcs.add( ld.wl_->valArr(), sz );
    ld.valrg_ = dcs.getRange( ld.clipratio_ );
    ld.zrg_.start = ld.wl_->dah( 0 );
    ld.zrg_.stop = ld.wl_->dah( sz-1 );
}


void uiWellLogDisplay::setAxisRanges( bool first )
{
    uiWellLogDisplay::LogData& ld = first ? ld1_ : ld2_;
    const int sz = ld.wl_ ? ld.wl_->size() : 0;
    if ( sz < 2 )
    {
	if ( !first )
	{
	    ld2_.xax_.setRange( ld1_.xax_.range() );
	    ld2_.yax_.setRange( ld1_.yax_.range() );
	}
	return;
    }

    Interval<float> dispvalrg( ld.valrg_ );
    if ( ld.unitmeas_ )
    {
	dispvalrg.start = ld.unitmeas_->userValue(ld.valrg_.start);
	dispvalrg.stop = ld.unitmeas_->userValue(ld.valrg_.stop);
    }
    AxisLayout alx( dispvalrg );
    if ( alx.sd.start < dispvalrg.start ) alx.sd.start += alx.sd.step;
    ld.xax_.setRange(
	    StepInterval<float>(dispvalrg.start,dispvalrg.stop,alx.sd.step),
	    &alx.sd.start );

    Interval<float> dispzrg( zrg_.start, zrg_.stop );
    if ( dispzinft_ )
	dispzrg.scale( mToFeetFactor );
    AxisLayout aly( dispzrg );
    if ( aly.sd.start < dispzrg.start ) aly.sd.start += aly.sd.step;
    ld.yax_.setRange(
	    StepInterval<float>(dispzrg.start,dispzrg.stop,aly.sd.step),
	    &aly.sd.start );
}


void uiWellLogDisplay::draw()
{
    setAxisRelations();
    if ( mIsUdf(zrg_.start) ) return;

    ld1_.xax_.plotAxis(); ld1_.yax_.plotAxis();
    ld2_.xax_.plotAxis(); ld2_.yax_.plotAxis();

    drawCurve( true );
    drawCurve( false );
}


void uiWellLogDisplay::drawCurve( bool first )
{
    uiWellLogDisplay::LogData& ld = first ? ld1_ : ld2_;
    deepErase( ld.curveitms_ );
    delete ld.curvenmitm_; ld.curvenmitm_ = 0;
    const int sz = ld.wl_ ? ld.wl_->size() : 0;
    if ( sz < 2 ) return;

    ObjectSet< TypeSet<uiPoint> > pts;
    TypeSet<uiPoint>* curpts = new TypeSet<uiPoint>;
    for ( int idx=0; idx<sz; idx++ )
    {
	float dah = ld.wl_->dah( idx );
	if ( dah < zrg_.start )
	    continue;
	else if ( dah > zrg_.stop )
	    break;

	float val = ld.wl_->value( idx );
	if ( mIsUdf(val) )
	{
	    if ( !curpts->isEmpty() )
	    {
		pts += curpts;
		curpts = new TypeSet<uiPoint>;
	    }
	    continue;
	}

	if ( dispzinft_ ) dah *= mToFeetFactor;
	if ( ld.unitmeas_ ) val = ld.unitmeas_->userValue( val );
	*curpts += uiPoint( ld.xax_.getPix(val), ld.yax_.getPix(dah) );
    }
    if ( curpts->isEmpty() )
	delete curpts;
    if ( pts.isEmpty() ) return;

    for ( int idx=0; idx<pts.size(); idx++ )
    {
	uiPolyLineItem* pli = scene().addItem( new uiPolyLineItem(*pts[idx]) );
	pli->setPenStyle( ld.linestyle_ );
	ld.curveitms_ += pli;
    }

    Alignment al( Alignment::HCenter,
	    	  first ? Alignment::Top : Alignment::Bottom );
    ld.curvenmitm_ = scene().addItem( new uiTextItem(ld.wl_->name(),al) );
    ld.curvenmitm_->setTextColor( ld.linestyle_.color_ );
    uiPoint txtpt;
    if ( first )
	txtpt = uiPoint( (*pts[0])[0] );
    else
    {
	TypeSet<uiPoint>& lastpts( *pts[pts.size()-1] );
	txtpt = lastpts[lastpts.size()-1];
    }
    ld.curvenmitm_->setPos( txtpt );

    deepErase( pts );
}


void uiWellLogDisplay::mouseRelease( CallBacker* )
{
}
