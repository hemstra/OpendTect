#pragma once
/*+
________________________________________________________________________

 Copyright:	(C) 1995-2022 dGB Beheer B.V.
 License:	https://dgbes.com/licensing
________________________________________________________________________

-*/

#include "vissurveymod.h"
#include "visobject.h"
#include "vissurvobj.h"

#include "iopar.h"
#include "mousecursor.h"
#include "prestackgather.h"

class IOObj;
class SeisPSReader;

namespace PreStack { class ProcessManager; }
namespace visBase
{
    class DepthTabPlaneDragger;
    class FaceSet;
    class FlatViewer;
}

namespace visSurvey
{

class PlaneDataDisplay;
class Seis2DDisplay;


mExpClass(visSurvey) PreStackDisplay : public visBase::VisualObjectImpl,
		 public visSurvey::SurveyObject
{ mODTextTranslationClass(PreStackDisplay)
public:

				PreStackDisplay();
				mDefaultFactoryInstantiation(
				    visSurvey::SurveyObject,PreStackDisplay,
				    "PreStackDisplay",
				    toUiString(sFactoryKeyword()) );

    void			allowShading(bool yn) override;
    void			setMultiID(const MultiID&);
    BufferString		getObjectName() const;
    bool			isInlCrl() const override	{ return true; }
    bool			isOrientationInline() const;
    const Coord			getBaseDirection() const;
    StepInterval<int>		getTraceRange(const BinID&,
					      bool oncurrentline=true) const;

				//for 3D only at present
    DataPackID			preProcess();

    bool			is3DSeis() const;
    DataPackID			getDataPackID(int i=0) const override;

    visBase::FlatViewer*	flatViewer()	{ return flatviewer_; }
    const visBase::FlatViewer*	flatViewer() const { return flatviewer_; }
    PreStack::ProcessManager*	procMgr()	{ return preprocmgr_; }
    void			setProcMgr(OD::GeomSystem);
    void			setProcPar(const IOPar&);
    void			getProcPar(IOPar&);

    TrcKey			getTrcKey() const;

				//3D case
    bool			setPosition(const TrcKey&);
    const BinID&		getPosition() const;
    void			setSectionDisplay(PlaneDataDisplay*);
    const PlaneDataDisplay*	getSectionDisplay() const;
    PlaneDataDisplay*		getSectionDisplay();

    Notifier<PreStackDisplay>	draggermoving;
    NotifierAccess*		getMovementNotifier() override
				{ return &draggermoving; }
    const BinID			draggerPosition() const	{ return draggerpos_; }

    bool			hasPosModeManipulator() const override
				{ return true; }

				//2D case
    const Seis2DDisplay*	getSeis2DDisplay() const;
    bool			setSeis2DData(const IOObj*);
    bool			setSeis2DDisplay(Seis2DDisplay*,int trcnr);
    void			setTraceNr(int trcnr);
    int				traceNr() const	  { return trcnr_; }
    BufferString		lineName() const;

    bool			displayAutoWidth() const { return autowidth_; }
    void			displaysAutoWidth(bool yn);
    bool			displayOnPositiveSide() const {return posside_;}
    void			displaysOnPositiveSide(bool yn);
    float			getFactor() { return factor_; }
    void			setFactor(float scale);
    float			getWidth() { return width_; }
    void			setWidth(float width);
    BinID			getBinID() const { return bid_; }
    MultiID			getMultiID() const override { return mid_; }

    const MouseCursor*		getMouseCursor() const override
				{ return &mousecursor_; }
    void			getMousePosInfo( const visBase::EventInfo& ei,
						 IOPar& iop ) const override
				{ SurveyObject::getMousePosInfo(ei,iop); }
    void			getMousePosInfo(const visBase::EventInfo&,
					    Coord3&,BufferString& val,
					    uiString& info) const override;
    void			otherObjectsMoved(
					const ObjectSet<const SurveyObject>&,
					VisID whichobj ) override;


    void			fillPar(IOPar&) const override;
    bool			usePar(const IOPar&) override;
    bool			updateDisplay() { return updateData(); }

    static const char*		sKeyParent()	{ return "Parent"; }
    static const char*		sKeyFactor()	{ return "Factor"; }
    static const char*		sKeyWidth()	{ return "Width"; }
    static const char*		sKeyAutoWidth() { return "AutoWidth"; }
    static const char*		sKeySide()	{ return "ShowSide"; }

protected:
				~PreStackDisplay();

    void			setDisplayTransformation(
					const mVisTrans*) override;
    void			setSceneEventCatcher(
					visBase::EventCatcher*) override;
    void			updateMouseCursorCB(CallBacker*) override;
    void			dataChangedCB(CallBacker*);
    void			sectionMovedCB(CallBacker*);
    void			seis2DMovedCB(CallBacker*);
    bool			updateData();
    int				getNearTraceNr(int) const;
    BinID			getNearBinID(const BinID& pos) const;

    void			draggerMotion(CallBacker*);
    void			finishedCB(CallBacker*);

    RefMan<PreStack::Gather>	gather_;

    BinID			bid_;
    BinID			draggerpos_;
    visBase::EventCatcher*	eventcatcher_			= nullptr;
    MouseCursor			mousecursor_;
    visBase::DepthTabPlaneDragger*	planedragger_;
    visBase::FlatViewer*	flatviewer_;
    PreStack::ProcessManager*	preprocmgr_			= nullptr;
    IOPar			preprociop_;

    MultiID			mid_;
    PlaneDataDisplay*		section_			= nullptr;
    Seis2DDisplay*		seis2d_				= nullptr;
    int				trcnr_				= -1;
    Coord			basedirection_			= Coord::udf();
    Coord			seis2dpos_			= Coord::udf();
    Coord			seis2dstoppos_			= Coord::udf();

    bool			posside_			= true;
    bool			autowidth_			= true;
    float			factor_				= 1.f;
    float			width_;
    Interval<float>		offsetrange_;
    Interval<float>		zrg_;

    SeisPSReader*		reader_				= nullptr;
    IOObj*			ioobj_				= nullptr;
    Notifier<PreStackDisplay>	movefinished_;

public:

    mDeprecated("Use TrcKey")
    bool			setPosition(const BinID&);
};

} // namespace visSurvey
