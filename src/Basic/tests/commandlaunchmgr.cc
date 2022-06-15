/*+
 * (C) dGB Beheer B.V.; (LICENSE) http://opendtect.org/OpendTect_license.txt
 * AUTHOR   : Wayne Mogg
 * DATE     : June 2022
 * FUNCTION :
-*/

#include "envvars.h"
#include "oscommand.h"
#include "testprog.h"
#include "commandlaunchmgr.h"

#include <iostream>



using namespace Threads;

class TestClass : public CallBacker
{
public:
    TestClass(const char* expected_ouput)
	: expout_(expected_ouput)
    {}

    ~TestClass()
    {}

    void finishedCB( CallBacker* cb )
    {
	const auto* cmdtask = CommandLaunchMgr::getMgr().getCommandTask( cb );
	finished_ = true;
	if ( cmdtask )
	{
	    output_ = cmdtask->getStdOutput();
	    error_ = cmdtask->getStdError();
	}
    }

    void wait4Finish()
    {
	while ( !finished_ )
	    Threads::sleep( 1 );
    }

    bool isOK(bool check_stderr=false)
    {
	return finished_ && (check_stderr ? expout_==error_ : expout_==output_);
    }

    BufferString output_;
    BufferString error_;
    bool finished_ = false;

    BufferString expout_;
};

static bool testCmdWithCallback()
{
    BufferString hello( "Hello Test with Callback" );
#ifdef __win__
    //TODO
#else
    OS::MachineCommand machcomm1( "echo", hello );
    OS::MachineCommand machcomm2( "echo", hello, ">&2" );
#endif
    CommandLaunchMgr& mgr = CommandLaunchMgr::getMgr();
    {
	TestClass test( hello );
	CallBack cb(mCB(&test, TestClass, finishedCB));
	mgr.execute( machcomm1, true, true, &cb );
	test.wait4Finish();
	mRunStandardTest( test.isOK(), "CommandLaunchMgr::read stdout" );
    }
    {
	TestClass test( hello );
	CallBack cb(mCB(&test, TestClass, finishedCB));
	mgr.execute( machcomm2, true, true, &cb );
	test.wait4Finish();
	mRunStandardTest( test.isOK(true), "CommandLaunchMgr::read stderr" );
    }
    return true;
}


int mTestMainFnName( int argc, char** argv )
{
    mInitTestProg();

    // Debugging output screws up the command output, needs to be disabled:
    UnsetOSEnvVar( "DTECT_DEBUG" );


    const bool result = testCmdWithCallback();

    return result ? 0 : 1;
}
