#pragma once
/*+
________________________________________________________________________

 Copyright:	(C) 1995-2022 dGB Beheer B.V.
 License:	https://dgbes.com/licensing
________________________________________________________________________

-*/

#include "uivismod.h"
#include "uimarkerstyledlg.h"
#include "emobject.h"

class uiColorInput;

mExpClass(uiVis) uiSeedPropDlg : public uiMarkerStyleDlg
{ mODTextTranslationClass(uiSeedPropDlg);
public:
			uiSeedPropDlg(uiParent*,EM::EMObject*,
					int posattr=EM::EMObject::sSeedNode());
			~uiSeedPropDlg();

protected:

    EM::EMObject*	emobject_;
    MarkerStyle3D	markerstyle_;
    int			posattr_;

    void		doFinalize(CallBacker*) override;

    void		sliderMove(CallBacker*) override;
    void		typeSel(CallBacker*) override;
    void		colSel(CallBacker*) override;
    void		updateMarkerStyle();
};
