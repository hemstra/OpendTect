#pragma once
/*+
________________________________________________________________________

 Copyright:	(C) 1995-2022 dGB Beheer B.V.
 License:	https://dgbes.com/licensing
________________________________________________________________________

-*/

#include "seismod.h"
#include "seistype.h"

#include "binid.h"
#include "bufstring.h"
#include "posgeomid.h"
#include "ranges.h"

class TrcKey;
namespace Pos { class Provider; }


namespace Seis
{

/*!\brief setup for subselection of seismic data */

mExpClass(Seis) SelSetup
{
public:

				SelSetup(Seis::GeomType);
				SelSetup(bool is_2d,bool is_ps=false);
				~SelSetup();

    mDefSetupClssMemb(SelSetup,bool,is2d)
    mDefSetupClssMemb(SelSetup,bool,isps)
    mDefSetupClssMemb(SelSetup,bool,onlyrange)		// true
    mDefSetupClssMemb(SelSetup,bool,fornewentry)	// false
    mDefSetupClssMemb(SelSetup,bool,multiline)		// false
    mDefSetupClssMemb(SelSetup,bool,withoutz)		// false
    mDefSetupClssMemb(SelSetup,bool,withstep)		// true
    mDefSetupClssMemb(SelSetup,uiString,seltxt)
    mDefSetupClssMemb(SelSetup,BufferString,zdomkey)
    mDefSetupClssMemb(SelSetup,BufferString,zunitstr)

    Seis::GeomType geomType() const	{ return geomTypeOf(is2d_,isps_); }

};


/*!\brief contains input (sub-)selection data from a cube or lineset

  This class exists so that without knowing the form of the subselection,
  other classes can find out whether a trace is included or not.
  The function selRes() returns an integer which gives more information than
  just yes/no. If 0 is returned, the position is included. If non-zero,
  the inline or crossline number can be one of:

  0 - this number is OK by itself, but not the combination
  1 - this number is the 'party-pooper' but there are selected posns with it
  2 - No selected position has this number

  Especially (2) is very useful: an entire inl or crl can be skipped from input.
  The return value of selRes is inl_result + 256 * crl_result.
  If you're not interested in all that, just use isOK().

 */

mExpClass(Seis) SelData
{
public:

    virtual		~SelData();

    typedef SelType	Type;
    virtual Type	type() const		= 0;
    static SelData*	get(Type);		//!< empty
    static SelData*	get(const IOPar&);	//!< fully filled
    static SelData*	get(const Pos::Provider&); //!< filled; some defaults
    virtual SelData*	clone() const		= 0;
    virtual void	copyFrom(const SelData&) = 0;

    bool		operator ==(const SelData&) const;
    bool		operator !=(const SelData&) const;

    bool		isAll() const		{ return isall_; }
    void		setIsAll( bool yn=true ) { isall_ = yn; }
    inline bool		isOK( const BinID& bid ) const	{ return !selRes(bid); }
    inline bool		isOK( Pos::GeomID gid, int trcnr ) const
			{ return !selRes(gid,trcnr); }
    bool		isOK(const TrcKey&) const;
			//!< will work in trckey's domain
    bool		isOK(const Pos::IdxPair&) const;
			//!< will convert to either BinID or GeomID/trcnr

    virtual Interval<float> zRange() const;
    virtual bool	setZRange(const Interval<float>&) { return false; }
    virtual Interval<int> inlRange() const;
    virtual bool	setInlRange(const Interval<int>&) { return false; }
    virtual Interval<int> crlRange() const;
    virtual bool	setCrlRange(const Interval<int>&) { return false; }
    virtual int		expectedNrTraces(bool for2d=false,
					 const BinID* step=nullptr) const = 0;

    virtual void	fillPar(IOPar&) const		= 0;
    virtual void	usePar(const IOPar&)		= 0;
    static void		removeFromPar(IOPar&);

    virtual void	extendZ(const Interval<float>&)	= 0;
    virtual void	extendH(const BinID& stepout,
				const BinID* stepoutstep=0);
    virtual void	include(const SelData&)		= 0;

			// Interesting in some 2D situations:
    inline Pos::GeomID  geomID() const	{ return geomid_; }
    virtual void	setGeomID(Pos::GeomID geomid) { geomid_ = geomid; }

protected:

			SelData();

    bool		isall_;
    Pos::GeomID		geomid_;

    int			tracesInSI() const;
    virtual void	doExtendH(BinID stepout,BinID stepoutstep) = 0;
    virtual int		selRes3D(const BinID&) const	= 0; //!< see class doc
    virtual int		selRes2D(Pos::GeomID,int trcnr) const;

public:
    inline int		selRes( const BinID& bid ) const
			{ return selRes3D(bid); }
    inline int		selRes( Pos::GeomID gid, int trcnr ) const
			{ return selRes2D( gid, trcnr ); }

};


inline bool isEmpty( const SelData* sd )
{
    return !sd || sd->isAll();
}


} // namespace Seis
