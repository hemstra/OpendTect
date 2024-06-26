#pragma once
/*+
________________________________________________________________________

 Copyright:	(C) 1995-2022 dGB Beheer B.V.
 License:	https://dgbes.com/licensing
________________________________________________________________________

-*/

#include "uivismod.h"

#include "uislicepos.h"
#include "visplanedatadisplay.h"
#include "visvolumedisplay.h"

class uiVisPartServer;

/*! \brief Toolbar for setting slice position _ 3D visualization display */

mExpClass(uiVis) uiSlicePos3DDisp : public uiSlicePos
{
public:
			uiSlicePos3DDisp(uiParent*,uiVisPartServer*);
			~uiSlicePos3DDisp();

    void		setDisplay(VisID dispid);
    VisID		getDisplayID() const;

private:

    RefMan<visSurvey::PlaneDataDisplay> curpdd_;
    RefMan<visSurvey::VolumeDisplay>	curvol_;
    uiVisPartServer*		vispartserv_;

    uiSlicePos::SliceDir	getOrientation() const override;
    TrcKeyZSampling		getSampling() const;

    void			slicePosChg(CallBacker*) override;
    void			sliceStepChg(CallBacker*) override;
    void			setBoxRanges() override;
    void			setPosBoxValue() override;
    void			setStepBoxValue() override;
};
