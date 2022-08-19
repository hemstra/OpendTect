#pragma once
/*+
________________________________________________________________________

 Copyright:	(C) 1995-2022 dGB Beheer B.V.
 License:	https://dgbes.com/licensing
________________________________________________________________________

-*/

#include "uitoolsmod.h"
#include "uigroup.h"
#include "uihistogramdisplay.h"
#include "datapack.h"


mExpClass(uiTools) uiHistogramSel : public uiGroup
{
public:
    				uiHistogramSel(uiParent*,
					       const uiHistogramDisplay::Setup&,
					       int an_id=0);
				~uiHistogramSel();

    int				ID() const		{ return id_; }
    void			setID( int id )		{ id_ = id; }


    enum SliderTextPolicy	{ Always, OnMove, Never };

    void			setSliderTextPolicy(SliderTextPolicy);
    SliderTextPolicy		sliderTextPolicy() const;

    void			setEmpty();
    bool			setDataPackID(DataPackID,DataPackMgr::MgrID,
					      int version);
    void			setData(const Array2D<float>*);
    void			setData(const float*,int sz);
    void			setMarkValue(float,bool forx);

    const Interval<float>&	getDataRange() const	{ return datarg_; }
    void			setDataRange(const Interval<float>&);
    const Interval<float>&	getSelRange() const	{ return cliprg_; }
    void			setSelRange(const Interval<float>&);

    uiHistogramDisplay&		getDisplay()	{ return *histogramdisp_; }

    Notifier<uiHistogramSel>	rangeChanged;

protected:

    int				id_;
    SliderTextPolicy		slidertextpol_;
    uiHistogramDisplay*		histogramdisp_;
    uiAxisHandler*		xax_;

    uiLineItem*			minhandle_;
    uiLineItem*			maxhandle_;
    uiTextItem*			minvaltext_;
    uiTextItem*			maxvaltext_;

    Interval<float>		datarg_;
    Interval<float>		cliprg_;
    int				startpix_;
    int				stoppix_;

    bool			mousedown_;

    virtual void		drawAgain();
    virtual void		drawLines();
    virtual void		drawText();
    virtual void		drawPixmaps()	{}
    virtual void		useClipRange()	{}
    virtual void		makeSymmetricalIfNeeded(bool)	{}

    void			init();
    bool			changeLinePos(bool pressedonly=false);

    void			mousePressed(CallBacker*);
    void			mouseMoved(CallBacker*);
    void			mouseReleased(CallBacker*);

    void			histogramResized(CallBacker*);
    void			histDRChanged(CallBacker*);
};
