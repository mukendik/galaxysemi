<?xml version="1.0" encoding="UTF-8"?>

<galaxy_template version="0.7">
	
	<report name="AZ Yield trend report">

		<datasets>
			<dataset ID="DS1" export="true" WithOverall="$P{P_OVERALL}">
			</dataset>
		</datasets>
	
		<title><![CDATA[$P{P_TITLE}]]></title>
		
		<group dataset="DS1" Split="DS1:SPLIT" Aggregate="DS1:AGGREGATECHART" Serie="DS1:SERIE" Order="DS1:ORDER">
			<chart visible="$P{P_SHOWCHARTS}">
				<xaxis pos="bottom" name="$PAggregate" limit="$P{P_MAX_NUM_AGGREGATES_IN_CHART}" maxLabelSize="$P{P_MAX_LABEL_SIZE}"/>
				<yaxis visible="true" pos="left" name="Yield (%)" type="$P{P_YIELD_AXIS_STYLE}" min="$P{P_CHART_LEFTAXIS_MIN}" max="$P{P_CHART_LEFTAXIS_MAX}">
					<serie  name="$FSerie + ' - Yield'" value="$P{P_YIELD}"/> 
				</yaxis>
				<yaxis visible="$P{P_SHOW_VOL_IN_CHART}" pos="right" name="Volume" type="$P{P_VOL_AXIS_STYLE}" >
					<serie  name="$FSerie + ' - Volume'" value="$P{P_VOLUME}" color="$P{P_BARS_COLOR}" />
				</yaxis>
				<legend visible="$P{P_SHOWLEGEND}"/>
			</chart>

			<table type="vertical" visible="$P{P_SHOWTABLES}">
				<header>
					<line>
						<cell value="$PAggregate" align="center"/>
						<cell value="$PSerie" align="center" split="true"/>
						<cell value="String('Volume')" align="center"/>
						<cell value="String('Good Volume')" align="center"/>
						<cell value="String('Yield')" align="center"/>
					</line>
				</header>
				<row>
					<line>
						<cell value="$FAggregate" align="center"  repeatedvalue="false"/>
						<cell value="$FSerie" align="center" split="true"/>
						<cell value="$P{P_VOLUME}" align="right"/>
						<cell value="$FGoodVolume" align="right"/>
						<cell value="(($P{P_VOLUME} == 0) ? 'n/a' : Number($P{P_YIELD}).toFixed(2) + String(' %'))" align="right"/>
					</line>
				</row>
			</table>
		</group>
		
		<summary><![CDATA[Notes :]]></summary>
		<summary><![CDATA[ - Total parts measures have been computed using consolidated data.]]></summary>
		<summary><![CDATA[ - The Yield measure is computed from the total number of good parts and the total $P{P_SUMMARY}.]]></summary>

	</report>

</galaxy_template>
