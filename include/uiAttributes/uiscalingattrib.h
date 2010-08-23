#ifndef uiscalingattrib_h
#define uiscalingattrib_h

/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:        Nanne Hemstra
 Date:          December 2004
 RCS:           $Id: uiscalingattrib.h,v 1.11 2010-08-23 13:44:03 cvsbert Exp $
________________________________________________________________________

-*/

#include "uiattrdesced.h"

namespace Attrib { class Desc; }
class uiAttrSel;
class uiGenInput;
class uiTable;


/*! \brief Scaling Attribute description editor */

mClass uiScalingAttrib : public uiAttrDescEd
{
public:

			uiScalingAttrib(uiParent*,bool);

protected:

    uiAttrSel*		inpfld;
    uiGenInput*		typefld;
    uiGenInput*		nfld;
    uiGenInput*		statsfld;
    uiTable*		table;
    uiGenInput*         windowfld;
    uiGenInput*         lowenergymute;
    uiGenInput*         sqrgfld;
    uiGenInput*         squrgfld;

    void		typeSel(CallBacker*);
    void		statsSel(CallBacker*);

    bool		setParameters(const Attrib::Desc&);
    bool		setInput(const Attrib::Desc&);

    bool		getParameters(Attrib::Desc&);
    bool		getInput(Attrib::Desc&);

    void		getEvalParams(TypeSet<EvalParam>&) const;

    			mDeclReqAttribUIFns
};

#endif
