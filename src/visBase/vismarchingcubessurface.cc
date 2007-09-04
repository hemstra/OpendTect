/*+
________________________________________________________________________

 CopyRight:     (C) dGB Beheer B.V.
 Author:        K. Tingdahl
 Date:          August 2006
 RCS:           $Id: vismarchingcubessurface.cc,v 1.2 2007-09-04 19:45:18 cvskris Exp $
________________________________________________________________________

-*/

#include "vismarchingcubessurface.h"

#include "explicitmarchingcubes.h"
#include "marchingcubes.h"
#include "viscoord.h"

#include <Inventor/nodes/SoIndexedTriangleStripSet.h>
#include <Inventor/nodes/SoShapeHints.h>

mCreateFactoryEntry( visBase::MarchingCubesSurface );

namespace visBase
{

MarchingCubesSurface::MarchingCubesSurface()
    : VisualObjectImpl( true )
    , coords_( Coordinates::create() )
    , hints_( new SoShapeHints )
    , side_( 0 )
    , surface_( new ExplicitMarchingCubesSurface( 0 ) )
{
    coords_->ref();
    addChild( coords_->getInventorNode() );
    addChild( hints_ );

    surface_->setCoordList( new CoordListAdapter(*coords_) );

    renderOneSide( 0 );
}


MarchingCubesSurface::~MarchingCubesSurface()
{
    for ( int idx=0; idx<triangles_.size(); idx++ )
	triangles_[idx]->unref();

    coords_->unRef();
    delete surface_;
}


void MarchingCubesSurface::renderOneSide( int side )
{
    side_ = side;
    updateHints();
}


void MarchingCubesSurface::updateHints() 
{
    hints_->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;

    if ( side_==0 )
    {
	hints_->shapeType = SoShapeHints::UNKNOWN_SHAPE_TYPE;
    }
    else if ( side_==1 )
    {
	hints_->shapeType = SoShapeHints::SOLID;
    }
    else
    {
	hints_->shapeType = SoShapeHints::SOLID;
    }
}


void MarchingCubesSurface::setSurface( ::MarchingCubesSurface& ns )
{
    surface_->setSurface( &ns );
    touch();
}


void MarchingCubesSurface::touch()
{
    if ( surface_->needsUpdate() )
	surface_->update();

    const int nrsets = surface_->nrIndicesSets();
    for ( int idx=0; idx<nrsets; idx++ )
    {
	if ( idx>=triangles_.size() )
	{
	    SoIndexedTriangleStripSet* nt = new SoIndexedTriangleStripSet;
	    nt->ref();
	    triangles_ += nt;
	    addChild( nt );
	}

	triangles_[idx]->coordIndex.setValuesPointer(
		surface_->nrIndices(idx), surface_->getIndices(idx) );
    }

    for ( int idx=triangles_.size()-1; idx>=nrsets; idx-- )
    {
	removeChild( triangles_[idx] );
	triangles_.remove( idx )->unref();
    }
}


::MarchingCubesSurface* MarchingCubesSurface::getSurface()
{ return surface_->getSurface(); }


const ::MarchingCubesSurface* MarchingCubesSurface::getSurface() const
{ return surface_->getSurface(); }


void MarchingCubesSurface::setScales(const SamplingData<float>& xrg,
				     const SamplingData<float>& yrg,
				     const SamplingData<float>& zrg)
{ surface_->setAxisScales( xrg, yrg, zrg ); }


const SamplingData<float>& MarchingCubesSurface::getScale( int dim ) const
{
    return surface_->getAxisScale( dim );
}
    

}; // namespace visBase
