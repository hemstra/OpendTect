#pragma once
/*+
________________________________________________________________________

 Copyright:	(C) 1995-2022 dGB Beheer B.V.
 License:	https://dgbes.com/licensing
________________________________________________________________________

-*/

#include "uitoolsmod.h"
#include "uigroup.h"
#include "mathformula.h"
#include "mnemonics.h"

namespace Math { class Formula; class Expression; }
class BufferStringSet;
class UnitOfMeasure;
class uiComboBox;
class uiGenInput;
class uiLabel;
class uiLineEdit;
class uiToolButton;
class uiUnitSel;


mExpClass(uiTools) uiMathExpressionVariable : public uiGroup
{ mODTextTranslationClass(uiMathExpressionVariable);
public:

			uiMathExpressionVariable(uiParent*,int varidx,
				    bool withuom=true,
				    bool withsubinps=false,
				    const Math::SpecVarSet* svs=nullptr);
			~uiMathExpressionVariable();

    void		addInpViewIcon(const char* inm,const uiString& tooltip,
					const CallBack&);
    void		setNonSpecInputs(const BufferStringSet&,
					 const MnemonicSelection* =nullptr);
    void		setNonSpecSubInputs(const BufferStringSet&);

    virtual void	use(const Math::Formula&,bool hasfixedunits =false);
    virtual void	use(const Math::Expression*);

    int			varIdx() const		{ return varidx_; }
    const OD::String&	varName() const		{ return varnm_; }
    bool		hasVarName( const char* nm ) const
						{ return varnm_ == nm; }

    bool		isActive() const	{ return isactive_; }
    bool		isConst() const		{ return isconst_; }
    bool		isSpec() const;
    int			specIdx() const		{ return specidx_; }
    const char*		getInput() const;
    const Mnemonic*	getMnemonic() const	{ return curmn_; }
    const char*		getDescription() const	{ return vardesc_.buf(); }
    uiLabel*		getFormLbl() const	{ return formlbl_; }
    const UnitOfMeasure* getUnit() const;
    void		fill(Math::Formula&) const;
    BufferStringSet	getInputNms(const Mnemonic* =nullptr,
				    bool sub=false) const;

    void		selectSubInput(int);
    void		setFormType(const Mnemonic&);
			//!<Mnemonic required by formula
    void		setFormUnit(const UnitOfMeasure*,bool dispyn);
			//!<unit required by formula

    Notifier<uiMathExpressionVariable> inpSel;
    Notifier<uiMathExpressionVariable> subInpSel;

    const uiToolButton*	viewBut() const		{ return vwbut_; }

protected:

			mOD_DisableCopy(uiMathExpressionVariable);

    void		initFlds( CallBacker* )	{ updateDisp(); }
    void		inpChg( CallBacker* )	{ inpSel.trigger(); }
    void		subInpChg( CallBacker* ) { subInpSel.trigger(); }
    void		showHideVwBut(CallBacker* =nullptr);

    const int		varidx_;
    BufferString	varnm_;
    BufferString	vardesc_;
    BufferStringSet	nonspecinputs_;
    const MnemonicSelection*	mnsel_ = nullptr;
    const Mnemonic*	curmn_ = nullptr;
    BufferStringSet	nonspecsubinputs_;
    bool		isactive_	= true;
    bool		isconst_	= false;
    int			specidx_	= -1;
    Math::SpecVarSet&	specvars_;

    uiGroup*		inpgrp_;
    uiLabel*		inplbl_;
    uiComboBox*		inpfld_;
    uiComboBox*		subinpfld_	= nullptr;
    uiGenInput*		constfld_;
    uiToolButton*	vwbut_		= nullptr;
    uiLabel*		formlbl_	= nullptr;
    uiUnitSel*		unfld_		= nullptr;

    void		selectInput(const char*,bool exact=false,
				    bool hasfixedunits =false);
    bool		isFormUnitDisplayed() const;
    void		updateDisp(bool hasfixedunits =false);
    void		updateInpNms(bool sub);
    void		displayUnitFld(bool yn);
    void		setActive(bool);
    void		setVariable(const char*,bool cst,
				    bool hasfixedunits);

public:

    mDeprecated("Use setFormUnit")
    void		setUnit(const char*);
    mDeprecated("Use setFormUnit")
    void		setUnit( const UnitOfMeasure* uom )
			{ setFormUnit( uom, true ); }
    mDeprecated("Provide a Mnemonic object")
    void		setPropType(Mnemonic::StdType);
    mDeprecatedObs
    void		setSelUnit(const UnitOfMeasure*);
    mDeprecatedObs
    uiGroup*		rightMostField()		{ return nullptr; }

};
