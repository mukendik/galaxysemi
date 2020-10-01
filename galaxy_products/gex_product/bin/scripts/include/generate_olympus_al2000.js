function getYYYYMMDDhhmmss(date)
{
    var str = new String();
    str += date.getFullYear();
    str +=
        (date.getMonth() < 10 ? "0" +
         (date.getMonth() + 1) : date.getMonth() + 1);
    str += (date.getDay() < 10 ? "0" + date.getDay() : date.getDay());
    str += (date.getUTCHours() < 10 ?
            '0' + date.getUTCHours() : date.getUTCHours());
    str +=
        (date.getMinutes() < 10 ? '0' + date.getMinutes() : date.getMinutes());
    str +=
        (date.getSeconds() < 10 ? '0' + date.getSeconds() : date.getSeconds());
    return str;
}

function generate_olympus_al2000(pp, output_path)
{
    if ((typeof pp) != 'object')
    {
        return 'error: PatProcessing not an object';
    }
    var wm = pp.GetWaferMap("soft");
    if (typeof wm != 'object')
    {
        return 'error: get a null wafermap from patprocessing';
    }

    // Let s execute any JS DTR if any before continuing
    if ((typeof pp.GalaxySemiJSDTR) != 'undefined')
        // could be either a string or an object
    {
        var JSDTR = new String(pp.GalaxySemiJSDTR);
        JSDTR = JSDTR.replace(/CurrentGSPATProcessing/g, "pp");
        // JSDTR = JSDTR.replace("CurrentGSPATProcessing", "pp");
		// GCORE-2133
		JSDTR = JSDTR.replace("CENTER_DISTANCE_X", "OutputDistanceX");
		JSDTR = JSDTR.replace("CENTER_DISTANCE_Y", "OutputDistanceY");
        JSDTR = JSDTR.replace("OUTPUT_CENTERCHIP_X", "OutputCenterX");
        JSDTR = JSDTR.replace("OUTPUT_CENTERCHIP_Y", "OutputCenterY");
        print("Evaluating JS DTR: " + JSDTR);
        eval(JSDTR);
    }

    // datasamples is not standard
    print("WaferNotch:" + wm.GetWaferNotch());  // 6 for datasamples
    print("Input center from stdf: " + wm.GetCenterX() + ":" + wm.GetCenterY());
    // 128:128 in datasamples....
    print("Input Bounds: from " + wm.GetLowDieX() + ":" + wm.GetLowDieY() +
          " to " + wm.GetHighDieX() + ":" + wm.GetHighDieY());
    // datasamples: 4:-45 to 45:-3

    var Xori = wm.GetPosXDirection();
    print("WaferMap X orientation: " + Xori);
    var Yori = wm.GetPosYDirection();
    print("WaferMap Y orientation: " + Yori);

    // Recipe
    if ((typeof pp.Get('Recipe')) != 'string')
    {
        return 'error: unknown recipe type: ' + (typeof pp.Get('Recipe'));
    }
    var RecipeString = GSFile.ReadAsText(pp.Get('Recipe'));
    if (RecipeString.substr(0, 5) == 'error')
    {
        return 'error: cannot read recipe file: ' + RecipeString;
    }
    var recipe = eval("(" + RecipeString + ")");
    if (typeof recipe != 'object')
    {
        return "error: cannot evaluate JSON recipe: " + pp.Get('Recipe');
    }
    print(
        "Recipe dpat bin:" +
        recipe.outlier_options.settings.dynamic_pat.soft_bin);
    print(
        "Recipe spat bin:" +
        recipe.outlier_options.settings.static_pat.soft_bin);

    // Lets compute CHIP_MATRIX
    print('OutputCenterX:' + pp.Get('OutputCenterX'));
    if (GSProductInfo.isExaminatorPAT() &&
        (typeof pp.Get('OutputCenterX') == 'undefined'))
    {
        pp.Set('OutputCenterX', 50); pp.Set('OutputCenterY', 50);
        // According to Seth : "OutputCenterChip - If not in stdf,
        // Use the same logic as for PAT-Man."
    }
    var OutputWidth = 250, OutputHeight = 250;
    var OUTPUT_CENTERCHIP = pp.Get('OutputCenterX');
    // was 'OUTPUT_CENTERCHIP');
    if (OUTPUT_CENTERCHIP == 50)
    {
        OutputWidth = 250; OutputHeight = 250;
    }
    else if (OUTPUT_CENTERCHIP == 125 &&
             (wm.GetHighDieX() < 250 || wm.GetHighDieY() < 250))
    {
        OutputWidth = 250; OutputHeight = 250;
    }
    else if (OUTPUT_CENTERCHIP == 125 &&
             (wm.GetHighDieX() >= 250 || wm.GetHighDieY() >= 250))
    {
        OutputWidth = 700; OutputHeight = 700;
    }
    else if (OUTPUT_CENTERCHIP == 350)
    {
        OutputWidth = 700; OutputHeight = 700;
    }
    else
    {
        return "error: cannot guess output center : " + pp.Get('OutputCenterX');
    }

    print("Output map size will be " + OutputWidth + ":" + OutputHeight);
    // Let's allocate an array to store outputmap
    var map = new Array();
    // datasamples wafermap is 43 x 42 but coord ranges from
    // 0-50 on X and 0 to -50 on Y
    // Let's init a full blank map : 0 = no die
    for (var x = 0; x < OutputWidth; x++)
    {
        map[x] = new Array;
        for (var y = 0; y < OutputHeight; y++)
        {
            map[x][y] = 0;
        }
    }
    var olympus_map = new String("");

    if (GSProductInfo.isExaminatorPAT())
    {
        olympus_map += "<HEADER>GEX-PAT Output      \r\n";
    }
    else
    {
        olympus_map += "<HEADER>PAT-MAN Output      \r\n";
        //  originally "<HEADER>Map Merge           " + "\r\n";
        // MUST BE 20 characters (filled with spaces)
    }
    // <LOT_DATA> : should be total 99 chars
    // should be about:
    // KZ381      80098    25KZ381                 49KAC     in_inspe21
    olympus_map += "<LOT_DATA>";
    // Lot : 20 chars : exple: "KZ381      80098    "
    if ((typeof pp.Lot) == 'undefined')
    {
        return 'error: Lot undefined';
    }
	//print('Lot: |'+pp.Lot+'|');
    olympus_map += (pp.Lot + new String("                    ")).substr(0, 20);
    // "Count of wafers" : 2 chars
    olympus_map += "  ";  // Filled with space according to specs

    // Product name : 20 chars : exple: "KZ381               "
    // According to the first specs, product had to be
    // the first word of the filename:
    // KZ381-0098.15(minimum)_unfinish.xlsx defined : 1st word<Product> of
    // Input STDF file name
    // var inputfilepath = new String(pp.Get('DataSources'));
    // var inputfilename =
    // inputfilepath.substr(inputfilepath.lastIndexOf('/') + 1,
    //                      inputfilepath.length);
    // var Product=inputfilename.substr(0, inputfilename.indexOf('_'));
    // The new specs are : "first part of MIR.lotid" : exple:
    // Mir.lotid is KZ3982-0105. In this case, D record of <LOT_DATA> is KZ3982
    if (pp.Lot.indexOf('-') == -1)
    {
        return "error: cannot induce Product name from LotID:" + pp.Lot;
    }
    var Product = pp.Lot.substr(0, pp.Lot.indexOf('-'));
    olympus_map += (Product + new String("                    ")).substr(0, 20);

    // Product version :2 chars : could be empty: "  "
    olympus_map += "  ";  //"??"; // todo : check me with Seth
    // Process : 10 chars : exple "49KAC     "
    olympus_map += "          ";
    // or (pp.ProcessID+"          ").substr(0,19);
    // Equipment ID : 15 chars : exple: "in_inspe21     "
    olympus_map += "               ";  // or "EquipmentID    " ?
    // OperatorID : 20 chars : can be empty (spaces) :
    // supposed to be STDF Field:MIR.OPER_NAM
    olympus_map += (pp.Operator + "                    ").substr(0, 20);
    //"OperatorID          ";
    olympus_map += "\r\n";

    // <WAFER_DATA>
    olympus_map += "<WAFER_DATA>";
	// 2132 : let's trim lot
    olympus_map += (pp.Lot.trim() + '.' + pp.Wafer + "                                     ").substr(0, 23);
    olympus_map += pp.Wafer;

    if ((typeof pp.Get("WaferStartTime")) == 'undefined')
    {
        return 'error: WaferStartTime undefined';
    }
    var startDate = new Date(pp.Get("WaferStartTime") * 1000);
    // Wafer start (from WIR)
    olympus_map += getYYYYMMDDhhmmss(startDate);

    if ((typeof pp.Get("WaferFinishTime")) == 'undefined')
    {
        return 'error: WaferFinishTime undefined';
    }
    var endDate = new Date(pp.Get("WaferFinishTime") * 1000);
    // Wafer finish time (from WRR)
    olympus_map += getYYYYMMDDhhmmss(endDate);

    if ((typeof pp.Get("TotalParts")) == 'undefined')
    {
        return "error: undefined TotalParts";
    }
    var TotalParts = new String("     " + pp.Get("TotalParts"));
    olympus_map += TotalParts.substr(TotalParts.length - 5, TotalParts.length);
    // 1587
    // var NumberOfFail =
    // new String("     " + (pp.Get("TotalParts") -
    // pp.Get('TotalGoodAfterPAT')));
    var NumberOfFail = new String("     " + pp.Get("TotalPATFails"));
    // New specs: only PAT fails
    olympus_map += NumberOfFail.substr(NumberOfFail.length - 5,
                                       NumberOfFail.length);
    olympus_map += "\r\n";

    // Let check for centers
    var OutputCenterX = 50, OutputCenterY = 50;
    // Center coord of Seth for datasamples: 24:-23
    // todo : compute/estimate center from wafermap ?
    print("Center estimation from wafermap size: " +
          parseInt((wm.GetHighDieX() -
                    wm.GetLowDieX()) / 2) + ":" +
          parseInt((wm.GetHighDieY() + wm.GetLowDieY()) / 2));
    print("WaferCenterX=" + pp.Get('WaferCenterX'));
    var InputCenterX = pp.Get('WaferCenterX');
    // was INPUT_CENTERCHIP_X'  // 34;
    if (GSProductInfo.isExaminatorPAT() &&
        (typeof pp.Get('WaferCenterX') == 'undefined'))
    {
        InputCenterX = parseInt((wm.GetHighDieX() - wm.GetLowDieX()) / 2);
    }
    if (typeof InputCenterX == 'undefined')
    {
        return "error: undefined input center X. According to input WaferMap: "
               + wm.GetCenterX();
    }
    var InputCenterY = pp.Get('WaferCenterY');
    // was INPUT_CENTERCHIP_Y'  // 34;
    if (GSProductInfo.isExaminatorPAT() &&
        (typeof pp.Get('WaferCenterY') == 'undefined'))
    {
        InputCenterY = parseInt((wm.GetHighDieY() - wm.GetLowDieY()) / 2);
    }

    print("Input center: " + InputCenterX + ":" + InputCenterY);
    // 20:-24 for datasamples

    olympus_map += "<COORDINATE_DATA> 50 50";  // hard coded with 50 50
	
	// GCORE-2133
	if ( (typeof pp.Get("CENTER_DISTANCE_X")) != 'undefined')
	 pp.Set('OutputDistanceX', pp.Get("CENTER_DISTANCE_X"));
	if ( (typeof pp.Get("CENTER_DISTANCE_Y")) != 'undefined')
	 pp.Set('OutputDistanceY', pp.Get("CENTER_DISTANCE_Y"));
	 
    if (GSProductInfo.isExaminatorPAT() && (typeof pp.Get("OutputDistanceX")) ==
        'undefined')
    {
        pp.Set("OutputDistanceX", "  ");
    }
    if ((typeof pp.Get("OutputDistanceX")) == 'undefined')
    {
        return "error: undefined OutputDistanceX";
    }
    var DistX = "       " + pp.Get("OutputDistanceX");
    olympus_map += DistX.substr(DistX.length - 7, DistX.length);

    if (GSProductInfo.isExaminatorPAT() && (typeof pp.Get("OutputDistanceY")) ==
        'undefined')
    {
        pp.Set("OutputDistanceY", "  ");
    }
    if ((typeof pp.Get("OutputDistanceY")) == 'undefined')
    {
        return "error: undefined OutputDistanceY";
    }
    var DistY = "       " + pp.Get("OutputDistanceY");
    olympus_map += DistY.substr(DistY.length - 7, DistY.length);
    olympus_map += "\r\n";

    olympus_map += "<LAYOUT_DATA>";

    print("Input WaferSize:" + pp.WaferSize);
    if (GSProductInfo.isExaminatorPAT() && ((typeof pp.WaferSize) != 'number'))
    {
        pp.WaferSize = 0;
    }
    if ((typeof pp.WaferSize) != 'number')
    {
        return "error: no WaferSize found";
    }

    // WF_UNITS:0(unknown) 1(inches) 2(cm) 3(mm) 4(mils)
    print("WaferUnits:" + pp.WaferUnits);
    if (GSProductInfo.isExaminatorPAT() && pp.WaferUnits != 3)
    {
        pp.WaferUnits = 3;
    }
    if (pp.WaferUnits != 3)
    {
        return "error: unsupported wafer units " + pp.WaferUnits;
    }
    //var WaferSize=new String("  "+pp.Get("WaferSize"));
    // Change of specs: from mm to inch
    var WaferSize = new String("  " + (pp.WaferSize / 25));  // 200mm = 8inch
    olympus_map += WaferSize.substr(WaferSize.length - 2, WaferSize.length);

    // Change of specs: input is supposed to be mm but output must be um
    // QA: Input ChipSizeX:40
    if (GSProductInfo.isExaminatorPAT() && (typeof pp.ChipSizeX) != 'number')
    {
        pp.ChipSizeX = 0;
    }
    if ((typeof pp.ChipSizeX) != 'number')
    {
        return 'error: ChipSizeX unknown: ' + typeof pp.ChipSizeX;
    }
    print("Input ChipSizeX:" + pp.ChipSizeX);
    // in KZ3564HT_KZ3564-0273_T7721D045_13_KZ3564MAIN_1_20140829031247.
    // fixed.std: width: 4700 mm / height: 5110 mm
    var ChipSizeX = new String("      " + (Math.round(pp.ChipSizeX * 1000)));
    // CHIP_SIZE X: 6 characters
    olympus_map += ChipSizeX.substr(ChipSizeX.length - 6, ChipSizeX.length);

    if (GSProductInfo.isExaminatorPAT() && (typeof pp.ChipSizeY) != 'number')
    {
        pp.ChipSizeY = 0;
    }
    if ((typeof pp.ChipSizeY) != 'number')
    {
        return 'error: ChipSizeY unknown: ' + typeof pp.ChipSizeY;
    }
    var ChipSizeY = new String("      " + (Math.round(pp.ChipSizeY * 1000)));
    olympus_map += ChipSizeY.substr(ChipSizeY.length - 6, ChipSizeY.length);

    olympus_map += new String(OutputWidth) + new String(OutputHeight);
    // should be either 250 or 700
    olympus_map += "\r\n";

    olympus_map += "<CATEGORY_DATA>\r\n";

    for (var ix = wm.GetLowDieX(); ix <= wm.GetHighDieX(); ix++)
        // datasamples wafermap is 43 x 42 but coord ranges from
        // 0-50 on X and 0 to -50 on Y
    {
        for (var iy = wm.GetLowDieY(); iy <= wm.GetHighDieY(); iy++)
        {
            var bin = wm.GetLastTestBinValue(ix, iy);
            if (bin == -1)
            {
                continue;  // coord out of wafer, no die
            }
            if (bin == recipe.outlier_options.settings.dynamic_pat.soft_bin ||
                bin == recipe.outlier_options.settings.static_pat.soft_bin)
                // usually 140, 141, ...
            {
                bin = 2;
                // for olympus : OFFWAFER:0, PASS:1, FAIL:2 : PAT=FAIL
            }
            else
            {
                /*
                  if (contains(recipe.outlier_options.settings.general.
                      good_soft_bins, bin.toString()))
                      // good_soft_bins is an array of string
                      bin=1; // pass
                  else
                      bin=2; // fail
                 */
                // change of specs: PAT=2 any other bin=1
                bin = 1;
            }
            var ox = ix - InputCenterX + OutputCenterX;
            ox = ox - 1;
            var oy = iy - InputCenterY + OutputCenterY;
            if (Yori == false)  // Y is increasing when going down
            {
                oy = -iy - (-InputCenterY) + OutputCenterY;
            }
            oy = oy - 1;
            if ((typeof oy) == 'undefined')
            {
                return "error: illegal output y value (" + oy + ") for input " +
                       ix + ":" + iy;
            }
            map[ox][oy] = bin;
            // -1 because olympus coord system starts from 1:1 not 0:0
        }
    }

    for (var y = 0; y < OutputHeight; y++)
    {
        for (var x = 0; x < OutputWidth; x++)
            // datasamples wafermap is 43 x 42 but coord ranges from
            // 0-50 on X and 0 to -50 on Y
        {
            olympus_map += map[x][y];
        }
        olympus_map += "\r\n";
    }
    // 2014 Sept: change of specs: no more starting with pp.Product+'-'+
	// 2014 Nov: GCORE-2131 
	var lTrimedLot=pp.Lot; 
	lTrimedLot=lTrimedLot.trim();
	// exple: MZ3982-KZ3982-0105.04
    return GSFile.WriteAsText(output_path + '/' + lTrimedLot + '.' + pp.Wafer, 
								olympus_map);
}
