This folder contains the GTL (GalaxyTesterLibrary) files needed in order for the testers to communicate with the Quantix servers product (GTM,...).
Always use the GTL libs included in the GTM package in order to satisfay compatibility between client (GTL) and server (GTM).

You should find from any GTM package :
- files in order to use the GTL with BorlandCBuilder 5 and 6
- include folder with the main header gtl_core.h
- lib folder with generic static and dynamic libs (win32, win64, linux32, linux64,...)
- a 'ltxc' folder with a special version of the GTL for the enVision platforms (MX,...)
- a 'teradyne' folder with a special version of the GTL for the IG-XL platforms (Flex,...) 
- a unit_test folder with a unit test sub folder to test the GTL before any integration
- a PDF for an introduction about the GTL solution
