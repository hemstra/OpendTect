#ifndef viscamera_h
#define viscamera_h

/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:	Kristofer Tingdahl
 Date:		4-11-2002
 RCS:		$Id$
________________________________________________________________________


-*/

#include "visbasemod.h"
#include "visdata.h"
#include "position.h"

namespace osg { class Camera; class RenderInfo; }

namespace visBase
{

class DrawCallback;

/*!\brief



*/

mExpClass(visBase) Camera : public DataObject
{
public:

    static Camera*	create()
			mCreateDataObj( Camera );

    osg::Camera*	osgCamera() const;

    void		setPosition(const Coord3&);
    Coord3		position() const;

    void		pointAt(const Coord3&);
    void		pointAt(const Coord3& pos,
	    			const Coord3& upvector );
    void		setOrientation( const Coord3& dirvector, float angle );
    void		getOrientation( Coord3& dirvector, float& angle ) const;

    void		setOrthogonal(bool yn)		{ }
    bool		isOrthogonal() const		{ return false; }

    void		setAspectRatio( float );
    float		aspectRatio() const;

    void		setNearDistance( float );
    float		nearDistance() const;

    void		setFarDistance( float );
    float		farDistance() const;

    void		setFocalDistance( float );
    float		focalDistance() const;

    void		setStereoAdjustment(float);
    float		getStereoAdjustment() const;

    void		setBalanceAdjustment(float);
    float		getBalanceAdjustment() const;

    Coord3 		centerFrustrum();

    int			usePar( const IOPar& );
    void		fillPar( IOPar& ) const;


    Notifier<Camera>		preDraw;
    Notifier<Camera>		postDraw;

    const osg::RenderInfo*	getRenderInfo() const { return renderinfo_; }
    				//!<Only available during pre/post draw cb

private:
    friend			DrawCallback;

    void			triggerDrawCallBack(const DrawCallback*,
                                	            const osg::RenderInfo&);

    virtual			~Camera();

    osg::Camera*		camera_;
    const osg::RenderInfo*	renderinfo_;
    DrawCallback*		predraw_;
    DrawCallback*		postdraw_;

    static const char*		sKeyPosition();
    static const char*		sKeyOrientation();
    static const char*		sKeyAspectRatio();
    static const char*		sKeyNearDistance();
    static const char*		sKeyFarDistance();
    static const char*		sKeyFocalDistance();
};


};


#endif

