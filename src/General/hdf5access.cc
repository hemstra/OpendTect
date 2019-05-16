/*+
 * (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 * AUTHOR   : Bert
 * DATE     : March 2018
-*/

#include "hdf5arraynd.h"
#include "arrayndimpl.h"
#include "iopar.h"
#include "uistrings.h"
#include "envvars.h"
#include "settings.h"
#include "file.h"
#include "genc.h"
#include "od_istream.h"

mImplFactory( HDF5::AccessProvider, HDF5::AccessProvider::factory );

#define mRetNoFileInUiRv() mRetInternalInUiRv( uirv, sOpenFileFirst() )
#define mRetNoDataInUiRv() mRetInternalInUiRv( uirv, sNoDataPassed() )
#define mRetDataSpaceBad() { uirv.add( sBadDataSpace() ); return uirv; }

const char* HDF5::Access::sOpenFileFirst()
{ return "HDF5: Access not open. Use/check open()"; }
const char* HDF5::Access::sNeedScope()
{ return "HDF5: No valid scope set for data retrieval. Use/check setScope()"; }
const char* HDF5::Access::sNoDataPassed()
{ return "HDF5: Null data passed"; }


BufferString HDF5::DataSetKey::fullDataSetName() const
{
    BufferString dsnm( dsnm_ );
    if ( dsnm.isEmpty() )
	dsnm.set( sGroupInfoDataSetName() );

    if ( grpnm_.isEmpty() )
	return BufferString( "/", dsnm );

    BufferString ret( "/", grpnm_, "/" );
    ret.add( dsnm );
    return ret;
}


bool HDF5::DataSetKey::hasGroup( const char* reqnm ) const
{
    const bool rootgrpreq = !reqnm || !*reqnm || FixedString(reqnm) == "/";
    const bool haverootgrp = grpnm_.isEmpty() || grpnm_ == "/";
    if ( rootgrpreq || haverootgrp )
	return rootgrpreq == haverootgrp;

    const char* grpnm = grpnm_.str();
    if ( *reqnm == '/' )
	reqnm++;
    if ( *grpnm == '/' )
	grpnm++;

    return FixedString(grpnm) == FixedString(reqnm);
}


bool HDF5::Access::isEnvBlocked( const char* typ )
{
    if ( GetEnvVarYN("OD_NO_HDF5") )
	return true;
    if ( !typ || !*typ )
	return false;

    const BufferString envvar( "OD_NO_HDF5_", BufferString(typ).toUpper() );
    return GetEnvVarYN( envvar );
}


bool HDF5::Access::isEnabled( const char* typ )
{
    if ( !HDF5::isAvailable() || isEnvBlocked(typ)
      || !Settings::common().isTrue(sSettingsEnabKey()) )
	return false;

    if ( FixedString(typ).isEmpty() )
	return true;

    const BufferString settky( sSettingsEnabKey(), ".", typ );
    return !Settings::common().isFalse( settky );
}


bool HDF5::Access::isHDF5File( const char* fnm )
{
    if ( !File::exists(fnm) )
	return false;

    od_istream strm( fnm );
    od_int64 magicnumb = 0;
    strm.getBin( magicnumb );
    if ( __islittle__ )
	SwapBytes( &magicnumb, sizeof(magicnumb) );
    return magicnumb == 0x894844460d0a1a0a;
}


HDF5::AccessProvider* HDF5::AccessProvider::mkProv( int idx )
{
    if ( idx<0 || idx>=factory().size() )
	idx = factory().size()-1;
    if ( idx<0 )
	return 0;

    return factory().create( factory().getNames().get(idx) );
}


HDF5::Reader* HDF5::AccessProvider::mkReader( int idx )
{
    AccessProvider* prov = mkProv( idx );
    Reader* rdr = 0;
    if ( prov )
    {
	rdr = prov->getReader();
	delete prov;
    }
    return rdr;
}


HDF5::Writer* HDF5::AccessProvider::mkWriter( int idx )
{
    AccessProvider* prov = mkProv( idx );
    Writer* wrr = 0;
    if ( prov )
    {
	wrr = prov->getWriter();
	delete prov;
    }
    return wrr;
}


HDF5::Access::Access()
    : file_(0)
    , myfile_(true)
{
}


HDF5::Access::~Access()
{
    // cannot do closeFile(), the reader or writer has already been destructed
}


uiRetVal HDF5::Access::open( const char* fnm )
{
    closeFile();

    uiRetVal uirv;
    openFile( fnm, uirv, false );
    myfile_ = true;

    return uirv;
}


bool HDF5::Access::setInfoScope( const char* grpnm )
{
    if ( !grpnm || !*grpnm )
	grpnm = "/";
    return setScope( DataSetKey(grpnm,DataSetKey::sGroupInfoDataSetName()) );
}


uiString HDF5::Access::sHDF5PackageDispName()
{
    return tr("HDF5 File Access");
}


uiString HDF5::Access::sHDF5NotAvailable()
{
    return tr("No HDF5 installation found");
}


uiString HDF5::Access::sHDF5FileNoLongerAccessibe()
{
    return tr("HDF5 file no longer accesible");
}


uiString HDF5::Access::sHDF5NotAvailable( const char* fnm )
{
    return sHDF5NotAvailable().append( tr("Needed to access '%1'").arg(fnm) );
}


uiString HDF5::Access::sHDF5Err( const uiString& err )
{
    uiString ret( tr("HDF5 Error") );
    if ( !err.isEmpty() )
	ret.addMoreInfo( err );
    return ret;
}


uiString HDF5::Access::sNotHDF5File( const char* fnm )
{
    return tr("'%1' is not an HDF5 file").arg( fnm );
}


uiString HDF5::Access::sDataSetNotFound( const DataSetKey& dsky )
{
    return sHDF5Err( tr("Could not find DataSet '%1'")
			.arg( dsky.fullDataSetName() ) );
}


uiString HDF5::Access::sCantSetScope( const DataSetKey& dsky ) const
{
    if ( isReader() )
	return sDataSetNotFound( dsky );

    return sHDF5Err( tr("Could not create DataSet '%1' in '%2'")
			.arg( dsky.fullDataSetName() )
			.arg(fileName()) );
}


uiString HDF5::Access::sCannotReadDataSet( const DataSetKey& dsky )
{
    return sHDF5Err( tr("Could not read DataSet '%1'")
			.arg( dsky.fullDataSetName() ) );
}


uiString HDF5::Access::sFileNotOpen()
{
    return sHDF5Err( tr("Could not open file") );
}


uiRetVal HDF5::Writer::open4Edit( const char* fnm )
{
    closeFile();

    uiRetVal uirv;
    openFile( fnm, uirv, true );
    myfile_ = true;

    return uirv;
}


uiRetVal HDF5::Writer::createDataSet( const DataSetKey& dsky,
				      int sz, ODDataType dt )
{
    return createDataSet( dsky, Array1DInfoImpl(sz), dt );
}


uiRetVal HDF5::Writer::createDataSet( const DataSetKey& dsky,
				      const ArrayNDInfo& inf, ODDataType dt )
{
    uiRetVal uirv;
    if ( !file_ )
	mRetNoFileInUiRv()
    else if ( inf.getTotalSz() < 1 )
	{ pErrMsg("zero dims"); }

    crDS( dsky, inf, dt, uirv );
    return uirv;
}


uiRetVal HDF5::Writer::resizeDataSet( const DataSetKey& dsky,
				      const ArrayNDInfo& inf )
{
    uiRetVal uirv;
    if ( !file_ )
	mRetNoFileInUiRv()
    else if ( inf.getTotalSz() < 1 )
	{ pErrMsg("zero dims"); }

    reSzDS( dsky, inf, uirv );
    return uirv;
}


bool HDF5::Writer::deleteObject( const DataSetKey& dsky )
{
    return file_ ? rmObj( dsky ) : true;
}



uiRetVal HDF5::Writer::putInfo( const IOPar& iop )
{
    uiRetVal uirv;
    if ( !file_ )
	mRetNoFileInUiRv()

    if ( !iop.isEmpty() )
	ptInfo( iop, uirv, 0 );
    return uirv;
}



uiRetVal HDF5::Writer::putInfo( const DataSetKey& dsky, const IOPar& iop )
{
    uiRetVal uirv;
    if ( !file_ )
	mRetNoFileInUiRv()

    if ( !iop.isEmpty() )
	ptInfo( iop, uirv, &dsky );
    return uirv;
}


uiRetVal HDF5::Writer::put( const DataSetKey& dsky, const BufferStringSet& bss )
{
    uiRetVal uirv;
    if ( !file_ )
	mRetNoFileInUiRv()

    if ( !bss.isEmpty() )
	ptStrings( dsky, bss, uirv );
    return uirv;
}


uiRetVal HDF5::Writer::putAll( const void* data )
{
    uiRetVal uirv;
    if ( !file_ )
	mRetNoFileInUiRv()
    if ( !data )
	mRetNoDataInUiRv()

    ptAll( data, uirv );
    return uirv;
}


uiRetVal HDF5::Writer::putSlab( const SlabSpec& spec, const void* data )
{
    uiRetVal uirv;
    if ( !file_ )
	mRetNoFileInUiRv()
    if ( !data )
	mRetNoDataInUiRv()

    ptSlab( spec, data, uirv );
    return uirv;
}



HDF5::Access::size_type HDF5::Reader::dimSize( dim_idx_type idim ) const
{
    ArrayNDInfo* inf = gtDataSizes();
    const size_type ret = inf ? inf->getSize( idim ) : 0;
    delete inf;
    return ret;
}


uiRetVal HDF5::Reader::getInfo( IOPar& iop ) const
{
    uiRetVal uirv;
    if ( !file_ )
	mRetNoFileInUiRv()

    gtInfo( iop, uirv );
    return uirv;
}


uiRetVal HDF5::Reader::get( BufferStringSet& bss ) const
{
    uiRetVal uirv;
    if ( !file_ )
	mRetNoFileInUiRv()
    const auto nrdims = nrDims();
    if ( nrdims != 1 )
	mRetDataSpaceBad()

    gtStrings( bss, uirv );
    return uirv;
}


uiRetVal HDF5::Reader::getAll( void* data ) const
{
    uiRetVal uirv;
    if ( !file_ )
	mRetNoFileInUiRv()
    if ( !data )
	mRetNoDataInUiRv()
    const auto nrdims = nrDims();
    if ( nrdims < 1 )
	mRetDataSpaceBad()

    gtAll( data, uirv );
    return uirv;
}


uiRetVal HDF5::Reader::getPoint( NDPos pos, void* data ) const
{
    uiRetVal uirv;
    if ( !file_ )
	mRetNoFileInUiRv()
    if ( !data )
	mRetNoDataInUiRv()
    if ( !pos )
	mRetInternalInUiRv( uirv, "No position provided" )
    const auto nrdims = nrDims();
    if ( nrdims < 1 )
	mRetDataSpaceBad()

    NDPosBufSet pts;
    pts += mNDPosBufFromPos( pos, nrdims );
    gtPoints( pts, data, uirv );
    return uirv;
}


uiRetVal HDF5::Reader::getPoints( const NDPosBufSet& pts, void* data ) const
{
    uiRetVal uirv;
    if ( !file_ )
	mRetNoFileInUiRv()
    if ( pts.isEmpty() )
	return uirv;
    if ( !data )
	mRetNoDataInUiRv()
    const auto nrdims = nrDims();
    if ( nrdims < 1 )
	mRetDataSpaceBad()

    gtPoints( pts, data, uirv );
    return uirv;
}


uiRetVal HDF5::Reader::getSlab( const SlabSpec& spec, void* data ) const
{
    uiRetVal uirv;
    if ( !file_ )
	mRetNoFileInUiRv()
    if ( !data )
	mRetNoDataInUiRv()
    const auto nrdims = nrDims();
    if ( nrdims < 1 )
	mRetDataSpaceBad()
    if ( spec.size() != nrdims )
    {
	if ( spec.size() < nrdims )
	    mRetInternalInUiRv( uirv, "Specify all dimensions in SlabSpec" );
	pErrMsg( "Probably wrong: SlabSpec too big" );
    }

    gtSlab( spec, data, uirv );
    return uirv;
}