********************************************************************
*          Thank you for choosing  Quantix                         *
*                                                                  *
* Welcome to Quantix Examinator, the solution that turns test data *
*        into knowledge at a blazing speed!                        *
********************************************************************


Please read the following paragraphs to complete your Examinator installation.



=======================================================
          SOFTWARE PACKAGE
=======================================================
The Quantix Examinator software package includes the following:
  o 'gex' software   : The complete Examinator application



=======================================================
Standalone or Evaluation installation: PC Windows
=======================================================

1. Once you have downloaded the application from Quantix's web site 
   (filename quantix_examinator_XXX.exe), you can run it as is on 
   each PC that will run a standalone license. This file 
   self-extracts and automatically installs the software, 
   so all you need to do is confirm the installation location, 
   and validate few assistant pages.
   Once file extraction is complete, the software is launched 
   automatically. You can restart it at any time by clicking 
   the Start Button, then look in the Programs submenu 
   for Quantix Examinator.

2. Examinator displays a welcome page that lets you decide your installation mode.

3. For a 'Standalone' installation, select the running mode to 'Standalone' 
   then enter your full name and Product Key ID (sent to you by email on 
   reception of your PO). Otherwise, select the 'Evaluation' running 
   mode to start a 10 days trial period.

4. Click the 'Next' button

5. Email the 'gex_request.txt' file created in your Windows directory
   to gex.license@galaxysemi.com

6. You will receive an automated reply with the attachment
   file 'gex_license.txt'. Copy this file to your Examinator directory.

7. Run the Examinator software again...it is now activated for 12 months! 
   (or 10 days for an Evaluation)

8. Any question? Check the support section on www.mentor.com



=======================================================
Standalone or Evaluation installation: Unix
=======================================================

1. Install the Quantix Examinator software on each Unix 
   workstation that will run Examinator. To do it, simply enter 
   the following command lines:
   
     gzip -d quantix_examinatorXXX.tar.gz
     tar xovf quantix_examinatorXXX.tar

Folder created:
  'quantix_examinator'

   Note: XXX is the software release and Unix target (e.g: 320_hp)

2. Edit your .login file, located in your HOME directory, and add the 
   environment variable GEX_PATH and set it to the path of the Quantix 
   Examinator folder you just created. Then, edit your PATH variable so 
   it includes this new path.

3. Edit (or create) the environment variable LD_LIBRARY_PATH (under Solaris)
   or SHLIB_PATH (under HP-UX) and set it to include the path to the 
    'quantix_examinator' folder.

4. Ensure that your HOME variable exists and points to your home directory.
   Quantix Examinator uses it to store its settings.

   Example of SOLARIS or Linux .login section with the Quantix Examinator changes:

     setenv GEX_PATH /export/home/quantix_examinator
     setenv PATH $PATH:$GEX_PATH

   Example of HP-UX.login section with the Quantix Examinator changes:

     setenv GEX_PATH /export/home/quantix_examinator
     setenv PATH $PATH:$GEX_PATH

5. Save your changes, and execute the command: source .login

6. Launch the Examinator software using the appropriate command: 
   gex     (for Examinator)
   gexpro  (for Examinator-Pro)


7. Examinator will display a welcome page that lets you decide your installation mode.

8. For a 'Standalone' installation, select the running mode to 'Standalone' then 
   enter your full name and Product Key ID (sent to you by email on reception of 
   your PO). Otherwise, select the 'Evaluation' running mode to start a 10 days
   trial period.

9. Click the 'Next' button

10.Email the 'gex_request.txt' file created in your HOME directory to 
   gex.license@galaxysemi.com

11.You will receive an automated reply with the attachment file 'gex_license.txt'.
   Copy this file to your Examinator directory.

12.Run the Examinator software again...it is now activated for 12 months! (or 10 
   days for an Evaluation)

13.Any question? Check the support section on www.mentor.com
   If you run into any problems during the installation, contact 
   the Quantix support team, or request help from a Unix expert.



=======================================================
Server installation
=======================================================

1. Install the Quantix Examinator software at each client machine (see 
   instructions above for Unix or PC clients).

2. Install the Quantix Examinator software on the server that 
   will run the license manager software.

3. On the server, run the 'gex-lm' software.

4. From the welcome page, enter your full name and Product Key ID 
   (sent to you by email on reception of your PO). Notice: If the 
   server is a PC it will ONLY service PC clients, and if the server 
   is a Solaris station, it will only service Solaris clients. 
   For servicing clients of different computer platforms, you need 
   as many servers as platforms (for details, contacts Quantix sales)

5. Email the 'gex_request.txt' file created in your Windows 
   (for a PC server) or HOME directory (for a Unix server) 
   to gex.license@galaxysemi.com

6. You will receive an automated reply with the attachment 
   file 'gex_license.txt'. Copy this file to your server GEX directory.

7. Run the GEX-LM software again...it is now activated for 12 months!

8. Any question? Check the support section on www.mentor.com



=======================================================
Running GEX-LM Server
=======================================================

1. Simply launch 'gex-lm', it is a background application that can be
   launched like any other startup service (daemon) on your server.

2. Run the 'gex-ls' software at any time to see the license usage status

3. Any question? Check the support section on www.mentor.com

Note: GEX-LM is communicating with GEX applications using the default
   socket port number 4242. To force GEX-LM to use a different port,
   simply launch it with the following arguments:
   
     gex-lm -port <PortNumber>
   
   E.g.: gex-lm -port 4240 will have GEX-LM use socket port# 4240




=======================================================
Running GEX clients
=======================================================

1. The 'gex-lm' license manager MUST be running on your server.
   If not, any GEX client will refuse to run and report an error.

2. From the GEX welcome page, select the running mode to 'Connect to Server'
   then enter the server name (or IP address) and the socket port#
   (default is port 4242) used for communication.

3. Run the 'gex-ls' software at any time to see the license usage status

4. Any question? Check the support section on www.mentor.com

Note: You can display the GEX welcome page at any time and change
   the server name or socket port# to use. To do it, simply launch
   GEX with the following argument: gex -W



=======================================================
============ FREE UPGRADES ============================
=======================================================

During the validity period of your GEX licenses, you can
download and install any new release posted on the Quantix web site.
Installation is simple as all you need is to overwrite your old
release!

About GEX-LM: If you upgrade your GEX server software, then 
first terminate/kill the GEX-LM application. It will also force 
all existing GEX nodes to close. Once the new GEX-LM is installed,
simply launch it to let GEX nodes connect again.




=======================================================
To contact us, simply use the following addresses:

Quantix support: support@mentor.com
Quantix sales: quantix_sales@mentor.com

We hope you will be satisfied with Quantix Examinator!
Kind regards,
The Sales Team at Quantix
=======================================================

