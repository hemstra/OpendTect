/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:        Bert
 Date:          Mar 2011
________________________________________________________________________

-*/
static const char* rcsID = "$Id: uisegytrchdrvalplot.cc,v 1.2 2011-03-11 11:20:44 cvsbert Exp $";

#include "uisegytrchdrvalplot.h"
#include "uifunctiondisplay.h"
#include "uilabel.h"
#include "segyhdrdef.h"
#include "statruncalc.h"
#include "bendpointfinder.h"


uiSEGYTrcHdrValPlot::uiSEGYTrcHdrValPlot( uiParent* p, bool sh )
    : uiGroup(p,"Trace header values plot")
    , issingle_(sh)
    , tlbl2_(0)
    , slbl2_(0)
{
    tlbl1_ = new uiLabel( this, "" );
    tlbl1_->setStretch( 2, 0 ); tlbl1_->setAlignment( Alignment::HCenter );
    if ( !issingle_ )
    {
	tlbl2_ = new uiLabel( this, "" ); tlbl2_->attach( ensureBelow, tlbl1_ );
	tlbl2_->setStretch( 2, 0 ); tlbl2_->setAlignment( Alignment::HCenter );
    }

    uiFunctionDisplay::Setup fdsu; fdsu.pointsz( 3 ).drawscatter( true );
    disp_ = new uiFunctionDisplay( this, fdsu );
    disp_->attach( centeredBelow, tlbl2_ ? tlbl2_ : tlbl1_ );
    tlbl1_->attach( widthSameAs, disp_ );
    if ( tlbl2_ )
	tlbl2_->attach( widthSameAs, disp_ );

    slbl1_ = new uiLabel( this, "" );
    slbl1_->attach( ensureBelow, disp_ ); slbl1_->attach( widthSameAs, disp_ );
    slbl1_->setStretch( 2, 0 ); slbl1_->setAlignment( Alignment::HCenter );
    if ( !issingle_ )
    {
	slbl2_ = new uiLabel( this, "" );
	slbl2_->attach( ensureBelow, slbl1_ );
	slbl2_->attach( widthSameAs, disp_ );
	slbl2_->setStretch( 2, 0 ); slbl2_->setAlignment( Alignment::HCenter );
    }
}


uiSEGYTrcHdrValPlot::~uiSEGYTrcHdrValPlot()
{
}


void uiSEGYTrcHdrValPlot::setData( const SEGY::HdrEntry& he,
				   const float* data, int sz, bool first )
{
    if ( issingle_ && !first ) return;

    (first ? tlbl1_ : tlbl2_)->setText(
	    BufferString(he.name()," (",he.description()).add(")") );

    Stats::RunCalcSetup rcsu( false );
    rcsu.require( Stats::Min ).require( Stats::Max );
    Stats::RunCalc<float> rc( rcsu );
    rc.addValues( sz, data );
    const Interval<int> rg( mNINT(rc.min()), mNINT(rc.max()) );
    const bool alleq = rg.start == rg.stop;

    BufferString lbltxt( alleq ? (issingle_ ? "" : he.name())
	    		       : (issingle_ ? "Range" : he.name()) );
    if ( !lbltxt.isEmpty() ) lbltxt += ": ";
    if ( alleq )
	lbltxt.add( "All values are: " ).add( rg.start );
    else
	lbltxt.add( "[" ).add( rg.start ).add( "," ).add( rg.stop ).add( "]" );
    lbltxt.add( " (N=" ).add( sz ).add( ")" );
    (first ? slbl1_ : slbl2_)->setText( lbltxt );

    getBendPoints( data, sz );
    if ( first )
	disp_->setVals( xvals_.arr(), yvals_.arr(), xvals_.size() );
    else
	disp_->setY2Vals( xvals_.arr(), yvals_.arr(), xvals_.size() );
}


void uiSEGYTrcHdrValPlot::getBendPoints( const float* inp, int sz )
{
    TypeSet<Coord> coords;
    for ( int idx=0; idx<sz; idx++ )
	coords += Coord( idx+1, inp[idx] );

    BendPointFinder2D bpf( coords, 0.1 );
    bpf.execute();

    for ( int idx=0; idx<bpf.bendPoints().size(); idx++ )
    {
	const Coord& coord = coords[ bpf.bendPoints()[idx] ];
	xvals_ += mNINT(coord.x);
	yvals_ += mNINT(coord.y);
    }
}
