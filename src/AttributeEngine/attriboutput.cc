/*+
 * COPYRIGHT: (C) dGB Beheer B.V.
 * AUTHOR   : A.H. Bril
 * DATE     : Sep 2003
-*/


static const char* rcsID = "$Id: attriboutput.cc,v 1.14 2005-08-02 12:03:03 cvshelene Exp $";

#include "attriboutput.h"
#include "survinfo.h"
#include "attribslice.h"
#include "seistrc.h"
#include "seistrctr.h"
#include "seisbuf.h"
#include "iopar.h"
#include "seiswrite.h"
#include "ioman.h"
#include "ptrman.h"
#include "ioobj.h"
#include "linekey.h"


namespace Attrib
{

const char* Output::outputstr = "Output";
const char* Output::typekey = "Type";
const char* Output::cubekey = "Cube";
const char* Output::surfkey = "Surface";
const char* Output::tskey = "Trace Selection";
const char* Output::scalekey = "Scale";

const char* CubeOutput::seisidkey = "Seismic ID";
const char* CubeOutput::attribkey = "Attributes";
const char* CubeOutput::inlrangekey = "In-line range";
const char* CubeOutput::crlrangekey = "Cross-line range";
const char* CubeOutput::depthrangekey = "Depth range";

const char* LocationOutput::filenamekey = "Output Filename";
const char* LocationOutput::locationkey = "Locations";
const char* LocationOutput::attribkey = "Attribute";
const char* LocationOutput::surfidkey = "Surface ID";


Output::~Output()
{
    delete &seldata_;
}


Output::Output()
    : seldata_(*new SeisSelData)
{
    mRefCountConstructor;
}


SliceSetOutput::SliceSetOutput( const CubeSampling& cs )
    : desiredvolume(cs)
    , sliceset(0)
    , udfval(mUdf(float))
{
    const float dz = SI().zRange().step;
    Interval<int> interval( mNINT(cs.zrg.start/dz), mNINT(cs.zrg.stop/dz) );
    sampleinterval += interval;
}


SliceSetOutput::~SliceSetOutput()
{ if ( sliceset ) sliceset->unRef(); }


bool SliceSetOutput::getDesiredVolume( CubeSampling& cs ) const
{ cs=desiredvolume; return true; }


bool SliceSetOutput::wantsOutput( const BinID& bid ) const
{ return desiredvolume.hrg.includes(bid); }


TypeSet< Interval<int> > SliceSetOutput::getLocalZRange( const BinID& ) const
{ return sampleinterval; }


void SliceSetOutput::collectData( const BinID& bid, const DataHolder& data,
				  float refstep, int )
{
    if ( !sliceset )
    {
	sliceset = new Attrib::SliceSet;
	sliceset->ref();
	sliceset->sampling = desiredvolume;
	sliceset->sampling.zrg.step = refstep; 
	sliceset->direction = desiredvolume.defaultDir();
#define mGetDim(nr) \
        const int dim##nr = \
	    desiredvolume.size( direction ( sliceset->direction, nr) )
		
	mGetDim(0); mGetDim(1); mGetDim(2);
	for ( int idx=0; idx<dim0; idx++ )
	    *sliceset += new Attrib::Slice( dim1, dim2, udfval );

	if ( dim0 == 1 && desoutputs.size() > 1 )
	    for ( int idx=0; idx<desoutputs.size()-1; idx++ )
		*sliceset += new Attrib::Slice( dim1, dim2, udfval );
    }

    int i0, i1, i2;
    int sampleoffset = mNINT(sliceset->sampling.zrg.start/refstep);
    for ( int idy=0; idy<desoutputs.size(); idy++ )
    {
	for ( int idx=sampleoffset; 
		idx<sliceset->sampling.nrZ()+sampleoffset; idx++)
	{
	    const bool valididx = idx>=data.t0_ && idx<data.t0_+data.nrsamples_;
	    const float val = valididx ? 
			data.item(desoutputs[idy])->value(idx-data.t0_): udfval;
	    sliceset->getIdxs( bid.inl, bid.crl, idx*refstep, i0, i1, i2 );
	    ((*sliceset)[i0+idy])->set( i1, i2, val );
	}
    }
}


SliceSet* SliceSetOutput::getSliceSet() const
{
    return sliceset;
}


void SliceSetOutput::setGeometry( const CubeSampling& cs )
{
    if ( cs.isEmpty() ) return;
    seldata_.copyFrom(cs);
}


CubeOutput::CubeOutput( const CubeSampling& cs , LineKey lkey )
    : desiredvolume(cs)
    , auxpars(0)
    , storid_(*new MultiID)
    , writer_(0)
    , calcurpos_(0)
    , prevpos_(-1,-1)
    , storinited_(0)
    , lkey_(lkey)
    , buf2d_(0)
    , errmsg(0)
{
    const float dz = SI().zRange().step;
    Interval<int> interval( mNINT(cs.zrg.start/dz), mNINT(cs.zrg.stop/dz) );
    sampleinterval += interval;

    seldata_.linekey_ = lkey;
}


bool CubeOutput::getDesiredVolume( CubeSampling& cs ) const
{ cs=desiredvolume; return true; }


bool CubeOutput::wantsOutput( const BinID& bid ) const
{ return desiredvolume.hrg.includes(bid); }


TypeSet< Interval<int> > CubeOutput::getLocalZRange( const BinID& ) const
{ return sampleinterval; }


bool CubeOutput::setStorageID( const MultiID& storid )
{
    if ( *((const char*)storid) )
    {
	PtrMan<IOObj> ioseisout = IOM().get( storid );
	if ( !ioseisout )
	{
	    errmsg = "Cannot find seismic data with ID: "; errmsg += storid;
	    return false;
	}
    }

    storid_ = storid;
    return true;
}


void CubeOutput::setGeometry( const CubeSampling& cs )
{
    if ( cs.isEmpty() ) return;
    seldata_.copyFrom(cs);
}


CubeOutput::~CubeOutput()
{
    delete writer_;
    delete &storid_;
    delete auxpars;
    if ( buf2d_ )
	buf2d_->erase();
}


bool CubeOutput::doUsePar( const IOPar& pars )
{
    errmsg = "";
    PtrMan<IOPar> outppar = pars.subselect("Output.1");
    const char* storid = outppar->find("Seismic ID");
    if ( !setStorageID( storid ) )
    {
        errmsg = "Could not find output ID: "; errmsg += storid;
        return false;
    }

    auxpars = pars.subselect("Aux");
    doInit();
    return true;
}//warning, only a small part of the old taken, see if some more is required


bool CubeOutput::doInit()
{
    if ( *((const char*)storid_) )
    {
	PtrMan<IOObj> ioseisout = IOM().get( storid_ );
	if ( !ioseisout )
	{
	    errmsg = "Cannot find seismic data with ID: "; errmsg += storid_;
	    return false;
	}

	writer_ = new SeisTrcWriter( ioseisout );
	is2d_ = writer_->is2D();
	if ( auxpars )
	{
	    writer_->lineAuxPars().merge( *auxpars );
	    delete auxpars; auxpars = 0;
	}
    }
    
    desiredvolume.normalise();
    seldata_.linekey_.setAttrName( "" );
    if ( seldata_.type_ != Seis::Range )
	seldata_.type_ = Seis::Range;

    if ( !is2d_ )
    {
	if ( seldata_.inlrg_.start > desiredvolume.hrg.start.inl )
	    desiredvolume.hrg.start.inl = seldata_.inlrg_.start;
	if ( seldata_.inlrg_.stop < desiredvolume.hrg.stop.inl )
	    desiredvolume.hrg.stop.inl = seldata_.inlrg_.stop;
	if ( seldata_.crlrg_.start > desiredvolume.hrg.start.crl )
	    desiredvolume.hrg.start.crl = seldata_.crlrg_.start;
	if ( seldata_.crlrg_.stop < desiredvolume.hrg.stop.crl )
	    desiredvolume.hrg.stop.crl = seldata_.crlrg_.stop;
	if ( seldata_.zrg_.start > desiredvolume.zrg.start )
	    desiredvolume.zrg.start = seldata_.zrg_.start;
	if ( seldata_.zrg_.stop < desiredvolume.zrg.stop )
	    desiredvolume.zrg.stop = seldata_.zrg_.stop;
    }

    return true;
}


bool CubeOutput::setReqs( const BinID& pos )
{
    calcurpos_ = true;
    if ( !is2d_ )
    {
	if ( pos == prevpos_ )
	{
	    calcurpos_ = false;
	    return false;
	}
	prevpos_ = pos;

	calcurpos_ = desiredvolume.hrg.includes( pos );
	if ( !calcurpos_ ) return false;
    }

    return true;
}


SeisTrcBuf* CubeOutput::getTrcBuf() const 
{
    SeisTrcBuf* ret = buf2d_;
    const_cast<CubeOutput*>(this)->buf2d_ = 0;
    return ret;
}


class COLineKeyProvider : public LineKeyProvider
{
public:

COLineKeyProvider( CubeOutput& c, const char* a, const char* lk )
	: co(c) , attrnm(a) , linename(lk) {}

LineKey lineKey() const
{
    LineKey lk(linename,attrnm);
    return lk;
}
    CubeOutput&   co;
    BufferString        attrnm;
    BufferString 	linename;

};


void CubeOutput::collectData( const BinID& bid, const DataHolder& data, 
			      float refstep, int trcnr )
{
    if ( !calcurpos_ ) return;

    int nrcomp = data.nrItems();
    if ( !nrcomp || nrcomp < desoutputs.size())
	return;

    const int sz = data.nrsamples_;
    DataCharacteristics dc;
    SeisTrc* trc = new SeisTrc( sz, dc );
    for ( int idx=trc->data().nrComponents(); idx<desoutputs.size(); idx++)
	trc->data().addComponent( sz, dc, false );

    trc->info().sampling.step = refstep;
    trc->info().sampling.start = data.t0_*refstep;
    trc->info().binid = bid;
    trc->info().coord = SI().transform( bid );
    if ( is2d_ )
	trc->info().nr = trcnr;

    for ( int comp=0; comp<desoutputs.size(); comp++ )
    {
	for ( int idx=0; idx<sz; idx++ )
	{
	    float val = data.item(desoutputs[comp])->value(idx);
	    trc->set(idx, val, comp);
	}
    }

    const bool dostor = *((const char*)storid_);
    if ( dostor && !storinited_ )
    {
	if ( writer_->is2D() )
	{
	    BufferString nm = curLineKey().attrName();
	    if ( nm == "inl_dip" || nm == "crl_dip" )
		nm = sKey::Steering;
	    else if ( IOObj::isKey(nm) )
		nm = IOM().nameOf(nm);
	    writer_->setLineKeyProvider( 
		new COLineKeyProvider( *this, nm, curLineKey().lineName()) );
	}

	if ( !writer_->prepareWork(*trc) )
	    { errmsg = writer_->errMsg(); return; }

	if ( !writer_->is2D() )
	{
	    SeisTrcTranslator* transl = writer_->translator();
	    ObjectSet<SeisTrcTranslator::TargetComponentData>& cis
		             = transl->componentInfo();
	    cis[0]->datatype = Seis::UnknowData;
	}

	storinited_ = true;
    }
    
    if ( dostor )
    {
        if ( !writer_->put(*trc) )
            { errmsg = writer_->errMsg(); }
    }
    else if ( is2d_ )
    {
	if ( !buf2d_ ) buf2d_ = new SeisTrcBuf;
	buf2d_->add( new SeisTrc(*trc) );
    }

    // TODO later on : create function on writer to handle dataholder directly
}


LocationOutput::LocationOutput( BinIDValueSet& bidvalset )
    : bidvalset_(bidvalset)
{
    seldata_.type_ = Seis::Table;
    seldata_.table_.allowDuplicateBids( true );
    seldata_.table_ = bidvalset;
}


void LocationOutput::collectData( const BinID& bid, const DataHolder& data,
				  float refstep, int trcnr )
{
    BinIDValueSet::Pos pos = bidvalset_.findFirst( bid );
    if ( !pos.valid() ) return;

    if ( bidvalset_.nrVals()-1 != desoutputs.size() )
	bidvalset_.setNrVals( desoutputs.size()+1 );

    float* vals = bidvalset_.getVals( pos );
    for ( int comp=0; comp<desoutputs.size(); comp++ )
    {
	float val = data.item(desoutputs[comp])->value(0);
	vals[comp+1] = val;
    }
}


bool LocationOutput::wantsOutput( const BinID& bid ) const
{
    BinIDValueSet::Pos pos = bidvalset_.findFirst( bid );
    return pos.valid();
}


TypeSet< Interval<int> > LocationOutput::getLocalZRange(const BinID& bid) const
{
    BinIDValueSet::Pos pos = bidvalset_.findFirst( bid );
    const float* vals = bidvalset_.getVals(pos);
    const float dz = SI().zRange().step;
    Interval<int> interval( mNINT(vals[0]/dz), mNINT(vals[0]/dz) );
    TypeSet< Interval<int> > sampleinterval;
    sampleinterval += interval;
    return sampleinterval;
}


TrcSelectionOutput::TrcSelectionOutput( const BinIDValueSet& bidvalset )
    : bidvalset_(bidvalset)
    , outpbuf_(0)
{
    seldata_.type_ = Seis::Table;
    seldata_.table_.allowDuplicateBids( true );
    seldata_.table_ = bidvalset;
    int nrinterv = bidvalset.nrVals()/2;
    float zmin = bidvalset.valRange(0).start;
    float zmax = bidvalset.valRange(1).stop;
    for ( int idx=2; idx<nrinterv; idx+=2 )
    { 
	zmin = bidvalset.valRange(idx).start < zmin ? 
	    		bidvalset.valRange(idx).start : zmin;
	zmax = bidvalset.valRange(idx+1).stop > zmax ? 
	    		bidvalset.valRange(idx+1).stop : zmax;
    }

    stdtrcsz_ = zmax - zmin;
    stdstarttime_ = zmin;
}


TrcSelectionOutput::~TrcSelectionOutput()
{
    if ( outpbuf_ )
	outpbuf_->erase();
}


void TrcSelectionOutput::collectData( const BinID& bid, const DataHolder& data,
				      float refstep, int trcnr )
{
    int nrcomp = data.nrItems();
    if ( !nrcomp || nrcomp < desoutputs.size() )
	return;

    const int sz = (int)(stdtrcsz_/refstep);
    const int startidx = (int)((data.t0_ - stdstarttime_) / refstep);
    const int index = outpbuf_->find( bid );
    SeisTrc* trc;

    if ( index == -1 )
    {
	const DataCharacteristics dc;
	trc = new SeisTrc( sz, dc );
	for ( int idx=trc->data().nrComponents(); idx<desoutputs.size(); idx++ )
	    trc->data().addComponent( sz, dc, false );

	trc->info().sampling.step = refstep;
	trc->info().sampling.start = stdstarttime_;
	trc->info().binid = bid;
	trc->info().coord = SI().transform( bid );
    }
    else
	trc = outpbuf_->remove( index );

    for ( int comp=0; comp<desoutputs.size(); comp++ )
    {
	for ( int idx=0; idx<data.nrsamples_; idx++ )
	{
	    float val = data.item(desoutputs[comp])->value(idx);
	    trc->set( startidx+idx, val, comp );
	}
    }

    if ( index == -1 )
	outpbuf_->add( trc );
    else
	outpbuf_-> insert( trc, index );
}


bool TrcSelectionOutput::wantsOutput( const BinID& bid ) const
{
    BinIDValueSet::Pos pos = bidvalset_.findFirst( bid );
    return pos.valid();
}


void TrcSelectionOutput::setOutput( SeisTrcBuf* outp_ )
{
    outpbuf_ = outp_;
    if ( outpbuf_ )
	outpbuf_->erase();
}


TypeSet< Interval<int> > 
		TrcSelectionOutput::getLocalZRange( const BinID& bid ) const
{
    BinIDValueSet::Pos pos = bidvalset_.findFirst( bid );
    BinID binid;
    const float dz = SI().zRange().step;
    TypeSet<float> values;
    bidvalset_.get( pos, binid, values );
    for ( int idx=0; idx<values.size()/2; idx+=2 )
    {
	Interval<int> interval( mNINT(values[idx]/dz), 
				mNINT(values[idx+1]/dz) );
	const_cast<TrcSelectionOutput*>(this)->sampleinterval += interval;
    }
 
    return sampleinterval;
}

} // namespace Attrib
