#ifndef uicolortable_h
#define uicolortable_h

/*+
________________________________________________________________________

 CopyRight:     (C) dGB Beheer B.V.
 Author:        Bert/Nanne
 Date:          Aug 2007
 RCS:           $Id: uicolortable.h,v 1.9 2008-05-29 11:55:19 cvssatyaki Exp $
________________________________________________________________________

-*/

#include "uigroup.h"
#include "flatview.h"

class uiColorTableCanvas;
class uiComboBox;
class uiLineEdit;

namespace ColTab { class Sequence; }
//namespace FlatView { class DataDispPars::VD; }

class uiColorTable : public uiGroup
{
public:

			uiColorTable(uiParent*,ColTab::Sequence& colseq,
				     bool vertical);
			   //!< Editable
			uiColorTable(uiParent*,const char*,bool vertical );
			   //!< Display only
			~uiColorTable();

    const ColTab::Sequence&	colTabSeq() const	{ return coltabseq_;}
    ColTab::Sequence&		colTabSeq()    		{ return coltabseq_;}

    void		setTable(const char*,bool emitnotif=true);
    void		setTable(const ColTab::Sequence&,bool emitnotif=true);
    void		setHistogram(const TypeSet<float>*);
    void		setInterval(const Interval<float>&);
    const Interval<float> getInterval()	const		{ return coltabrg_; }

    void                setEnabManage( bool yn )	{ enabmanage_ = yn; }
    void                setAutoScale( bool yn )		{ autoscale_ = yn; }
    bool                autoScale() const		{ return autoscale_; }
    void                setClipRate( float r )		{ cliprate_ = r; }
    float               getClipRate() const		{ return cliprate_; }
    void                setSymMidval( float val )	{ symmidval_ = val; }
    float               getSymMidval() const		{ return symmidval_; }

    void		setDispPars(const FlatView::DataDispPars::VD&);
    void		getDispPars(FlatView::DataDispPars::VD&) const;

    Notifier<uiColorTable>	seqChanged;
    Notifier<uiColorTable>	scaleChanged;

protected:

    bool		autoscale_;
    float		cliprate_;
    float		symmidval_;
    bool		enabmanage_;

    ColTab::Sequence&	coltabseq_;
    Interval<float> 	coltabrg_;

    TypeSet<float>	histogram_;
    uiColorTableCanvas*	canvas_;
    uiLineEdit*		minfld_;
    uiLineEdit*		maxfld_;
    uiComboBox*		selfld_;

    void		canvasClick(CallBacker*);
    void		tabSel(CallBacker*);
    void		tableAdded(CallBacker*);
    void		rangeEntered(CallBacker*);
    void		doManage(CallBacker*);
    void		doFlip(CallBacker*);
    void		setAsDefault(CallBacker*);
    void		editScaling(CallBacker*);
    void		makeSymmetrical(CallBacker*);
    void		colTabChgdCB(CallBacker*);
    void		colTabManChgd(CallBacker*);

    bool		isEditable() const	{ return maxfld_; }
    void		fillTabList();
};


#endif
