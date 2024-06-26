#pragma once
/*+
________________________________________________________________________

 Copyright:	(C) 1995-2022 dGB Beheer B.V.
 License:	https://dgbes.com/licensing
________________________________________________________________________

-*/

#include "uiodmainmod.h"
#include "uioddisplaytreeitem.h"
#include "odcommonenums.h"
#include "datapack.h"
#include "flatview.h"

class uiSliceSelDlg;
class TrcKeyZSampling;
namespace Attrib { class DescID; }
namespace Well { class Data; }


mExpClass(uiODMain) uiODPlaneDataTreeItem : public uiODDisplayTreeItem
{ mODTextTranslationClass(uiODPlaneDataTreeItem)
public:

    enum Type		{ Empty, Select, Default, RGBA };

    virtual		~uiODPlaneDataTreeItem();

    bool		init() override;
    void		setAtWellLocation(const Well::Data&);
    void		setTrcKeyZSampling(const TrcKeyZSampling&);
    bool		displayDefaultData();
    bool		displayGuidance();
    bool		displayDataFromDesc(const Attrib::DescID&,bool stored);
    bool		displayDataFromDataPack(DataPackID,
					    const Attrib::SelSpec&,
					    const FlatView::DataDispPars::VD&);
    bool		displayDataFromOther(VisID visid);

    static uiString	sAddEmptyPlane();
    static uiString	sAddAndSelectData();
    static uiString	sAddDefaultData();
    static uiString	sAddColorBlended();
    static uiString	sAddAtWellLocation();

protected:
			uiODPlaneDataTreeItem(VisID displayid,
					      OD::SliceType,Type);

    uiString		createDisplayName() const override;

    void		createMenu(MenuHandler*,bool istb) override;
    void		handleMenuCB(CallBacker*) override;

    void		updatePlanePos(CallBacker*);
    void		updatePositionDlg(CallBacker*);
    void		posDlgClosed(CallBacker*);
    void		keyPressCB(CallBacker*) override;
    void		movePlane(bool forward,int step=1);

    void		selChg(CallBacker*);
    void		posChange(CallBacker*);
    void		movePlaneAndCalcAttribs(const TrcKeyZSampling&);
    void		keyUnReDoPressedCB(CallBacker*);

    const OD::SliceType	orient_;
    const Type		type_;
    MenuItem		positionmnuitem_;
    MenuItem		gridlinesmnuitem_;
    MenuItem		addinlitem_;
    MenuItem		addcrlitem_;
    MenuItem		addzitem_;

    uiSliceSelDlg*	positiondlg_;
};


// In-line items
mExpClass(uiODMain) uiODInlineParentTreeItem : public uiODParentTreeItem
{ mODTextTranslationClass(uiODInlineParentTreeItem);
public:
			uiODInlineParentTreeItem();
			~uiODInlineParentTreeItem();

    static CNotifier<uiODInlineParentTreeItem,uiMenu*>& showMenuNotifier();

protected:
    const char*		iconName() const override;
    bool		showSubMenu() override;
};


mExpClass(uiODMain) uiODInlineTreeItemFactory : public uiODTreeItemFactory
{
public:
    const char*		name() const override { return typeid(*this).name(); }
    uiTreeItem*		create() const override
			{ return new uiODInlineParentTreeItem; }
    uiTreeItem*		createForVis(VisID visid,uiTreeItem*) const override;
};


mExpClass(uiODMain) uiODInlineTreeItem : public uiODPlaneDataTreeItem
{
public:
			uiODInlineTreeItem(VisID displayid,Type);
			~uiODInlineTreeItem();

protected:
    const char*		parentType() const override
			{ return typeid(uiODInlineParentTreeItem).name(); }
};


// Cross-line items
mExpClass(uiODMain) uiODCrosslineParentTreeItem : public uiODParentTreeItem
{ mODTextTranslationClass(uiODCrosslineParentTreeItem);
public:
			uiODCrosslineParentTreeItem();
			~uiODCrosslineParentTreeItem();

    static CNotifier<uiODCrosslineParentTreeItem,uiMenu*>& showMenuNotifier();

protected:
    const char*		iconName() const override;
    bool		showSubMenu() override;
};


mExpClass(uiODMain) uiODCrosslineTreeItemFactory : public uiODTreeItemFactory
{
public:
    const char*		name() const override { return typeid(*this).name(); }
    uiTreeItem*		create() const override
			{ return new uiODCrosslineParentTreeItem; }
    uiTreeItem*		createForVis(VisID visid,uiTreeItem*) const override;
};


mExpClass(uiODMain) uiODCrosslineTreeItem : public uiODPlaneDataTreeItem
{
public:
			uiODCrosslineTreeItem(VisID displayid,Type);
			~uiODCrosslineTreeItem();

protected:
    const char*		parentType() const override
			{ return typeid(uiODCrosslineParentTreeItem).name(); }
};


// Z slice items
mExpClass(uiODMain) uiODZsliceParentTreeItem : public uiODParentTreeItem
{ mODTextTranslationClass(uiODZsliceParentTreeItem);
public:
			uiODZsliceParentTreeItem();
			~uiODZsliceParentTreeItem();

    static CNotifier<uiODZsliceParentTreeItem,uiMenu*>& showMenuNotifier();

protected:
    const char*		iconName() const override;
    bool		showSubMenu() override;
};


mExpClass(uiODMain) uiODZsliceTreeItemFactory : public uiODTreeItemFactory
{
public:
    const char*		name() const override { return typeid(*this).name(); }
    uiTreeItem*		create() const override
			{ return new uiODZsliceParentTreeItem; }
    uiTreeItem*		createForVis(VisID visid,uiTreeItem*) const override;
};


mExpClass(uiODMain) uiODZsliceTreeItem : public uiODPlaneDataTreeItem
{
public:
			uiODZsliceTreeItem(VisID displayid,Type);
			~uiODZsliceTreeItem();

protected:
    const char*		parentType() const override
			{ return typeid(uiODZsliceParentTreeItem).name(); }
};
