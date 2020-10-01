<?xml version="1.0" encoding="UTF-8"?>

<galaxy_template version="0.7">

	<section_home_page HomeText="" HomeLogo="" />
	
	<report name="Bin Pareto Report">

		<datasets>
			<dataset ID="DS1" export="true" WithOverall="false" >
			</dataset>
		</datasets>
		
		<title><![CDATA[(('$P{P_TOP_BOTTOM}' == 'desc') ? 'Top ' : 'Bottom ') + $P{P_MAX_NUM_AGGREGATES_IN_CHART} + (('$P{P_DISPLAY_BINS}' == 'fail_bins') ? ' failing' : (('$P{P_DISPLAY_BINS}' == 'pass_bins') ? ' pass' : '')) + ' bins Pareto']]></title>
		
		<group dataset="DS1" Split="DS1:SPLIT" Aggregate="DS1:AGGREGATE" Serie="DS1:SERIE" Order="$PBinVolume|$P{P_TOP_BOTTOM}">
			<chart visible="$P{P_SHOWCHARTS}">
				<xaxis pos="bottom" name="$PAggregate" limit="$P{P_MAX_NUM_AGGREGATES_IN_CHART}" labelExpression="$FBinNo + ' - ' + $FBinName" maxLabelSize="$P{P_MAX_LABEL_SIZE}"/>
				<yaxis visible="true" pos="left" name="Yield (%)" type="line" min="0" max="100">
					<serie  name="$FSerie + ' - Yield'" value="($FBinVolume / $FGroupSumBinVolume) * 100" 
							printdatawhen="('$P{P_DISPLAY_BINS}' == 'pass_bins') ? $FBinCat == 'P' : (('$P{P_DISPLAY_BINS}' == 'fail_bins') ? $FBinCat == 'F' : true)"/> 
				</yaxis>
				<yaxis visible="true" pos="right" type="bars" name="Volume">
					<serie  name="$FSerie + ' - Volume'" value="$FBinVolume" 
							printdatawhen="('$P{P_DISPLAY_BINS}' == 'pass_bins') ? $FBinCat == 'P' : (('$P{P_DISPLAY_BINS}' == 'fail_bins') ? $FBinCat == 'F' : true)" /> 
				</yaxis>
				<legend visible="$P{P_SHOWLEGEND}"/>
			</chart>

			<table type="vertical" visible="$P{P_SHOWTABLES}">
				<header>
					<line>
						<cell value="String('Bin Name')" align="center"/>
						<cell value="$PAggregate" align="center"/>
						<cell value="String('Type')" align="center"/>
						<cell value="String('Volume')" align="center"/>
						<cell value="String('Total Volume')" align="center"/>
						<cell value="String('Yield')" align="center"/>
					</line>
				</header>
				<row printwhen="('$P{P_DISPLAY_BINS}' == 'pass_bins') ? $FBinCat == 'P' : (('$P{P_DISPLAY_BINS}' == 'fail_bins') ? $FBinCat == 'F' : true)">
					<line>
						<cell value="$FBinName + '(' + $FBinNo +')'" align="center"/>
						<cell value="$FAggregate" align="center"/>
						<cell value="$FBinCat" align="center"/>
						<cell value="$FBinVolume" align="center"/>
						<cell value="$FGroupSumBinVolume" align="center"/>
						<cell value="Number(($FBinVolume / $FGroupSumBinVolume) * 100).toFixed(2) + String(' %')" align="center"/>
					</line>
				</row>
			</table>
		</group>

		<summary><![CDATA[Notes :<br> - Total parts measures have been computed using consolidated data. <br> - The Yield measure is computed from the number of parts with the specified binning compared to the total number of parts.]]></summary>
	</report>

</galaxy_template>
