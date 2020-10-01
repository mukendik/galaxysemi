<galaxy_template version="0.7">
	<section_home_page HomeText="" HomeLogo="" />

	<report name="AZ Yield Details">

		<datasets>
			<dataset ID="DS1" export="true" WithOverall="$P{P_OVERALL}">
			</dataset>
		</datasets>
		
		<title><![CDATA[$P{P_TITLE}]]></title>
		
		<group dataset="DS1" Split="DS1:SPLIT" Aggregate="DS1:AGGREGATE" Serie="DS1:SERIE" Order="$PSerie|desc">
			<table type="custom">
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
						<cell value="''"  align="center"/>
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
						<cell value="$FSerie" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FSumVolume" align="right"/>
					</line>
					<line>
						<cell value="String('')" align="right"/>
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
						<cell value="String('')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FSumYield).toFixed(2) + String(' %')" align="right"/>
					</line>
				</row>
			</table>
		</group>
		
		<group dataset="DS1" name="'Summary Section'" Aggregate="DS1:AGGREGATE" Serie="DS1:SPLIT,DS1:SERIE" Order="$PSerie|desc">
			<table type="custom">
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
						<cell value="''"  align="center"/>
						<cell value="''"  align="center"/>
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
						<cell value="($FSerie.lastIndexOf(',') != -1) ? $FSerie.slice(0,$FSerie.lastIndexOf(',')).replace(',', ' - ') : String('')" align="left" repeatedvalue="false"/>
						<cell value="$FSerie.slice($FSerie.lastIndexOf(',')+1)" align="center"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FSumVolume" align="right"/>
					</line>
					<line>
						<cell value="String('')" align="right"/>
						<cell value="String('')" align="right"/>
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
						<cell value="String('')" align="right"/>
						<cell value="String('')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FSumYield).toFixed(2) + String(' %')" align="right"/>
					</line>
				</row>
			</table>
		</group>
		
		<group dataset="DS1" name="'Roll Up'" Aggregate="DS1:AGGREGATE" Serie="DS1:SERIE" Order="$PSerie|desc">
			<table type="custom">
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
						<cell value="''"  align="center"/>
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
						<cell value="$FSerie" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FVolume" align="right"/>
						<cell value="$FSumVolume" align="right"/>
					</line>
					<line>
						<cell value="String('')" align="right"/>
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
						<cell value="String('')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FYield).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($FSumYield).toFixed(2) + String(' %')" align="right"/>
					</line>
				</row>
			</table>
		</group>

		<summary><![CDATA[Notes : IN is the total $P{P_SUMMARY} of the month. OUT is the total good parts (all Pass bins). For each binning, the % is computed from the total $P{P_SUMMARY} of the month.]]></summary>
		
	</report>
</galaxy_template>
