/*
___________________________________________________________________

 * COPYRIGHT: (C) dGB Beheer B.V.
 * AUTHOR   : K. Tingdahl
 * DATE     : Nov 2003
___________________________________________________________________

-*/

static const char* rcsID = "$Id: horizonadjuster.cc,v 1.56 2009-07-20 11:49:09 cvsumesh Exp $";

#include "horizonadjuster.h"

#include "arrayndimpl.h"
#include "attribdatacubes.h"
#include "attribsel.h"
#include "emhorizon.h"
#include "emhorizon2d.h"
#include "genericnumer.h"
#include "iopar.h"
#include "linear.h"
#include "mpeengine.h"
#include "samplingdata.h"
#include "survinfo.h"
#include "valseriestracker.h"

#include <math.h>

namespace MPE {


HorizonAdjuster::HorizonAdjuster( EM::Horizon& hor,
	const EM::SectionID& sid )
    : SectionAdjuster(sid)
    , horizon_(hor)
    , attribsel_(0)
    , attrdata_(0)
    , tracker_( new EventTracker )
{
    tracker_->setSimilarityWindow(
	    Interval<float>(-10*SI().zStep(), 10*SI().zStep() ) );
    tracker_->setPermittedRange(
	    Interval<float>(-5*SI().zStep(), 5*SI().zStep() ) );
    tracker_->setRangeStep( SI().zStep() );
}


HorizonAdjuster::~HorizonAdjuster()
{
    delete attribsel_;
    if ( attrdata_ ) attrdata_->unRef();
    delete tracker_;
}


int HorizonAdjuster::getNrAttributes() const
{ return 1; }


const Attrib::SelSpec* HorizonAdjuster::getAttributeSel( int idx ) const
{ return !idx ? attribsel_ : 0; }


void HorizonAdjuster::reset()
{
    if ( attrdata_ ) attrdata_->unRef();
    attrdata_ = attribsel_ ? engine().getAttribCache( *attribsel_ ) : 0;
    if ( attrdata_ ) attrdata_->ref();
}


void HorizonAdjuster::setPermittedZRange( const Interval<float>& rg )
{ tracker_->setPermittedRange( rg ); }


Interval<float> HorizonAdjuster::permittedZRange() const
{ return tracker_->permittedRange(); }


void HorizonAdjuster::setTrackByValue( bool yn )
{ tracker_->useSimilarity( !yn ); }


bool HorizonAdjuster::trackByValue() const
{ return !tracker_->usesSimilarity(); }


void HorizonAdjuster::setTrackEvent( VSEvent::Type ev )
{ tracker_->setTrackEvent( ev ); }


VSEvent::Type HorizonAdjuster::trackEvent() const
{ return tracker_->trackEvent(); }


void HorizonAdjuster::setAmplitudeThreshold( float th )
{ tracker_->setAmplitudeThreshold( th ); }


float HorizonAdjuster::amplitudeThreshold() const
{ return tracker_->amplitudeThreshold(); }


void HorizonAdjuster::setAllowedVariances( const TypeSet<float>& avs )
{ tracker_->setAllowedVariances( avs ); }


TypeSet<float>& HorizonAdjuster::getAllowedVariances()
{ return tracker_->getAllowedVariances(); }


void HorizonAdjuster::setAllowedVariance( float v )
{ tracker_->setAllowedVariance( v ); }


float HorizonAdjuster::allowedVariance() const
{ return tracker_->allowedVariance(); }


void HorizonAdjuster::setUseAbsThreshold( bool abs )
{ tracker_->setUseAbsThreshold( abs ); }


bool HorizonAdjuster::useAbsThreshold() const
{ return tracker_->useAbsThreshold(); }


void HorizonAdjuster::setSimilarityWindow( const Interval<float>& rg )
{ tracker_->setSimilarityWindow( rg ); }


Interval<float> HorizonAdjuster::similarityWindow() const
{ return tracker_->similarityWindow(); }


void HorizonAdjuster::setSimilarityThreshold( float th )
{ tracker_->setSimilarityThreshold( th ); }


float HorizonAdjuster::similarityThreshold() const
{ return tracker_->similarityThreshold(); }


int HorizonAdjuster::nextStep()
{
    if ( !attrdata_ || !attrdata_->nrCubes() )
	return ErrorOccurred();

    int count = 0;
    for ( int idx=0; idx<pids_.size(); idx++ )
    {
	BinID targetbid;
	targetbid.setSerialized( pids_[idx] );
	float targetz;
	bool res;
	if ( pidsrc_.size() > idx )
	{
	    BinID refbid;
	    refbid.setSerialized( pidsrc_[idx] );
	    res = track( refbid, targetbid, targetz );
	}
	else
	{
	    const bool wasusingsim = tracker_->usesSimilarity();
	    tracker_->useSimilarity( false );
	    res = track( BinID(-1,-1), targetbid, targetz );
	    tracker_->useSimilarity( wasusingsim );
	}

	if ( res )
	    setHorizonPick( targetbid, targetz );
	else if ( removeonfailure_ )
	    setHorizonPick(targetbid, mUdf(float) );
    }

    return Finished();
}


bool HorizonAdjuster::track( const BinID& from, const BinID& to,
			     float& targetz) const 
{
    const Array3D<float>& cube = attrdata_->getCube(0);
    const ValueSeries<float>* storage = cube.getStorage();
    if ( !storage ) return false; 

    const int zsz = attrdata_->getZSz();

    const SamplingData<double> sd( attrdata_->z0*attrdata_->zstep,
	    			   attrdata_->zstep );

    const int toinlidx = 
	attrdata_->inlsampling.nearestIndex(attrDataBinId(to).inl);
    if ( toinlidx<0 || toinlidx>=attrdata_->getInlSz() )
	return false;

    const int tocrlidx = 
	attrdata_->crlsampling.nearestIndex(attrDataBinId(to).crl);
    if ( tocrlidx<0 || tocrlidx>=attrdata_->getCrlSz() )
	return false;

    const OffsetValueSeries<float> toarr( 
		    const_cast<ValueSeries<float>&>(*storage), 
		    cube.info().getOffset( toinlidx, tocrlidx, 0 ) ); 

    if ( !horizon_.isDefined(sectionid_, to.getSerialized()) )
	return false;
    const float startz = horizon_.getPos( sectionid_, to.getSerialized() ).z;
    tracker_->setRangeStep( sd.step );

    tracker_->setTarget( &toarr, zsz, sd.getIndex(startz) );

    if ( from.inl!=-1 && from.crl!=-1 )
    {
	const int frominlidx = 
	    attrdata_->inlsampling.nearestIndex(attrDataBinId(from).inl);
	if ( frominlidx<0 || frominlidx>=attrdata_->getInlSz() )
	    return false;

	const int fromcrlidx = 
	    attrdata_->crlsampling.nearestIndex(attrDataBinId(from).crl);
	if ( fromcrlidx<0 || fromcrlidx>=attrdata_->getCrlSz() )
	    return false;

	const OffsetValueSeries<float> fromarr( 
		    const_cast<ValueSeries<float>&>(*storage), 
		    cube.info().getOffset( frominlidx, fromcrlidx, 0 ) ); 
	if ( !horizon_.isDefined(sectionid_, from.getSerialized()) )
	    return false;
	const float fromz = horizon_.getPos(sectionid_,from.getSerialized()).z;
	tracker_->setSource( &fromarr, zsz, sd.getIndex(fromz) );

	if ( !tracker_->isOK() )
	    return false;

	const bool res = tracker_->track();
	const float resz = sd.atIndex( tracker_->targetDepth() );

	if ( !permittedZRange().includes(resz-startz) )
	    return false;

	if ( res ) targetz = resz;
	return res;
    }

    tracker_->setSource( 0, zsz, 0 );

    if ( !tracker_->isOK() )
	return false;

    const bool res = tracker_->track();
    const float resz = sd.atIndex( tracker_->targetDepth() );

    if ( !permittedZRange().includes(resz-startz) )
	return false;

    if ( res ) targetz = resz;
    return res;
}


void HorizonAdjuster::getNeededAttribs(
	ObjectSet<const Attrib::SelSpec>& specs ) const
{
    if ( !attribsel_ || !attribsel_->id().isValid() )
	return;

    for ( int idx=specs.size()-1; idx>=0; idx-- )
    {
	if ( *specs[idx] == *attribsel_ )
	    return;
    }

    specs += attribsel_;
}


CubeSampling HorizonAdjuster::getAttribCube( const Attrib::SelSpec& sp ) const
{
    if ( !attribsel_ || sp != *attribsel_ )
	return SectionAdjuster::getAttribCube( sp );

    CubeSampling res = engine().activeVolume();

    res.zrg.start += tracker_->permittedRange().start;
    res.zrg.stop += tracker_->permittedRange().stop;
    if ( !trackByValue() )
    {
	res.zrg.start += tracker_->similarityWindow().start;
	res.zrg.stop += tracker_->similarityWindow().stop;
    }

    res.snapToSurvey();
    return res;
}


bool HorizonAdjuster::is2D() const
{
    mDynamicCastGet(const EM::Horizon2D*,hor2d,&horizon_)
    return hor2d;
}


const BinID HorizonAdjuster::attrDataBinId( const BinID& bid ) const 
{
    return is2D() && attrdata_->getInlSz()==1 ? 
	   BinID( attrdata_->inlsampling.start, bid.crl ) : bid;
}


void HorizonAdjuster::setHorizonPick(const BinID&  bid, float val )
{
    Coord3 pos = horizon_.getPos( sectionid_, bid.getSerialized() );
    pos.z = val;
    horizon_.setPos( sectionid_, bid.getSerialized(), pos, true );
}


void HorizonAdjuster::setAttributeSel( int idx, const Attrib::SelSpec& as )
{
    if ( idx )
	return;

    if ( !attribsel_ ) attribsel_ = new Attrib::SelSpec;
    *attribsel_ = as;

    if ( attrdata_ ) attrdata_->unRef();
    attrdata_ = engine().getAttribCache( *attribsel_ );
    if ( attrdata_ ) attrdata_->ref();
}


bool HorizonAdjuster::hasInitializedSetup() const
{
   return ( attribsel_ && attribsel_->id().isValid() );
}


void HorizonAdjuster::fillPar( IOPar& iopar ) const
{
    SectionAdjuster::fillPar( iopar );
    IOPar trackerpar;
    tracker_->fillPar( trackerpar );
    iopar.mergeComp( trackerpar, sKeyTracker() );
    if ( attribsel_ ) attribsel_->fillPar( iopar );
}


bool HorizonAdjuster::usePar( const IOPar& iopar )
{
    PtrMan<IOPar> oldpar = iopar.subselect("attrval.Attrib 0");
    if ( !attribsel_ ) attribsel_ = new Attrib::SelSpec;
    if ( !oldpar || !attribsel_->usePar(*oldpar) )
    {
	if ( !attribsel_->usePar(iopar) )
	    return false;
    }

    PtrMan<IOPar> trackerpar = iopar.subselect( sKeyTracker() );
    if ( trackerpar )
    {
	if ( !tracker_->usePar( *trackerpar ) )
	    return false;
    }
    else
    {
	//OD3 format (old)

	const char* res = iopar.find( "Track event" );
	if ( res && *res )
	   tracker_->setTrackEvent( eEnum(VSEvent::Type,res) );
	float valthreshold;
	if ( iopar.get( "Value threshhold", valthreshold ) )
	    tracker_->setAmplitudeThreshold( valthreshold );
	float variance;
	if ( iopar.get( "Allowed variance", variance) )
	    tracker_->setAllowedVariance( variance );
	bool absthreshold;
	if ( iopar.getYN( "Use abs threshhold", absthreshold ) )
	    tracker_->setUseAbsThreshold( absthreshold );
	float simthreshold;
	if ( iopar.get( "Similarity threshhold", simthreshold ) )
	    tracker_->setSimilarityThreshold( simthreshold );
	bool byvalue;
	if ( iopar.getYN( "Track by value", byvalue ) )
	    tracker_->useSimilarity( !byvalue );
    }

    //The ranges was written in OD3.2, so this can be 
    //removed when OD5 is released.
    //Range is now stored with tracker
    Interval<float> permzrange;
    if ( iopar.get( "Permitted Z range", permzrange ) )
	tracker_->setPermittedRange( permzrange );

    Interval<float> similaritywin;
    if ( iopar.get( "Similarity window", similaritywin ) )
	tracker_->setSimilarityWindow(similaritywin);

    return SectionAdjuster::usePar( iopar );
}


}; // namespace MPE
