Everything you need to build a library should be contained in the Cfiles directory.

It contains sample code to create a ramp, create a sin wave, do an fft (forward fft), do an inverse fft.
It also contains code to demonstrate the use of strings in library calls.

The CADENCE directory contains a test program to open the library and call each of the functions in it.
You will need to add the arrays to the status display to see the results of the program.

As far as the BINARY directory is concerned, this whole directory structure can be used whenever one wants to create
a new library of some kind. The BINARIES directory would contain any binary files that may need to be read 
into the test program to verify the operation of the library. Since this sample program does not need any binaries,
there are none there to copy.

The create_REL_DIR script will create a REL_DIR directory and copy all the libraries and Cadence code into it
as well as the verify test program. It will also create a tar'd/zipped version of the REL_DIR.

If you are in the REL_DIR when you run launcher, you can open the verify.eva file and the test program
assumes that the libraries will be contained in the same directory as the test program. This is 
the reccommended location for the libraries.

The verify.tp program has a procedure in it to open the library. It detects which OS and which platform
you are running on and opens the appropriate library. The code that allows you to detect the architecture
of the tester is the following in the eva file:

      OperatorVariable WS_type {
          Comment = "";
          Expr = Expr { String = "getenv('ARCH_NAME')"; }
          UserMode = Production;
      }
The Cadence code to use this is as follows:

        R14_3=false
        os_name=tester_os
        if os_name[1:3] = "R11" then
            lib_name="libMyLib.R11.so." + chr(0x30+CCC_LIB_VERSION)
        elseif os_name[1:4] >= "R14." then
            get_expr("WS_type",WS_type)
            if WS_type[1:3] = "sun" then
                if os_name[5] >="3" then
                    lib_name="libMyLib.R14.3.so." + chr(0x30+CCC_LIB_VERSION)
                    R14_3=true
                else
                    lib_name="libMyLib.R12.so." +chr(0x30+CCC_LIB_VERSION)
                endif 
            elseif os_name[5] >="3" then
                lib_name="libMyLib.LinuxR14.3.so." + chr(0x30+CCC_LIB_VERSION)
                R14_3=true
            else
                lib_name="libMyLib.Linux.so." + chr(0x30+CCC_LIB_VERSION)
            endif
        else
            lib_name="libMyLib.R12.so." +chr(0x30+CCC_LIB_VERSION)
        endif
    
This does not work in OS's prior to R14, and since earlier OS's do not support Linux, it is not 
needed when we are working in those OS's.

The status_display file contains a snapshot of what the status display should look like after all 
calls have been made to the library. You can look at the file using the SUN Image Viewer application
or Gimp in Linux.

The Cfiles/Makefile has comments at it's beginning to help you customize the Makefile for 
your application.

Any questions, email bob_nardi@ltx.com

-------------------------------------------------------
Note: This software is all covered under the:
                       LTX SHAREWARE LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

    Version 1.1
    January 20, 2006

  0. This License Agreement applies to any program or other work which
contains a notice placed by the copyright holder saying it may be
distributed under the terms of this LTX Shareware License.  The
"Program", below, refers to any such program or work, and a "work based
on the Program" means either the Program or any work containing the
Program or a portion of it, either verbatim or with modifications.  Each
licensee is addressed as "you".

  1. You may copy and distribute verbatim copies of the Program's source
code as you receive it, in any medium, provided that you conspicuously and
appropriately publish on each copy an appropriate copyright notice and
disclaimer of warranty; keep intact all the notices that refer to this
LTX Shareware License and to the absence of any warranty; and give any
other recipients of the Program a copy of this LTX Shareware License
along with the Program.   LTX Shareware Programs may only be distributed
to customers of LTX Corporation.

  2. You may modify your copy or copies of the Program or any portion of
it, and copy and distribute such modifications under the terms of Paragraph
1 above, provided that you also do the following:

    a) cause the modified files to carry prominent notices stating that
    you changed the files and the date of any change; and

    b) cause the whole of any work that you distribute or publish, that
    in whole or in part contains the Program or any part thereof, either
    with or without modifications, to be licensed at no charge to LTX
    and to all LTX Customers under the terms of this LTX Shareware License.

    c) If the modified program normally reads commands interactively when
    run, you must cause it, when started running for such interactive use
    in the simplest and most usual way, to print or display an
    announcement including an appropriate copyright notice and a notice
    that there is no warranty (or else, saying that you provide a
    warranty) and that users may redistribute the program under these
    conditions, and telling the user how to view a copy of this LTX
    Shareware License.

Mere aggregation of another independent work with the Program (or its
derivative) on a volume of a storage or distribution medium does not bring
the other work under the scope of these terms.

  3. You may copy and distribute the Program (or a portion or derivative of
it, under Paragraph 2) in object code or executable form under the terms of
Paragraphs 1 and 2 above provided that you also do one of the following:

    a) accompany it with the complete corresponding machine-readable
    source code, which must be distributed under the terms of
    Paragraphs 1 and 2 above; or,

    b) accompany it with a written offer to give any LTX Customer free
    (except for a nominal charge for the cost of distribution) a complete
    machine-readable copy of the corresponding source code, to be distributed
    under the terms of Paragraphs 1 and 2 above; or,

    c) accompany it with the information you received as to where the
    corresponding source code may be obtained.  (This alternative is
    allowed only for noncommercial distribution and only if you
    received the program in object code or executable form alone.)

Source code for a work means the preferred form of the work for making
modifications to it.  For an executable file, complete source code means
all the source code for all modules it contains; but, as a special
exception, it need not include source code for modules which are standard
libraries that accompany the operating system on which the executable
file runs, or for standard header files or definitions files that
accompany that operating system.

  4. You may not copy, modify, sublicense, distribute or transfer the
Program except as expressly provided under this LTX Shareware License.
Any attempt otherwise to copy, modify, sublicense, distribute or transfer
the Program is void, and will automatically terminate your rights to use
the Program under this License.  However, parties who have received
copies, or rights to use copies, from you under this LTX Shareware
License will not have their licenses terminated so long as such parties
remain in full compliance.

  5. By copying, distributing or modifying the Program (or any work based
on the Program) you indicate your acceptance of this license to do so,
and all its terms and conditions.

  6. Each time you redistribute the Program (or any work based on the
Program), the recipient automatically receives a license from the original
licensor to copy, distribute or modify the Program subject to these
terms and conditions.  You may not impose any further restrictions on the
recipients' exercise of the rights granted herein.

  7. LTX Corporation may publish revised and/or new versions
of the LTX Shareware License from time to time.  Such new versions will
be similar in spirit to the present version, but may differ in detail to
address new problems or concerns.

Each version is given a distinguishing version number.  If the Program
specifies a version number of the license which applies to it and "any
later version", you have the option of following the terms and conditions
either of that version or of any later version published by LTX Corporation.
If the Program does not specify a version number of
the license, you may choose any version ever published by LTX
Corporation.

                            NO WARRANTY

  9. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY
FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN
OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES
PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS
TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
REPAIR OR CORRECTION.

  10. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR
REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,
INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER
PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES.


