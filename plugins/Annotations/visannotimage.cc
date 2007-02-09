/*+
________________________________________________________________________

 CopyRight:     (C) dGB Beheer B.V.
 Author:        K. Tingdahl
 Date:          Jan 2005
 RCS:           $Id: visannotimage.cc,v 1.1 2007-02-09 14:10:43 cvskris Exp $
________________________________________________________________________

-*/

#include "visannotimage.h"

#include "pickset.h"
#include "viscoord.h"
#include "vistexturecoords.h"
#include "visfaceset.h"
#include "visimage.h"
#include "vispolygonoffset.h"
#include "vistransform.h"

namespace Annotations
{

class Image : public visBase::VisualObjectImpl
{
public:
    static Image*		create()
				mCreateDataObj(Image);
    void			setShape(visBase::FaceSet*);

    void			setPick(const Pick::Location&);
    void			setDisplayTransformation(mVisTrans*);

protected:
    				~Image();

    visBase::Transformation*	position_;
    visBase::Transformation*	transform_;
    visBase::FaceSet*		shape_;
};


mCreateFactoryEntry( ImageDisplay );
mCreateFactoryEntry( Image );

Image::Image()
    : visBase::VisualObjectImpl( false )
    , position_( visBase::Transformation::create() )
    , transform_( 0 )
    , shape_( 0 )
{
    setMaterial( 0 );
    position_->ref();
    addChild( position_->getInventorNode() );
}


Image::~Image()
{
    shape_->unRef();
    position_->unRef();
    if ( transform_ ) transform_->unRef();
}


void Image::setShape( visBase::FaceSet* fs )
{
    if ( shape_ )
    {
	removeChild( shape_->getInventorNode() );
	shape_->unRef();
    }

    shape_ = fs;

    if ( shape_ )
    {
	shape_->ref();
	addChild( shape_->getInventorNode() );
    }
}


void Image::setDisplayTransformation( mVisTrans* trans )
{
    if ( transform_ ) transform_->unRef();
    transform_ = trans;
    if ( transform_ ) transform_->ref();
}


void Image::setPick( const Pick::Location& loc )
{
    const Coord3 displaypos =
	transform_ ? transform_->transform(loc.pos) : loc.pos;
    position_->setTranslation( displaypos );
    position_->setRotation( Coord3(0,0,1), loc.dir.phi );
}


ImageDisplay::ImageDisplay()
    : shape_( visBase::FaceSet::create() )
    , offset_( visBase::PolygonOffset::create() )
    , image_( visBase::Image::create() )
{
    offset_->ref();
    insertChild(childIndex( group_->getInventorNode() ),
			offset_->getInventorNode());

    image_->ref();
    image_->replaceMaterial(true);
    insertChild( childIndex( group_->getInventorNode() ),
	    	image_->getInventorNode());

    shape_->ref();
    shape_->setVertexOrdering(
		visBase::VertexShape::cCounterClockWiseVertexOrdering() );

    visBase::Coordinates* facecoords = shape_->getCoordinates();
    facecoords->setPos( 0, Coord3(-1,-1,0) );
    facecoords->setPos( 1, Coord3(-1,1,0) );
    facecoords->setPos( 2, Coord3(1,1,0) );
    facecoords->setPos( 3, Coord3(1,-1,0) );

    shape_->setCoordIndex(0,3);
    shape_->setCoordIndex(1,2);
    shape_->setCoordIndex(2,1);
    shape_->setCoordIndex(3,0);
    shape_->setCoordIndex(4,-1);

    visBase::TextureCoords* texturecoords = visBase::TextureCoords::create();
    shape_->setTextureCoords( texturecoords );
    texturecoords->setCoord( 0, Coord3(0,0,0) );
    texturecoords->setCoord( 1, Coord3(0,1,0) );
    texturecoords->setCoord( 2, Coord3(1,1,0) );
    texturecoords->setCoord( 3, Coord3(1,0,0) );
    shape_->setTextureCoordIndex( 0, 3 );
    shape_->setTextureCoordIndex( 1, 2 );
    shape_->setTextureCoordIndex( 2, 1 );
    shape_->setTextureCoordIndex( 3, 0 );
    shape_->setTextureCoordIndex( 4, -1 );
}


ImageDisplay::~ImageDisplay()
{
    shape_->unRef();
    offset_->unRef();
    image_->unRef();
}


bool ImageDisplay::setFileName( const char* nm )
{
    image_->setFileName( nm );
    return true;
}


const char* ImageDisplay::getFileName() const
{
    return image_->getFileName();
}


void ImageDisplay::setScene( visSurvey::Scene* scene )
{
    if ( scene_ ) scene_->zscalechange.remove(
	    mCB( this, ImageDisplay, updateCoords));

    visSurvey::SurveyObject::setScene( scene );

    if ( scene_ ) scene_->zscalechange.notify(
	    mCB( this, ImageDisplay, updateCoords));

    updateCoords();
}


visBase::VisualObject* ImageDisplay::createLocation() const
{
    Image* res = Image::create();
    res->setShape( shape_ );
    res->setSelectable( false );

    return res;
}


void ImageDisplay::dispChg( CallBacker* cb )
{
    LocationDisplay::dispChg( cb );
    updateCoords();
}


int ImageDisplay::isMarkerClick(const TypeSet<int>& path) const
{
    for ( int idx=group_->size()-1; idx>=0; idx-- )
    {
	mDynamicCastGet( Image*, image, group_->getObject(idx) );
	if ( !image )
	    continue;
	
	if ( path.indexOf(image->id())!=-1 )
	    return idx;
    }

    return -1;
}


void ImageDisplay::setPosition( int idx, const Pick::Location& pick )
{
    mDynamicCastGet( Image*, image, group_->getObject(idx) );
    image->setPick( pick );
}


void ImageDisplay::updateCoords(CallBacker*)
{
    const float size = set_ ? set_->disp_.pixsize_ : 100;
    visBase::Coordinates* facecoords = shape_->getCoordinates();
    facecoords->setPos( 0, Coord3(-size,0,-2*size/scene_->getZScale()) );
    facecoords->setPos( 1, Coord3(-size,0,2*size/scene_->getZScale()) );
    facecoords->setPos( 2, Coord3(size,0,2*size/scene_->getZScale()) );
    facecoords->setPos( 3, Coord3(size,0,-2*size/scene_->getZScale()) );
}

} //namespace
