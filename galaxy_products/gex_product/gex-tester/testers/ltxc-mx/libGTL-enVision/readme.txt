GalaxySemi GTL-enVision library version 0.3
linked with GalaxySemi GTL core 3.2
July 11th 2013

Purpose : 
This library is needed in order for an LTXC enVision (MX,...) test program to interact with the GTL solution of GalaxySemi.

Tested & validated with : enVision R15.7.2 running on CentOS 4.6.

Installation and execution of the unit test:
- copy/paste the libGTL-enVision.so.0 into your test program folder (near the .eva/.tp files)
- from the test program folder (the folder containing the eva/tp and libGTLenVision.so.0, it is very important), run the launcher (/opt/ltx/bin/launcher)
- run a GTM server wherever you like
- configure/check your tester.conf file to point to your gtm server
- in enVision "Tool Op", open the unit_test.eva in order to run the unit test 

Special notes about the GTL Cadence interface:
- there is no special documentaiton for the GTL envision wrapper as the interface is 99.99% the same as the C one. Documentation regarding the GTL interface is consequenlty inside the gtl_core.h file. 
- cad_gtl_get_number_messages_in_stack() does not return a status code but directly the number of messages in stack.
- all the other functions returns a status code as defined in gtl_core.h

__________________________________________________________

 Copyright Galaxy
 This computer program is protected by copyright law
 and international treaties. Unauthorized reproduction or
 distribution of this program, or any portion of it, may
 result in severe civil and criminal penalties, and will be
 prosecuted to the maximum extent possible under the law.
