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

#include "datapointset.h"
#include "dpsdispmgr.h"
#include "vistransform.h"

class Executor;

namespace visBase { class PointSet; class Transformation; }

namespace visSurvey
{


mExpClass(visSurvey) PointSetDisplay : public visBase::VisualObjectImpl,
			 public visSurvey::SurveyObject
{ mODTextTranslationClass(PointSetDisplay)
public:
				PointSetDisplay();
				mDefaultFactoryInstantiation(
				    visSurvey::SurveyObject,PointSetDisplay,
				    "PointSetDisplay",
				    toUiString(sFactoryKeyword()) );

    void			setPointSize(int);
    int				getPointSize() const;

    void			setDispProp(const DataPointSetDisplayProp*);
    const DataPointSetDisplayProp* dispProp() const	{ return dpsdispprop_; }
    bool			hasColor() const override { return true; }

    void			update(TaskRunner*);
    Executor*			getUpdater();
    void			updateColors();
    bool			setDataPack(DataPackID);
    ConstRefMan<DataPointSet>	getDataPack() const	{ return data_; }
    void			setDisplayTransformation(
						const mVisTrans*) override;
    const mVisTrans*		getDisplayTransformation() const override;
    const visBase::PointSet*	getPointSet() const	{ return pointset_; }

    const char*			errMsg() const override { return errmsg_.str();}

    bool			removeSelections(TaskRunner*) override;
    bool			selectable() const override	{ return true; }
    bool			canRemoveSelection() const override
				{ return true; }
    bool			allowMaterialEdit() const override
				{ return false;}
    void			setPixelDensity(float) override;

    void			getMousePosInfo(const visBase::EventInfo&,
					    Coord3& xyzpos,
					    BufferString& val,
					    uiString& info) const override;
    void			getMousePosInfo(const visBase::EventInfo& ei,
						IOPar& iop ) const override
				{ return SurveyObject::getMousePosInfo(ei,iop);}

protected:
				~PointSetDisplay();
    DataPointSetDisplayProp*	dpsdispprop_;
    visBase::PointSet*		pointset_;
    RefMan<DataPointSet>	data_;
    const mVisTrans*		transformation_;

};

};
