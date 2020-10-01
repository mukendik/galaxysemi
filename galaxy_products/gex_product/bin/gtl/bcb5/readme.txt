This folder contains the unit test example files in order to run a GTL/GTM simulation using a Borland C Builder 5 client using the dynamic GTL library:

- On the Borland C Builder computer, open a command prompt with PATH access to c:\Program Files (x86)\Borland\CBuilder5\Bin\ or whatever your BCB installation folder   
- Launch implib_gtl.bat in order to create a BCB compatible library : the script should generate libgtl.lib from the gtl.dll file
- Open Borland C Builder
- Open the project UnitTest.bpr
- Be sure the link to the libgtl.lib file generated above, in the project pan, is correct 
- Be sure that the define DYNGTL is correctly added inside the "Conditionnals" project options
- Make
- Be sure the gtl.dll and mingwm10.dll are in the running folder 
- check your recipe and tester conf file are in the working folder
- Run
- a log file should have been created in the working folder. Open it to check for any error. 

 Copyright Quantix
 This computer program is protected by copyright law
 and international treaties. Unauthorized reproduction or
 distribution of this program, or any portion of it, may
 result in severe civil and criminal penalties, and will be
 prosecuted to the maximum extent possible under the law.
