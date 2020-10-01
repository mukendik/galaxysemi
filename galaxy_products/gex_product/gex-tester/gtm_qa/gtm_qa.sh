#!/bin/bash

export DEVDIR=/s/galaxy_dev_master
export GEX_PATH=/s/galaxy_dev_master/galaxy_products/gex_product/bin
export PATH=$GEX_PATH:$PATH
export LD_LIBRARY_PATH=$GEX_PATH
export GEX_SERVER_PROFILE=T:/gtm_qa/ServerProfile
export GS_GTS_TESTERCONF=./gtm/gtl_tester.conf
export GS_GTS_RECIPE=./nospat_bl50_notuning_recipe.csv
export GS_QA=1

cd /t/gtm_qa

echo "Current directory is `pwd`"
echo "GEX_SERVER_PROFILE=$GEX_SERVER_PROFILE"
echo "GEX_LOGLEVEL=$GEX_LOGLEVEL"
echo "GS_QA=$GS_QA"
echo "GS_GTS_TESTERCONF=$GS_GTS_TESTERCONF"
echo "GS_GTS_RECIPE=$GS_GTS_RECIPE"

writeGsLogXml()
{
	site=$1	
	echo '<?xml version="1.0" encoding="UTF-8"?>' > ${GEX_SERVER_PROFILE}/GalaxySemi/gslog.xml
	echo '<gslog version="0.2" threaded="true" http_port="8080">' >> ${GEX_SERVER_PROFILE}/GalaxySemi/gslog.xml
	echo '<log type="txt" outputfile="$GEX_SERVER_PROFILE/../gtm/s'${site}'/logs/$MODULE_$ISODATE_$PID.txt" />' >> ${GEX_SERVER_PROFILE}/GalaxySemi/gslog.xml
	echo '</gslog>' >> ${GEX_SERVER_PROFILE}/GalaxySemi/gslog.xml
}

writeGexLogXml()
{
	site=$1	
	echo '<?xml version="1.0" encoding="UTF-8"?>' > ${GEX_SERVER_PROFILE}/GalaxySemi/gexlog.xml
	echo '<gexlog version="0.2" threaded="false" thread_priority="0" logs_center="false" >' >> ${GEX_SERVER_PROFILE}/GalaxySemi/gexlog.xml
	echo '<log type="txt" outputfile="$GEX_SERVER_PROFILE/../gtm/s'${site}'/logs/$MODULE_$ISODATE_$PID.txt" />' >> ${GEX_SERVER_PROFILE}/GalaxySemi/gexlog.xml
	echo '</gexlog>' >> ${GEX_SERVER_PROFILE}/GalaxySemi/gexlog.xml
}

prepareSite()
{
	site=$1
	# Remove all logs
	rm -rf ${GEX_SERVER_PROFILE}/../gtm/s${site}/logs/* >/dev/null 2>&1
	# Remove output files
	rm -rf ${GS_QA_OUTPUT_FOLDER}/* >/dev/null 2>&1
	# Write LOG xml files
	writeGsLogXml ${site}
	writeGexLogXml ${site}
}
	
processSite()
{
	site=$1
	export GS_QA_OUTPUT_FOLDER=./gtm/s${site}/out
    if [ $site = all ]; then
		export GS_GTS_STDF=./gtm/s${site}/TNA0KA031E_R_FT_ROOM_ETS88-016A_20120614_012427.stdf
	else
		export GS_GTS_STDF=./gtm/s${site}/60_PASS_GEXDB_FT_PMAX8556ETE__TETS88_016A_LTNA0KA031E_R_S${site}_1300100001.stdf
    fi
	
	prepareSite ${site}
	
	echo "#### Processing Site ${site}"
	#echo "GS_QA_OUTPUT_FOLDER=$GS_QA_OUTPUT_FOLDER"
	#echo "GS_GTS_STDF=$GS_GTS_STDF"
	echo "Starting GTM"
	gexd.exe -GTM -H & 
	ret=$?
	gtm_pid=$!
	echo "GTM output=$ret"
	sleep 30
	echo 'Starting GTS'
	gts-stationd --hidden --autoclose
	ret=$?
	echo "GTS output=$ret"
	echo "Killing GTM (pid=$gtm_pid)"
	kill -9 $gtm_pid >/dev/null 2>&1
	echo "GTM killed"
    if [ $ret = 0 ]; then
		echo "#### Site ${site} done: PASS"
    else
		echo "#### Site ${site} done: FAIL"
    fi
	
	return $ret
}

status=pass
processSite 0
if [ $? != 0 ]; then 
	status=fail 
fi
processSite 1
if [ $? != 0 ]; then 
	status=fail 
fi
processSite 2
if [ $? != 0 ]; then 
	status=fail 
fi
processSite 3
if [ $? != 0 ]; then 
	status=fail 
fi
processSite 4
if [ $? != 0 ]; then 
	status=fail 
fi
processSite 5
if [ $? != 0 ]; then 
	status=fail 
fi
processSite 6
if [ $? != 0 ]; then 
	status=fail 
fi
processSite 7
if [ $? != 0 ]; then 
	status=fail 
fi
processSite all
if [ $? != 0 ]; then 
	status=fail 
fi

echo ""
echo "Scenario completed with status ${status}"
echo ""
