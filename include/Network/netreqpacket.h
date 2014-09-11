#ifndef netreqpacket_h
#define netreqpacket_h

/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:	K. Tingdahl
 Date:		August 2014
 RCS:		$Id$
________________________________________________________________________

-*/

#include "networkmod.h"
#include "bufstringset.h"

#define mRequestPacketHeaderSize		10


namespace Network
{

/*\brief Standardized packet that can be sent over a Tcp connection

  The request header contains a unique (int) request ID and a (short) sub-ID.
  The sub-ID is a flag that can be used to implement your protocol. Negative
  sub-IDs are special and cannot be freely used.

  */

mExpClass(Network) RequestPacket
{
public:
			RequestPacket(od_int32 payloadsize=-1);
			~RequestPacket();

    bool		isOK() const;
			//!< checks whether the header is reasonable

    static od_int32	headerSize() { return mRequestPacketHeaderSize; }
    static od_int32	getPayloadSize(const void*);

    int			setIsNewRequest(); //!< conveniently returns reqID()
    bool		isNewRequest() const { return subID()==cBeginSubID(); }
    void		setIsRequestEnd(int reqid);
    bool		isRequestEnd() const { return subID()==cEndSubID(); }

    od_int32		requestID() const;
    void		setRequestID(od_int32);
    od_int16		subID() const;
    void		setSubID(od_int16);

    od_int32		payloadSize() const;
    const void*		payload() const;
    void*		payload(bool takeover=false);
			/*!< if takeover, cast to char*, and delete with []. */
    void		getStringPayload(BufferString&) const;

    void		setPayload(void*);
			/*!< should be allocated as new char[payloadSize()] */
    void		setPayload(void*,od_int32 size);
			//!< becomes mine
    void		setStringPayload(const char*);

    static od_int16	cBeginSubID()		{ return -2; }
    static od_int16	cEndSubID()		{ return -1; }
    static od_int16	cNormalSubID()	{ return 0; }

    void*		getRawHeader()		{ return header_.int32s_; }
    const void*		getRawHeader() const	{ return header_.int32s_; }

    /*\brief interprets what is in a packet.

      You need to know what should be in there. Arrays/sets are always
      preceeded by their size.

      */

    mExpClass(Network) Interpreter
    {
    public:
				Interpreter( const RequestPacket& p )
				    : pkt_(p)
				    , curpos_(mRequestPacketHeaderSize) {}

	template <class T> void	get(T&) const;
	inline int		getInt() const;
	inline float		getFloat() const;
	inline double		getDouble() const;
	inline BufferString	getString() const;

	template <class T> void	getArr(T*,int maxsz) const;
	template <class T> void	getSet(TypeSet<T>&,int maxsz=-1) const;
	inline void		getSet(BufferStringSet&,int maxsz=-1) const;

    protected:

	const RequestPacket& pkt_;
	mutable int	curpos_;

    };

protected:

    union Header
    {
	od_int32	int32s_[2];
	od_int16	int16s_[5]; //only number 4 is used
    };


    Header		header_;
    char*		payload_;

    friend class	Interpreter;
};


template <class T>
inline void RequestPacket::Interpreter::get( T& var ) const
{
    OD::memCopy( &var, pkt_.payload_+curpos_, sizeof(T) );
    curpos_ += sizeof(T);
}

template <>
inline void RequestPacket::Interpreter::get( BufferString& var ) const
{
    int sz; get( sz );
    var.setBufSize( sz+1 );
    char* cstr = var.getCStr();
    OD::memCopy( cstr, pkt_.payload_+curpos_, sz );
    cstr[sz] = '\0';
    curpos_ += sz;
}

inline int RequestPacket::Interpreter::getInt() const
{ int ret = 0; get(ret); return ret; }
inline float RequestPacket::Interpreter::getFloat() const
{ float ret = 0.f; get(ret); return ret; }
inline double RequestPacket::Interpreter::getDouble() const
{ double ret = 0.; get(ret); return ret; }
inline BufferString RequestPacket::Interpreter::getString() const
{ BufferString ret; get(ret); return ret; }

template <class T>
inline void RequestPacket::Interpreter::getArr( T* arr, int maxsz ) const
{
    int arrsz; get( arrsz );
    int sz = arrsz;
    if ( sz > maxsz )
	sz = maxsz;
    if ( arrsz < 1 )
	return;

    OD::memCopy( arr, pkt_.payload_+curpos_, sz*sizeof(T) );
    curpos_ += arrsz;
}

template <class T>
inline void RequestPacket::Interpreter::getSet( TypeSet<T>& ts, int maxsz) const
{
    int arrsz; get( arrsz );
    int sz = arrsz;
    if ( maxsz >= 0 && sz > maxsz )
	sz = maxsz;
    if ( arrsz < 1 )
	{ ts.setEmpty(); return; }

    ts.setSize( sz );
    OD::memCopy( ts.arr(), pkt_.payload_+curpos_, sz*sizeof(T) );
    curpos_ += arrsz;
}

inline void RequestPacket::Interpreter::getSet( BufferStringSet& bss,
						int maxsz ) const
{
    int setsz; get( setsz );
    int sz = setsz;
    if ( maxsz >= 0 && sz > maxsz )
	sz = maxsz;
    bss.setEmpty();
    if ( setsz < 1 )
	return;

    for ( int idx=0; idx<sz; idx++ )
    {
	BufferString* bs = new BufferString;
	get( *bs );
	bss += bs;
    }

    for ( int idx=sz; idx<setsz; idx++ )
	getString();
}


}; //Namespace


#endif
