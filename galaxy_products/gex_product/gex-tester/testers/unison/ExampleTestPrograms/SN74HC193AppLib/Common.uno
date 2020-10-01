Unison:SyntaxRevision6.430000;
__ExternalRef {
   __Path = "../Libraries";
   __File = "Libraries.uno";
}
__Spec LevelsSpec {
    __Category LevelsBasicCat {
        vcc_spec.Min = "4.75V";
        vcc_spec.Typ = "Site(5.0V,6V,5V,5V)";
        vcc_spec.Max = "5.25V";
        vss_spec = "0V";
        vil_spec.Typ = "vss_spec";
        vil_spec.Max = "0.8V";
        vih_spec.Min = "2V";
        vih_spec.Typ = "vcc_spec";
        iol_spec = "10mA";
        ioh_spec = "-400uA";
        vol_spec.Typ = "vcc_spec/2";
        vol_spec.Max = "400mV";
        voh_spec.Min = "2.4V";
        voh_spec.Typ = "vcc_spec/2";
        iil_spec = "1.6mA";
        iih_spec = "1.6mA";
        iil_force = "vss_spec";
        iih_force = "vcc_spec";
        icc_min = "-200uA";
        icc_max = "20mA";
        iout = "100uA";
        cont_current = "-100uA";
        cont_limit = "-1V";
    }
    __ParamGlobals {
            vcc_spec { __Type = V; }
            vss_spec { __Type = V; }
            vil_spec { __Type = V; }
            vih_spec { __Type = V; }
            iol_spec { __Type = A; }
            ioh_spec { __Type = A; }
            vol_spec { __Type = V; }
            voh_spec { __Type = V; }
            iil_spec { __Type = A; }
            iih_spec { __Type = A; }
            iil_force { __Type = V; }
            iih_force { __Type = V; }
            icc_min { __Type = A; }
            icc_max { __Type = A; }
            iout { __Type = A; }
            cont_current { __Type = A; }
            cont_limit { __Type = V; }
    }
}
__Spec TimingSpec {
    __Category TimingBasicCat {
        period_spec.Min = "50.0ns";
        period_spec.Typ = "100.0ns";
        period_spec.Max = "200.0ns";
        drive_data_spec.Min = "0.0ns";
        drive_data_spec.Typ = "5.0ns";
        drive_data_spec.Max = "10.0ns";
        clock_pw = "50ns";
        clock_stop = "period_spec/2";
        compare_data_spec = "period_spec*0.95";
    }
    __ParamGlobals {
            period_spec { __Type = s; }
            drive_data_spec { __Type = s; }
            clock_pw { __Type = s; }
            clock_stop { __Type = s; }
            compare_data_spec { __Type = s; }
    }
}
__Mask LevelsMaxSel {
    vcc_spec = Max;
    vss_spec = Typ;
    iil_spec = Typ;
    iih_spec = Typ;
    iil_force = Typ;
    iih_force = Typ;
    icc_min = Typ;
    icc_max = Typ;
}
__Mask LevelsNomSel {
    vcc_spec = Typ;
    iout = Typ;
    vss_spec = Typ;
    iil_spec = Typ;
    iih_spec = Typ;
    iil_force = Typ;
    iih_force = Typ;
    icc_min = Typ;
    vih_spec = Typ;
    vil_spec = Typ;
    voh_spec = Typ;
    vol_spec = Typ;
    cont_current = Typ;
    cont_limit = Typ;
}
__Mask TimingNomSel {
    period_spec = Typ;
    drive_data_spec = Min;
    compare_data_spec = Typ;
    clock_stop = Typ;
    clock_pw = Typ;
}
__Levels BasicDCLevels {
    __Column[0] {
    	__LevelsColumnType = __VIType;
    	__Group = __Expression { __String = "VCC"; }
    	__ForceValue = __Expression { __String = "vcc_spec"; }
    }
    __Column[1] {
    	__LevelsColumnType = __DigitalType;
    	__Title = IOPins;
    	__Group = __Expression { __String = "AllPins"; }
    	Vil = __Expression { __String = "vil_spec"; }
    	Vih = __Expression { __String = "if (InduceFuncFails,Site(vih_spec,0.5V,vih_spec),vih_spec)"; }
    	Vol = __Expression { __String = "vol_spec"; }
    	Voh = __Expression { __String = "voh_spec"; }
    	Iol = __Expression { __String = "iol_spec"; }
    	Ioh = __Expression { __String = "ioh_spec"; }
    	Vref = __Expression { __String = "(vol_spec+voh_spec)/2"; }
    }
}
__Levels PowerDownLevels {
    __Column[0] {
    	__LevelsColumnType = __DigitalType;
    	__Title = IOLevels;
    	__Group = __Expression { __String = "AllPins"; }
    	Vil = __Expression { __String = "0V"; }
    	Vih = __Expression { __String = "300mV"; }
    	Vol = __Expression { __String = "0V"; }
    	Voh = __Expression { __String = "0V"; }
    	Vref = __Expression { __String = "0V"; }
    }
    __Column[1] {
    	__LevelsColumnType = __VIType;
    	__Group = __Expression { __String = "VCC"; }
    	__ForceValue = __Expression { __String = "vss_spec"; }
    }
}
__SubFlow MainFlow_OnLoad {
    __Node PassOnload_118 {
        __XCoord = (45,47);
        __InputPosition = 0;
        __TestID = "";
        __Exec = PassOnload;
    }
    __StartNode = PassOnload_118;
}
__SubFlow MainFlow_OnInitFlow {
    __TestGroup ConnectPins {
        __XCoord = (72,64);
        __Port[0] {
            __PortPosition = 160;
        }
        __Port[1] {
            __PortPosition = 180;
        }
        __InputPosition = 0;
        __TestID = "3000000";
        __Exec = __Test {
            __Entry[0] = PowerDownLevels;
            __PortExpression[0] = __Expression { __String = ".Result = TM_RESULT:TM_PASS"; }
            __PortExpression[1] = __Expression { __String = "TRUE"; }
            __Block[0] = {
                __Title = ConnPwr;
                __EnableExpression = __Expression { __String = "TRUE"; }
                __TestMethod {
                    __Name = LTXC::Connections;
                    ShowAdditionalArgs = __Expression { __String = "FALSE"; }
                    ExecuteSitesSerially = __Expression { __String = "FALSE"; }
                    TestPins = __Expression { __String = "AllPowerPins"; }
                    PinControl = __Expression { __String = "CONNECTION_ENUM:CONNECT_TO_DUT"; }
                    SimulatedTestResult = __Expression { __String = "TM_RESULT:TM_PASS"; }
                }
            }
            __Block[1] = {
                __Title = ConnIO;
                __EnableExpression = __Expression { __String = "TRUE"; }
                __TestMethod {
                    __Name = LTXC::Connections;
                    ShowAdditionalArgs = __Expression { __String = "FALSE"; }
                    ExecuteSitesSerially = __Expression { __String = "FALSE"; }
                    TestPins = __Expression { __String = "AllPins"; }
                    PinControl = __Expression { __String = "CONNECTION_ENUM:CONNECT_TO_DUT"; }
                    SimulatedTestResult = __Expression { __String = "TM_RESULT:TM_PASS"; }
                }
            }
        }
    }
    __StartNode = ConnectPins;
}
__SubFlow MainFlow_OnPowerDown {
    __TestGroup DisconnectPins {
        __XCoord = (75,65);
        __Port[0] {
            __PortPosition = 160;
        }
        __Port[1] {
            __PortPosition = 180;
        }
        __InputPosition = 0;
        __TestID = "5000000";
        __Exec = __Test {
            __Entry[0] = PowerDownLevels;
            __PortExpression[0] = __Expression { __String = ".Result = TM_RESULT:TM_PASS"; }
            __PortExpression[1] = __Expression { __String = "TRUE"; }
            __Block[0] = {
                __Title = Block1;
                __EnableExpression = __Expression { __String = "TRUE"; }
                __TestMethod {
                    __Name = LTXC::Connections;
                    ShowAdditionalArgs = __Expression { __String = "FALSE"; }
                    ExecuteSitesSerially = __Expression { __String = "FALSE"; }
                    TestPins = __Expression { __String = "AllPins"; }
                    PinControl = __Expression { __String = "CONNECTION_ENUM:DISCONNECT_FROM_DUT"; }
                    SimulatedTestResult = __Expression { __String = "TM_RESULT:TM_PASS"; }
                }
            }
            __Block[1] = {
                __Title = Block2;
                __EnableExpression = __Expression { __String = "TRUE"; }
                __TestMethod {
                    __Name = LTXC::Connections;
                    ShowAdditionalArgs = __Expression { __String = "FALSE"; }
                    ExecuteSitesSerially = __Expression { __String = "FALSE"; }
                    TestPins = __Expression { __String = "AllPowerPins"; }
                    PinControl = __Expression { __String = "CONNECTION_ENUM:DISCONNECT_FROM_DUT"; }
                    SimulatedTestResult = __Expression { __String = "TM_RESULT:TM_PASS"; }
                }
            }
        }
    }
    __StartNode = DisconnectPins;
}
__Bin Pass {
    __Number = 1;
    __Result = __True;
    __CheckOverFlow = __True;
    __Color = 0;
}
__Bin OpensFail {
    __Number = 20;
    __Result = __False;
    __CheckOverFlow = __True;
    __Color = 2;
}
__Bin InputLeakageFail {
    __Number = 3;
    __Result = __False;
    __CheckOverFlow = __True;
    __Color = 2;
}
__Bin StaticIddFail {
    __Number = 4;
    __Result = __False;
    __CheckOverFlow = __True;
    __Color = 2;
}
__Bin FunctionalFail {
    __Number = 5;
    __Result = __False;
    __CheckOverFlow = __True;
    __Color = 2;
}
__Bin ScanFail {
    __Number = 7;
    __Result = __False;
    __CheckOverFlow = __True;
    __Color = 2;
}
__Bin FreqCounterFail {
    __Number = 10;
    __Result = __False;
    __CheckOverFlow = __True;
    __Color = 2;
}
__Bin evResetBin {
    __Number = 102;
    __Result = __False;
    __CheckOverFlow = __True;
    __Color = 3;
}
__BinMap SN74HC193_BinMap {
    __Bin evResetBin = 0;
    __Bin Pass = 1;
    __Bin OpensFail = 2;
    __Bin InputLeakageFail = 3;
    __Bin StaticIddFail = 4;
    __Bin FunctionalFail = 5;
    __Bin ScanFail = 7;
    __Bin FreqCounterFail = 10;
    __Bin MeasLevelsFail = 6;
    __Bin OutputLevelFail = 8;
    __Bin PassOnload = 99;
    __Bin FailOnload = 100;
    __Bin PassUsrCal = 97;
    __Bin FailUserCal = 98;
    __Bin DatalogFail = 50;
    __Bin SPAT = 140;
    __Bin DPAT = 141;
    __Bin evDefaultBin = 0;
}
__Bin MeasLevelsFail {
    __Number = 6;
    __Result = __False;
    __CheckOverFlow = __True;
    __Color = 2;
}
__Axis PropDelayAxis {
    __NumberSteps = __Expression { __String = "100"; }
    __ParameterVariance {
        __Param compare_data_spec;
        __Start = __Expression { __String = "0ns"; __Type = s; }
        __Stop = __Expression { __String = "period_spec"; __Type = s; }
        __PinGroup = __Expression { __String = "allouts"; }
    }
}
__Plot VIH_Plot {
    __Axis =     __Axis  {
        __NumberSteps = __Expression { __String = "20"; }
        __ParameterVariance {
            __Param vih_spec;
            __Start = __Expression { __String = "1V"; __Type = V; }
            __Stop = __Expression { __String = "3V"; __Type = V; }
        }
    }
    __Axis =     __Axis  {
        __NumberSteps = __Expression { __String = "20"; }
        __ParameterVariance {
            __Param vil_spec;
            __Start = __Expression { __String = "0V"; __Type = V; }
            __Stop = __Expression { __String = "2V"; __Type = V; }
        }
    }
    __TestSpace {
        __DataSpace {
        	__DataGen = __Expression { }
        }
    }
}
__Plot OutputLevels {
    __Axis =     __Axis  {
        __NumberSteps = __Expression { __String = "50"; }
        __ParameterVariance {
            __Param voh_spec;
            __Start = __Expression { __String = "0V"; __Type = V; }
            __Stop = __Expression { __String = "4V"; __Type = V; }
        }
    }
    __Axis =     __Axis  {
        __NumberSteps = __Expression { __String = "50"; }
        __ParameterVariance {
            __Param vol_spec;
            __Start = __Expression { __String = "0V"; __Type = V; }
            __Stop = __Expression { __String = "4V"; __Type = V; }
        }
    }
    __TestSpace {
        __DataSpace {
        	__DataGen = __Expression { }
        }
    }
}
__Margin AC_Margin {
    __Mode __Scan, __CallbackFunction, __Binary;
    __PinPriority = __False;
    __Row =     __Axis  {
        __Title = "Tpd";
        __NumberSteps = __Expression { __String = "50"; }
        __ParameterVariance {
            __Param compare_data_spec;
            __Start = __Expression { __String = "75ns"; __Type = s; }
            __Stop = __Expression { __String = "90ns"; __Type = s; }
            __PinGroup = __Expression { __String = "allouts"; }
        }
    }
    __Row =     __Axis  {
        __Title = "Pw";
        __NumberSteps = __Expression { __String = "50"; }
        __ParameterVariance {
            __Param clock_pw;
            __Start = __Expression { __String = "50ns"; __Type = s; }
            __Stop = __Expression { __String = "0ns"; __Type = s; }
            __PinGroup = __Expression { __String = "clocks"; }
        }
    }
    __Row = VilAxis;
    __Row = VihAxis;
}
__Axis VilAxis {
    __Title = "Vil";
    __NumberSteps = __Expression { __String = "50"; }
    __ParameterVariance {
        __Param vil_spec;
        __Start = __Expression { __String = "0V"; __Type = V; }
        __Stop = __Expression { __String = "4V"; __Type = V; }
        __PinGroup = __Expression { __String = "AllPins"; }
    }
}
__Axis VihAxis {
    __Title = "Vih";
    __NumberSteps = __Expression { __String = "50"; }
    __ParameterVariance {
        __Param vih_spec;
        __Start = __Expression { __String = "0V"; __Type = V; }
        __Stop = __Expression { __String = "5V"; __Type = V; }
        __PinGroup = __Expression { __String = "clocks"; }
    }
}
__Plot VddIddPlot {
    __Title = "StaticIddTest.Block1.PerPinMeasurements[0]";    __Axis =     __Axis  {
        __NumberSteps = __Expression { __String = "25"; }
        __ParameterVariance {
            __Param vcc_spec;
            __Start = __Expression { __String = "2V"; __Type = V; }
            __Stop = __Expression { __String = "6V"; __Type = V; }
        }
    }
    __TestSpace {
        __TestGroup = StaticIddTest;
        __DataSpace {
        	__DataGen = __Expression { __String = "StaticIddTest.Block1.PerPinMeasurements[0]"; }
        }
        __TestObject = __True;
    }
}
__OperatorVariable CharacterizeDevice {
    __Comment = "";
    __Expression = __Expression { __String = "FALSE"; }
    __UserMode = Production;
}
__OperatorVariable InduceFuncFails {
    __Comment = "";
    __Expression = __Expression { __String = "FALSE"; }
    __UserMode = Production;
}
__Margin ContMargin {
    __Mode __CallbackFunction, __Binary;
    __PinPriority = __False;
    __Row = ContAxis;
    __Row = ContLimitAxis;
}
__Axis ContAxis {
    __NumberSteps = __Expression { __String = "10"; }
    __ParameterVariance {
        __Param cont_current;
        __Start = __Expression { __String = "0A"; __Type = A; }
        __Stop = __Expression { __String = "-500uA"; __Type = A; }
        __PinGroup = __Expression { }
    }
}
__Plot a {
    __Axis = ContAxis;
    __TestSpace {
        __DataSpace {
        	__DataGen = __Expression { }
        }
    }
}
__SubFlow MainFlow_UsrCal {
    __TestGroup LengthCal {
        __XCoord = (103,63);
        __Port[0] {
            __PortPosition = 160;
        }
        __Port[1] {
            __PortPosition = 180;
        }
        __InputPosition = 0;
        __TestID = "7000000";
        __Exec = __Test {
            __PortExpression[0] = __Expression { __String = ".Result = TM_RESULT:TM_PASS"; }
            __PortExpression[1] = __Expression { __String = "TRUE"; }
            __Block[0] = {
                __Title = FixtureCal;
                __EnableExpression = __Expression { __String = "TRUE"; }
                __TestMethod {
                    __Name = LTXC::FocusCalibration;
                    ShowAdditionalArgs = __Expression { __String = "FALSE"; }
                    TestPins = __Expression { __String = "AllPins"; }
                    CalibrationType = __Expression { __String = "FOCUS_CAL_TYPE_ENUM:CALIBRATE_FIXTURE"; }
                    CalibrationFile = __Expression { __String = "'SN74193Lengths'"; }
                    LoadFromFile = __Expression { __String = "FALSE"; }
                    SimulatedTestResult = __Expression { __String = "TM_RESULT:TM_PASS"; }
                }
            }
            __Block[1] = {
                __Title = TimmingCal;
                __EnableExpression = __Expression { __String = "((strcmp(TestProgData.ActiveLoadBrdName,'XSeriesSim')) = 0)"; }
                __TestMethod {
                    __Name = LTXC::FocusCalibration;
                    ShowAdditionalArgs = __Expression { __String = "FALSE"; }
                    TestPins = __Expression { __String = "AllPins"; }
                    CalibrationType = __Expression { __String = "FOCUS_CAL_TYPE_ENUM:CALIBRATE_TIMING"; }
                    CalibrationObject = __Expression { __String = "'SN74HC193Timing'"; }
                    LoadFromFile = __Expression { __String = "FALSE"; }
                    SimulatedTestResult = __Expression { __String = "TM_RESULT:TM_PASS"; }
                }
            }
        }
    }
    __Node FailUserCal_91 {
        __XCoord = (64,241);
        __InputPosition = 0;
        __TestID = "";
        __Exec = FailUserCal;
    }
    __Node PassUsrCal_92 {
        __XCoord = (145,238);
        __InputPosition = 0;
        __TestID = "";
        __Exec = PassUsrCal;
    }
    __StartNode = LengthCal;
    __PortConnections {
        LengthCal __Port[0] = PassUsrCal_92;
        LengthCal __Port[1] = FailUserCal_91;
    }
}
__OperatorVariable EnableCal {
    __Comment = "";
    __Expression = __Expression { __String = "FALSE"; }
    __UserMode = Production;
}
__Axis ContLimitAxis {
    __NumberSteps = __Expression { __String = "20"; }
    __ParameterVariance {
        __Param cont_limit;
        __Start = __Expression { __String = "-1V"; __Type = V; }
        __Stop = __Expression { __String = "0V"; __Type = V; }
        __PinGroup = __Expression { }
    }
}
__Margin PowerMargin {
    __Mode __Scan, __CallbackFunction, __BinaryLinear;
    __PinPriority = __False;
    __Row = IccAxis;
}
__Margin PowerMarginDual {
    __Mode __CallbackFunction, __Binary;
    __PinPriority = __False;
    __Axis = VccAxis;
    __Row = IccAxis;
}
__Axis VccAxis {
    __NumberSteps = __Expression { __String = "10"; }
    __ParameterVariance {
        __Param vcc_spec;
        __Start = __Expression { __String = "4V"; __Type = V; }
        __Stop = __Expression { __String = "6V"; __Type = V; }
        __PinGroup = __Expression { }
    }
}
__Axis IccAxis {
    __NumberSteps = __Expression { __String = "40"; }
    __ParameterVariance {
        __Param icc_max;
        __Start = __Expression { __String = "-1mA"; __Type = A; }
        __Stop = __Expression { __String = "2mA"; __Type = A; }
        __PinGroup = __Expression { }
    }
}
__Margin SearchMargin {
    __Mode __CallbackFunction, __Binary;
    __PinPriority = __False;
    __Row = VihAxis;
    __Row = VilAxis;
}
__Bin OutputLevelFail {
    __Number = 8;
    __Result = __False;
    __CheckOverFlow = __True;
    __Color = 2;
}
__Mask OutLevelsSpec {
    vcc_spec = Min;
    iout = Typ;
    vss_spec = Typ;
    iil_spec = Typ;
    iih_spec = Typ;
    iil_force = Typ;
    iih_force = Typ;
    icc_min = Typ;
    vih_spec = Min;
    vil_spec = Max;
    voh_spec = Min;
    vol_spec = Max;
    cont_current = Typ;
    cont_limit = Typ;
    iol_spec = Typ;
    ioh_spec = Typ;
}
__Levels DCSpecLevels {
    __Column[0] {
    	__LevelsColumnType = __DigitalType;
    	__Group = __Expression { __String = "AllPins"; }
    }
}
__Levels SearchLevels {
    __Column[0] {
    	__LevelsColumnType = __VIType;
    	__Group = __Expression { __String = "VCC"; }
    	__ForceValue = __Expression { __String = "vcc_spec"; }
    }
    __Column[1] {
    	__LevelsColumnType = __DigitalType;
    	__Title = IOPins;
    	__Group = __Expression { __String = "AllPins"; }
    	Vil = __Expression { __String = "vil_spec"; }
    	Vih = __Expression { __String = "vih_spec"; }
    	Vol = __Expression { __String = "vol_spec"; }
    	Voh = __Expression { __String = "voh_spec"; }
    	Iol = __Expression { __String = "iol_spec"; }
    	Ioh = __Expression { __String = "ioh_spec"; }
    	Vref = __Expression { __String = "(vol_spec+voh_spec)/2"; }
    }
}
__WaferDescriptor example {
	Diameter = __Expression { __String = "8"; __Type = ANY_REAL; }
	DieXLength = __Expression { __String = "10"; }
	DieYLength = __Expression { __String = "10"; }
	Orientation = Top;
	Indicator = Flat;
	NumberOfDie = 100;
	IncreasingXDirection = Right;
	IncreasingYDirection = Bottom;
	SaveDestination = "'${LTXHOME}/testers/${LTX_TESTER}/wafermap/${ObjName}_${LotId}_${SublotId}_${WaferId}_${DlogSetupTime}.xml'";
}
__Flow MainFlow {
    OnStart = MainFlow_OnStart;
    OnLoad = MainFlow_OnLoad;
    OnReset = MainFlow_OnReset;
    OnPowerDown = MainFlow_OnPowerDown;
    OnInitFlow = MainFlow_OnInitFlow;
    UsrCal = MainFlow_UsrCal;
    __LoopNotify = __False;
    __ProgramConfig ActiveAdapterBoardSelection;
}
__FunctionCall ActiveAdapterBoardSelection {
    __WrapCells = __True;
    __Function = ChooseActiveAdapterBoard;
    __ArrayOfGroup {
        __ArrayOf = TesterType;
        __ArrayOf = SystemAdapterBoard;
        __Row  {
            TesterType = __Expression { __String = "'DMD'"; }
            SystemAdapterBoard = __Expression { __String = "&DMDSeriesSim"; }
        }
        __Row  {
            TesterType = __Expression { __String = "'DMDx'"; }
            SystemAdapterBoard = __Expression { __String = "&DMDxSeriesSim"; }
        }
        __Row  {
            TesterType = __Expression { __String = "'EX'"; }
            SystemAdapterBoard = __Expression { __String = "&XSeriesSim"; }
        }
        __Row  {
            TesterType = __Expression { __String = "'MX'"; }
            SystemAdapterBoard = __Expression { __String = "&XSeriesSim"; }
        }
        __Row  {
            TesterType = __Expression { __String = "'LX'"; }
            SystemAdapterBoard = __Expression { __String = "&XSeriesSim"; }
        }
    }
}
__Bin PassOnload {
    __Number = 99;
    __Result = __True;
    __CheckOverFlow = __True;
    __Color = 0;
}
__Bin FailOnload {
    __Number = 100;
    __Result = __False;
    __CheckOverFlow = __True;
    __Color = 2;
}
__SubFlow MainFlow_OnReset {
    __Node evResetBin_90 {
        __XCoord = (200,79);
        __InputPosition = 0;
        __TestID = "";
        __Exec = evResetBin;
    }
    __TestGroup DisconnectOnReset {
        __XCoord = (43,20);
        __Port[0] {
            __PortPosition = 160;
        }
        __Port[1] {
            __PortPosition = 180;
        }
        __InputPosition = 0;
        __TestID = "12000000";
        __Exec = __Test {
            __Entry[0] = PowerDownLevels;
            __PortExpression[0] = __Expression { __String = ".Result = TM_RESULT:TM_PASS"; }
            __PortExpression[1] = __Expression { __String = "TRUE"; }
            __Block[0] = {
                __Title = Block1;
                __EnableExpression = __Expression { __String = "TRUE"; }
                __TestMethod {
                    __Name = LTXC::Connections;
                    ShowAdditionalArgs = __Expression { __String = "FALSE"; }
                    ExecuteSitesSerially = __Expression { __String = "FALSE"; }
                    TestPins = __Expression { __String = "AllPins"; }
                    PinControl = __Expression { __String = "CONNECTION_ENUM:DISCONNECT_FROM_DUT"; }
                    SimulatedTestResult = __Expression { __String = "TM_RESULT:TM_PASS"; }
                }
            }
            __Block[1] = {
                __Title = Block2;
                __EnableExpression = __Expression { __String = "TRUE"; }
                __TestMethod {
                    __Name = LTXC::Connections;
                    ShowAdditionalArgs = __Expression { __String = "FALSE"; }
                    ExecuteSitesSerially = __Expression { __String = "FALSE"; }
                    TestPins = __Expression { __String = "AllPowerPins"; }
                    PinControl = __Expression { __String = "CONNECTION_ENUM:DISCONNECT_FROM_DUT"; }
                    SimulatedTestResult = __Expression { __String = "TM_RESULT:TM_PASS"; }
                }
            }
        }
    }
    __StartNode = DisconnectOnReset;
    __PortConnections {
        DisconnectOnReset __Port[0] = evResetBin_90;
        DisconnectOnReset __Port[1] = evResetBin_90;
    }
}
__Bin PassUsrCal {
    __Number = 97;
    __Result = __True;
    __CheckOverFlow = __True;
    __Color = 0;
}
__Bin FailUserCal {
    __Number = 98;
    __Result = __False;
    __CheckOverFlow = __True;
    __Color = 2;
}
__OperatorVariable Training {
    __Comment = "";
    __Expression = __Expression { __String = "FALSE"; }
    __UserMode = Production;
}
