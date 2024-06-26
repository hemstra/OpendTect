#pragma once
/*+
________________________________________________________________________

 Copyright:	(C) 1995-2022 dGB Beheer B.V.
 License:	https://dgbes.com/licensing
________________________________________________________________________

-*/

#include "earthmodelmod.h"
#include "tableascio.h"
#include "od_istream.h"

namespace ZDomain { class Def; }

namespace EM
{

/*!
\brief Ascii I/O for Horizon3D.
*/

mExpClass(EarthModel) Horizon3DAscIO : public Table::AscIO
{
public:
				Horizon3DAscIO(const Table::FormatDesc&,
						const char* filenm);
				~Horizon3DAscIO();

    static Table::FormatDesc*   getDesc();
    static void			updateDesc(Table::FormatDesc&,
					   const BufferStringSet&);
    static void			createDescBody(Table::FormatDesc*,
					   const BufferStringSet&);

    bool			isXY() const;
    bool			isOK() const { return strm_.isOK(); }
    int				getNextLine(Coord&,TypeSet<float>&);
    const UnitOfMeasure*	getSelZUnit() const;

    static const char*		sKeyFormatStr();
    static const char*		sKeyAttribFormatStr();

protected:

    od_istream			strm_;
    float			udfval_				= mUdf(float);
    bool			finishedreadingheader_		= false;

};


/*!
\brief Ascii I/O for Horizon2D.
*/

mExpClass(EarthModel) Horizon2DAscIO : public Table::AscIO
{ mODTextTranslationClass(Horizon2DAscIO)
public:
				Horizon2DAscIO(const Table::FormatDesc&,
						const char* filenm);
				~Horizon2DAscIO();

    static Table::FormatDesc*	getDesc(const ZDomain::Def&);
    static void			updateDesc(Table::FormatDesc&,
						    const ZDomain::Def&);
    static void			createDescBody(Table::FormatDesc*,
							const ZDomain::Def&);

    static bool			isFormatOK(const Table::FormatDesc&,
					   uiString&);
    int				getNextLine(BufferString& lnm,Coord& crd,
					    int& trcnr,float& spnm,
					    TypeSet<float>& data);
    bool			isTraceNr() const;
    bool			isOK() const { return strm_.isOK(); }

protected:

    od_istream			strm_;
    float			udfval_				= mUdf(float);
    bool			finishedreadingheader_		= false;

};

} // namespace EM
