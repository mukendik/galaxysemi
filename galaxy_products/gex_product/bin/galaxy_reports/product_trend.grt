<?xml version="1.0" encoding="UTF-8"?>

<galaxy_template version="0.7">

	<section_home_page HomeText="" HomeLogo="" />

	<report name="Product trend report">

	<datasets>
		<dataset ID="DS1" export="true" WithOverall="true" >
		</dataset>
	</datasets>
	
		<title><![CDATA[Product Trend]]></title>
		
		<group dataset="DS1" Split="DS1:SPLIT" Aggregate="DS1:AGGREGATE" Serie="DS1:SERIE" Order="DS1:ORDER">
			<chart visible="$P{P_SHOWCHARTS}">
				<xaxis pos="bottom" name="$PAggregate" limit="$P{P_MAX_NUM_AGGREGATES_IN_CHART}" maxLabelSize="$P{P_MAX_LABEL_SIZE}"/>
				<yaxis visible="true" pos="left" name="Yield (%)" min="$P{P_CHART_LEFTAXIS_MIN}" max="$P{P_CHART_LEFTAXIS_MAX}" type="$P{P_YIELD_AXIS_STYLE}">
					<serie  name="$FSerie + ' - Yield'" value="$P{P_OVERALL_YIELD}"  printwhen="$FSerie=='Overall'"/>
					<serie  name="(('$P{P_BIN_NO}' == 'Worst') ? $FSerie + ' (Worst)' : $FSerie) + ' - Yield'" value="$P{P_YIELD}"  printwhen="(('$P{P_BIN_NO}' == 'Worst') ? $FSerie == $P{P_BIN_NO} : (String('$P{P_BIN_NO}').split('|').indexOf($FSerie)!=-1))"/> 
				</yaxis>
				<yaxis visible="$P{P_SHOW_VOL_IN_CHART}" pos="right" name="Volume" type="$P{P_VOL_AXIS_STYLE}">
					<serie  name="$FSerie + ' - Volume'" value="$P{P_VOLUME}"  printwhen="$FSerie=='Overall'"/> 
				</yaxis>
				<legend visible="$P{P_SHOWLEGEND}"/>
			</chart>

			<table type="vertical" visible="$P{P_SHOWTABLES}">
				<header>
					<line>
						<cell value="$PAggregate" align="center"/>
						<cell value="String('Volume')" align="center"/>
						<cell value="$PSerie" align="center"/>
						<cell value="String('Bin name')" align="center"/>
						<cell value="String('Bin Volume')" align="center"/>
						<cell value="String('Bin Yield')" align="center"/>
					</line>
				</header>
				<row printwhen="$FSerie != 'Overall'">
					<line>
						<cell value="$FAggregate" align="center" repeatedvalue="false"/>
						<cell value="$P{P_VOLUME}" align="center" repeatedvalue="false"/>
						<cell value="$FSerie" align="right"/>
						<cell value="$FBinName" align="center"/>
						<cell value="$FBinVolume" align="right"/>
						<cell value="(($P{P_VOLUME} == 0) ? 'n/a' : Number($P{P_YIELD}).toFixed(2) + String(' %'))" align="right"/>
					</line>
				</row>
			</table>
		</group>

		<summary><![CDATA[Notes :<br> - Total parts measures have been computed using consolidated data. <br> - The Yield measure is computed from the total number of parts with the specified binning(s) and the total $P{P_SUMMARY}.]]></summary>
		
	</report>

</galaxy_template>
