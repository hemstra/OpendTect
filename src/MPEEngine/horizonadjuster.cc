/*
___________________________________________________________________

 * COPYRIGHT: (C) dGB Beheer B.V.
 * AUTHOR   : K. Tingdahl
 * DATE     : Nov 2003
___________________________________________________________________

-*/

static const char* rcsID = "$Id: horizonadjuster.cc,v 1.19 2005-10-20 03:29:50 cvsduntao Exp $";

#include "horizonadjuster.h"

#include "attribpsc.h"
#include "emhorizon.h"
#include "genericnumer.h"
#include "iopar.h"
#include "linear.h"
#include "samplingdata.h"
#include "survinfo.h"
#include "valseriesevent.h"

#include <math.h>

namespace MPE {


HorizonAdjuster::HorizonAdjuster( EM::Horizon& hor,
	const EM::SectionID& sid )
    : SectionAdjuster( sid )
    , horizon_( hor )
{
    computers_ += new AttribPositionScoreComputer();

    float dist = 5 * SI().zStep();
    permzrange_ = Interval<float>(-dist,dist);
    dist = 10 * SI().zStep();
    similaritywin_ = Interval<float>(-dist, dist);
    
    ampthreshold_ = mUndefValue;
    allowedvar_ = 0.20;
    similaritythreshold_ = 0.80;
    trackbyvalue_ = true;
    trackevent_ = VSEvent::Max;
    useabsthreshold_ = false;
}


int HorizonAdjuster::nextStep()
{
    initTrackParam();
    const bool didchecksupport = horizon_.geometry.checkSupport(false);
    int count = 0;
    for ( int idx=0; idx<pids_.size(); idx++ )
    {
	BinID bid;	bid.setSerialized( pids_[idx] );
	BinID refbid;
	if ( refpos_ )
	    refbid.setSerialized(*refpos_);
	else if ( pidsrc_.size() > idx )
	    refbid.setSerialized( pidsrc_[idx] );
	else
	    refbid = bid;
	
	float targetz;
 	if ( trackTrace(refbid, bid, targetz, 0) )
	    ++count, setHorizonPick(bid, targetz);
	else if ( removeonfailure_ )
	    setHorizonPick(bid, mUdf(float) );
    }
    horizon_.geometry.checkSupport(didchecksupport);

#ifdef __debug__
    BufferString msg( "Tracked horizon picks: " ); msg += count;
    ErrMsg( msg );
#endif
    return 0;
}


bool HorizonAdjuster::trackTrace( const BinID& refbid,
				 const BinID& targetbid, float& targetz, 
				 float* refsamples )
{
    const Coord3 pos = horizon_.getPos(sectionid_,targetbid.getSerialized());
    if ( !pos.isDefined() )	return false;
    targetz = pos.z;

    const AttribPositionScoreComputer* acomp = getAttribComputer();
    const PositionScoreComputerAttribData* attrdata = acomp->getAttribData();
    const HorSampling hrg = attrdata->getCubeSampling(0).hrg;
    const StepInterval<float> zrg = attrdata->getCubeSampling(0).zrg;
    if ( !zrg.includes(targetz) )
	return false;

    float smplbuf[matchwinsamples_+2];
    if ( ! refsamples ) {
	BinID srcbid(refbid.inl, refbid.crl);
	if ( srcbid.inl == -1 && srcbid.crl == -1 )
	    srcbid = targetbid;
	if ( !hrg.includes(srcbid) || !hrg.includes(srcbid) )
	    return false;
	if ( ! getCompSamples(attrdata, srcbid,
	    	horizon_.getPos(sectionid_,srcbid.getSerialized()).z, smplbuf) )
	    return false;
	refsamples = smplbuf;
    }

    Interval<double> permrange( targetz+permzrange_.start,
	    			targetz+permzrange_.stop );
    if ( !trackbyvalue_ )
    {
	permrange.start += similaritywin_.start;
	permrange.stop += similaritywin_.stop;
    }

    if ( permrange.start<zrg.start )	permrange.start = zrg.start;
    if ( permrange.stop>zrg.stop )	permrange.stop = zrg.stop;
    if ( permrange.start>permrange.stop || !permrange.includes(targetz) )
	return false;
    
    const int nrcubesamples = zrg.nrSteps();
    float trcbuffer[zrg.nrSteps() + 2 ];
    const Interval<int> samplerg( mMAX(zrg.nearestIndex(permrange.start),0),
			mMIN(zrg.nearestIndex(permrange.stop),nrcubesamples-1));
    if ( ! attrdata->getValueBySample( 0, targetbid, trcbuffer,
				       samplerg.start, samplerg.stop) )
    	return false;
    const int nrsamples = samplerg.stop - samplerg.start + 1;
    int refpos = zrg.nearestIndex(targetz)-samplerg.start;
    float refval;
    
    if ( ! trackbyvalue_ )
    {
	int upmatchpos, downmatchpos;  upmatchpos = downmatchpos = -1;
	float upmatchv, downmatchv;    bool upeq, downeq;
    
	Interval<double> matchrg( targetz+similaritywin_.start,
				  targetz+similaritywin_.stop );
	if ( !( permrange.includes(matchrg.start)
	     && permrange.includes(matchrg.stop) ) )
	    return false; 

	int stsmpl = zrg.nearestIndex(matchrg.start)-samplerg.start;
 	upmatchpos = matchingSampleBySimilarity( trcbuffer, stsmpl,
			    0, refsamples, upmatchv, upeq );
	downmatchpos = matchingSampleBySimilarity( trcbuffer, stsmpl+1,
			    nrsamples - matchwinsamples_,
			    refsamples, downmatchv, downeq );

	int matchpos = -1;
	if ( upmatchpos!=-1 &&  downmatchpos!=-1 )
	{
	    if ( downmatchv == upmatchv && upeq && downeq )
		matchpos = ( upmatchpos + downmatchpos ) / 2;
	    else 
		matchpos = downmatchv>upmatchv ? downmatchpos : upmatchpos;
	}
	else if ( upmatchpos!=-1 )
	    matchpos = upmatchpos;
	else if ( downmatchpos!=-1 )
	    matchpos = downmatchpos;

	if ( matchpos == -1 )	    return false;
	// snap to event
	refpos = matchpos + simlaritymatchpt_;
	refval = refsamples[simlaritymatchpt_];
    }
    else
	refval = useAbsThreshold() ? ampthreshold_ : *refsamples;

    ArrayValueSeries<float> valarr( trcbuffer );
    SamplingData<float> sd(0, 1);
    ValueSeriesEvFinder<float, float> evfinder( valarr, nrsamples-1, sd);
    if ( !useAbsThreshold() )
    {
       const float alloweddev = fabs( *refsamples * allowedvar_ );
       refval += (trackevent_==VSEvent::Min ? alloweddev : -alloweddev);
    }
    ValueSeriesEvent<float, float> ev = evfinder.findAdjoining(trackevent_,
					    refpos, refval);
    if ( Values::isUdf(ev.pos) )
	return false;
    targetz = zrg.atIndex(samplerg.start);
    targetz += (ev.pos*zrg.step);
    return true;
}


int HorizonAdjuster::matchingSampleBySimilarity( const float* srctrc,
			int startsample, int endsample, const float* refval,
			float &matchratio, bool& eqfromstart )
{
    const int noiselevel = 1;	int noise = 0;
    int matchpos = -1;    eqfromstart = false;

    int eqsamples = 0;    float prevratio = 0;
    int step = startsample>endsample ? -1 : 1;
    for ( int smpl=startsample;  ; smpl += step )
    {
	if ( step==1 && smpl>endsample || step==-1 && smpl<endsample )
	    break;
	
	float curratio = similarity<const float*, const float*>(refval, srctrc,
				matchwinsamples_, false, 0, smpl );
	if ( curratio<prevratio )
	{
	    if ( noise < noiselevel ) { ++noise;    continue; }
	    else			break;
	}
	
	if ( smpl==startsample || prevratio != curratio )
	{
	    matchpos = smpl;
	    matchratio = prevratio = curratio;
	    eqsamples = 0;
	}
	else if ( smpl!=startsample && prevratio == curratio )
	    eqsamples++;
	noise = 0;
    }
    if ( matchpos != -1 && eqsamples )
    {
	eqfromstart = (eqsamples == (matchpos - startsample + 1));
	matchpos += ( eqsamples / 2 * step );
    }
    return matchratio>=similaritythreshold_ ? matchpos : -1;
}


void HorizonAdjuster::initTrackParam()
{
    const AttribPositionScoreComputer* acomp = getAttribComputer();
    
    if (trackbyvalue_)
    {
	matchwinsamples_ = 1;
	simlaritymatchpt_ = 0;
    }
    else {
	const PositionScoreComputerAttribData* attrdata = acomp->getAttribData();
	const StepInterval<float> zrg = attrdata->getCubeSampling(0).zrg;
	matchwinsamples_ = (int)( (similaritywin_.stop - similaritywin_.start)
    				  / zrg.step ) + 1;
	simlaritymatchpt_ = (int)(-similaritywin_.start / zrg.step);
    }
}


const AttribPositionScoreComputer* 
    HorizonAdjuster::getAttribComputer() const
{
    for ( int idx=0; idx<nrComputers(); idx++ )
    {
	mDynamicCastGet(const AttribPositionScoreComputer*,attr,
			getComputer(idx));
	if ( attr ) return attr;
    }

    return 0;
}


void HorizonAdjuster::setHorizonPick(const BinID&  bid, float val )
{
    horizon_.setPos( sectionid_, bid.getSerialized(), Coord3(0,0,val), true );
}


bool HorizonAdjuster::getCompSamples(
	const PositionScoreComputerAttribData*	attrdata,
	const BinID& bid, float z, float* buf )
{
    float *smplbuf = buf;
    if ( trackbyvalue_ )
    {
	const StepInterval<float> zrg = attrdata->getCubeSampling(0).zrg;
	int stsmpl = zrg.nearestIndex(z);
	*smplbuf = attrdata->getValueBySample( 0, bid, stsmpl );
	return ! Values::isUdf(*smplbuf);
    }
    else
    {
	const StepInterval<float> zrg = attrdata->getCubeSampling(0).zrg;
	int stsmpl = zrg.nearestIndex(z+similaritywin_.start);
	return attrdata->getValueBySample(0, bid, smplbuf, stsmpl,
					  stsmpl + matchwinsamples_ - 1);
    }
}


void HorizonAdjuster::fillPar( IOPar& iopar ) const
{
    SectionAdjuster::fillPar( iopar );
    iopar.set( sKeyTrackEvent(), VSEvent::TypeRef(trackevent_) );
    iopar.set( sKeyPermittedZRange(), permzrange_.start, permzrange_.stop );
    iopar.set( sKeyValueThreshold(), ampthreshold_ );
    iopar.set( sKeyAllowedVariance(), allowedvar_);
    iopar.setYN( sKeyUseAbsThreshold(), useabsthreshold_ );
    iopar.set( sKeySimWindow(), similaritywin_.start, similaritywin_.stop );
    iopar.set( sKeySimThreshold(), similaritythreshold_ );
    iopar.setYN( sKeyTrackByValue(), trackbyvalue_ );
}


bool HorizonAdjuster::usePar( const IOPar& iopar )
{
    EnumRef tmpref = VSEvent::TypeRef(trackevent_);

    return
	SectionAdjuster::usePar( iopar ) &&
	iopar.get( sKeyTrackEvent(),  tmpref ) &&
	iopar.get( sKeyPermittedZRange(),permzrange_.start,permzrange_.stop ) &&
	iopar.get( sKeyValueThreshold(), ampthreshold_ ) &&
	iopar.get( sKeyAllowedVariance(), allowedvar_) &&
	iopar.getYN( sKeyUseAbsThreshold(), useabsthreshold_ ) &&
	iopar.get( sKeySimWindow(),similaritywin_.start,similaritywin_.stop ) &&
	iopar.get( sKeySimThreshold(), similaritythreshold_ ) &&
	iopar.getYN( sKeyTrackByValue(), trackbyvalue_ );
}


}; // namespace MPE
