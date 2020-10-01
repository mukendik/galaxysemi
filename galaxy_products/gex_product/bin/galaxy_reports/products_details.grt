<galaxy_template version="0.7">
	<section_home_page HomeText="" HomeLogo="" />

	<report name="Products Details">

		<datasets>
			<dataset ID="DS1" export="true" WithOverall="true">
			</dataset>
		</datasets>
		
		<title><![CDATA[$P{P_TITLE}]]></title>
		
		<settings>
			<custom>
				<label>Yield Threshold :</label>
				<value><![CDATA[($P{P_YIELD_THRESHOLD} == 0) ? 'none' : '$P{P_YIELD_THRESHOLD} %']]></value>
			</custom>
		</settings>
		
		<group dataset="DS1" Split="Product ID,DS1:SPLIT" Aggregate="DS1:AGGREGATE" breakpage="$P{P_BREAKPAGE}" printwhen="$ELowYieldProduct == true || $P{P_YIELD_THRESHOLD}==0">
			<exception name="LowYieldProduct">
				<![CDATA[$P{P_GROUP_VOLUME} > 0 && $P{P_GROUP_YIELD} < $P{P_YIELD_THRESHOLD}]]>
			</exception>
			
			<table type="custom">
				<alarms>
					<condition name="YieldThresholdByMonth" bkcolor="$P{P_YIELD_THRESHOLD_CLR}">
						<![CDATA[$P{P_GROUP_VOLUME} > 0 && $P{P_GROUP_YIELD} < $P{P_YIELD_THRESHOLD}]]>
					</condition>
				</alarms>
							
				<cols>
					<col aggregate=""/> 
					<col aggregate="1"/> 
					<col aggregate="2"/>
					<col aggregate="3"/>
					<col aggregate="4"/>
					<col aggregate="5"/>
					<col aggregate="6"/>
					<col aggregate="7"/>
					<col aggregate="8"/>
					<col aggregate="9"/>
					<col aggregate="10"/>
					<col aggregate="11"/>
					<col aggregate="12"/>
					<col aggregate=""/>
				</cols>
				<header>
					<line>
						<cell value="String('')"  align="center"/>
						<cell value="String('Jan')" align="center"/>
						<cell value="String('Feb')" align="center"/>
						<cell value="String('Mar')" align="center"/>
						<cell value="String('Apr')" align="center"/>
						<cell value="String('May')" align="center"/>
						<cell value="String('Jun')" align="center"/>
						<cell value="String('Jul')" align="center"/>
						<cell value="String('Aug')" align="center"/>
						<cell value="String('Sep')" align="center"/>
						<cell value="String('Oct')" align="center"/>
						<cell value="String('Nov')" align="center"/>
						<cell value="String('Dec')" align="center"/>
						<cell value="String('YTD')" align="center"/>
					</line>
				</header>
				<row printfirstonly="true">
					<line>
						<cell value="String('IN')" align="right"/>
						<cell value="$P{P_GROUP_VOLUME}" align="right"/>
						<cell value="$P{P_GROUP_VOLUME}" align="right"/>
						<cell value="$P{P_GROUP_VOLUME}" align="right"/>
						<cell value="$P{P_GROUP_VOLUME}" align="right"/>
						<cell value="$P{P_GROUP_VOLUME}" align="right"/>
						<cell value="$P{P_GROUP_VOLUME}" align="right"/>
						<cell value="$P{P_GROUP_VOLUME}" align="right"/>
						<cell value="$P{P_GROUP_VOLUME}" align="right"/>
						<cell value="$P{P_GROUP_VOLUME}" align="right"/>
						<cell value="$P{P_GROUP_VOLUME}" align="right"/>
						<cell value="$P{P_GROUP_VOLUME}" align="right"/>
						<cell value="$P{P_GROUP_VOLUME}" align="right"/>
						<cell value="$P{P_GROUP_SUM_VOLUME}" align="right"/>
					</line>
					<line>
						<cell value="String('OUT')" align="right"/>
						<cell value="$FGroupGoodVolume" align="right"/>
						<cell value="$FGroupGoodVolume" align="right"/>
						<cell value="$FGroupGoodVolume" align="right"/>
						<cell value="$FGroupGoodVolume" align="right"/>
						<cell value="$FGroupGoodVolume" align="right"/>
						<cell value="$FGroupGoodVolume" align="right"/>
						<cell value="$FGroupGoodVolume" align="right"/>
						<cell value="$FGroupGoodVolume" align="right"/>
						<cell value="$FGroupGoodVolume" align="right"/>
						<cell value="$FGroupGoodVolume" align="right"/>
						<cell value="$FGroupGoodVolume" align="right"/>
						<cell value="$FGroupGoodVolume" align="right"/>
						<cell value="$FGroupSumGoodVolume" align="right"/>
					</line>
					<line>
						<cell value="String('Yield')" align="right"/>
						<cell value="Number($P{P_GROUP_YIELD}).toFixed(2) + String(' %')" align="right" alarm="YieldThresholdByMonth"/>
						<cell value="Number($P{P_GROUP_YIELD}).toFixed(2) + String(' %')" align="right" alarm="YieldThresholdByMonth"/>
						<cell value="Number($P{P_GROUP_YIELD}).toFixed(2) + String(' %')" align="right" alarm="YieldThresholdByMonth"/>
						<cell value="Number($P{P_GROUP_YIELD}).toFixed(2) + String(' %')" align="right" alarm="YieldThresholdByMonth"/>
						<cell value="Number($P{P_GROUP_YIELD}).toFixed(2) + String(' %')" align="right" alarm="YieldThresholdByMonth"/>
						<cell value="Number($P{P_GROUP_YIELD}).toFixed(2) + String(' %')" align="right" alarm="YieldThresholdByMonth"/>
						<cell value="Number($P{P_GROUP_YIELD}).toFixed(2) + String(' %')" align="right" alarm="YieldThresholdByMonth"/>
						<cell value="Number($P{P_GROUP_YIELD}).toFixed(2) + String(' %')" align="right" alarm="YieldThresholdByMonth"/>
						<cell value="Number($P{P_GROUP_YIELD}).toFixed(2) + String(' %')" align="right" alarm="YieldThresholdByMonth"/>
						<cell value="Number($P{P_GROUP_YIELD}).toFixed(2) + String(' %')" align="right" alarm="YieldThresholdByMonth"/>
						<cell value="Number($P{P_GROUP_YIELD}).toFixed(2) + String(' %')" align="right" alarm="YieldThresholdByMonth"/>	
						<cell value="Number($P{P_GROUP_YIELD}).toFixed(2) + String(' %')" align="right" alarm="YieldThresholdByMonth"/>
						<cell value="Number($P{P_GROUP_SUM_YIELD}).toFixed(2) + String(' %')" align="right"/>
					</line>
				</row>
			</table>
		</group>
		
		<group dataset="DS1" name="'Summary Total Report'" Aggregate="DS1:AGGREGATE" Serie="DS1:SPLIT" Order="$PSerie|asc">
			<table type="custom">
				<alarms>
					<condition name="SummaryYieldThresholdByMonth" bkcolor="$P{P_YIELD_THRESHOLD_CLR}">
						<![CDATA[$P{P_SERIE_VOLUME} > 0 && '$P{P_YIELD_THRESHOLD}' != 'none' && $P{P_SERIE_YIELD} < $P{P_YIELD_THRESHOLD}]]>
					</condition>
				</alarms>
				<cols>
					<col aggregate=""/> 
					<col aggregate=""/> 
					<col aggregate="1"/> 
					<col aggregate="2"/>
					<col aggregate="3"/>
					<col aggregate="4"/>
					<col aggregate="5"/>
					<col aggregate="6"/>
					<col aggregate="7"/>
					<col aggregate="8"/>
					<col aggregate="9"/>
					<col aggregate="10"/>
					<col aggregate="11"/>
					<col aggregate="12"/>
					<col aggregate=""/>
				</cols>
				<header>
					<line>
						<cell value="String('')"  align="center"/>
						<cell value="String('')"  align="center"/>
						<cell value="String('Jan')" align="center"/>
						<cell value="String('Feb')" align="center"/>
						<cell value="String('Mar')" align="center"/>
						<cell value="String('Apr')" align="center"/>
						<cell value="String('May')" align="center"/>
						<cell value="String('Jun')" align="center"/>
						<cell value="String('Jul')" align="center"/>
						<cell value="String('Aug')" align="center"/>
						<cell value="String('Sep')" align="center"/>
						<cell value="String('Oct')" align="center"/>
						<cell value="String('Nov')" align="center"/>
						<cell value="String('Dec')" align="center"/>
						<cell value="String('YTD')" align="center"/>
					</line>
				</header>
				<row>
					<line>
						<cell value="$FSerie.replace(',', ' - ')" align="left"/>
						<cell value="'IN'" align="right"/>
						<cell value="$P{P_SERIE_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_SUM_VOLUME}" align="right"/>
					</line>
					<line>
						<cell value="''" align="left"/>
						<cell value="'OUT'" align="right"/>
						<cell value="$FGoodVolume" align="right"/>
						<cell value="$FGoodVolume" align="right"/>
						<cell value="$FGoodVolume" align="right"/>
						<cell value="$FGoodVolume" align="right"/>
						<cell value="$FGoodVolume" align="right"/>
						<cell value="$FGoodVolume" align="right"/>
						<cell value="$FGoodVolume" align="right"/>
						<cell value="$FGoodVolume" align="right"/>
						<cell value="$FGoodVolume" align="right"/>
						<cell value="$FGoodVolume" align="right"/>
						<cell value="$FGoodVolume" align="right"/>
						<cell value="$FGoodVolume" align="right"/>
						<cell value="$FSumGoodVolume" align="right"/>
					</line>
					<line>
						<cell value="''" align="left"/>
						<cell value="'YLD'" align="right"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right" alarm="SummaryYieldThresholdByMonth"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right" alarm="SummaryYieldThresholdByMonth"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right" alarm="SummaryYieldThresholdByMonth"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right" alarm="SummaryYieldThresholdByMonth"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right" alarm="SummaryYieldThresholdByMonth"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right" alarm="SummaryYieldThresholdByMonth"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right" alarm="SummaryYieldThresholdByMonth"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right" alarm="SummaryYieldThresholdByMonth"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right" alarm="SummaryYieldThresholdByMonth"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right" alarm="SummaryYieldThresholdByMonth"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right" alarm="SummaryYieldThresholdByMonth"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right" alarm="SummaryYieldThresholdByMonth"/>
						<cell value="Number($P{P_SERIE_SUM_YIELD}).toFixed(2) + String(' %')" align="right"/>
					</line>
				</row>
			</table>
		</group>

		<summary><![CDATA[Notes :<br> - Total parts measures have been computed using consolidated data.<br> - The Yield measure is computed from the total number of good parts and the total $P{P_SUMMARY}.]]></summary>
		
	</report>
</galaxy_template>
