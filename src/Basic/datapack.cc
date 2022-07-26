/*+
 * (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 * AUTHOR   : A.H. Bril
 * DATE     : Jan 2007
-*/


#include "datapack.h"

#include "ascstream.h"
#include "atomic.h"
#include "debug.h"
#include "file.h"
#include "iopar.h"
#include "keystrs.h"
#include "od_ostream.h"
#include "envvars.h"
#include "msgh.h"

#include <iostream>

DataPackMgr::MgrID DataPackMgr::BufID()		{ return MgrID::get(1); }
DataPackMgr::MgrID DataPackMgr::PointID()	{ return MgrID::get(2); }
DataPackMgr::MgrID DataPackMgr::SeisID()	{ return MgrID::get(3); }
DataPackMgr::MgrID DataPackMgr::FlatID()	{ return MgrID::get(4); }
DataPackMgr::MgrID DataPackMgr::SurfID()	{ return MgrID::get(5); }
const char* DataPack::sKeyCategory()		{ return "Category"; }
float DataPack::sKb2MbFac()			{ return 0.0009765625; }

Threads::Lock DataPackMgr::mgrlistlock_;
ManagedObjectSet<DataPackMgr> DataPackMgr::mgrs_;

#ifdef __debug__
# define mTrackDPMsg(msg) \
    if ( trackDataPacks() ) \
	DBG::message( msg )
#else
# define mTrackDPMsg(msg) \
    if ( trackDataPacks() ) \
	UsrMsg( msg, MsgClass::Info )
#endif

static bool trackDataPacks()
{
    mDefineStaticLocalObject( bool, dotrack,
			      = GetEnvVarYN( "OD_TRACK_DATAPACKS" ) );
#ifdef __debug__
    if ( !DBG::isOn() )
	DBG::turnOn( DBG_DBG );
#endif
    return dotrack;
}


DataPack::FullID DataPack::FullID::getFromString( const char* str )
{
    FullID fid = getInvalid();
    if ( !isValidString(str) )
	return fid;

    fid.fromString( str );
    return fid;
}


DataPack::FullID DataPack::FullID::getInvalid()
{
    const GroupedID grid = GroupedID::getInvalid();
    return FullID( grid.groupID(), grid.objID() );
}


static Threads::Atomic<int> curdpidnr( 0 );
static Threads::Atomic<int> deletedid_( DataPack::ID::udf().asInt() );

DataPack::DataPack( const char* categry )
    : SharedObject("<?>")
    , category_(categry)
    , id_(getNewID())
{
}


DataPack::DataPack( const DataPack& dp )
    : SharedObject( dp.name().buf() )
    , category_( dp.category_ )
    , id_(getNewID())
{
}


DataPack::~DataPack()
{
    if ( manager_ && trackDataPacks() )
	deletedid_ = id_.asInt();
}


DataPack::ID DataPack::getNewID()
{
    return ID::get( ++curdpidnr );
}


void DataPack::setManager( const DataPackMgr* mgr )
{
    if ( manager_ && mgr )
    {
	if ( manager_ != mgr )
	    DBG::forceCrash( false );

	return;
    }

    manager_ = mgr;
}


DataPackMgr* DataPackMgr::gtDPM( DataPackMgr::MgrID dpid, bool crnew )
{
    Threads::Locker lock( mgrlistlock_ );

    for ( auto* mgr : mgrs_ )
    {
	if ( mgr->id() == dpid )
	    return mgr;
    }
    if ( !crnew )
	return nullptr;

    auto* newmgr = new DataPackMgr( dpid );
    mgrs_ += newmgr;
    return newmgr;
}


DataPackMgr& DataPackMgr::DPM( DataPackMgr::MgrID dpid )
{
    return *gtDPM( dpid, true );
}


DataPackMgr& DPM( DataPackMgr::MgrID dpid )
{
    return DataPackMgr::DPM( dpid );
}


DataPackMgr& DPM( const DataPack::FullID& fid )
{
    const DataPackMgr::MgrID manid = fid.mgrID();
    DataPackMgr* dpm = DataPackMgr::gtDPM( manid, false );
    if ( dpm ) return *dpm;

    mDefineStaticLocalObject( PtrMan<DataPackMgr>, emptymgr, = 0 );
    emptymgr = new DataPackMgr( manid );
    return *emptymgr;
}


const char* DataPackMgr::nameOf( const DataPack::FullID& fid )
{
    return ::DPM(fid).nameOf( fid.packID() );
}


const char* DataPackMgr::categoryOf( const DataPack::FullID& fid )
{
    return ::DPM(fid).categoryOf( fid.packID() );
}


void DataPackMgr::dumpDPMs( od_ostream& strm )
{
    Threads::Locker lock( mgrlistlock_ );
    strm << "** Data Pack Manager dump **\n";
    if ( mgrs_.isEmpty() )
	{ strm << "No Data pack managers (yet)" << od_newline; return; }

    for ( const auto* mgr : mgrs_ )
    {
	strm << "\n\n";
	mgr->dumpInfo(strm);
    }
}


DataPackMgr::DataPackMgr( DataPackMgr::MgrID dpid )
    : id_(dpid)
    , newPack(this)
    , packToBeRemoved(this)
{
}


DataPackMgr::~DataPackMgr()
{
    detachAllNotifiers();
#ifdef __debug__
    //Don't do in release mode, as we may have race conditions of sta-tic
    //variables deleting at different times
    for ( int idx=0; idx<packs_.size(); idx++ )
    {
	ConstRefMan<DataPack> pack = packs_[idx];
	if ( !pack )
	    continue;

	//Using std C++ function because we cannot use pErrMsg or BufferString
	std::cerr << "(PE) DataPackMgr | Datapack " << pack->id().asInt();
	if ( pack->category() )
	    std::cerr << " with category " << pack->category();
	std::cerr << " is still referenced.\n";
    }
#endif
}


const char* DataPackMgr::categoryOf( PackID dpid ) const
{
    ConstRefMan<DataPack> pack = getDP( dpid );
    if ( !pack )
	return nullptr;

    pack.setNoDelete( true );
    return pack->category();
}


bool DataPackMgr::doAdd( const DataPack* constdp )
{
    auto* dp = const_cast<DataPack*>( constdp );
    if ( !dp )
	{ pErrMsg("null datapack to add"); return false; }
    else if ( isPresent(dp->id()) )
	return false;

    RefMan<DataPack> keeper = dp;
    keeper.setNoDelete( true );
    dp->setManager( this );
    mAttachCB( dp->objectToBeDeleted(), DataPackMgr::packDeleted );

    packs_ += dp;

    mTrackDPMsg( BufferString("[DP]: add ",dp->id().asInt(),
		 BufferString(" '",dp->name(),"'")) );

    newPack.trigger( dp );
    return true;
}


void DataPackMgr::packDeleted( CallBacker* cb )
{
    mDynamicCastGet(SharedObject*,shobj,cb)
    if ( !shobj )
	return;

    const RefCount::WeakPtrSetBase::CleanupBlocker cleanupblock( packs_ );
    for ( int idx=0; idx<packs_.size(); idx++ )
    {
	ConstRefMan<DataPack> pack = packs_[idx];
	pack.setNoDelete(true);
	if ( pack )
	    continue;

	if ( trackDataPacks() )
	{
	    mTrackDPMsg( BufferString("[DP]: delete ", deletedid_,
			 BufferString(" '",shobj->name(),"'")) );
	    deletedid_ = DataPack::ID().udf().asInt();
	}

	packs_.removeSingle( idx );
	break;
    }
}


void DataPackMgr::dumpInfo( od_ostream& strm ) const
{
    strm << "Manager.ID: " << id().asInt() << od_newline;
    const auto nrkb = static_cast<od_int64>( nrKBytes()+0.5f );
    strm << "Total memory: " << File::getFileSizeString(nrkb)
			     << od_newline;
    ascostream astrm( strm );
    astrm.newParagraph();

    const RefCount::WeakPtrSetBase::CleanupBlocker cleanupblock( packs_ );
    for ( int idx=0; idx<packs_.size(); idx++ )
    {
	ConstRefMan<DataPack> pack = packs_[idx];
	pack.setNoDelete(true);
	if ( !pack )
	    continue;

	IOPar iop;
	pack->dumpInfo( iop );
	if ( !iop.isEmpty() )
	    iop.putTo( astrm );

    }
}


void DataPackMgr::dumpInfoFor( PackID dpid, IOPar& iop ) const
{
    ConstRefMan<DataPack> pack = getDP( dpid );
    pack.setNoDelete( true );
    if ( pack )
	pack->dumpInfo( iop );
}


RefMan<DataPack> DataPackMgr::getDP( PackID dpid )
{
    const RefCount::WeakPtrSetBase::CleanupBlocker cleanupblock( packs_ );

    for ( int idx=0; idx<packs_.size(); idx++ )
    {
	RefMan<DataPack> pack = packs_[idx];
	if ( pack && pack->id() == dpid )
	    return pack;

	pack.setNoDelete( true );
    }

    return nullptr;
}


ConstRefMan<DataPack> DataPackMgr::getDP( PackID dpid ) const
{
    const RefCount::WeakPtrSetBase::CleanupBlocker cleanupblock( packs_ );

    for ( int idx=0; idx<packs_.size(); idx++ )
    {
	ConstRefMan<DataPack> pack = packs_[idx];
	if ( pack && pack->id() == dpid )
	    return pack;

	pack.setNoDelete( true );
    }

    return nullptr;
}


void DataPackMgr::getPackIDs( TypeSet<PackID>& ids ) const
{
    const RefCount::WeakPtrSetBase::CleanupBlocker cleanupblock( packs_ );

    for ( int idx=0; idx<packs_.size(); idx++ )
    {
	ConstRefMan<DataPack> pack = packs_[idx];
	pack.setNoDelete(true);
	if ( pack )
	    ids += pack->id();
    }
}


bool DataPackMgr::isPresent( PackID packid ) const
{
    const RefCount::WeakPtrSetBase::CleanupBlocker cleanupblock( packs_ );

    for ( int idx=0; idx<packs_.size(); idx++ )
    {
	ConstRefMan<DataPack> pack = packs_[idx];
	pack.setNoDelete(true);
	if ( pack && pack->id() == packid )
	    return true;
    }
    return false;
}


const char* DataPackMgr::nameOf( PackID dpid ) const
{
    ConstRefMan<DataPack> pack = getDP( dpid );
    if ( !pack )
	return nullptr;

    pack.setNoDelete( true );
    return pack->name();
}


float DataPackMgr::nrKBytes() const
{
    float res = 0.f;
    for ( int idx=0; idx<packs_.size(); idx++ )
    {
	ConstRefMan<DataPack> pack = packs_[idx];
	pack.setNoDelete(true);
	if ( pack )
	    res += pack->nrKBytes();
    }
    return res;
}


float DataPackMgr::nrKBytesOf( PackID dpid ) const
{
    ConstRefMan<DataPack> pack = getDP( dpid );
    if ( !pack )
	return 0.f;

    pack.setNoDelete( true );
    return pack->nrKBytes();
}



WeakPtr<DataPack> DataPackMgr::observeDP( PackID dpid ) const
{
    const RefCount::WeakPtrSetBase::CleanupBlocker cleanupblock( packs_ );
    for ( int idx=0; idx<packs_.size(); idx++ )
    {
	RefMan<DataPack> pack = packs_[idx];
	pack.setNoDelete(true);
	if ( pack && pack->id() == dpid )
	    return WeakPtr<DataPack>( pack );
    }

    return nullptr;
}


bool DataPackMgr::ref( PackID dpid )
{
    RefMan<DataPack> pack = getDP( dpid );
    if ( pack )
	{ pack->ref(); return true; }
    return false;
}


bool DataPackMgr::unRef( PackID dpid )
{
    RefMan<DataPack> pack = getDP( dpid );
    if ( pack )
	{ pack->unRef(); return true; }

    return false;
}


void DataPack::dumpInfo( IOPar& iop ) const
{
    iop.set( sKeyCategory(), category() );
    iop.set( sKey::Name(), name() );
    iop.set( "Pack.ID", id_.asInt() );
    iop.set( "Nr users", nrRefs() > 0 ? nrRefs()-1 : nrRefs() );
		// Omitting the reference coming from this function call
    const od_int64 nrkb = static_cast<od_int64>( nrKBytes()+0.5f );
    iop.set( "Memory consumption", File::getFileSizeString(nrkb) );
}


BufferDataPack::BufferDataPack( char* b, od_int64 sz, const char* catgry )
    : DataPack(catgry)
{
    setBuf( b, sz );
}


BufferDataPack::BufferDataPack( const BufferDataPack& oth )
    : DataPack(oth)
{
    setBuf( oth.sz_>0 ? new char[oth.sz_] : nullptr, oth.sz_ );
    if ( buf_ )
	OD::memCopy( buf_, oth.buf_, sz_ );
}


BufferDataPack::~BufferDataPack()
{
    delete [] buf_;
}


void BufferDataPack::setBuf( char* b, od_int64 sz )
{
    delete [] buf_;
    buf_ = b;
    sz_ = buf_ ? sz : 0;
}
