<?xml version="1.0" encoding="UTF-8"?>

<galaxy_template version="0.7">

	<report name="Stacked Bin trend report">

		<datasets>
			<dataset ID="DS1" export="true" WithOverall="false">
				<postprocessing>
					<topnserie count="$P{P_TOP_N_SERIE}" othername="Other Failed Bins">
						<field name="bin_volume" order="desc" />
						<criteria>
							<field name="bin_cat" operator="=" value="F" />
						</criteria>
					</topnserie>
				</postprocessing>
			</dataset>
		</datasets>
		
		
				
		<title><![CDATA[$P{P_TITLE}]]></title>
		
		<group dataset="DS1" Split="DS1:SPLIT" Aggregate="DS1:AGGREGATE" Serie="DS1:SERIE" Order="DS1:ORDER">
			<chart visible="$P{P_SHOWCHARTS}">
				<xaxis pos="bottom" name="$PAggregate" limit="$P{P_MAX_NUM_AGGREGATES_IN_CHART}" maxLabelSize="$P{P_MAX_LABEL_SIZE}"/>
				<yaxis visible="true" pos="left" name="Yield for fail (%)" type="stackedbars" min="$P{P_CHART_LEFTAXIS_MIN}" max="$P{P_CHART_LEFTAXIS_MAX}">
					<serie  name="$FSerie + ' - ' + $FBinName" value="$P{P_YIELD}" printwhen="$FSerieBinCat!='P'"/> 
				</yaxis>
				<yaxis visible="true" pos="right" zorder="front" name="Yield for pass (%)" type="line" min="$P{P_CHART_RIGHTAXIS_MIN}" max="$P{P_CHART_RIGHTAXIS_MAX}">
					<serie  name="$FSerie + ' - ' + $FBinName" value="$P{P_YIELD}" printwhen="$FSerieBinCat=='P'"/> 
				</yaxis>
				<legend visible="$P{P_SHOWLEGEND}"/>
			</chart>
			<table type="vertical" visible="$P{P_SHOWTABLES}">
				<header>
					<line>
						<cell value="$PAggregate" align="center"/>
						<cell value="$PSerie" align="center"/>
						<cell value="'Bin name'" align="center"/>
						<cell value="String('Volume')" align="center"/>
						<cell value="String('SerieVolume')" align="center"/>
						<cell value="String('Yi	eld')" align="center"/>
					</line>
				</header>
				<row printwhen="$P{P_VOLUME} != 0 || $P{P_SHOW_NULL_VOLUME}">
					<line>
						<cell value="$FAggregate" align="center" repeatedvalue="false"/>
						<cell value="$FSerie" align="center"/>
						<cell value="$Fbin_name" align="center"/>
						<cell value="$P{P_VOLUME}" align="right"/>
						<cell value="$FSumBinVolume" align="right"/>
						<cell value="(($P{P_VOLUME} == 0) ? 'n/a' : Number($P{P_YIELD}).toFixed(2) + String(' %'))" align="right"/>
						
					</line>
				</row>
			</table>
		</group>
		
		<summary><![CDATA[Notes :<br> - Total parts measures have been computed using consolidated data. <br> - The Yield measure is computed from the total number of parts with the specified binning(s) and the total $P{P_SUMMARY}.]]></summary>

	</report>

</galaxy_template>
