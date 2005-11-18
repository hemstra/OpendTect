/*+
________________________________________________________________________

 CopyRight:     (C) dGB Beheer B.V.
 Author:        A.H. Lammertink
 Date:          21/09/2000
 RCS:           $Id: uifiledlg.cc,v 1.24 2005-11-18 14:02:50 cvsarend Exp $
________________________________________________________________________

-*/

#include "uifiledlg.h"
#include "filegen.h"
#include "filepath.h"
#include "oddirs.h"
#include "uiparentbody.h"

// Needed to change "Ok" and "Cancel" texts.
#define private public
#define protected public
#include <qfiledialog.h>
#undef private
#undef public
#include <qpushbutton.h>

#ifdef USEQT4
# include <private/qfiledialog_p.h>
#endif

const char* uiFileDialog::filesep = ";";

class dgbQFileDialog : public QFileDialog
{
public:
			    dgbQFileDialog( const QString& dirName,
				const QString& filter=QString::null,
				QWidget* parent=0,
				const char* name=0, bool modal = FALSE )
#ifdef USEQT4
				: QFileDialog( parent, QString(name),
					       dirName, filter )
#else
				: QFileDialog( dirName, filter, parent, name )
#endif
			    { setModal(modal); }

			    dgbQFileDialog( QWidget* parent=0,
				const char* name=0, bool modal = FALSE )
				: QFileDialog( parent, name )
			    { setModal(modal); }


};

QFileDialog::Mode qmodeForUiMode( uiFileDialog::Mode mode )
{
    switch( mode )
    {
    case uiFileDialog::AnyFile		: return QFileDialog::AnyFile;
    case uiFileDialog::ExistingFile	: return QFileDialog::ExistingFile;
    case uiFileDialog::Directory	: return QFileDialog::Directory;
    case uiFileDialog::DirectoryOnly	: return QFileDialog::DirectoryOnly;
    case uiFileDialog::ExistingFiles	: return QFileDialog::ExistingFiles;
    }
    return QFileDialog::AnyFile;
}

uiFileDialog::uiFileDialog( uiParent* parnt, bool forread,
			    const char* fname, const char* filter,
			    const char* caption )
	: UserIDObject( "uiFileDialog" )
	, mode_( forread ? ExistingFile : AnyFile )
	, fname_( fname )
	, filter_( filter )
	, caption_( caption )
	, oktxt_( "Select" )
	, parnt_( parnt )
{
    if( !caption || !*caption )
	caption_ = forread ? "Open" : "Save As";
}

uiFileDialog::uiFileDialog( uiParent* parnt, Mode mode,
			    const char* fname, const char* filter,
			    const char* caption )
	: UserIDObject( "uiFileDialog" )
	, mode_( mode )
	, fname_( fname )
	, filter_( filter )
	, caption_( caption )
	, oktxt_( "Select" )
	, parnt_( parnt )
{}

#ifdef USEQT4
# define mSetFilter	setFilter
# define mSelectFilter	selectFilter
#else
# define mSetFilter	setFilters
# define mSelectFilter	setSelectedFilter
#endif


int uiFileDialog::go()
{
    if ( !File_exists(fname_) && !File_isDirectory(fname_) )
    {
	if ( !File_isDirectory( FilePath(fname_).pathOnly() ) )
	    fname_ = GetPersonalDir();
    }

    QWidget* qp =0;

    if ( parnt_ )
	{ qp = parnt_->pbody() ? parnt_->pbody()->managewidg() : 0; }

    dgbQFileDialog* fd = new dgbQFileDialog( qp, name(), TRUE );

    fd->setMode( qmodeForUiMode(mode_) );
    fd->mSetFilter( QString(filter_) );
    fd->setCaption( QString(caption_) );
    fd->setDir( QString(fname_) );
    if ( selectedfilter_.size() )
	fd->mSelectFilter( QString(selectedfilter_) );
    
#ifdef USEQT4
    if ( fd->d_ptr )
    {
	QFileDialogPrivate* priv;
       
	priv = dynamic_cast<QFileDialogPrivate*>(fd->d_ptr);

	if ( priv )
	{
	    if ( oktxt_ != "" && priv->acceptButton )
		priv->acceptButton->setText( (const char*)oktxt_ );

	    if ( cnclxt_ != "" && priv->rejectButton )
		priv->rejectButton->setText( (const char*)cnclxt_ );
	}
    }
#else
    if ( oktxt_ != "" ) fd->okB->setText( (const char*)oktxt_ );
    if ( cnclxt_ != "") fd->cancelB->setText( (const char*)cnclxt_ );
#endif


    if ( fd->exec() != QDialog::Accepted )
	return 0;


    QStringList list = fd->selectedFiles();
#ifdef USEQT4
    if (  list.size() )
	fn = list[0].toAscii().constData();
    else 
	fn = fd->selectedFile().toAscii().constData();

    selectedfilter_ = fd->selectedFilter().toAscii().constData();
#else
    fn = list.size() ? list[0] : fd->selectedFile();
    selectedfilter_ = fd->selectedFilter();
#endif
    
#ifdef __win__
    replaceCharacter( fn.buf(), '/', '\\' );
#endif

    for ( int idx=0; idx<list.size(); idx++ )
    {
#ifdef USEQT4
	BufferString* bs = new BufferString( list[idx].toAscii().constData() );
#else
	BufferString* bs = new BufferString( list[idx] );
#endif

#ifdef __win__
	replaceCharacter( bs->buf(), '/', '\\' );
#endif
	filenames += bs;
    }

    return 1;
}


void uiFileDialog::getFileNames( BufferStringSet& fnms ) const
{
    deepCopy( fnms, filenames );
}


void uiFileDialog::list2String( const BufferStringSet& list,
				BufferString& string )
{
    QStringList qlist;
    for ( int idx=0; idx<list.size(); idx++ )
	qlist.append( (QString)list[idx]->buf() );

#ifdef USEQT4
    string = qlist.join( (QString)filesep ).toAscii().constData();
#else
    string = qlist.join( (QString)filesep );
#endif
}


void uiFileDialog::string2List( const BufferString& string,
				BufferStringSet& list )
{
    QStringList qlist = QStringList::split( (QString)filesep, (QString)string );
    for ( int idx=0; idx<qlist.size(); idx++ )
#ifdef USEQT4
	list += new BufferString( qlist[idx].toAscii().constData() );
#else
	list += new BufferString( qlist[idx] );
#endif
}
