Quantix GTL-enVision library

Purpose : 
This library is needed in order for an LTXC enVision (MX,...) test program to interact with the GTL solution of Quantix.

Tested & validated with : enVision R15.7.2 running on CentOS 4.6.

Installation and execution of the unit test:
- copy/paste the libGTL-enVision.so.0 into your test program folder (near the .eva/.tp files)
- from the test program folder (the folder containing the eva/tp and libGTLenVision.so.0, it is very important), run the launcher (/opt/ltx/bin/launcher)
- run a GTM server wherever you like
- configure/check your tester.conf file to point to your gtm server
- in enVision "Tool Op", open the unit_test.eva in order to run the unit test 

If enVision cannot open GTL, check the log:
/ltx/testers/galaxy_sim/log/evtc.log

Special notes about the GTL Cadence interface:
- there is no special documentaiton for the GTL envision wrapper as the interface is 99.99% the same as the C one. Documentation regarding the GTL interface is consequenlty inside the gtl_core.h file. 
- cad_gtl_get_number_messages_in_stack() does not return a status code but directly the number of messages in stack.
- all the other functions returns a status code as defined in gtl_core.h
- because of the CCC LTXC string bug, it is not possible to use several strings in the params of a function. Consequenlty most GTL functions asking for several string params are using integer table instead of strings in order to pass such arguments: example:
    integer: MyIntegerString1[MAXSTRLEN]
    ltx_atoi(".", MyIntegerString1, false) --Copy string to a integer table with final 0
    sts=call_c_library("cad_gtl_set", "output_folder", MyIntegerString1)

__________________________________________________________

 Copyright Quantix
 This computer program is protected by copyright law
 and international treaties. Unauthorized reproduction or
 distribution of this program, or any portion of it, may
 result in severe civil and criminal penalties, and will be
 prosecuted to the maximum extent possible under the law.
