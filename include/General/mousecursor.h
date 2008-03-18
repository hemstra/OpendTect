#ifndef mousecursor_h
#define mousecursor_h

/*+
________________________________________________________________________

 CopyRight:     (C) dGB Beheer B.V.
 Author:        K. Tingdahl
 Date:          March 2008
 RCS:           $Id: mousecursor.h,v 1.2 2008-03-18 18:42:12 cvskris Exp $
________________________________________________________________________

-*/

#include "bufstring.h"
#include "ptrman.h"

/*!Definition of a mouse cursor, can be either a predefined shape (from the
   enum, or a file. */

class MouseCursor
{
public:
    virtual		~MouseCursor()					{}
		    /*! This enum type defines the various cursors that can be
		        used.

			\value Arrow		standard arrow cursor
			\value UpArrow		upwards arrow
			\value Cross		crosshair
			\value Wait		hourglass/watch
			\value Ibeam		ibeam/text entry
			\value SizeVer		vertical resize
			\value SizeHor 		horizontal resize
			\value SizeFDiag	diagonal resize (\)
			\value SizeBDiag	diagonal resize (/)
			\value SizeAll		all directions resize
			\value Blank		blank/invisible cursor
			\value SplitV		vertical splitting
			\value SplitH		horizontal splitting
			\value PointingHand	a pointing hand
			\value Forbidden	a slashed circle
			\value WhatsThis	an arrow with a question mark
			\value Bitmap
			\value NotSet 

			Arrow is the default for widgets in a normal state.
		    */
    enum Shape	{ Arrow, UpArrow, Cross, Wait, Ibeam,
		  SizeVer, SizeHor, SizeBDiag, SizeFDiag, SizeAll,
		  Blank, SplitV, SplitH, PointingHand, Forbidden,
		  WhatsThis, Last = WhatsThis, Bitmap = 24, NotSet
		};
    			MouseCursor() : shape_( NotSet )		{}
    bool		operator==(const MouseCursor&) const;
    bool		operator!=(const MouseCursor&) const;

    Shape		shape_;
    BufferString	filename_;
    int			hotx_;
    int			hoty_;
};


/*! \brief Sets another cursor for current application

    example:

    \code
	MouseCursorManager::setOverride( MouseCursor::Wait );
	calculateHugeMandelbrot();              // lunch time...
	MouseCursorManager::restoreOverride();
    \endcode    

    Application cursors are stored on an internal stack.
    setOverride() pushes the cursor onto the stack, and
    restoreOverride() pops the active cursor off the stack.
    Every setOverride() must eventually be followed by a
    corresponding restoreOverride(), otherwise the stack will
    never be emptied.  

    If replace is true, the new cursor will replace the last
    overridecw cursor (the stack keeps its depth). If replace is
    FALSE, the new cursor is pushed onto the top of the stack.
*/


class MouseCursorManager
{
public:

    virtual	~MouseCursorManager()					{}

    static void	setOverride(MouseCursor::Shape,bool replace=false);
    static void	setOverride(const MouseCursor&,bool replace=false);
    static void setOverride(const char* filenm,int hotx=-1, int hoty=-1,
	    	            bool replace=false);	
    static void	restoreOverride();


    static MouseCursorManager*		mgr();
    static void				setMgr(MouseCursorManager*);

protected:
    virtual void setOverrideShape(MouseCursor::Shape,bool replace)	= 0;
    virtual void setOverrideCursor(const MouseCursor&,bool replace)	= 0;
    virtual void setOverrideFile(const char* filenm,
	    			 int hotx,int hoty, bool replace)	= 0;	
    virtual void restoreInternal()					= 0;


    static PtrMan<MouseCursorManager>	mgr_;
};


/*! Class to automaticly change cursor, and change it back automaticly when
    class is running out of scope.
*/

class MouseCursorChanger
{
public:
		MouseCursorChanger(const char* fnm, int hotx, int hoty)
		{ MouseCursorManager::setOverride(fnm,hotx,hoty); }
		MouseCursorChanger(MouseCursor::Shape cs)
		{ MouseCursorManager::setOverride(cs); }
		~MouseCursorChanger() { MouseCursorManager::restoreOverride(); }
};


#endif
