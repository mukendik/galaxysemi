// FromMaps is an Array of JSBinDescription Object
function MergeBinRuleFirstFailBin(FromMaps)
{
    // Merged bin returned
    var MergedBin = new GSBinInfo();

    // loop over all dies
    for (i = 0; i < FromMaps.length; i++)
    {
        // once a die is fail, use this bin as it is the first fail bin
        if (FromMaps[i].GetNumber() >= 0 && FromMaps[i].GetCategory() == 'F')
        {
            MergedBin = FromMaps[i];
            break;
        }
        else if (i === 0)
        {
            MergedBin = FromMaps[i];
        }
    }

    return MergedBin;
}

// FromMaps is an Array of JSBinDescription Object
function MergeBinRuleHighestBin(FromMaps)
{
    // Merged bin returned
    var MergedBin = new GSBinInfo();

    // loop over all dies
    for (i = 0; i < FromMaps.length; i++)
    {
        if (i === 0)
        {
            MergedBin = FromMaps[i];
        }
        // Use the highest bin found
        else if (FromMaps[i].GetNumber() > MergedBin.GetNumber())
        {
            MergedBin = FromMaps[i];
        }
    }

    return MergedBin;
}

// FromMaps is an Array of JSBinDescription Object
function MergeMapBinRule(FromMaps)
{
    var MergedBin = new GSBinInfo;
    
    if (FromMaps.length != 2)
    	throw 'Error: MergeMapBinRule hook only supports 2 maps';
    	
    var STDFBin = FromMaps[0];
    var ExternalBin = FromMaps[1];

    // External die is unstested 
    if (ExternalBin.GetNumber() == -1)
    {
        // STDF die is tested, use it
        if (STDFBin.GetNumber() >= 0)
        {
            MergedBin = STDFBin;
        }
    }
    // External die is Pass
    else if (ExternalBin.GetCategory() == 'P')
    {
        // If STDF die is pass, use it 
        if (STDFBin.GetCategory() == 'P')
        {
            MergedBin = STDFBin;
        }
        else
        {
            // Untested or Failed die in STDF
            // Keep the prober one
            MergedBin = ExternalBin;
        }
    }
    // External die is failed
    else
    {
        // Keep the prober one
        MergedBin = ExternalBin;
    }
    
    return MergedBin;
}