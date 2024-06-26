#pragma once
/*+
________________________________________________________________________

 Copyright:	(C) 1995-2022 dGB Beheer B.V.
 License:	https://dgbes.com/licensing
________________________________________________________________________

-*/

#include "expattribsmod.h"
#include "attribprovider.h"
#include "statruncalc.h"

/*!\brief GrubbsFilter Attribute

GrubbsFilter grubbsval gate= pos0= pos1= stepout=1,1 

*/

namespace Attrib
{

mExpClass(ExpAttribs) GrubbsFilter : public Provider
{
public:
    static void			initClass();
				GrubbsFilter(Desc&);

    static const char*		attribName()	{ return "GrubbsFilter"; }

    static const char*		grubbsvalStr()	{ return "grubbsvalue"; }
    static const char*		gateStr()	{ return "gate"; }
    static const char*		stepoutStr()	{ return "stepout"; }
    static const char*		replaceValStr()	{ return "replacewith"; }

    void			prepPriorToBoundsCalc() override;
    enum ReplaceType		{ Average, Median, Threshold, GrubbsValue,
				  Interpolate };

protected:
    				~GrubbsFilter() {}
    static Provider*		createInstance(Desc&);
    static void			updateDesc(Desc&);

    bool			allowParallelComputation() const override
				{ return false; }

    float			replaceVal(const Stats::RunCalc<float>&) const;
    bool			getInputData(const BinID&,int zintv) override;
    bool			computeData(const DataHolder&,
	    				    const BinID& relpos,
					    int z0,int nrsamples,
					    int threadid) const override;

    const BinID*		desStepout(int input,int output) const override;
    const Interval<float>*	desZMargin(int input,int output) const override;
    bool			getTrcPos();

    float			cogrubbsval_;
    BinID			stepout_;
    Interval<float>		gate_;
    TypeSet<BinID>		trcpos_;
    int				centertrcidx_;
    int				type_;

    int				dataidx_;

    ObjectSet<const DataHolder>	inputdata_;
};

} // namespace Attrib
