#pragma once
/*+
________________________________________________________________________

 Copyright:	(C) 1995-2022 dGB Beheer B.V.
 License:	https://dgbes.com/licensing
________________________________________________________________________

-*/

#include "mpeenginemod.h"
#include "refcount.h"

#include "emposid.h"
#include "factory.h"
#include "trckeyzsampling.h"
#include "uistring.h"

class Executor;

namespace Geometry { class Element; }
namespace EM { class EMObject; }
namespace Attrib { class SelSpec; }

namespace MPE
{

class EMSeedPicker;
class SectionTracker;

/*!
\brief Tracks EM objects.
*/

mExpClass(MPEEngine) EMTracker : public ReferencedObject
{
mODTextTranslationClass(EMTracker)
public:
    BufferString		objectName() const;
    EM::EMObject*		emObject()		{ return emobject_; }
    EM::ObjectID		objectID() const;

    virtual bool		is2D() const		{ return false; }

    void			setTypeStr( const char* tp )
				{ type_ = tp; }
    const char*			getTypeStr() const	{ return type_.buf(); }

    virtual bool		isEnabled() const	{ return isenabled_; }
    virtual void		enable(bool yn)		{ isenabled_=yn; }

    virtual bool		snapPositions(const TypeSet<TrcKey>&);

    virtual TrcKeyZSampling	getAttribCube(const Attrib::SelSpec&) const;
    void			getNeededAttribs(
					TypeSet<Attrib::SelSpec>&) const;

    virtual SectionTracker*	createSectionTracker() = 0;
    SectionTracker*		cloneSectionTracker();
    SectionTracker*		getSectionTracker(bool create=false);
    virtual EMSeedPicker*	getSeedPicker(bool createifnotpresent=true)
				{ return nullptr; }
    void			applySetupAsDefault();

    const char*			errMsg() const;

    void			fillPar(IOPar&) const;
    bool			usePar(const IOPar&);

// Deprecated public functions
    mDeprecated("Use without SectionID")
    SectionTracker*		createSectionTracker(EM::SectionID)
				{ return createSectionTracker(); }
    mDeprecated("Use without SectionID")
    SectionTracker*		getSectionTracker(EM::SectionID,
						  bool create=false)
				{ return getSectionTracker(create); }
    mDeprecated("Use without SectionID")
    void			applySetupAsDefault(const EM::SectionID)
				{ applySetupAsDefault(); }


protected:
				EMTracker(EM::EMObject*);
    virtual			~EMTracker();

    bool			isenabled_		= true;
    ObjectSet<SectionTracker>	sectiontrackers_;
    BufferString		errmsg_;
    BufferString		type_;

    void			setEMObject(EM::EMObject*);

    static const char*		setupidStr()	{ return "SetupID"; }
    static const char*		sectionidStr()	{ return "SectionID"; }

private:
    EM::EMObject*		emobject_		= nullptr;
};

mDefineFactory1Param( MPEEngine, EMTracker, EM::EMObject*, TrackerFactory );

} // namespace MPE
