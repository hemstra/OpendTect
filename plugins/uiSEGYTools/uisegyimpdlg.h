#pragma once
/*+
________________________________________________________________________

 Copyright:	(C) 1995-2022 dGB Beheer B.V.
 License:	https://dgbes.com/licensing
________________________________________________________________________

-*/

#include "uisegycommon.h"
#include "uisegyreaddlg.h"
#include "uistring.h"

class uiSeisSel;
class uiCheckBox;
class uiSeisTransfer;
class uiBatchJobDispatcherSel;


/*!\brief Dialog to import SEG-Y files after basic setup. */

mExpClass(uiSEGYTools) uiSEGYImpDlg : public uiSEGYReadDlg
{ mODTextTranslationClass(uiSEGYImpDlg);
public :

			uiSEGYImpDlg(uiParent*,const Setup&,IOPar&);

    void		use(const IOObj*,bool force) override;
    MultiID		outputID() const override;


protected:

    uiSeisTransfer*	transffld_;
    uiSeisSel*		seissel_;
    uiCheckBox*		morebut_;
    uiBatchJobDispatcherSel* batchfld_;

    bool		doWork(const IOObj&) override;

    friend class	uiSEGYImpSimilarDlg;
    bool		impFile(const IOObj&,const IOObj&,const char*);

};
