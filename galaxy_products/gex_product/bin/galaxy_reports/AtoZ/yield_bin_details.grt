	<galaxy_template version="0.7">
	<section_home_page HomeText="" HomeLogo="" />

	<report name="AZ Yield Bin Details">

		<datasets>
			<dataset ID="DS1" export="true" WithOverall="$P{P_OVERALL}">
			</dataset>
		</datasets>
		
		<title><![CDATA[$P{P_TITLE}]]></title>
		
		<group dataset="DS1" Split="DS1:SPLIT" Aggregate="DS1:AGGREGATE" Serie="DS1:SERIE,DS1:SUBSERIE" Order="Production Stage|desc,bin_cat|desc,bin_no" breakpage="5">
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
				<body sequencecount="2">
					<row sequence="1" printfirstonly="true" printwhen="(($FSerie.slice(0, $FSerie.indexOf(',')) == 'WT') ? ($FSerieBinNo == 'all') : false)">
						<line>
							<cell value="'WT'" align="right"/>
							<cell value="'IN'" align="right"/>
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
							<cell value="String('OUT')" align="right"/>
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
							<cell value="String('YLD')" align="right"/>
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
					<row sequence="1" printfirstonly="true" printwhen="(($FSerie.slice(0, $FSerie.indexOf(',')) == 'FT') ? ($FSerieBinNo == 'all') : false)">
						<line>
							<cell value="'FT'" align="right"/>
							<cell value="String('IN')" align="right"/>
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
							<cell value="String('OUT')" align="right"/>
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
							<cell value="String('YLD')" align="right"/>
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
					<row sequence="1" printfirstonly="true" printwhen="(($FSerie.slice(0, $FSerie.indexOf(',')) == 'AZ') ? ($FSerieBinNo == 'all') : false)">
						<line>
							<cell value="'AZ'" align="right"/>
							<cell value="String('IN')" align="right"/>
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
							<cell value="String('OUT')" align="right"/>
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
							<cell value="String('YLD')" align="right"/>
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
					<row  sequence="2" printwhen="(($FSerie.slice(0, $FSerie.indexOf(',')) != 'AZ') ? ($FSerieBinNo != 'all') : false)">
						<line>
							<cell value="$FSerie.slice(0, $FSerie.lastIndexOf(','))" align="right" repeatedvalue="false"/>
							<cell value="$FSerieBinName + ' (' + $FSerieBinNo + ')'" align="right"/>
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
							<cell value="''" align="right"/>
							<cell value="Number($FBinYield).toFixed(2) + String(' %')" align="right"/>
							<cell value="Number($FBinYield).toFixed(2) + String(' %')" align="right"/>
							<cell value="Number($FBinYield).toFixed(2) + String(' %')" align="right"/>
							<cell value="Number($FBinYield).toFixed(2) + String(' %')" align="right"/>
							<cell value="Number($FBinYield).toFixed(2) + String(' %')" align="right"/>
							<cell value="Number($FBinYield).toFixed(2) + String(' %')" align="right"/>
							<cell value="Number($FBinYield).toFixed(2) + String(' %')" align="right"/>
							<cell value="Number($FBinYield).toFixed(2) + String(' %')" align="right"/>
							<cell value="Number($FBinYield).toFixed(2) + String(' %')" align="right"/>
							<cell value="Number($FBinYield).toFixed(2) + String(' %')" align="right"/>
							<cell value="Number($FBinYield).toFixed(2) + String(' %')" align="right"/>
							<cell value="Number($FBinYield).toFixed(2) + String(' %')" align="right"/>
							<cell value="Number($FSumBinYield).toFixed(2) + String(' %')" align="right"/>
						</line>
					</row>
				</body>
			</table>
		</group>

		<summary><![CDATA[Notes : IN is the total $P{P_SUMMARY} of the month. OUT is the total good parts (all Pass bins). For each binning, the % is computed from the total $P{P_SUMMARY} of the month.]]></summary>
		
	</report>
</galaxy_template>
