// -------------------------------------------------------------------------- //
// main
// -------------------------------------------------------------------------- //
function BinPrecedence(FromMap, FromSTDF)
{
	var MergedBin = new JSBinDescription();
	
	if (FromMap.GetNumber() == -1)
	{
		if (FromSTDF.GetNumber() == -1)
		{
			MergedBin.SetNumber(-1);
		}
	}
	else if (FromMap.GetCategory() == 'P')
	{
		if (FromSTDF.GetNumber() != -1)
			MergedBin = FromSTDF;
		else
		{
			// Untested die in STDF map
			// Keep the prober one
			MergedBin = FromMap;
		}
	}
	else
	{
		// Pass die in STDF map
		// Keep the prober one
		MergedBin = FromMap;
	
	}
	
	FromMap.SetName("Updated through JS");
	
	return MergedBin;
}

function MergeMap(ExternalMap, STDFMap)
{
	var MergedMap = new JSWaferMap();
	
	var MinX = (ExternalMap.GetLowX() < STDFMap.GetLowX()) ? ExternalMap.GetLowX() : STDFMap.GetLowX();
	var MinY = (ExternalMap.GetLowY() < STDFMap.GetLowY()) ? ExternalMap.GetLowY() : STDFMap.GetLowY();
	var MaxX = (ExternalMap.GetHighX() > STDFMap.GetHighX()) ? ExternalMap.GetHighX() : STDFMap.GetHighX();
	var MaxY = (ExternalMap.GetHighY() > STDFMap.GetHighY()) ? ExternalMap.GetHighY() : STDFMap.GetHighY();
	var binMerged;
	
	MergedMap.Create(MinX, MinY, MaxX, MaxY);
	
	for (indexX = MergedMap.GetLowX(); indexX < MergedMap.GetHighX(); indexX++) 
	{
		for (indexY = MergedMap.GetLowY(); indexY < MergedMap.GetHighY(); indexY++)
		{
			var binExt 	= ExternalMap.GetBinAt(indexX, indexY);
			var binSTDF	= STDFMap.GetBinAt(indexX, indexY);
			
			if (binExt == -1)
			{
				if (binSTDF == -1)
					binMerged = -1;
				else 
					binMerged = binSTDF;
			}
			else if (binExt == 1)
			{
				if (binSTDF != -1)
					binMerged = binSTDF;
				else
					binMerged = binExt;
			}
			else
				binMerged = binExt;
				
			MergedMap.SetBinAt(binMerged, indexX, indexY);
		}
	}
	
	return MergedMap;

}