/*+
 * COPYRIGHT: (C) dGB Beheer B.V.
 * AUTHOR   : K. Tingdahl
 * DATE     : Oct 1999
-*/

static const char* rcsID = "$Id: initvissurvey.cc,v 1.6 2009-06-01 10:14:05 cvsnanne Exp $";


#include "initvissurvey.h"

#include "visfaultdisplay.h"
#include "visfaultsticksetdisplay.h"
#include "vishingeline.h"
#include "vishorizon2ddisplay.h"
#include "vishorizondisplay.h"
#include "vismarchingcubessurfacedisplay.h"
#include "vismpe.h"
#include "vismpeeditor.h"
#include "vismpeseedcatcher.h"
#include "vispicksetdisplay.h"
#include "visplanedatadisplay.h"
#include "vispolygonbodydisplay.h"
#include "vispolylinedisplay.h"
#include "visrandomposbodydisplay.h"
#include "visrandomtrackdisplay.h"
#include "visseis2ddisplay.h"
#include "vissurvscene.h"
#include "visvolumedisplay.h"
#include "viswelldisplay.h"


namespace visSurvey
{

void initStdClasses()
{
    EdgeLineSetDisplay::initClass();
    FaultDisplay::initClass();
    FaultStickSetDisplay::initClass();
    Horizon2DDisplay::initClass();
    HorizonDisplay::initClass();
    MarchingCubesDisplay::initClass();
    MPEDisplay::initClass();
    MPEEditor::initClass();
    MPEClickCatcher::initClass();
    PickSetDisplay::initClass();
    RandomPosBodyDisplay::initClass();
    PlaneDataDisplay::initClass();
    PolygonBodyDisplay::initClass();
    PolyLineDisplay::initClass();
    RandomTrackDisplay::initClass();
    Seis2DDisplay::initClass();
    Scene::initClass();
    VolumeDisplay::initClass();
    WellDisplay::initClass();
}

}; // namespace visBase
