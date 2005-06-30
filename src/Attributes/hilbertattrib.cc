/*+
________________________________________________________________________

 CopyRight:     (C) dGB Beheer B.V.
 Author:        N. Hemstra
 Date:          May 2005
 RCS:           $Id: hilbertattrib.cc,v 1.4 2005-06-30 11:26:43 cvshelene Exp $
________________________________________________________________________

-*/

#include "hilbertattrib.h"

#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribfactory.h"
#include "attribparam.h"
#include "datainpspec.h"
#include "genericnumer.h"

namespace Attrib
{

void Hilbert::initClass()
{
    Desc* desc = new Desc( attribName(), updateDesc );
    desc->ref();

    ValParam* halflen = new ValParam( halflenStr(), new IntInpSpec() );
    desc->addParam(halflen);
//  halflen->setDefaultVal( 30 );

    InputSpec inputspec( "Input data", true );
    desc->addInput( inputspec );
    desc->addOutputDataType( Seis::UnknowData );
    desc->init();

    PF().addDesc( desc, createInstance );
    desc->unRef();
}


Provider* Hilbert::createInstance( Desc& desc )
{
    Hilbert* res = new Hilbert( desc );
    res->ref();

    if ( !res->isOK() )
    {
	res->unRef();
	return 0;
    }

    res->unRefNoDelete();
    return res;
}


void Hilbert::updateDesc( Desc& desc )
{
}


Hilbert::Hilbert( Desc& desc_ )
    : Provider( desc_ )
{
    if ( !isOK() ) return;

    mGetInt( halflen, halflenStr() );
    hilbfilterlen = halflen * 2 + 1;
    hilbfilter = makeHilbFilt( halflen );
    gate = Interval<float>( -halflen-3, halflen+3 );
}


bool Hilbert::getInputOutput( int input, TypeSet<int>& res ) const
{
    return Provider::getInputOutput( input, res );
}


bool Hilbert::getInputData( const BinID& relpos, int idx )
{
    inputdata = inputs[0]->getData( relpos, idx );
    return inputdata;
}


class Masker
{
public:
Masker( const DataHolder* dh, int shift, float avg )
    : data_(dh )
    , avg_(avg)
    , shift_(shift)	{}

float operator[](int idx) const
{
    const int pos = shift_ + idx;
    if ( pos < 0 )
	return data_->item(0)->value(0) - avg_;
    if ( pos >= data_->nrsamples_ )
	return data_->item(0)->value(data_->nrsamples_-1) - avg_;
    return data_->item(0)->value( pos ) - avg_;
}

    const DataHolder*	data_;
    const int		shift_;
    float		avg_;
};


bool Hilbert::computeData( const DataHolder& output, const BinID& relpos, 
			   int t0, int nrsamples ) const
{
    if ( !inputdata ) return false;

    const int shift = t0 - inputdata->t0_;
    Masker masker( inputdata, shift, 0 );
    float avg=0;
    for ( int idx=0; idx<nrsamples; idx++ )
	avg += masker[idx];

    masker.avg_ = avg / nrsamples;
    float* outp = output.item(0)->arr();
    GenericConvolve( hilbfilterlen, -halflen, hilbfilter, nrsamples,
		     0, masker, nrsamples, 0, outp );

    return true;
}


/*const Interval<float>* Hilbert::reqZMargin( int inp, int ) const
{
    
    const_cast<Interval<float>*>(&timegate)->start = gate.start * refstep;
    const_cast<Interval<float>*>(&timegate)->stop =  gate.stop * refstep;
    return &timegate; 
}
*/

float* Hilbert::makeHilbFilt( int hlen )
{
    float* h = new float[hlen*2+1];
    h[hlen] = 0;
    for ( int i=1; i<=hlen; i++ )
    {
	const float taper = 0.54 + 0.46 * cos( M_PI*(float)i / (float)(hlen) );
	h[hlen+i] = taper * ( -(float)(i%2)*2.0 / (M_PI*(float)(i)) );
	h[hlen-i] = -h[hlen+i];
    }

    return h;
}


}; // namespace Attrib


