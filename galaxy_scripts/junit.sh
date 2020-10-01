#!/bin/bash

# Example:
# $ junit.sh true
# result:0
# $ junit.sh false
# result:1

# <testsuites>
#     <testsuite name="packagename.classname">
#       <testcase name="test_name.log" time="231"></testcase>
#     </testsuite>
# </testsuites>

set -x

xmlstarletCmd=xmlstarlet
dateCmd=date
if [ `uname` = "Darwin" ]; then
  xmlstarletCmd=xml
  dateCmd=gdate
fi

checkOpt=$(getopt -o c:,h,d,o:,p:,t:,e:: -l classname:,help,debug,dryrun:,output:,package:,pwd:,testname:: -n $0 -- "$@")
if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi
eval set -- "$checkOpt"
debug=0
dryrun=0
while true; do
    case "$1" in
        -c | --classname ) classname="$2"; shift 2 ;;
        -d | --debug ) debug=1; shift ;;
        -h | --help )
            echo "NAME"
            echo "       $0 - Launch Execute and generate Unit test JUnit report"
            echo "SYNTAX"
            echo "       $0 [-o|--output   output file] cmd [args]"
            echo "       $0 [-d|--debug]"
            echo "         Verbose mode."
            echo "       $0 [-e|--dryrun]"
            echo "         Generate an empty JUnit output in error state, without executing the test."
            echo "       $0 -p|--package   JUnit package"
            echo "       $0 -t|--testname  JUnit testname"
            echo "       $0 -c|--classname JUnit classname"
            echo "       $0 --pwd          working directory"
            echo "       $0 -h|--help      display this help"
            echo "EXAMPLES"
            echo "       $0 --output --output=target/surefire-report/TEST-galaxy_unit_test_1.xml target/galaxy_unit_tests/galaxy_unit_test_1/galaxy_unit_test_1"
            echo "       $0 --debug  --output=target/xxx.xml ./tester01.sh coco"
            ;;
        -e | --dryrun ) dryrun=1; shift 2 ;;
        -o | --output ) outputFile="$2"; shift 2 ;;
        -p | --package ) package="$2"; shift 2 ;;
        --pwd ) myPwd="$2"; shift 2 ;;
        -t | --testname ) testname="$2"; shift 2 ;;
        -- ) shift; cmd="$@"; break ;;
        * ) break ;;
    esac
done

#cmd="$@"
shift
myVar=$@
if [ $debug -ne 0 ]; then echo "command: $cmd"; fi
myPid=$$
if [ -z "$outputFile" ]; then
  outputFile="junit_$myPid.xml"
fi
if [ ! -z "$myPwd" ]; then
  cd $myPwd
fi
mkdir -p $(dirname $outputFile)

# ELAPSED_TIME=$( echo "$( { time -p ($cmd 2> errFile_$myPid 1> outFile_$myPid);   } 2>&1 )" |grep "real" |sed "s/real *//g" )
START_TIME=$(${dateCmd} +%s$(printf "%.5s" $(${dateCmd} +%N)))
if [ $dryrun -eq 0 ]; then
  $cmd 2> errFile_$myPid 1> outFile_$myPid
else
  cat << EOF > errFile_$myPid
Execution failure.
EOF
  > outFile_$myPid
  false
fi
result=$?
END_TIME=$(${dateCmd} +%s$(printf "%.5s" $(${dateCmd} +%N)))
ET_=$( printf "%05d" $(( $END_TIME - $START_TIME )) )
myLen=$(echo $ET_ | awk '{ print length }')
ELAPSED_TIME="$(printf "%01d" ${ET_:0:$(( ${myLen} - 5 ))}).${ET_:$(( ${myLen} - 5 ))}"
myStdout=$(<outFile_$myPid)
myStderr=$(<errFile_$myPid)
rm ./outFile_$myPid
rm ./errFile_$myPid

if [ $debug -ne 0 ]; then echo "myStdout: $myStdout"; fi
if [ $debug -ne 0 ]; then echo "myStderr: $myStderr"; fi
if [ $debug -ne 0 ]; then echo "result: $result"; fi
if [ $debug -ne 0 ]; then echo "START_TIME: $START_TIME"; fi
if [ $debug -ne 0 ]; then echo "END_TIME: $END_TIME"; fi
if [ $debug -ne 0 ]; then echo "ET_: $ET_"; fi
if [ $debug -ne 0 ]; then echo "ELAPSED_TIME: $ELAPSED_TIME"; fi

if [ $debug -ne 0 ]; then echo "result: $result"; fi

cat << EOF > $outputFile
<?xml version="1.0" encoding="UTF-8"?>
<testsuite />
EOF

if [ -z "${package}" ]; then package="root"; fi
if [ -z "${classname}" ]; then classname="$(basename $cmd)"; fi
if [ -z "${testname}" ]; then testname="$(basename $cmd) $myVar"; fi

$xmlstarletCmd ed -L \
-s /testsuite -t attr -n "name" -v "${package}" \
-s /testsuite -t attr -n "tests" -v "1" \
-s /testsuite -t attr -n "failures" -v "$result" \
-s /testsuite -t attr -n "errors" -v "0" \
-s /testsuite -t attr -n "skipped" -v "0" \
-s /testsuite -t attr -n "time" -v "$ELAPSED_TIME" \
-s /testsuite -t elem -n testcase -v "" \
-s /testsuite/testcase -t attr -n "classname" -v "${package}.${classname}" \
-s /testsuite/testcase -t attr -n "name" -v "${testname}" \
-s /testsuite/testcase -t attr -n "time" -v "$ELAPSED_TIME" \
$outputFile

# FAILURE flag
if [ $result -ne 0 ]; then
  $xmlstarletCmd ed -L \
-s /testsuite/testcase -t elem -n failure -v "" \
$outputFile
fi

# stdout
xmlStdout="`echo $myStdout | $xmlstarletCmd esc`"

if [ ! -z "$myStdout" ]; then
  $xmlstarletCmd ed -L \
-s /testsuite/testcase -t elem -n system-out -v "$xmlStdout" \
$outputFile
fi

#stderr
myStderr="`echo $myStderr | $xmlstarletCmd esc`"

if [ ! -z "$myStderr" ]; then
  $xmlstarletCmd ed -L \
-s /testsuite/testcase -t elem -n system-err -v "$xmlStderr" \
$outputFile
fi

if [ $debug -ne 0 ]; then echo "outputFile: $outputFile"; fi
if [ $debug -ne 0 ]; then echo "JUnit xml report:"; cat $outputFile; fi
