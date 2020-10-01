<galaxy_template version="0.7">
	<section_home_page HomeText="" HomeLogo="" />

	<report name="Products Bin Details">

		<datasets>
			<dataset ID="DS1" export="true" WithOverall="false" >
			</dataset>
		</datasets>
		
		<title><![CDATA[$P{P_TITLE}]]></title>
		
		<group dataset="DS1" Split="Product ID,DS1:SPLIT" Aggregate="DS1:AGGREGATE" Serie="DS1:SERIE" breakpage="$P{P_BREAKPAGE}">
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
						<cell value="$P{P_VOLUME}" align="right"/>
						<cell value="$P{P_VOLUME}" align="right"/>
						<cell value="$P{P_VOLUME}" align="right"/>
						<cell value="$P{P_VOLUME}" align="right"/>
						<cell value="$P{P_VOLUME}" align="right"/>
						<cell value="$P{P_VOLUME}" align="right"/>
						<cell value="$P{P_VOLUME}" align="right"/>
						<cell value="$P{P_VOLUME}" align="right"/>
						<cell value="$P{P_VOLUME}" align="right"/>
						<cell value="$P{P_VOLUME}" align="right"/>
						<cell value="$P{P_VOLUME}" align="right"/>
						<cell value="$P{P_VOLUME}" align="right"/>
						<cell value="$P{P_SUM_VOLUME}" align="right"/>
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
						<cell value="Number($P{P_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_SUM_YIELD}).toFixed(2) + String(' %')" align="right"/>
					</line>
				</row>
				<row>
					<line>
						<cell value="$FSerieBinName + '(' + $FSerieBinNo + ')'" align="right"/>
						<cell value="$FBinVolume" align="right"/>
						<cell value="$FBinVolume" align="right"/>
						<cell value="$FBinVolume" align="right"/>
						<cell value="$FBinVolume" align="right"/>
						<cell value="$FBinVolume" align="right"/>
						<cell value="$FBinVolume" align="right"/>
						<cell value="$FBinVolume" align="right"/>
						<cell value="$FBinVolume" align="right"/>
						<cell value="$FBinVolume" align="right"/>
						<cell value="$FBinVolume" align="right"/>
						<cell value="$FBinVolume" align="right"/>
						<cell value="$FBinVolume" align="right"/>
						<cell value="$FSumBinVolume" align="right"/>
					</line>
					<line>
						<cell value="String('')" align="right"/>
						<cell value="Number($P{P_BIN_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_BIN_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_BIN_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_BIN_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_BIN_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_BIN_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_BIN_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_BIN_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_BIN_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_BIN_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_BIN_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_BIN_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_SUM_BIN_YIELD}).toFixed(2) + String(' %')" align="right"/>
					</line>
				</row>
			</table>
		</group>
		
		<group dataset="DS1" name="'Summary Total Report'" Aggregate="DS1:AGGREGATE" Serie="DS1:SPLIT,DS1:SERIE" Order="DS1:SPLIT,DS1:SERIE">
			<table type="custom">
				<cols>
					<col aggregate=""/> 
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
						<cell value="String('Bin#')"  align="center"/>
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
						<cell value="$FSerie.slice(0,$FSerie.lastIndexOf(',')).replace(',', ' - ')" align="left" repeatedvalue="false"/>
						<cell value="$FSerie.slice($FSerie.lastIndexOf(',')+1)" align="center"/>
						<cell value="'IN'" align="center"/>
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
						<cell value="''" align="center"/>
						<cell value="'OUT'" align="center"/>
						<cell value="$P{P_SERIE_BIN_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_BIN_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_BIN_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_BIN_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_BIN_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_BIN_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_BIN_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_BIN_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_BIN_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_BIN_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_BIN_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_BIN_VOLUME}" align="right"/>
						<cell value="$P{P_SERIE_SUM_BIN_VOLUME}" align="right"/>
					</line>
					<line>
						<cell value="String('')" align="left"/>
						<cell value="String('')" align="left"/>
						<cell value="'YLD'" align="center"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_SERIE_YIELD}).toFixed(2) + String(' %')" align="right"/>
						<cell value="Number($P{P_SERIE_SUM_YIELD}).toFixed(2) + String(' %')" align="right"/>
					</line>
				</row>
			</table>
		</group>

		<summary><![CDATA[Notes : IN is the total $P{P_SUMMARY} of the month. OUT is the total good parts (all Pass bins). For each binning, the % is computed from the total $P{P_SUMMARY} of the month.]]></summary>
		
	</report>
</galaxy_template>
