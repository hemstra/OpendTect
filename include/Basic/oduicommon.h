#ifndef uiodcommon_h
#define uiodcommon_h

/*+
________________________________________________________________________

 (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 Author:        Bert
 Date:          May 2014
 RCS:           $Id$
________________________________________________________________________

-*/

#include "basicmod.h"


namespace OD
{

/*!\brief Fundamental orientation in 2D UIs */

enum Orientation
{
    Horizontal=0,
    Vertical=1
};


/*!\brief OpendTect flat slice types */

enum SliceType
{
    InlineSlice=0,
    CrosslineSlice=1,
    ZSlice=2
};


/*!\brief What to choose from any list-type UI object */

enum ChoiceMode
{
    ChooseNone=0,
    ChooseOnlyOne=1,
    ChooseAtLeastOne=2,
    ChooseZeroOrMore=3
};


/*!\brief State of check objects */

enum CheckState
{
    Unchecked=0,
    PartiallyChecked=1,
    Checked=2
};


enum StdActionType
{
    NoIcon=0,
    Apply,
    Cancel,
    Define,
    Delete,
    Edit,
    Help,
    Ok,
    Options,
    Properties,
    Examine,
    Rename,
    Remove,
    Save,
    SaveAs,
    Select,
    Settings,
    Unload
};

} // namespace OD


inline bool isHorizontal( OD::Orientation orient )
				{ return orient == OD::Horizontal; }
inline bool isVertical( OD::Orientation orient )
				{ return orient == OD::Vertical; }
inline bool isMultiChoice( OD::ChoiceMode cm )
				{ return cm > 1; }
inline bool isOptional( OD::ChoiceMode cm )
				{ return cm == OD::ChooseZeroOrMore; }


#endif
