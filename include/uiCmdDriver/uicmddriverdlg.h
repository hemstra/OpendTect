#pragma once
/*+
________________________________________________________________________

 Copyright:	(C) 1995-2022 dGB Beheer B.V.
 License:	https://dgbes.com/licensing
________________________________________________________________________

-*/

#include "uicmddrivermod.h"
#include "uidialog.h"

class uiCheckBox;
class uiFileInput;
class uiLabel;
class uiLabeledComboBox;
class uiPushButton;
class uiTextEdit;
class uiToolButton;
class uiTreeView;
class uiTreeViewItemIterator;

namespace CmdDrive
{

class CmdDriver;
class CmdRecorder;
class InteractSpec;
class ScriptItem;


mExpClass(uiCmdDriver) uiCmdInteractDlg : public uiDialog
{
mODTextTranslationClass(uiCmdInteractDlg)
public:
				uiCmdInteractDlg(uiParent*,const InteractSpec&);

    bool			unHide() const		{ return unhide_; }

protected:
    bool			rejectOK(CallBacker*) override;
    uiTextEdit*			infofld_;
    uiLabel*			resumelbl_;
    bool			unhide_;
};


mExpClass(uiCmdDriver) uiCmdDriverDlg : public uiDialog
{
mODTextTranslationClass(uiCmdDriverDlg)
public:
				uiCmdDriverDlg(uiParent*,
					       CmdDriver&,CmdRecorder&,
					       const char* defaultscriptsdir=0,
					       const char* defaultlogdir=0);
				~uiCmdDriverDlg();

    void			popUp();
    void			autoStartGo( const char* fnm);
    void			executeFinished();
    void			beforeSurveyChg();
    void			afterSurveyChg();

protected:

    void			selChgCB(CallBacker*);
    void			inpSelCB(CallBacker*);
    void			selectGoCB(CallBacker*);
    void			selectAbortCB(CallBacker*);
    void			selectPauseCB(CallBacker*);
    void			selectStartRecordCB(CallBacker*);
    void			selectStopRecordCB(CallBacker*);
    void			interactCB(CallBacker*);
    bool			rejectOK(CallBacker*) override;
    void			interactClosedCB(CallBacker*);
    void			toolTipChangeCB(CallBacker*);

    void			refreshDisplay(bool runmode,bool idle);
    void			setDefaultSelDirs();
    void			setDefaultLogFile();

    uiFileInput*		inpfld_;
    uiFileInput*		outfld_;
    uiFileInput*		logfld_;

    bool			inpfldsurveycheck_;
    bool			outfldsurveycheck_;
    bool			logfldsurveycheck_;
    BufferString		logproposal_;

    uiLabeledComboBox*		cmdoptionfld_;
    uiPushButton*		gobut_;
    uiPushButton*		abortbut_;
    uiPushButton*		pausebut_;
    uiPushButton*		startbut_;
    uiPushButton*		stopbut_;
    uiCheckBox*			tooltipfld_;
    CmdDriver&			drv_;
    CmdRecorder&		rec_;

    BufferString		defaultscriptsdir_;
    BufferString		defaultlogdir_;

    uiCmdInteractDlg*		interactdlg_;

private:

    static uiString sInterrupting() { return tr("-Interrupting-"); }
};


mExpClass(uiCmdDriver) uiScriptRunnerDlg : public uiDialog
{
mODTextTranslationClass(uiScriptRunnerDlg)
public:
				uiScriptRunnerDlg(uiParent*,CmdDriver&);
				~uiScriptRunnerDlg();

protected:
    uiFileInput*		scriptfld_;
    uiTreeView*			scriptlistfld_;
    uiToolButton*		editbut_;
    uiToolButton*		logbut_;
    uiToolButton*		cleanbut_;
    uiPushButton*		gobut_;
    uiPushButton*		stopbut_;

    void			inpSelCB(CallBacker*);
    void			settingsCB(CallBacker*);
    void			addChildren(ScriptItem&);
    bool			acceptOK(CallBacker*) override;
    void			selChgCB(CallBacker*);
    void			editScriptCB(CallBacker*);
    void			showLogCB(CallBacker*);
    void			cleanCB(CallBacker*);
    void			cleanLogs();
    void			goCB(CallBacker*);
    void			stopCB(CallBacker*);
    void			executeFinishedCB(CallBacker*);
    bool			executeNext();
    void			doubleClickCB(CallBacker*);
    void			rightClickCB(CallBacker*);

    CmdDriver&			drv_;
    uiTreeViewItemIterator*	iter_		= nullptr;
    bool			abort_		= false;
};

} // namespace CmdDrive
