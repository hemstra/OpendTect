#ifndef linesetposinfo_h
#define linesetposinfo_h

/*+
________________________________________________________________________

 CopyRight:	(C) dGB Beheer B.V.
 Author:	Bert
 Date:		2005 / Mar 2008
 RCS:		$Id: linesetposinfo.h,v 1.1 2008-03-12 09:48:03 cvsbert Exp $
________________________________________________________________________

-*/

#include "posinfo.h"
class BinIDValueSet;


namespace PosInfo
{

/*!\brief Position info for a set of 2D lines */

class LineSet2DData
{
public:

    virtual		~LineSet2DData()	{ deepErase(data_); }

    Line2DData&		addLine(const char*);
    int			nrLines() const		{ return data_.size(); }
    const BufferString&	lineName( int idx ) const
    			{ return data_[idx]->lnm_; }
    const Line2DData&	lineData( int idx ) const
    			{ return data_[idx]->pos_; }
    const Line2DData*	getLineData(const char*) const;
    void		removeLine(const char*);

    struct IR
    {
			IR() : posns_(0)	{}
			~IR(); //		{ delete posns_; }

	BufferString	lnm_;
	BinIDValueSet*	posns_;
    };
    void		intersect(const BinIDValueSet&,ObjectSet<IR>&) const;

protected:

    struct Info
    {
	BufferString	lnm_;
	Line2DData	pos_;
    };

    ObjectSet<Info>	data_;

    Info*		findLine(const char*) const;

};


} // namespace PosInfo

#endif
