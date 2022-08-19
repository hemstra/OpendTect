#pragma once
/*+
________________________________________________________________________

 Copyright:	(C) 1995-2022 dGB Beheer B.V.
 License:	https://dgbes.com/licensing
________________________________________________________________________

-*/

/*

Inverse gate=[-64,64] steering=Yes window=[Box] neighbourhood=1,1 pos1=0,1

Input:
0	Real data
1	Inline Dip
2	Crossline Dip

Output:
0	(x0-x1)/(x0+x1)
1	x0/(x_avg)

@$*/

#include "attribcalc.h"
#include "task.h"
#include "position.h"
#include <limits.h>
#include "seistrc.h"
#include <complex>
#include "arrayndimpl.h"
#include "fft.h"
#include "attribparamimpl.h"

#include <arrayndalgo.h>


mClass(AttribExp) InverseAttrib : public AttribCalc
{
public:
    mAttrib5Param(InverseAttrib
	,"Inverse"
	, TimeGateAttribParameter
	    , gate
	    , TimeGateAttribParameter(	"samplegate"
		, AttribParameter::Required
		, TimeGate(-64,64)
		, TimeGate(-mUndefValue, mUndefValue)
	    )
	, BinIDAttribParameter
	    , pos1
	    , BinIDAttribParameter(	"pos1"
		, AttribParameter::Required
		, BinID(0,1)
		, Interval<int>(-100,100)
		, Interval<int>(-100,100)
	    )
	, BinIDAttribParameter
	    , neighbourhood
	    , BinIDAttribParameter(	"neighbourhood"
		, AttribParameter::Required
		, BinID(1,1)
		, Interval<int>(1,10)
		, Interval<int>(1,10)
	    )
	, EnumAttribParameter
	    , window
	    , EnumAttribParameter(	"window"
		, ArrayNDWindow::WindowTypeNames
		, AttribParameter::Required
		, 0
	    )
	, BoolAttribParameter
	    , steering
	    , BoolAttribParameter(	"steering"
		, AttribParameter::Default
		, true
	    )
	, mAttribParamFormHasNoUpdate );


				InverseAttrib( Parameters* );
				~InverseAttrib();

    bool			init();

    const Interval<float>*	reqInterval( int, int ) const {return &gate;}
    const BinID*		reqStepout( int i, int ) const
				{ return i ? 0 : &neighbourhood; }

    int nrAttribs() const { return 2; }

    const char* attribName(int val) const
				{
				    switch (val)
				    {
					case 0:     return "1";
					case 1:     return "2";
				    }

				    return 0;
				}

    Seis::DataType		dataType(int,
					 const TypeSet<Seis::DataType>&) const
				{ return Seis::UnknowData; }

    const char*		definitionStr() const { return desc; }
    void		setCommonInfo( const AttribProcessCommonInfo& ni )
				{ common = &ni; }


protected:
    Interval<float>		gate;
    bool			steering;
    BinID			neighbourhood;
    BinID			pos1;

    FFT				fft;
    FFT				ifft;
    int			fftsz;
    ArrayNDWindow::WindowType	windowtype;
    ArrayNDWindow*		window;
    float			inpstep;
    float			df;

    BufferString		desc;
    const AttribProcessCommonInfo*	common;

    mClass(AttribExp) Task : public AttribCalc::Task
    {
    public:
	mClass(AttribExp) Input : public AttribCalc::Task::Input
	{
	public:
				Input( const InverseAttrib& calculator_ )
				    : calculator ( calculator_ )
				    , trcs( 0 )
				    , inldiptrc( 0 )
				    , crldiptrc( 0 )
				{}

				~Input();

	    bool		set( const BinID&,
				    const ObjectSet<AttribProvider>&,
				    const TypeSet<int>&,
				    const TypeSet<float*>& );

	    AttribCalc::Task::Input* clone() const
			{ return new InverseAttrib::Task::Input(*this); }

	    Array2DImpl<SeisTrc*>*	trcs;
	    SeisTrc*			inldiptrc;
	    SeisTrc*			crldiptrc;

	    int				dataattrib;
	    int				inldipattrib;
	    int				crldipattrib;

	    const InverseAttrib&	calculator;
	};

			    Task( const InverseAttrib& calculator_ );
			    Task( const Task& );
			    // Not impl. Only to give error if someone uses it

			    ~Task();

	void		    set( float t1_, int nrtimes_, float step_,
					    const AttribCalc::Task::Input* inp,
                                            const TypeSet<float*>& outp_)
				{
				    t1 = t1_;
				    nrtimes = nrtimes_;
				    step = step_;
				    input = inp;
				    out0 = outp_[0];
				    out1 = outp_[1];
				}

	AttribCalc::Task*    clone() const;

	int		    getFastestSz() const { return 25; }

	int		    nextStep();

	AttribCalc::Task::Input* getInput() const
		    { return new InverseAttrib::Task::Input( calculator ); }

    protected:
	float*				out0;
	float*				out1;

	const InverseAttrib&		calculator;
	Array2D<Array1D<float_complex>*>*	tracesegments;
	Array1D<float_complex>*		spectrum0;
	Array1D<float_complex>*		spectrum1;
	Array1D<float_complex>*		spectrumaverage;
	Array1D<float_complex>*		spectrumoutput;
	Array1D<float_complex>*		traceoutput;
    };

    friend class	InverseAttrib::Task;
    friend class	InverseAttrib::Task::Input;
};

