#ifndef viswelldisplay_h
#define viswelldisplay_h

/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:	Kristofer Tingdahl
 Date:		4-11-2002
 RCS:		$Id: viswelldisplay.h,v 1.56 2009-08-19 08:21:57 cvsbert Exp $



-*/


#include "visobject.h"
#include "vissurvobj.h"
#include "viswell.h"
#include "multiid.h"
#include "ranges.h"
#include "welllogdisp.h"

class LineStyle;
namespace Well { class Log; class LogDisplayPars; class LogDisplayParSet; }


namespace visBase
{
    class DataObjectGroup;
    class EventCatcher;
    class EventInfo;
    class Transformation;
}

namespace Well
{
    class Data;
    class Log;
    class LogDisplayPars;
    class LogDisplayParSet;
    class Track;
}

namespace visSurvey
{
class Scene;

/*!\brief Used for displaying welltracks, markers and logs


*/

mClass WellDisplay :	public visBase::VisualObjectImpl,
			public visSurvey::SurveyObject
{
public:
    static WellDisplay*		create()
				mCreateDataObj(WellDisplay);

    bool			setMultiID(const MultiID&);
    MultiID			getMultiID() const 	{ return wellid_; }

    //track
    void 			fillTrackParams(visBase::Well::TrackParams&);

    bool			wellTopNameShown() const;
    void			showWellTopName(bool);
    bool			wellBotNameShown() const;
    void			showWellBotName(bool);
    TypeSet<Coord3>             getWellCoords()	const;

    //markers
    void 			fillMarkerParams(visBase::Well::MarkerParams&);

    bool			canShowMarkers() const;
    bool			markersShown() const;
    void			showMarkers(bool);
    bool			markerNameShown() const;
    void			showMarkerName(bool);
    int				markerScreenSize() const;
    void			setMarkerScreenSize(int);
    
    //logs
    void 			fillLogParams(visBase::Well::LogParams&,int);

    const LineStyle*		lineStyle() const;
    void			setLineStyle(LineStyle);
    bool			hasColor() const	{ return true; }
    Color			getColor() const;
    void 			setLogData(visBase::Well::LogParams&,bool);
    void			setLogDisplay(int);
    void			calcClippedRange(float,Interval<float>&,int);
    void			displayRightLog();
    void			displayLeftLog();
    void			setOneLogDisplayed(bool);
    Well::LogDisplayParSet*	getLogParSet() const	{ return &logparset_; }
    const Color&		logColor(int) const;
    void			setLogColor(const Color&,int);
    float			logLineWidth(int) const;
    void			setLogLineWidth(float,int);
    int				logWidth() const;
    void			setLogWidth(int,int);
    bool			logsShown() const;
    void			showLogs(bool);
    const char*			logName(bool left) const
				{return left ? logparset_.getLeft()->name_						     : logparset_.getRight()->name_;}

    bool			logNameShown() const;
    void			showLogName(bool);

    mVisTrans*			getDisplayTransformation();
    void			setDisplayTransformation(mVisTrans*);
    void 			setDisplayTransformForPicks(mVisTrans*);
    
    void                        setSceneEventCatcher(visBase::EventCatcher*);
    void 			addPick(Coord3);
    				//only used for user-made wells
    void			getMousePosInfo(const visBase::EventInfo& pos,
	    					Coord3&,BufferString& val,
						BufferString& info) const;
    NotifierAccess*             getManipulationNotifier() { return &changed_; }
    bool			hasChanged() const 	{ return needsave_; }
    bool			isHomeMadeWell() const { return picksallowed_; }
    void			setChanged( bool yn )	{ needsave_ = yn; }
    void			setupPicking(bool);
    void			showKnownPositions();
    
    virtual void                fillPar(IOPar&,TypeSet<int>&) const;
    virtual int                 usePar(const IOPar&);

protected:

    virtual			~WellDisplay();
    void			setWell(visBase::Well*);
    void			updateMarkers(CallBacker*);
    void			fullRedraw(CallBacker*);
    TypeSet<Coord3>		getTrackPos(Well::Data*);
    void			displayLog(Well::LogDisplayPars*,int);
    void			setLogProperties(visBase::Well::LogParams&);
    void                        pickCB(CallBacker* cb=0);

    Coord3                      mousepressposition_;
    mVisTrans*			transformation_;
    MultiID			wellid_;
    visBase::EventCatcher*	eventcatcher_;
    visBase::DataObjectGroup*	group_;
    visBase::Well*		well_;
    Well::LogDisplayParSet&	logparset_;
    Well::Track*		pseudotrack_;
    Well::Data*			wd_;

    Notifier<WellDisplay>	changed_;

    int 			logsnumber_;
    int                         mousepressid_;
    bool			needsave_;
    bool			onelogdisplayed_;
    bool			picksallowed_;
    const bool			zistime_;
    const bool			zinfeet_;
    
    static const char*		sKeyEarthModelID;
    static const char*		sKeyWellID;
    static const char*		sKeyLog1Name;
    static const char*		sKeyLog1Range;
    static const char*		sKeyLog1Scale;
    static const char*		sKeyLog1Repeat;
    static const char*		sKeyLog1Ovlap;
    static const char*		sKeyLog1Clip;
    static const char*		sKeyLog1Style;
    static const char*		sKeyLog1Color;
    static const char*		sKeyLog1FillColor;
    static const char*		sKeyLog1SeisFillColor;
    static const char*		sKeyLog2Name;
    static const char*		sKeyLog2Range;
    static const char*		sKeyLog2Repeat;
    static const char*		sKeyLog2Ovlap;
    static const char*		sKeyLog2Clip;
    static const char*		sKeyLog2Scale;
    static const char*		sKeyLog2Style;
    static const char*		sKeyLog2Color;
    static const char*		sKeyLog2FillColor;
    static const char*		sKeyLog2SeisFillColor;

    Well::Data*			getWD() const;
    void                        welldataDelNotify(CallBacker* cb=0);

};

} // namespace visSurvey


#endif
