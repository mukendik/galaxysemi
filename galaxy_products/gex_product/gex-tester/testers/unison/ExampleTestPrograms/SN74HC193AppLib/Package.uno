Unison:SyntaxRevision6.430000;
__PinType DibUtilities {
    __Type = NullPin;
}
__PinType SharedNullPins {
    __Type = NullPin;
}
__PinType In_Type {
    __Type = DigitalPin;
    __Direction = Input;
    __Termination = NoTermination;
}
__PinType Out_Type {
    __Type = DigitalPin;
    __Direction = Output;
    __Termination = NoTermination;
}
__PinType InOut_Type {
    __Type = DigitalPin;
    __Direction = Bidirect;
    __Termination = NoTermination;
}
__PinType DigitalInput {
    __Type = DigitalPin;
    __Direction = Input;
    __Termination = ActiveLoad;
}
__PinType DigitalBidirect {
    __Type = DigitalPin;
    __Direction = Bidirect;
    __Termination = ActiveLoad;
}
__PinType DigitalOutput {
    __Type = DigitalPin;
    __Direction = Output;
    __Termination = ActiveLoad;
}
__PinType Supply_Type {
    __Type = VIPin;
    __ControlFlags EnableGanging = __True;
    __ControlFlags ReloadAtTest = __True;
    Min {
        PinCurrent = __Expression { __String = "-0.1A"; }
        PinVoltage = __Expression { __String = "-1V"; }
    }
    Max {
        PinCurrent = __Expression { __String = ".1A"; }
        PinVoltage = __Expression { __String = "6.5V"; }
    }
    __SupplyType = __Expression { __String = "PowerType:HCOVI"; }
}
__PinType DCRef_Type {
    __Type = VIPin;
    __ControlFlags ReloadAtTest = __True;
}
__PinType Resource_Type {
    __Type = ResourcePin;
}
__PinGroup AllPins {
	__Group = __Expression { __String = "A_in+BO_+B_in+CLR+CO_+C_in+DOWN+D_in+LOAD_+QA+QB+QC+QD+UP"; }
}
__PinGroup AllPowerPins {
	__Group = __Expression { __String = "VCC"; }
}
__PinGroup all {
	__Group = __Expression { __String = "allins+allouts"; }
}
__PinGroup allin_no_clks {
	__Group = __Expression { __String = "CLR+LOAD_+D_in+C_in+B_in+A_in"; }
}
__PinGroup allins {
	__Group = __Expression { __String = "UP+DOWN+CLR+LOAD_+D_in+C_in+B_in+A_in"; }
}
__PinGroup allouts {
	__Group = __Expression { __String = "QD+QC+QB+QA+BO_+CO_"; }
}
__PinGroup clocks {
	__Group = __Expression { __String = "UP+DOWN"; }
}
__PinGroup ScanHeader_group {
	__Group = __Expression { __String = "QA+UP"; }
}
__PinGroup AllResource {
	__Group = __Expression { __String = "DIB_N5V_U+DIB_P12V_U+DIB_P5V_RLY+DIB_P5V_U"; }
}
__AdapterBoard XSeriesSim {
	__Comment = "Prefixes:
74HC193 		dc 	Up Dn Counter
PIC12F683 	Mcu	MicroController
LT1121 		Reg	Regulator
AD5541		Dac	Digital to Analog Converter
MAX195		Adc	Analog to Digital Converter

DIBU		Dib	DIBU Resources
MULTIWAVE	Mws	Multiwave Loopback Connection (short board)
DPIN		Dcs	DPin Loopback Connection (short board)	";
	__Pin { __Name = DIB_P5V_RLY; __Ppid = "DIBU_5V Relay Supply"; __XCoord = (0,0); __Shape = 0; __PinType = DibUtilities; 
		__Connection[0] { __Resource = shared; __TesterChannel[1] = DIBUR5V1; __TesterChannel[2] = DIBUR5V1; __TesterChannel[3] = DIBUR5V1; __TesterChannel[4] = DIBUR5V1; __TesterChannel[5] = DIBUR5V1; __TesterChannel[6] = DIBUR5V1; __TesterChannel[7] = DIBUR5V1; __TesterChannel[8] = DIBUR5V1; __TesterChannel[9] = DIBUR5V1; __TesterChannel[10] = DIBUR5V1; __TesterChannel[11] = DIBUR5V1; __TesterChannel[12] = DIBUR5V1; }
	}
	__Pin { __Name = DIB_P12V_U; __Ppid = "DIBU +12V Supply"; __XCoord = (0,0); __Shape = 0; __PinType = DibUtilities; 
		__Connection[0] { __Resource = shared; __TesterChannel[1] = DIBUP12V1; __TesterChannel[2] = DIBUP12V1; __TesterChannel[3] = DIBUP12V1; __TesterChannel[4] = DIBUP12V1; __TesterChannel[5] = DIBUP12V1; __TesterChannel[6] = DIBUP12V1; __TesterChannel[7] = DIBUP12V1; __TesterChannel[8] = DIBUP12V1; __TesterChannel[9] = DIBUP12V1; __TesterChannel[10] = DIBUP12V1; __TesterChannel[11] = DIBUP12V1; __TesterChannel[12] = DIBUP12V1; }
	}
	__Pin { __Name = DIB_P5V_U; __Ppid = "DIBU +5V Supply"; __XCoord = (0,0); __Shape = 0; __PinType = DibUtilities; 
		__Connection[0] { __Resource = shared; __TesterChannel[1] = DIBUP5V1; __TesterChannel[2] = DIBUP5V1; __TesterChannel[3] = DIBUP5V1; __TesterChannel[4] = DIBUP5V1; __TesterChannel[5] = DIBUP5V1; __TesterChannel[6] = DIBUP5V1; __TesterChannel[7] = DIBUP5V1; __TesterChannel[8] = DIBUP5V1; __TesterChannel[9] = DIBUP5V1; __TesterChannel[10] = DIBUP5V1; __TesterChannel[11] = DIBUP5V1; __TesterChannel[12] = DIBUP5V1; }
	}
	__Pin { __Name = DIB_N5V_U; __Ppid = "DIBU -5V Supply"; __XCoord = (0,0); __Shape = 0; __PinType = DibUtilities; 
		__Connection[0] { __Resource = shared; __TesterChannel[1] = DIBUN5V1; __TesterChannel[2] = DIBUN5V1; __TesterChannel[3] = DIBUN5V1; __TesterChannel[4] = DIBUN5V1; __TesterChannel[5] = DIBUN5V1; __TesterChannel[6] = DIBUN5V1; __TesterChannel[7] = DIBUN5V1; __TesterChannel[8] = DIBUN5V1; __TesterChannel[9] = DIBUN5V1; __TesterChannel[10] = DIBUN5V1; __TesterChannel[11] = DIBUN5V1; __TesterChannel[12] = DIBUN5V1; }
	}
	__Pin { __Name = B_in; __Ppid = "74HC193-1"; __XCoord = (0,0); __Shape = 0; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 0; __TesterChannel[2] = 16; __TesterChannel[3] = 32; __TesterChannel[4] = 48; __TesterChannel[5] = 128; __TesterChannel[6] = 144; __TesterChannel[7] = 160; __TesterChannel[8] = 176; __TesterChannel[9] = 512; __TesterChannel[10] = 528; __TesterChannel[11] = HS64; __TesterChannel[12] = HS80; }
	}
	__Pin { __Name = QB; __Ppid = "74HC193-2"; __XCoord = (0,0); __Shape = 0; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 1; __TesterChannel[2] = 17; __TesterChannel[3] = 33; __TesterChannel[4] = 49; __TesterChannel[5] = 129; __TesterChannel[6] = 145; __TesterChannel[7] = 161; __TesterChannel[8] = 177; __TesterChannel[9] = 513; __TesterChannel[10] = 529; __TesterChannel[11] = HS65; __TesterChannel[12] = HS81; }
	}
	__Pin { __Name = QA; __Ppid = "74HC193-3"; __XCoord = (0,0); __Shape = 0; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 2; __TesterChannel[2] = 18; __TesterChannel[3] = 34; __TesterChannel[4] = 50; __TesterChannel[5] = 130; __TesterChannel[6] = 146; __TesterChannel[7] = 162; __TesterChannel[8] = 178; __TesterChannel[9] = 514; __TesterChannel[10] = 530; __TesterChannel[11] = HS66; __TesterChannel[12] = HS82; }
	}
	__Pin { __Name = DOWN; __Ppid = "74HC193-4"; __XCoord = (0,0); __Shape = 0; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 3; __TesterChannel[2] = 19; __TesterChannel[3] = 35; __TesterChannel[4] = 51; __TesterChannel[5] = 131; __TesterChannel[6] = 147; __TesterChannel[7] = 163; __TesterChannel[8] = 179; __TesterChannel[9] = 515; __TesterChannel[10] = 531; __TesterChannel[11] = HS67; __TesterChannel[12] = HS83; }
	}
	__Pin { __Name = UP; __Ppid = "74HC193-5"; __XCoord = (0,0); __Shape = 0; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 4; __TesterChannel[2] = 20; __TesterChannel[3] = 36; __TesterChannel[4] = 52; __TesterChannel[5] = 132; __TesterChannel[6] = 148; __TesterChannel[7] = 164; __TesterChannel[8] = 180; __TesterChannel[9] = 516; __TesterChannel[10] = 532; __TesterChannel[11] = HS68; __TesterChannel[12] = HS84; }
	}
	__Pin { __Name = QC; __Ppid = "74HC193-6"; __XCoord = (0,0); __Shape = 0; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 5; __TesterChannel[2] = 21; __TesterChannel[3] = 37; __TesterChannel[4] = 53; __TesterChannel[5] = 133; __TesterChannel[6] = 149; __TesterChannel[7] = 165; __TesterChannel[8] = 181; __TesterChannel[9] = 517; __TesterChannel[10] = 533; __TesterChannel[11] = HS69; __TesterChannel[12] = HS85; }
	}
	__Pin { __Name = QD; __Ppid = "74HC193-7"; __XCoord = (0,0); __Shape = 0; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 6; __TesterChannel[2] = 22; __TesterChannel[3] = 38; __TesterChannel[4] = 54; __TesterChannel[5] = 134; __TesterChannel[6] = 150; __TesterChannel[7] = 166; __TesterChannel[8] = 182; __TesterChannel[9] = 518; __TesterChannel[10] = 534; __TesterChannel[11] = HS70; __TesterChannel[12] = HS86; }
	}
	__Pin { __Name = GND; __Ppid = "74HC193-8"; __XCoord = (0,0); __Shape = 0; __PinType = SharedNullPins; 
		__Connection[0] { __TesterChannel[1] = GND; __TesterChannel[2] = GND; __TesterChannel[3] = GND; __TesterChannel[4] = GND; __TesterChannel[5] = GND; __TesterChannel[6] = GND; __TesterChannel[7] = GND; __TesterChannel[8] = GND; __TesterChannel[9] = GND; __TesterChannel[10] = GND; __TesterChannel[11] = GND; __TesterChannel[12] = GND; }
	}
	__Pin { __Name = D_in; __Ppid = "74HC193-9"; __XCoord = (0,0); __Shape = 0; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 7; __TesterChannel[2] = 23; __TesterChannel[3] = 39; __TesterChannel[4] = 55; __TesterChannel[5] = 135; __TesterChannel[6] = 151; __TesterChannel[7] = 167; __TesterChannel[8] = 183; __TesterChannel[9] = 519; __TesterChannel[10] = 535; __TesterChannel[11] = HS71; __TesterChannel[12] = HS87; }
	}
	__Pin { __Name = C_in; __Ppid = "74HC193-10"; __XCoord = (0,0); __Shape = 0; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 8; __TesterChannel[2] = 24; __TesterChannel[3] = 40; __TesterChannel[4] = 56; __TesterChannel[5] = 136; __TesterChannel[6] = 152; __TesterChannel[7] = 168; __TesterChannel[8] = 184; __TesterChannel[9] = 520; __TesterChannel[10] = 536; __TesterChannel[11] = HS72; __TesterChannel[12] = HS88; }
	}
	__Pin { __Name = LOAD_; __Ppid = "74HC193-11"; __XCoord = (0,0); __Shape = 0; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 9; __TesterChannel[2] = 25; __TesterChannel[3] = 41; __TesterChannel[4] = 57; __TesterChannel[5] = 137; __TesterChannel[6] = 153; __TesterChannel[7] = 169; __TesterChannel[8] = 185; __TesterChannel[9] = 521; __TesterChannel[10] = 537; __TesterChannel[11] = HS73; __TesterChannel[12] = HS89; }
	}
	__Pin { __Name = CO_; __Ppid = "74HC193-12"; __XCoord = (0,0); __Shape = 0; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 10; __TesterChannel[2] = 26; __TesterChannel[3] = 42; __TesterChannel[4] = 58; __TesterChannel[5] = 138; __TesterChannel[6] = 154; __TesterChannel[7] = 170; __TesterChannel[8] = 186; __TesterChannel[9] = 522; __TesterChannel[10] = 538; __TesterChannel[11] = HS74; __TesterChannel[12] = HS90; }
	}
	__Pin { __Name = BO_; __Ppid = "74HC193-13"; __XCoord = (0,0); __Shape = 0; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 11; __TesterChannel[2] = 27; __TesterChannel[3] = 43; __TesterChannel[4] = 59; __TesterChannel[5] = 139; __TesterChannel[6] = 155; __TesterChannel[7] = 171; __TesterChannel[8] = 187; __TesterChannel[9] = 523; __TesterChannel[10] = 539; __TesterChannel[11] = HS75; __TesterChannel[12] = HS91; }
	}
	__Pin { __Name = CLR; __Ppid = "74HC193-14"; __XCoord = (0,0); __Shape = 0; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 12; __TesterChannel[2] = 28; __TesterChannel[3] = 44; __TesterChannel[4] = 60; __TesterChannel[5] = 140; __TesterChannel[6] = 156; __TesterChannel[7] = 172; __TesterChannel[8] = 188; __TesterChannel[9] = 524; __TesterChannel[10] = 540; __TesterChannel[11] = HS76; __TesterChannel[12] = HS92; }
	}
	__Pin { __Name = A_in; __Ppid = "74HC193-15"; __XCoord = (0,0); __Shape = 0; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 13; __TesterChannel[2] = 29; __TesterChannel[3] = 45; __TesterChannel[4] = 61; __TesterChannel[5] = 141; __TesterChannel[6] = 157; __TesterChannel[7] = 173; __TesterChannel[8] = 189; __TesterChannel[9] = 525; __TesterChannel[10] = 541; __TesterChannel[11] = HS77; __TesterChannel[12] = HS93; }
	}
	__Pin { __Name = VCC; __Ppid = "74HC193-16"; __XCoord = (0,0); __Shape = 0; __PinType = Supply_Type; 
		__Connection[0] { __TesterChannel[1] = OVI1; __TesterChannel[2] = OVI2; __TesterChannel[3] = OVI3; __TesterChannel[4] = OVI4; __TesterChannel[5] = OVI65; __TesterChannel[6] = OVI66; __TesterChannel[7] = OVI67; __TesterChannel[8] = OVI68; __TesterChannel[9] = OVI129; __TesterChannel[10] = OVI130; __TesterChannel[11] = OVI131; __TesterChannel[12] = OVI132; }
	}
	__MaxSite = 12;
	__TesterChannelCal 0 = __Expression { __String = " 1.32ns"; __Type = s; }
	__TesterChannelCal 1 = __Expression { __String = " 1.41ns"; __Type = s; }
	__TesterChannelCal 2 = __Expression { __String = " 1.41ns"; __Type = s; }
	__TesterChannelCal 3 = __Expression { __String = " 1.32ns"; __Type = s; }
	__TesterChannelCal 4 = __Expression { __String = " 1.31ns"; __Type = s; }
	__TesterChannelCal 5 = __Expression { __String = "1.67ns"; __Type = s; }
	__TesterChannelCal 6 = __Expression { __String = " 1.61ns"; __Type = s; }
	__TesterChannelCal 7 = __Expression { __String = "1.55ns"; __Type = s; }
	__TesterChannelCal 8 = __Expression { __String = "1.49ns"; __Type = s; }
	__TesterChannelCal 9 = __Expression { __String = "1.31ns"; __Type = s; }
	__TesterChannelCal 10 = __Expression { __String = "1.20ns"; __Type = s; }
	__TesterChannelCal 11 = __Expression { __String = "1.53ns"; __Type = s; }
	__TesterChannelCal 12 = __Expression { __String = "1.24ns"; __Type = s; }
	__TesterChannelCal 13 = __Expression { __String = "1.23ns"; __Type = s; }
	__TesterChannelCal 16 = __Expression { __String = "633ps"; __Type = s; }
	__TesterChannelCal 17 = __Expression { __String = "726ps"; __Type = s; }
	__TesterChannelCal 18 = __Expression { __String = "576ps"; __Type = s; }
	__TesterChannelCal 19 = __Expression { __String = "631ps"; __Type = s; }
	__TesterChannelCal 20 = __Expression { __String = "848ps"; __Type = s; }
	__TesterChannelCal 21 = __Expression { __String = "823ps"; __Type = s; }
	__TesterChannelCal 22 = __Expression { __String = "778ps"; __Type = s; }
	__TesterChannelCal 23 = __Expression { __String = "804ps"; __Type = s; }
	__TesterChannelCal 24 = __Expression { __String = "736ps"; __Type = s; }
	__TesterChannelCal 25 = __Expression { __String = "882ps"; __Type = s; }
	__TesterChannelCal 26 = __Expression { __String = "865ps"; __Type = s; }
	__TesterChannelCal 27 = __Expression { __String = "827ps"; __Type = s; }
	__TesterChannelCal 28 = __Expression { __String = "881ps"; __Type = s; }
	__TesterChannelCal 29 = __Expression { __String = "684ps"; __Type = s; }
	__TesterChannelCal 32 = __Expression { __String = "1.58ns"; __Type = s; }
	__TesterChannelCal 33 = __Expression { __String = "1.41ns"; __Type = s; }
	__TesterChannelCal 34 = __Expression { __String = "1.35ns"; __Type = s; }
	__TesterChannelCal 35 = __Expression { __String = "1.32ns"; __Type = s; }
	__TesterChannelCal 36 = __Expression { __String = "1.35ns"; __Type = s; }
	__TesterChannelCal 37 = __Expression { __String = "1.41ns"; __Type = s; }
	__TesterChannelCal 38 = __Expression { __String = "1.33ns"; __Type = s; }
	__TesterChannelCal 39 = __Expression { __String = "1.45ns"; __Type = s; }
	__TesterChannelCal 40 = __Expression { __String = "1.29ns"; __Type = s; }
	__TesterChannelCal 41 = __Expression { __String = "1.30ns"; __Type = s; }
	__TesterChannelCal 42 = __Expression { __String = "1.35ns"; __Type = s; }
	__TesterChannelCal 43 = __Expression { __String = "1.56ns"; __Type = s; }
	__TesterChannelCal 44 = __Expression { __String = "1.46ns"; __Type = s; }
	__TesterChannelCal 45 = __Expression { __String = "1.61ns"; __Type = s; }
	__TesterChannelCal 48 = __Expression { __String = "1.01ns"; __Type = s; }
	__TesterChannelCal 49 = __Expression { __String = "918ps"; __Type = s; }
	__TesterChannelCal 50 = __Expression { __String = "891ps"; __Type = s; }
	__TesterChannelCal 51 = __Expression { __String = "792ps"; __Type = s; }
	__TesterChannelCal 52 = __Expression { __String = "961ps"; __Type = s; }
	__TesterChannelCal 53 = __Expression { __String = "754ps"; __Type = s; }
	__TesterChannelCal 54 = __Expression { __String = "819ps"; __Type = s; }
	__TesterChannelCal 55 = __Expression { __String = "632ps"; __Type = s; }
	__TesterChannelCal 56 = __Expression { __String = "577ps"; __Type = s; }
	__TesterChannelCal 57 = __Expression { __String = "791ps"; __Type = s; }
	__TesterChannelCal 58 = __Expression { __String = "938ps"; __Type = s; }
	__TesterChannelCal 59 = __Expression { __String = "713ps"; __Type = s; }
	__TesterChannelCal 60 = __Expression { __String = "946ps"; __Type = s; }
	__TesterChannelCal 61 = __Expression { __String = "825ps"; __Type = s; }
	__TesterChannelCal 64 = __Expression { __String = "513ps"; __Type = s; }
	__TesterChannelCal 65 = __Expression { __String = "512ps"; __Type = s; }
	__TesterChannelCal 66 = __Expression { __String = "493ps"; __Type = s; }
	__TesterChannelCal 67 = __Expression { __String = "443ps"; __Type = s; }
	__TesterChannelCal 68 = __Expression { __String = "539ps"; __Type = s; }
	__TesterChannelCal 69 = __Expression { __String = "487ps"; __Type = s; }
	__TesterChannelCal 70 = __Expression { __String = "518ps"; __Type = s; }
	__TesterChannelCal 71 = __Expression { __String = "511ps"; __Type = s; }
	__TesterChannelCal 72 = __Expression { __String = "470ps"; __Type = s; }
	__TesterChannelCal 73 = __Expression { __String = "463ps"; __Type = s; }
	__TesterChannelCal 74 = __Expression { __String = "379ps"; __Type = s; }
	__TesterChannelCal 75 = __Expression { __String = "299ps"; __Type = s; }
	__TesterChannelCal 76 = __Expression { __String = "411ps"; __Type = s; }
	__TesterChannelCal 77 = __Expression { __String = "365ps"; __Type = s; }
	__TesterChannelCal 80 = __Expression { __String = "494ps"; __Type = s; }
	__TesterChannelCal 81 = __Expression { __String = "432ps"; __Type = s; }
	__TesterChannelCal 82 = __Expression { __String = "463ps"; __Type = s; }
	__TesterChannelCal 83 = __Expression { __String = "414ps"; __Type = s; }
	__TesterChannelCal 84 = __Expression { __String = "520ps"; __Type = s; }
	__TesterChannelCal 85 = __Expression { __String = "458ps"; __Type = s; }
	__TesterChannelCal 86 = __Expression { __String = "498ps"; __Type = s; }
	__TesterChannelCal 87 = __Expression { __String = "367ps"; __Type = s; }
	__TesterChannelCal 88 = __Expression { __String = "420ps"; __Type = s; }
	__TesterChannelCal 89 = __Expression { __String = "339ps"; __Type = s; }
	__TesterChannelCal 90 = __Expression { __String = "361ps"; __Type = s; }
	__TesterChannelCal 91 = __Expression { __String = "312ps"; __Type = s; }
	__TesterChannelCal 92 = __Expression { __String = "431ps"; __Type = s; }
	__TesterChannelCal 93 = __Expression { __String = "447ps"; __Type = s; }
	__TesterChannelCal 128 = __Expression { __String = "1.02ns"; __Type = s; }
	__TesterChannelCal 129 = __Expression { __String = "837ps"; __Type = s; }
	__TesterChannelCal 130 = __Expression { __String = "796ps"; __Type = s; }
	__TesterChannelCal 131 = __Expression { __String = "799ps"; __Type = s; }
	__TesterChannelCal 132 = __Expression { __String = "879ps"; __Type = s; }
	__TesterChannelCal 133 = __Expression { __String = "784ps"; __Type = s; }
	__TesterChannelCal 134 = __Expression { __String = "709ps"; __Type = s; }
	__TesterChannelCal 135 = __Expression { __String = "603ps"; __Type = s; }
	__TesterChannelCal 136 = __Expression { __String = "629ps"; __Type = s; }
	__TesterChannelCal 137 = __Expression { __String = "699ps"; __Type = s; }
	__TesterChannelCal 138 = __Expression { __String = "738ps"; __Type = s; }
	__TesterChannelCal 139 = __Expression { __String = "691ps"; __Type = s; }
	__TesterChannelCal 140 = __Expression { __String = "914ps"; __Type = s; }
	__TesterChannelCal 141 = __Expression { __String = "719ps"; __Type = s; }
	__TesterChannelCal 144 = __Expression { __String = "1.01ns"; __Type = s; }
	__TesterChannelCal 145 = __Expression { __String = "965ps"; __Type = s; }
	__TesterChannelCal 146 = __Expression { __String = "1ns"; __Type = s; }
	__TesterChannelCal 147 = __Expression { __String = "1.09ns"; __Type = s; }
	__TesterChannelCal 148 = __Expression { __String = "1.18ns"; __Type = s; }
	__TesterChannelCal 149 = __Expression { __String = "1.1ns"; __Type = s; }
	__TesterChannelCal 150 = __Expression { __String = "1.19ns"; __Type = s; }
	__TesterChannelCal 151 = __Expression { __String = "2.45ns"; __Type = s; }
	__TesterChannelCal 152 = __Expression { __String = "1.32ns"; __Type = s; }
	__TesterChannelCal 153 = __Expression { __String = "1.27ns"; __Type = s; }
	__TesterChannelCal 154 = __Expression { __String = "1.26ns"; __Type = s; }
	__TesterChannelCal 155 = __Expression { __String = "1.18ns"; __Type = s; }
	__TesterChannelCal 156 = __Expression { __String = "1.27ns"; __Type = s; }
	__TesterChannelCal 157 = __Expression { __String = "1.11ns"; __Type = s; }
	__TesterChannelCal 160 = __Expression { __String = "746ps"; __Type = s; }
	__TesterChannelCal 161 = __Expression { __String = "676ps"; __Type = s; }
	__TesterChannelCal 162 = __Expression { __String = "709ps"; __Type = s; }
	__TesterChannelCal 163 = __Expression { __String = "746ps"; __Type = s; }
	__TesterChannelCal 164 = __Expression { __String = "774ps"; __Type = s; }
	__TesterChannelCal 165 = __Expression { __String = "865ps"; __Type = s; }
	__TesterChannelCal 166 = __Expression { __String = "956ps"; __Type = s; }
	__TesterChannelCal 167 = __Expression { __String = "1ns"; __Type = s; }
	__TesterChannelCal 168 = __Expression { __String = "896ps"; __Type = s; }
	__TesterChannelCal 169 = __Expression { __String = "913ps"; __Type = s; }
	__TesterChannelCal 170 = __Expression { __String = "976ps"; __Type = s; }
	__TesterChannelCal 171 = __Expression { __String = "878ps"; __Type = s; }
	__TesterChannelCal 172 = __Expression { __String = "939ps"; __Type = s; }
	__TesterChannelCal 173 = __Expression { __String = "683ps"; __Type = s; }
	__TesterChannelCal 176 = __Expression { __String = "1.27ns"; __Type = s; }
	__TesterChannelCal 177 = __Expression { __String = "1.38ns"; __Type = s; }
	__TesterChannelCal 178 = __Expression { __String = "1.34ns"; __Type = s; }
	__TesterChannelCal 179 = __Expression { __String = "1.26ns"; __Type = s; }
	__TesterChannelCal 180 = __Expression { __String = "1.20ns"; __Type = s; }
	__TesterChannelCal 181 = __Expression { __String = "1.23ns"; __Type = s; }
	__TesterChannelCal 182 = __Expression { __String = "1.25ns"; __Type = s; }
	__TesterChannelCal 183 = __Expression { __String = "1.03ns"; __Type = s; }
	__TesterChannelCal 184 = __Expression { __String = "906ps"; __Type = s; }
	__TesterChannelCal 185 = __Expression { __String = "1.15ns"; __Type = s; }
	__TesterChannelCal 186 = __Expression { __String = "1.18ns"; __Type = s; }
	__TesterChannelCal 187 = __Expression { __String = "1.07ns"; __Type = s; }
	__TesterChannelCal 188 = __Expression { __String = "1.17ns"; __Type = s; }
	__TesterChannelCal 189 = __Expression { __String = "1.06ns"; __Type = s; }
	__TesterChannelCal 512 = __Expression { __String = "451ps"; __Type = s; }
	__TesterChannelCal 513 = __Expression { __String = "728ps"; __Type = s; }
	__TesterChannelCal 514 = __Expression { __String = "428ps"; __Type = s; }
	__TesterChannelCal 515 = __Expression { __String = "483ps"; __Type = s; }
	__TesterChannelCal 516 = __Expression { __String = "443ps"; __Type = s; }
	__TesterChannelCal 517 = __Expression { __String = "381ps"; __Type = s; }
	__TesterChannelCal 518 = __Expression { __String = "333ps"; __Type = s; }
	__TesterChannelCal 519 = __Expression { __String = "567ps"; __Type = s; }
	__TesterChannelCal 520 = __Expression { __String = "459ps"; __Type = s; }
	__TesterChannelCal 521 = __Expression { __String = "625ps"; __Type = s; }
	__TesterChannelCal 522 = __Expression { __String = "603ps"; __Type = s; }
	__TesterChannelCal 523 = __Expression { __String = "440ps"; __Type = s; }
	__TesterChannelCal 524 = __Expression { __String = "474ps"; __Type = s; }
	__TesterChannelCal 525 = __Expression { __String = "735ps"; __Type = s; }
	__TesterChannelCal 530 = __Expression { __String = "477ps"; __Type = s; }
	__TesterChannelCal 531 = __Expression { __String = "700ps"; __Type = s; }
	__TesterChannelCal 532 = __Expression { __String = "646ps"; __Type = s; }
	__TesterChannelCal 533 = __Expression { __String = "572ps"; __Type = s; }
	__TesterChannelCal 534 = __Expression { __String = "630ps"; __Type = s; }
	__TesterChannelCal 535 = __Expression { __String = "931ps"; __Type = s; }
	__TesterChannelCal 536 = __Expression { __String = "526ps"; __Type = s; }
	__TesterChannelCal 537 = __Expression { __String = "649ps"; __Type = s; }
	__TesterChannelCal 538 = __Expression { __String = "708ps"; __Type = s; }
	__TesterChannelCal 539 = __Expression { __String = "364ps"; __Type = s; }
	__TesterChannelCal 540 = __Expression { __String = "408ps"; __Type = s; }
	__TesterChannelCal 541 = __Expression { __String = "578ps"; __Type = s; }
}
__AdapterBoard DMDxSeriesSim {
	__Pin { __Name = A_in; __Ppid = "1"; __XCoord = (120,120); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 1385; __TesterChannel[2] = 1414; __TesterChannel[3] = 3305; __TesterChannel[4] = 3334; }
	}
	__Pin { __Name = BO_; __Ppid = "2"; __XCoord = (240,120); __Shape = 16; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 1384; __TesterChannel[2] = 1407; __TesterChannel[3] = 3304; __TesterChannel[4] = 3327; }
	}
	__Pin { __Name = B_in; __Ppid = "3"; __XCoord = (360,120); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 1373; __TesterChannel[2] = 1409; __TesterChannel[3] = 3293; __TesterChannel[4] = 3329; }
	}
	__Pin { __Name = CLR; __Ppid = "4"; __XCoord = (120,240); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 1391; __TesterChannel[2] = 1408; __TesterChannel[3] = 3311; __TesterChannel[4] = 3328; }
	}
	__Pin { __Name = CO_; __Ppid = "5"; __XCoord = (240,240); __Shape = 16; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 1383; __TesterChannel[2] = 1400; __TesterChannel[3] = 3303; __TesterChannel[4] = 3320; }
	}
	__Pin { __Name = C_in; __Ppid = "6"; __XCoord = (360,240); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 1381; __TesterChannel[2] = 1398; __TesterChannel[3] = 3301; __TesterChannel[4] = 3318; }
	}
	__Pin { __Name = DOWN; __Ppid = "7"; __XCoord = (120,360); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 1377; __TesterChannel[2] = 1401; __TesterChannel[3] = 3297; __TesterChannel[4] = 3321; }
	}
	__Pin { __Name = D_in; __Ppid = "8"; __XCoord = (240,360); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 1386; __TesterChannel[2] = 1404; __TesterChannel[3] = 3306; __TesterChannel[4] = 3324; }
	}
	__Pin { __Name = LOAD_; __Ppid = "9"; __XCoord = (360,360); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 1388; __TesterChannel[2] = 1399; __TesterChannel[3] = 3308; __TesterChannel[4] = 3319; }
	}
	__Pin { __Name = QA; __Ppid = "10"; __XCoord = (120,480); __Shape = 16; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 1378; __TesterChannel[2] = 1402; __TesterChannel[3] = 3298; __TesterChannel[4] = 3322; }
	}
	__Pin { __Name = QB; __Ppid = "11"; __XCoord = (240,480); __Shape = 16; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 1372; __TesterChannel[2] = 1403; __TesterChannel[3] = 3292; __TesterChannel[4] = 3323; }
	}
	__Pin { __Name = QC; __Ppid = "12"; __XCoord = (360,480); __Shape = 16; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 1375; __TesterChannel[2] = 1393; __TesterChannel[3] = 3295; __TesterChannel[4] = 3313; }
	}
	__Pin { __Name = QD; __Ppid = "13"; __XCoord = (120,600); __Shape = 16; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 1380; __TesterChannel[2] = 1392; __TesterChannel[3] = 3300; __TesterChannel[4] = 3312; }
	}
	__Pin { __Name = UP; __Ppid = "14"; __XCoord = (240,600); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 1382; __TesterChannel[2] = 1394; __TesterChannel[3] = 3302; __TesterChannel[4] = 3314; }
	}
	__Pin { __Name = Gnd; __Ppid = "20"; __XCoord = (120,120); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { __TesterChannel[1] = GND; __TesterChannel[2] = GND; __TesterChannel[3] = GND1; __TesterChannel[4] = GND1; }
	}
	__Pin { __Name = VCC; __Ppid = "15"; __XCoord = (360,600); __Shape = 16; __PinType = Supply_Type; 
		__Connection[0] { __TesterChannel[1] = DPS16_273; __TesterChannel[2] = DPS16_274; __TesterChannel[3] = DPS16_481; __TesterChannel[4] = DPS16_482; }
	}
	__MaxSite = 4;
	__TesterChannelCal 1372 = __Expression { __String = "4.202e-09s"; __Type = s; }
	__TesterChannelCal 1373 = __Expression { __String = "4.239e-09s"; __Type = s; }
	__TesterChannelCal 1375 = __Expression { __String = "4.169e-09s"; __Type = s; }
	__TesterChannelCal 1377 = __Expression { __String = "4.244e-09s"; __Type = s; }
	__TesterChannelCal 1378 = __Expression { __String = "4.167e-09s"; __Type = s; }
	__TesterChannelCal 1380 = __Expression { __String = "4.015e-09s"; __Type = s; }
	__TesterChannelCal 1381 = __Expression { __String = "4.083e-09s"; __Type = s; }
	__TesterChannelCal 1382 = __Expression { __String = "3.999e-09s"; __Type = s; }
	__TesterChannelCal 1383 = __Expression { __String = "4.17e-09s"; __Type = s; }
	__TesterChannelCal 1384 = __Expression { __String = "4.1e-09s"; __Type = s; }
	__TesterChannelCal 1385 = __Expression { __String = "4.159e-09s"; __Type = s; }
	__TesterChannelCal 1386 = __Expression { __String = "3.936e-09s"; __Type = s; }
	__TesterChannelCal 1387 = __Expression { __String = "5e-09s"; __Type = s; }
	__TesterChannelCal 1388 = __Expression { __String = "3.988e-09s"; __Type = s; }
	__TesterChannelCal 1391 = __Expression { __String = "4.134e-09s"; __Type = s; }
	__TesterChannelCal 1392 = __Expression { __String = "4.344e-09s"; __Type = s; }
	__TesterChannelCal 1393 = __Expression { __String = "4.407e-09s"; __Type = s; }
	__TesterChannelCal 1394 = __Expression { __String = "4.612e-09s"; __Type = s; }
	__TesterChannelCal 1395 = __Expression { __String = "9.19e-09s"; __Type = s; }
	__TesterChannelCal 1396 = __Expression { __String = "-7.72e-10s"; __Type = s; }
	__TesterChannelCal 1398 = __Expression { __String = "5.175e-09s"; __Type = s; }
	__TesterChannelCal 1399 = __Expression { __String = "5.231e-09s"; __Type = s; }
	__TesterChannelCal 1400 = __Expression { __String = "5.248e-09s"; __Type = s; }
	__TesterChannelCal 1401 = __Expression { __String = "5.296e-09s"; __Type = s; }
	__TesterChannelCal 1402 = __Expression { __String = "5.21e-09s"; __Type = s; }
	__TesterChannelCal 1403 = __Expression { __String = "5.231e-09s"; __Type = s; }
	__TesterChannelCal 1404 = __Expression { __String = "5.222e-09s"; __Type = s; }
	__TesterChannelCal 1405 = __Expression { __String = "6.005e-09s"; __Type = s; }
	__TesterChannelCal 1407 = __Expression { __String = "5.279e-09s"; __Type = s; }
	__TesterChannelCal 1408 = __Expression { __String = "5.274e-09s"; __Type = s; }
	__TesterChannelCal 1409 = __Expression { __String = "5.235e-09s"; __Type = s; }
	__TesterChannelCal 1412 = __Expression { __String = "1.025e-09s"; __Type = s; }
	__TesterChannelCal 1414 = __Expression { __String = "5.409e-09s"; __Type = s; }
	__TesterChannelCal 3292 = __Expression { __String = "4.202e-09s"; __Type = s; }
	__TesterChannelCal 3293 = __Expression { __String = "4.239e-09s"; __Type = s; }
	__TesterChannelCal 3295 = __Expression { __String = "4.169e-09s"; __Type = s; }
	__TesterChannelCal 3297 = __Expression { __String = "4.244e-09s"; __Type = s; }
	__TesterChannelCal 3298 = __Expression { __String = "4.167e-09s"; __Type = s; }
	__TesterChannelCal 3300 = __Expression { __String = "4.015e-09s"; __Type = s; }
	__TesterChannelCal 3301 = __Expression { __String = "4.083e-09s"; __Type = s; }
	__TesterChannelCal 3302 = __Expression { __String = "3.999e-09s"; __Type = s; }
	__TesterChannelCal 3303 = __Expression { __String = "4.17e-09s"; __Type = s; }
	__TesterChannelCal 3304 = __Expression { __String = "4.1e-09s"; __Type = s; }
	__TesterChannelCal 3305 = __Expression { __String = "4.159e-09s"; __Type = s; }
	__TesterChannelCal 3306 = __Expression { __String = "3.936e-09s"; __Type = s; }
	__TesterChannelCal 3307 = __Expression { __String = "5e-09s"; __Type = s; }
	__TesterChannelCal 3308 = __Expression { __String = "3.988e-09s"; __Type = s; }
	__TesterChannelCal 3311 = __Expression { __String = "4.134e-09s"; __Type = s; }
	__TesterChannelCal 3312 = __Expression { __String = "4.344e-09s"; __Type = s; }
	__TesterChannelCal 3313 = __Expression { __String = "4.407e-09s"; __Type = s; }
	__TesterChannelCal 3314 = __Expression { __String = "4.612e-09s"; __Type = s; }
	__TesterChannelCal 3315 = __Expression { __String = "9.19e-09s"; __Type = s; }
	__TesterChannelCal 3316 = __Expression { __String = "-7.72e-10s"; __Type = s; }
	__TesterChannelCal 3318 = __Expression { __String = "5.175e-09s"; __Type = s; }
	__TesterChannelCal 3319 = __Expression { __String = "5.231e-09s"; __Type = s; }
	__TesterChannelCal 3320 = __Expression { __String = "5.248e-09s"; __Type = s; }
	__TesterChannelCal 3321 = __Expression { __String = "5.296e-09s"; __Type = s; }
	__TesterChannelCal 3322 = __Expression { __String = "5.21e-09s"; __Type = s; }
	__TesterChannelCal 3323 = __Expression { __String = "5.231e-09s"; __Type = s; }
	__TesterChannelCal 3324 = __Expression { __String = "5.222e-09s"; __Type = s; }
	__TesterChannelCal 3327 = __Expression { __String = "5.279e-09s"; __Type = s; }
	__TesterChannelCal 3328 = __Expression { __String = "5.274e-09s"; __Type = s; }
	__TesterChannelCal 3329 = __Expression { __String = "5.235e-09s"; __Type = s; }
	__TesterChannelCal 3332 = __Expression { __String = "1.025e-09s"; __Type = s; }
	__TesterChannelCal 3334 = __Expression { __String = "5.409e-09s"; __Type = s; }
}
__AdapterBoard DMDSeriesSim {
	__Pin { __Name = A_in; __Ppid = "1"; __XCoord = (120,120); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 41; __TesterChannel[2] = 70; __TesterChannel[3] = 905; __TesterChannel[4] = 934; __TesterChannel[5] = 1769; __TesterChannel[6] = 1798; }
	}
	__Pin { __Name = BO_; __Ppid = "2"; __XCoord = (240,120); __Shape = 16; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 40; __TesterChannel[2] = 63; __TesterChannel[3] = 904; __TesterChannel[4] = 927; __TesterChannel[5] = 1768; __TesterChannel[6] = 1791; }
	}
	__Pin { __Name = B_in; __Ppid = "3"; __XCoord = (360,120); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 29; __TesterChannel[2] = 65; __TesterChannel[3] = 893; __TesterChannel[4] = 929; __TesterChannel[5] = 1757; __TesterChannel[6] = 1793; }
	}
	__Pin { __Name = CLR; __Ppid = "4"; __XCoord = (120,240); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 47; __TesterChannel[2] = 64; __TesterChannel[3] = 911; __TesterChannel[4] = 928; __TesterChannel[5] = 1775; __TesterChannel[6] = 1792; }
	}
	__Pin { __Name = CO_; __Ppid = "5"; __XCoord = (240,240); __Shape = 16; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 39; __TesterChannel[2] = 56; __TesterChannel[3] = 903; __TesterChannel[4] = 920; __TesterChannel[5] = 1767; __TesterChannel[6] = 1784; }
	}
	__Pin { __Name = C_in; __Ppid = "6"; __XCoord = (360,240); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 37; __TesterChannel[2] = 54; __TesterChannel[3] = 901; __TesterChannel[4] = 918; __TesterChannel[5] = 1765; __TesterChannel[6] = 1782; }
	}
	__Pin { __Name = DOWN; __Ppid = "7"; __XCoord = (120,360); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 33; __TesterChannel[2] = 57; __TesterChannel[3] = 897; __TesterChannel[4] = 921; __TesterChannel[5] = 1761; __TesterChannel[6] = 1785; }
	}
	__Pin { __Name = D_in; __Ppid = "8"; __XCoord = (240,360); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 42; __TesterChannel[2] = 60; __TesterChannel[3] = 906; __TesterChannel[4] = 924; __TesterChannel[5] = 1770; __TesterChannel[6] = 1788; }
	}
	__Pin { __Name = LOAD_; __Ppid = "9"; __XCoord = (360,360); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 44; __TesterChannel[2] = 55; __TesterChannel[3] = 908; __TesterChannel[4] = 919; __TesterChannel[5] = 1772; __TesterChannel[6] = 1783; }
	}
	__Pin { __Name = QA; __Ppid = "10"; __XCoord = (120,480); __Shape = 16; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 34; __TesterChannel[2] = 58; __TesterChannel[3] = 898; __TesterChannel[4] = 922; __TesterChannel[5] = 1762; __TesterChannel[6] = 1786; }
	}
	__Pin { __Name = QB; __Ppid = "11"; __XCoord = (240,480); __Shape = 16; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 28; __TesterChannel[2] = 59; __TesterChannel[3] = 892; __TesterChannel[4] = 923; __TesterChannel[5] = 1756; __TesterChannel[6] = 1787; }
	}
	__Pin { __Name = QC; __Ppid = "12"; __XCoord = (360,480); __Shape = 16; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 31; __TesterChannel[2] = 49; __TesterChannel[3] = 895; __TesterChannel[4] = 913; __TesterChannel[5] = 1759; __TesterChannel[6] = 1777; }
	}
	__Pin { __Name = QD; __Ppid = "13"; __XCoord = (120,600); __Shape = 16; __PinType = Out_Type; 
		__Connection[0] { __TesterChannel[1] = 36; __TesterChannel[2] = 48; __TesterChannel[3] = 900; __TesterChannel[4] = 912; __TesterChannel[5] = 1764; __TesterChannel[6] = 1776; }
	}
	__Pin { __Name = UP; __Ppid = "14"; __XCoord = (240,600); __Shape = 16; __PinType = In_Type; 
		__Connection[0] { __TesterChannel[1] = 38; __TesterChannel[2] = 50; __TesterChannel[3] = 902; __TesterChannel[4] = 914; __TesterChannel[5] = 1766; __TesterChannel[6] = 1778; }
	}
	__Pin { __Name = Gnd; __Ppid = "20"; __XCoord = (120,120); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { __Resource = shared; __TesterChannel[1] = GND; __TesterChannel[2] = GND; __TesterChannel[3] = GND1; __TesterChannel[4] = GND1; __TesterChannel[5] = GND2; __TesterChannel[6] = GND2; }
	}
	__Pin { __Name = VCC; __Ppid = "15"; __XCoord = (360,600); __Shape = 16; __PinType = Supply_Type; 
		__Connection[0] { __TesterChannel[1] = DPS16_65; __TesterChannel[2] = DPS16_66; __TesterChannel[3] = DPS16_209; __TesterChannel[4] = DPS16_210; __TesterChannel[5] = DPS16_353; __TesterChannel[6] = DPS16_354; }
	}
	__MaxSite = 6;
	__TesterChannelCal 28 = __Expression { __String = "5.83e-10s"; __Type = s; }
	__TesterChannelCal 29 = __Expression { __String = "5.69e-10s"; __Type = s; }
	__TesterChannelCal 31 = __Expression { __String = "5.81e-10s"; __Type = s; }
	__TesterChannelCal 33 = __Expression { __String = "5.77e-10s"; __Type = s; }
	__TesterChannelCal 34 = __Expression { __String = "6.01e-10s"; __Type = s; }
	__TesterChannelCal 36 = __Expression { __String = "5.73e-10s"; __Type = s; }
	__TesterChannelCal 37 = __Expression { __String = "5.22e-10s"; __Type = s; }
	__TesterChannelCal 38 = __Expression { __String = "5.64e-10s"; __Type = s; }
	__TesterChannelCal 39 = __Expression { __String = "5.43e-10s"; __Type = s; }
	__TesterChannelCal 40 = __Expression { __String = "5.62e-10s"; __Type = s; }
	__TesterChannelCal 41 = __Expression { __String = "4.93e-10s"; __Type = s; }
	__TesterChannelCal 42 = __Expression { __String = "5.4e-10s"; __Type = s; }
	__TesterChannelCal 43 = __Expression { __String = "5e-09s"; __Type = s; }
	__TesterChannelCal 44 = __Expression { __String = "5.22e-10s"; __Type = s; }
	__TesterChannelCal 47 = __Expression { __String = "5.239e-10s"; __Type = s; }
	__TesterChannelCal 48 = __Expression { __String = "6.06e-10s"; __Type = s; }
	__TesterChannelCal 49 = __Expression { __String = "6.04e-10s"; __Type = s; }
	__TesterChannelCal 50 = __Expression { __String = "6.25e-10s"; __Type = s; }
	__TesterChannelCal 51 = __Expression { __String = "5.162e-09s"; __Type = s; }
	__TesterChannelCal 52 = __Expression { __String = "-1.324e-09s"; __Type = s; }
	__TesterChannelCal 54 = __Expression { __String = "5.99e-10s"; __Type = s; }
	__TesterChannelCal 55 = __Expression { __String = "6.23e-10s"; __Type = s; }
	__TesterChannelCal 56 = __Expression { __String = "6.11e-10s"; __Type = s; }
	__TesterChannelCal 57 = __Expression { __String = "5.61e-10s"; __Type = s; }
	__TesterChannelCal 58 = __Expression { __String = "5.89e-10s"; __Type = s; }
	__TesterChannelCal 59 = __Expression { __String = "5.72e-10s"; __Type = s; }
	__TesterChannelCal 60 = __Expression { __String = "5.92e-10s"; __Type = s; }
	__TesterChannelCal 61 = __Expression { __String = "1.434e-9s"; __Type = s; }
	__TesterChannelCal 63 = __Expression { __String = "6.25e-10s"; __Type = s; }
	__TesterChannelCal 64 = __Expression { __String = "6.57e-10s"; __Type = s; }
	__TesterChannelCal 65 = __Expression { __String = "5.61e-10s"; __Type = s; }
	__TesterChannelCal 68 = __Expression { __String = "-1.099e-09s"; __Type = s; }
	__TesterChannelCal 70 = __Expression { __String = "6.139e-10s"; __Type = s; }
	__TesterChannelCal 892 = __Expression { __String = "5.83e-10s"; __Type = s; }
	__TesterChannelCal 893 = __Expression { __String = "5.69e-10s"; __Type = s; }
	__TesterChannelCal 895 = __Expression { __String = "5.81e-10s"; __Type = s; }
	__TesterChannelCal 897 = __Expression { __String = "5.77e-10s"; __Type = s; }
	__TesterChannelCal 898 = __Expression { __String = "6.01e-10s"; __Type = s; }
	__TesterChannelCal 900 = __Expression { __String = "5.73e-10s"; __Type = s; }
	__TesterChannelCal 901 = __Expression { __String = "5.22e-10s"; __Type = s; }
	__TesterChannelCal 902 = __Expression { __String = "5.64e-10s"; __Type = s; }
	__TesterChannelCal 903 = __Expression { __String = "5.43e-10s"; __Type = s; }
	__TesterChannelCal 904 = __Expression { __String = "5.62e-10s"; __Type = s; }
	__TesterChannelCal 905 = __Expression { __String = "4.93e-10s"; __Type = s; }
	__TesterChannelCal 906 = __Expression { __String = "5.4e-10s"; __Type = s; }
	__TesterChannelCal 907 = __Expression { __String = "5e-09s"; __Type = s; }
	__TesterChannelCal 908 = __Expression { __String = "5.22e-10s"; __Type = s; }
	__TesterChannelCal 911 = __Expression { __String = "5.239e-10s"; __Type = s; }
	__TesterChannelCal 912 = __Expression { __String = "6.06e-10s"; __Type = s; }
	__TesterChannelCal 913 = __Expression { __String = "6.04e-10s"; __Type = s; }
	__TesterChannelCal 914 = __Expression { __String = "6.25e-10s"; __Type = s; }
	__TesterChannelCal 915 = __Expression { __String = "5.162e-09s"; __Type = s; }
	__TesterChannelCal 916 = __Expression { __String = "-1.324e-09s"; __Type = s; }
	__TesterChannelCal 918 = __Expression { __String = "5.99e-10s"; __Type = s; }
	__TesterChannelCal 919 = __Expression { __String = "6.23e-10s"; __Type = s; }
	__TesterChannelCal 920 = __Expression { __String = "6.11e-10s"; __Type = s; }
	__TesterChannelCal 921 = __Expression { __String = "5.61e-10s"; __Type = s; }
	__TesterChannelCal 922 = __Expression { __String = "5.89e-10s"; __Type = s; }
	__TesterChannelCal 923 = __Expression { __String = "5.72e-10s"; __Type = s; }
	__TesterChannelCal 924 = __Expression { __String = "5.92e-10s"; __Type = s; }
	__TesterChannelCal 927 = __Expression { __String = "6.25e-10s"; __Type = s; }
	__TesterChannelCal 928 = __Expression { __String = "6.57e-10s"; __Type = s; }
	__TesterChannelCal 929 = __Expression { __String = "5.61e-10s"; __Type = s; }
	__TesterChannelCal 932 = __Expression { __String = "-1.099e-09s"; __Type = s; }
	__TesterChannelCal 934 = __Expression { __String = "6.139e-10s"; __Type = s; }
	__TesterChannelCal 1756 = __Expression { __String = "5.83e-10s"; __Type = s; }
	__TesterChannelCal 1757 = __Expression { __String = "5.69e-10s"; __Type = s; }
	__TesterChannelCal 1759 = __Expression { __String = "5.81e-10s"; __Type = s; }
	__TesterChannelCal 1761 = __Expression { __String = "5.77e-10s"; __Type = s; }
	__TesterChannelCal 1762 = __Expression { __String = "6.01e-10s"; __Type = s; }
	__TesterChannelCal 1764 = __Expression { __String = "5.73e-10s"; __Type = s; }
	__TesterChannelCal 1765 = __Expression { __String = "5.22e-10s"; __Type = s; }
	__TesterChannelCal 1766 = __Expression { __String = "5.64e-10s"; __Type = s; }
	__TesterChannelCal 1767 = __Expression { __String = "5.43e-10s"; __Type = s; }
	__TesterChannelCal 1768 = __Expression { __String = "5.62e-10s"; __Type = s; }
	__TesterChannelCal 1769 = __Expression { __String = "4.93e-10s"; __Type = s; }
	__TesterChannelCal 1770 = __Expression { __String = "5.4e-10s"; __Type = s; }
	__TesterChannelCal 1771 = __Expression { __String = "5e-09s"; __Type = s; }
	__TesterChannelCal 1772 = __Expression { __String = "5.22e-10s"; __Type = s; }
	__TesterChannelCal 1775 = __Expression { __String = "5.239e-10s"; __Type = s; }
	__TesterChannelCal 1776 = __Expression { __String = "6.06e-10s"; __Type = s; }
	__TesterChannelCal 1777 = __Expression { __String = "6.04e-10s"; __Type = s; }
	__TesterChannelCal 1778 = __Expression { __String = "6.25e-10s"; __Type = s; }
	__TesterChannelCal 1779 = __Expression { __String = "5.162e-09s"; __Type = s; }
	__TesterChannelCal 1780 = __Expression { __String = "-1.324e-09s"; __Type = s; }
	__TesterChannelCal 1782 = __Expression { __String = "5.99e-10s"; __Type = s; }
	__TesterChannelCal 1783 = __Expression { __String = "6.23e-10s"; __Type = s; }
	__TesterChannelCal 1784 = __Expression { __String = "6.11e-10s"; __Type = s; }
	__TesterChannelCal 1785 = __Expression { __String = "5.61e-10s"; __Type = s; }
	__TesterChannelCal 1786 = __Expression { __String = "5.89e-10s"; __Type = s; }
	__TesterChannelCal 1787 = __Expression { __String = "5.72e-10s"; __Type = s; }
	__TesterChannelCal 1788 = __Expression { __String = "5.92e-10s"; __Type = s; }
	__TesterChannelCal 1791 = __Expression { __String = "6.25e-10s"; __Type = s; }
	__TesterChannelCal 1792 = __Expression { __String = "6.57e-10s"; __Type = s; }
	__TesterChannelCal 1793 = __Expression { __String = "5.61e-10s"; __Type = s; }
	__TesterChannelCal 1796 = __Expression { __String = "-1.099e-09s"; __Type = s; }
	__TesterChannelCal 1798 = __Expression { __String = "6.139e-10s"; __Type = s; }
}
__AdapterBoard Null {
	__Pin { __Name = A_in; __Ppid = "1"; __XCoord = (120,120); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = BO_; __Ppid = "2"; __XCoord = (240,120); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = B_in; __Ppid = "3"; __XCoord = (360,120); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = CLR; __Ppid = "4"; __XCoord = (120,240); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = CO_; __Ppid = "5"; __XCoord = (240,240); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = C_in; __Ppid = "6"; __XCoord = (360,240); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = DOWN; __Ppid = "7"; __XCoord = (120,360); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = D_in; __Ppid = "8"; __XCoord = (240,360); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = LOAD_; __Ppid = "9"; __XCoord = (360,360); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = QA; __Ppid = "10"; __XCoord = (120,480); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = QB; __Ppid = "11"; __XCoord = (240,480); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = QC; __Ppid = "12"; __XCoord = (360,480); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = QD; __Ppid = "13"; __XCoord = (120,600); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = UP; __Ppid = "14"; __XCoord = (240,600); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = Gnd; __Ppid = "20"; __XCoord = (120,120); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__Pin { __Name = VCC; __Ppid = "15"; __XCoord = (360,600); __Shape = 16; __PinType = SharedNullPins; 
		__Connection[0] { }
	}
	__TesterChannelCal 28 = __Expression { __String = "5.83e-10s"; __Type = s; }
	__TesterChannelCal 29 = __Expression { __String = "5.69e-10s"; __Type = s; }
	__TesterChannelCal 31 = __Expression { __String = "5.81e-10s"; __Type = s; }
	__TesterChannelCal 33 = __Expression { __String = "5.77e-10s"; __Type = s; }
	__TesterChannelCal 34 = __Expression { __String = "6.01e-10s"; __Type = s; }
	__TesterChannelCal 36 = __Expression { __String = "5.73e-10s"; __Type = s; }
	__TesterChannelCal 37 = __Expression { __String = "5.22e-10s"; __Type = s; }
	__TesterChannelCal 38 = __Expression { __String = "5.64e-10s"; __Type = s; }
	__TesterChannelCal 39 = __Expression { __String = "5.43e-10s"; __Type = s; }
	__TesterChannelCal 40 = __Expression { __String = "5.62e-10s"; __Type = s; }
	__TesterChannelCal 41 = __Expression { __String = "4.93e-10s"; __Type = s; }
	__TesterChannelCal 42 = __Expression { __String = "5.4e-10s"; __Type = s; }
	__TesterChannelCal 43 = __Expression { __String = "5e-09s"; __Type = s; }
	__TesterChannelCal 44 = __Expression { __String = "5.22e-10s"; __Type = s; }
	__TesterChannelCal 47 = __Expression { __String = "5.239e-10s"; __Type = s; }
	__TesterChannelCal 48 = __Expression { __String = "6.06e-10s"; __Type = s; }
	__TesterChannelCal 49 = __Expression { __String = "6.04e-10s"; __Type = s; }
	__TesterChannelCal 50 = __Expression { __String = "6.25e-10s"; __Type = s; }
	__TesterChannelCal 51 = __Expression { __String = "5.162e-09s"; __Type = s; }
	__TesterChannelCal 52 = __Expression { __String = "-1.324e-09s"; __Type = s; }
	__TesterChannelCal 54 = __Expression { __String = "5.99e-10s"; __Type = s; }
	__TesterChannelCal 55 = __Expression { __String = "6.23e-10s"; __Type = s; }
	__TesterChannelCal 56 = __Expression { __String = "6.11e-10s"; __Type = s; }
	__TesterChannelCal 57 = __Expression { __String = "5.61e-10s"; __Type = s; }
	__TesterChannelCal 58 = __Expression { __String = "5.89e-10s"; __Type = s; }
	__TesterChannelCal 59 = __Expression { __String = "5.72e-10s"; __Type = s; }
	__TesterChannelCal 60 = __Expression { __String = "5.92e-10s"; __Type = s; }
	__TesterChannelCal 61 = __Expression { __String = "1.434e-9s"; __Type = s; }
	__TesterChannelCal 63 = __Expression { __String = "6.25e-10s"; __Type = s; }
	__TesterChannelCal 64 = __Expression { __String = "6.57e-10s"; __Type = s; }
	__TesterChannelCal 65 = __Expression { __String = "5.61e-10s"; __Type = s; }
	__TesterChannelCal 68 = __Expression { __String = "-1.099e-09s"; __Type = s; }
	__TesterChannelCal 70 = __Expression { __String = "6.139e-10s"; __Type = s; }
	__TesterChannelCal 892 = __Expression { __String = "5.83e-10s"; __Type = s; }
	__TesterChannelCal 893 = __Expression { __String = "5.69e-10s"; __Type = s; }
	__TesterChannelCal 895 = __Expression { __String = "5.81e-10s"; __Type = s; }
	__TesterChannelCal 897 = __Expression { __String = "5.77e-10s"; __Type = s; }
	__TesterChannelCal 898 = __Expression { __String = "6.01e-10s"; __Type = s; }
	__TesterChannelCal 900 = __Expression { __String = "5.73e-10s"; __Type = s; }
	__TesterChannelCal 901 = __Expression { __String = "5.22e-10s"; __Type = s; }
	__TesterChannelCal 902 = __Expression { __String = "5.64e-10s"; __Type = s; }
	__TesterChannelCal 903 = __Expression { __String = "5.43e-10s"; __Type = s; }
	__TesterChannelCal 904 = __Expression { __String = "5.62e-10s"; __Type = s; }
	__TesterChannelCal 905 = __Expression { __String = "4.93e-10s"; __Type = s; }
	__TesterChannelCal 906 = __Expression { __String = "5.4e-10s"; __Type = s; }
	__TesterChannelCal 907 = __Expression { __String = "5e-09s"; __Type = s; }
	__TesterChannelCal 908 = __Expression { __String = "5.22e-10s"; __Type = s; }
	__TesterChannelCal 911 = __Expression { __String = "5.239e-10s"; __Type = s; }
	__TesterChannelCal 912 = __Expression { __String = "6.06e-10s"; __Type = s; }
	__TesterChannelCal 913 = __Expression { __String = "6.04e-10s"; __Type = s; }
	__TesterChannelCal 914 = __Expression { __String = "6.25e-10s"; __Type = s; }
	__TesterChannelCal 915 = __Expression { __String = "5.162e-09s"; __Type = s; }
	__TesterChannelCal 916 = __Expression { __String = "-1.324e-09s"; __Type = s; }
	__TesterChannelCal 918 = __Expression { __String = "5.99e-10s"; __Type = s; }
	__TesterChannelCal 919 = __Expression { __String = "6.23e-10s"; __Type = s; }
	__TesterChannelCal 920 = __Expression { __String = "6.11e-10s"; __Type = s; }
	__TesterChannelCal 921 = __Expression { __String = "5.61e-10s"; __Type = s; }
	__TesterChannelCal 922 = __Expression { __String = "5.89e-10s"; __Type = s; }
	__TesterChannelCal 923 = __Expression { __String = "5.72e-10s"; __Type = s; }
	__TesterChannelCal 924 = __Expression { __String = "5.92e-10s"; __Type = s; }
	__TesterChannelCal 927 = __Expression { __String = "6.25e-10s"; __Type = s; }
	__TesterChannelCal 928 = __Expression { __String = "6.57e-10s"; __Type = s; }
	__TesterChannelCal 929 = __Expression { __String = "5.61e-10s"; __Type = s; }
	__TesterChannelCal 932 = __Expression { __String = "-1.099e-09s"; __Type = s; }
	__TesterChannelCal 934 = __Expression { __String = "6.139e-10s"; __Type = s; }
	__TesterChannelCal 1756 = __Expression { __String = "5.83e-10s"; __Type = s; }
	__TesterChannelCal 1757 = __Expression { __String = "5.69e-10s"; __Type = s; }
	__TesterChannelCal 1759 = __Expression { __String = "5.81e-10s"; __Type = s; }
	__TesterChannelCal 1761 = __Expression { __String = "5.77e-10s"; __Type = s; }
	__TesterChannelCal 1762 = __Expression { __String = "6.01e-10s"; __Type = s; }
	__TesterChannelCal 1764 = __Expression { __String = "5.73e-10s"; __Type = s; }
	__TesterChannelCal 1765 = __Expression { __String = "5.22e-10s"; __Type = s; }
	__TesterChannelCal 1766 = __Expression { __String = "5.64e-10s"; __Type = s; }
	__TesterChannelCal 1767 = __Expression { __String = "5.43e-10s"; __Type = s; }
	__TesterChannelCal 1768 = __Expression { __String = "5.62e-10s"; __Type = s; }
	__TesterChannelCal 1769 = __Expression { __String = "4.93e-10s"; __Type = s; }
	__TesterChannelCal 1770 = __Expression { __String = "5.4e-10s"; __Type = s; }
	__TesterChannelCal 1771 = __Expression { __String = "5e-09s"; __Type = s; }
	__TesterChannelCal 1772 = __Expression { __String = "5.22e-10s"; __Type = s; }
	__TesterChannelCal 1775 = __Expression { __String = "5.239e-10s"; __Type = s; }
	__TesterChannelCal 1776 = __Expression { __String = "6.06e-10s"; __Type = s; }
	__TesterChannelCal 1777 = __Expression { __String = "6.04e-10s"; __Type = s; }
	__TesterChannelCal 1778 = __Expression { __String = "6.25e-10s"; __Type = s; }
	__TesterChannelCal 1779 = __Expression { __String = "5.162e-09s"; __Type = s; }
	__TesterChannelCal 1780 = __Expression { __String = "-1.324e-09s"; __Type = s; }
	__TesterChannelCal 1782 = __Expression { __String = "5.99e-10s"; __Type = s; }
	__TesterChannelCal 1783 = __Expression { __String = "6.23e-10s"; __Type = s; }
	__TesterChannelCal 1784 = __Expression { __String = "6.11e-10s"; __Type = s; }
	__TesterChannelCal 1785 = __Expression { __String = "5.61e-10s"; __Type = s; }
	__TesterChannelCal 1786 = __Expression { __String = "5.89e-10s"; __Type = s; }
	__TesterChannelCal 1787 = __Expression { __String = "5.72e-10s"; __Type = s; }
	__TesterChannelCal 1788 = __Expression { __String = "5.92e-10s"; __Type = s; }
	__TesterChannelCal 1791 = __Expression { __String = "6.25e-10s"; __Type = s; }
	__TesterChannelCal 1792 = __Expression { __String = "6.57e-10s"; __Type = s; }
	__TesterChannelCal 1793 = __Expression { __String = "5.61e-10s"; __Type = s; }
	__TesterChannelCal 1796 = __Expression { __String = "-1.099e-09s"; __Type = s; }
	__TesterChannelCal 1798 = __Expression { __String = "6.139e-10s"; __Type = s; }
}
