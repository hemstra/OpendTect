/*+
 * COPYRIGHT: (C) dGB Beheer B.V.
 * AUTHOR   : A.H. Bril
 * DATE     : Jul 2006
-*/

static const char* rcsID = "$Id: tableconv.cc,v 1.3 2006-07-26 15:48:39 cvsbert Exp $";

#include "tableconvimpl.h"


void TableImportHandler::addToCol( char c )
{
    if ( colpos_ < col_.bufSize() )
	*(col_.buf() + colpos_) = c;
    else
    {
	char buf[2]; buf[0] = c; buf[1] = '\0';
	col_ += buf;
    }
    colpos_++;
}


bool TableExportHandler::isNumber( const char* str )
{
    return isNumberString( str, NO );
}


int TableConverter::nextStep()
{
    selcolnr_ = colnr_ = 0;

    while ( true )
    {
	char c = readNewChar();
	if ( istrm_.eof() )
	    return 0;

	TableImportHandler::State impstate = imphndlr_.add( c );
	if ( !handleImpState(impstate) )
	    return -1;
	if ( impstate == TableImportHandler::EndRow )
	    return 1;
    }
}


bool TableConverter::handleImpState( TableImportHandler::State impstate )
{
    switch ( impstate )
    {

    case TableImportHandler::Error:

	msg_ = imphndlr_.errMsg();

    return false;

    case TableImportHandler::EndCol: {

	if ( colSel() )
	{
	    row_.add( imphndlr_.getCol() );
	    selcolnr_++;
	}
	colnr_++;
	imphndlr_.newCol();

    return true; }

    case TableImportHandler::EndRow: {

	if ( !handleImpState(TableImportHandler::EndCol) )
	    return false;

	if ( !manipulator_ || manipulator_->accept(row_) )
	{
	    const char* msg = exphndlr_.putRow( row_, ostrm_ );
	    if ( msg && *msg )
	    {
		msg_ = msg;
		return false;
	    }

	    rowsdone_++;
	}
	row_.deepErase();
	imphndlr_.newRow();

    return true; }

    default:
    break;

    }

    return true;

}


TableImportHandler::State CSVTableImportHandler::add( char c )
{
    if ( c == '"' )
	{ instring_ = !instring_; return InCol; }
    else if ( c == '\n' )
    {
	if ( instring_ )
	    c = nlreplace_;
	else
	{
	    addToCol( '\0' );
	    return EndRow;
	}
    }
    else if ( c == ',' && !instring_ )
    {
	addToCol( '\0' );
	return EndCol;
    }

    addToCol( c );
    return InCol;
}


void CSVTableExportHandler::addVal( std::ostream& strm, int col,
				    const char* val )
{
    if ( col )
	strm << ',';
    const bool needsquotes = *val && !isNumber( val );
    if ( needsquotes )
	strm << '"';
    if ( *val )
	strm << val;
    if ( needsquotes )
	strm << '"';
}


const char* CSVTableExportHandler::putRow( const BufferStringSet& row,
					   std::ostream& strm )
{
    for ( int idx=0; idx<row.size(); idx++ )
	addVal( strm, idx, row.get(idx) );
    strm << std::endl;

    return strm.good() ? 0 : "Error writing to output";
}


void SQLInsertTableExportHandler::addVal( std::ostream& strm, int col,
					  const char* val )
{
    if ( col )
	strm << ',';
    const bool needsquotes = val && *val && !isNumber( val );
    if ( needsquotes )
	strm << "'";
    if ( val && *val )
	strm << val;
    if ( needsquotes )
	strm << "'";
}


const char* SQLInsertTableExportHandler::putRow( const BufferStringSet& row,
						 std::ostream& strm )
{
    if ( tblname_ == "" )
	return "No table name provided";

    strm << "INSERT INTO " << tblname_;
    if ( colnms_.size() )
    {
	strm << " (" << colnms_.get(0).buf();
	for ( int idx=1; idx<colnms_.size(); idx++ )
	    strm << ',' << colnms_.get(idx).buf();
	strm << ')';
    }
    strm << " VALUES (";

    for ( int idx=0; idx<row.size(); idx++ )
	addVal( strm, idx, row.get(idx) );

    strm << ");" << std::endl;

    return strm.good() ? 0 : "Error writing to output";
}


void TCDuplicateKeyRemover::setPrevKeys( const BufferStringSet& cols ) const
{
    if ( prevkeys_.size() < 1 )
    {
	for ( int idx=0; idx<keycols_.size(); idx++ )
	    prevkeys_.add( "" );
    }

    for ( int idx=0; idx<keycols_.size(); idx++ )
	prevkeys_.get(idx) = cols.get( keycols_[idx] );
}


bool TCDuplicateKeyRemover::accept( BufferStringSet& cols ) const
{
    nrdone_++;
    if ( keycols_.size() < 1 )
	return true;

    if ( nrdone_ == 1 )
	{ setPrevKeys( cols ); return true; }

    for ( int idx=0; idx<keycols_.size(); idx++ )
    {
	if ( cols.get(keycols_[idx]) != prevkeys_.get(idx) )
	    { setPrevKeys( cols ); return true; }
    }

    nrremoved_++;
    return false;
}
