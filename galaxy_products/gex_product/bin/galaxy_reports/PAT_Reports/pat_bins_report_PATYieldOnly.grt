<galaxy_template version="0.7">
	<report name="PAT Bins Report Extended">

		<datasets>
			<dataset ID="DS1" export="true" WithOverall="false">
			</dataset>
		</datasets>
		
		<title><![CDATA[$P{P_TITLE}]]></title>
		
		<group dataset="DS1" Split="DS1:SPLIT" Aggregate="hbin#" Serie="DS1:SERIE" Order="DS1:ORDER">
			<table type="custom">
				<cols>
					<col aggregate=""/> 
					<col aggregate=""/> 
					<!-- <col aggregate=""/> -->
					<!-- <col aggregate=""/> -->
					<col aggregate=""/> 
					<col aggregate="80"/>
					<col aggregate="81"/>
					<col aggregate="87"/>
				</cols>
				<header>
					<line>
						<cell value="$PSerie.replace(',',' - ')" align="center"/>
						<cell value="String('Wafer')" 		align="center"/>
						<!-- <cell value="String('Yield')" 		align="center"/> -->
						<!-- <cell value="String('Yield')" 		align="center"/> -->
						<cell value="String('PAT')" 		align="center"/>
						<cell value="String('GPAT')" 		align="center"/>
						<cell value="String('PPAT')" 		align="center"/>
						<cell value="String('ZPAT')" 		align="center"/>s
					</line>
					<line>
						<cell value="String('')" align="center"/>
						<cell value="String('volume')" align="center"/>
						<!-- <cell value="String('before PAT')" align="center"/> -->
						<!-- <cell value="String('after PAT')" align="center"/> -->
						<cell value="String('Yield loss')" align="center"/>
						<cell value="String('')" align="center"/>
						<cell value="String('')" align="center"/>
						<cell value="String('')" align="center"/>
					</line>
				</header>
				<row>
					<line>
						<cell value="$FSerie.replace(',',' - ')" 		align="center"/>
						<cell value="$FSumWaferCount" 	align="center"/>
						<!-- <cell value="Number($FSumYield+$FSumBinYield).toFixed(2) + String(' %')" 		align="center"/> -->
						<!-- <cell value="Number($FSumYield).toFixed(2) + String(' %')" 	align="center"/> --> 
						<cell value="Number($FSumBinYield).toFixed(2) + String(' %')" 	align="center"/>
						<cell value="Number($FBinYield).toFixed(2) + String(' %')" 	align="center"/>
						<cell value="Number($FBinYield).toFixed(2) + String(' %')" 	align="center"/>
						<cell value="Number($FBinYield).toFixed(2) + String(' %')" 	align="center"/>
					</line>
					<line>
						<cell value="String('')" 	align="center"/>
						<cell value="String('')" 	align="center"/>
						<!-- <cell value="'(' + Number($FSumGoodVolume+$FSumBinVolume) + ')'" 	align="center"/> -->
						<!-- <cell value="'(' + $FSumGoodVolume + ')'" 	align="center"/>  -->
						<cell value="'(' + $FSumBinVolume + ')'" 	align="center"/>
						<cell value="'(' + $FBinVolume + ')'" 	align="center"/>
						<cell value="'(' + $FBinVolume + ')'" 	align="center"/>
						<cell value="'(' + $FBinVolume + ')'" 	align="center"/>
					</line>
				</row>
				<footer>
					<line>
						<cell value="String('Summary')" 	align="left"/>
						<cell value="$FGroupSumWaferCount" 	align="center"/>
						<!-- <cell value="Number($FGroupSumYield+$FGroupSumBinYield).toFixed(2) + String(' %')" 		align="center"/> -->
						<!-- <cell value="Number($FGroupSumYield).toFixed(2) + String(' %')" 		align="center"/> -->
						<cell value="Number($FGroupSumBinYield).toFixed(2) + String(' %')" 	align="center"/>
						<cell value="Number($FGroupBinYield).toFixed(2) + String(' %')" 	align="center"/>
						<cell value="Number($FGroupBinYield).toFixed(2) + String(' %')" 	align="center"/>
						<cell value="Number($FGroupBinYield).toFixed(2) + String(' %')" 	align="center"/>
					</line>
					<line>
						<cell value="String('')" 	align="center"/>
						<cell value="String('')" 	align="center"/>
						<!-- <cell value="'(' + Number($FGroupSumGoodVolume+$FGroupSumBinVolume) + ')'" 		align="center"/>  -->
						<!-- <cell value="'(' + $FGroupSumGoodVolume + ')'" 	align="center"/> -->
						<cell value="'(' + $FGroupSumBinVolume + ')'" 	align="center"/>
						<cell value="'(' + $FGroupBinVolume + ')'" 	align="center"/>
						<cell value="'(' + $FGroupBinVolume + ')'" 	align="center"/>
						<cell value="'(' + $FGroupBinVolume + ')'" 	align="center"/>
					</line>
				</footer>
			</table>
		</group>
		
	</report>
</galaxy_template>
