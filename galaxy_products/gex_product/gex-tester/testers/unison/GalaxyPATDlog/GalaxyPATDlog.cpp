// *************************************************************************************
//  Module      : GalaxyPATDlog.cpp
//  Description : GTL Datalog method
//
//  Copyright (C) LTX-Credence Corporation 2015.  All rights reserved.
// *************************************************************************************

#include "GalaxyPATDlog.h"
#include <Unison.h>
#include "gtl_core.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <fstream>

using namespace std;
// Note to anyone making additions to the list of formatters
// Make sure the formatters have unique first characters.  Code below has a shortcut that
// only compares the first character of the formatter name for performance reasons.
static const char *formats[] = {"GTL", 0};
const int GTL_INDEX = 0;        // must match formats array above

// Set following define to 0 to hide GTL/plugin debug messages
#define GTL_DEBUG 0

// Following global variables are used to store Galaxy PAT datalog settings.
// CHECKME: it might make sense to move them into an existing or new object.
int SPATColumnIndex = -1;
int DPATColumnIndex = -1;

// Message types
enum MessageType
{
    eGTL,       // Message from GTL
    eDEBUG,     // Plugin Debug message
    eDLOG       // Datalogger message
};


// Folllowing global functions are utility functions when interacting with the GTL.
void GTL_DisplayMessage(const StringS &str, const MessageType type);
int  GTL_FlushMessages();
int  GTL_Set(char * key, char * value, bool flush=true);
int  GTL_Get(char * key, char * value, bool flush=true);
int  GTL_Command(char * command, bool flush=true);
bool GTL_ModifyLimits(const SITE site, const int ColumnIndex, const unsigned int ArraySize, const long int *TestNb, const unsigned int *Flags, const double *LowLimits, const double *HighLimits);

bool GTL_CheckPatConfig();
TMResultM ModifyLimits(LimitStruct & lim, const FloatM & newll, const FloatM & newhl) ;

DATALOG_METHOD_CLASS(GalaxyPATDlog);

GalaxyPATDlog::
GalaxyPATDlog() :
DatalogMethod(formats),
SummaryNeeded(false),
// UnitAutoscaling(),
NumTestsExecuted(0),
FinishTime(UTL_VOID)
{
    //    RegisterAttribute(UnitAutoscaling, "UnitAutoscaling", true);
    
    RegisterEvent(GetSystemEventName(DatalogMethod::StartOfTest), &GalaxyPATDlog::StartOfTest);
    RegisterEvent(GetSystemEventName(DatalogMethod::EndOfTest), &GalaxyPATDlog::EndOfTest);
    RegisterEvent(GetSystemEventName(DatalogMethod::ProgramLoad), &GalaxyPATDlog::ProgramLoad);
    RegisterEvent(GetSystemEventName(DatalogMethod::ProgramUnload), &GalaxyPATDlog::ProgramUnload);
    RegisterEvent(GetSystemEventName(DatalogMethod::ProgramReset), &GalaxyPATDlog::ProgramReset);
    RegisterEvent(GetSystemEventName(DatalogMethod::Summary), &GalaxyPATDlog::Summary);
    RegisterEvent(GetSystemEventName(DatalogMethod::StartOfWafer), &GalaxyPATDlog::StartOfWafer);
    RegisterEvent(GetSystemEventName(DatalogMethod::EndOfWafer), &GalaxyPATDlog::EndOfWafer);
    RegisterEvent(GetSystemEventName(DatalogMethod::StartOfLot), &GalaxyPATDlog::StartOfLot);
    RegisterEvent(GetSystemEventName(DatalogMethod::EndOfLot), &GalaxyPATDlog::EndOfLot);
    RegisterEvent(GetSystemEventName(DatalogMethod::StartTestNode), &GalaxyPATDlog::StartTestNode);
    RegisterEvent(GetSystemEventName(DatalogMethod::StartTestBlock), &GalaxyPATDlog::StartTestBlock);
    RegisterEvent(GetSystemEventName(DatalogMethod::ParametricTest), &GalaxyPATDlog::ParametricTest);
    RegisterEvent(GetSystemEventName(DatalogMethod::ParametricTestArray), &GalaxyPATDlog::ParametricTestArray);
    RegisterEvent(GetSystemEventName(DatalogMethod::FunctionalTest), &GalaxyPATDlog::FunctionalTest);
    RegisterEvent(GetSystemEventName(DatalogMethod::Text), &GalaxyPATDlog::Text);
    RegisterEvent(GetSystemEventName(DatalogMethod::Generic), &GalaxyPATDlog::Generic);
}

GalaxyPATDlog::
~GalaxyPATDlog()
{
}

bool GalaxyPATDlog::
GetSummaryNeeded() const
{
    return SummaryNeeded;
}

// *****************************************************************************
// *****************************************************************************
// GalaxyPATDlogData
// The following class is an extension for the DatalogData that contains common collection
// members used for the GalaxyPATDlogMethod data.
class GalaxyPATDlogData : public DatalogData {
public:
    GalaxyPATDlogData(DatalogMethod::SystemEvents event, GalaxyPATDlog &parent);
    virtual ~GalaxyPATDlogData() = 0;
    
    // public methods for reading the members
    //bool GetUnitAutoscaling() const;
    const FloatS &GetDlogTime() const;
    const DatalogMethod::SystemEvents GetEvent() const;
    void ResetNumTestsExecuted();
    void IncNumTestsExecuted();
    unsigned int GetNumTestsExecuted(SITE site) const;
    void SetFinishTime();
    const FloatS &GetFinishTime() const;
    static void GetChar(const StringS str, char * retval);
    
protected:
    // The members will be collected upon construction
    FloatS DlogTime;                // Time a collection
    DatalogMethod::SystemEvents Event;        // Event type
    GalaxyPATDlog *Parent;                // My parent object
    
    void SetSummaryNeeded(bool is_needed);
    void FormatTestDescription(StringS &str, const StringS &user_desc) const;
    //    STDFV4Stream GetSTDFV4Stream(bool make_private) const;
private:
    GalaxyPATDlogData();                // disable default constructor
    GalaxyPATDlogData(const GalaxyPATDlogData &);    // disable copy
    GalaxyPATDlogData &operator=(const GalaxyPATDlogData &);    // disable copy
};

GalaxyPATDlogData::
GalaxyPATDlogData(DatalogMethod::SystemEvents event, GalaxyPATDlog &parent) :
DatalogData(),
DlogTime(RunTime.GetCurrentLocalTime()),
Event(event),
Parent(&parent)
{
}

GalaxyPATDlogData::
~GalaxyPATDlogData()
{
}

/*
 bool GalaxyPATDlogData::
 GetUnitAutoscaling() const
 {
 if (Parent != NULL)
 return Parent -> UnitAutoscaling.GetValue();
 return true;
 }
 */

const FloatS &GalaxyPATDlogData::
GetDlogTime() const
{
    return DlogTime;
}

const DatalogMethod::SystemEvents GalaxyPATDlogData::
GetEvent() const
{
    return Event;
}

void GalaxyPATDlogData::
GetChar(const StringS str, char * retval)
{
    int size = str.Length();
    for (int i=0; i < size; ++i) {
        retval[i] = str[i];
    }
    retval[size] = '\00';
}

void GTL_DisplayMessage(const StringS &str, const MessageType type)
{
    switch(type) {
        case eGTL:
            cerr << "GTL - " << str << endl;
            break;
            
        case eDEBUG:
#if GTL_DEBUG
            cerr << "DEBUG - " << str << endl;
#endif
            break;
            
        case eDLOG:
            cerr << "DLOG - " << str << endl;
            break;
            
        default:
            cerr << str << endl;
            break;
    }
}

int GTL_FlushMessages()
{
    char lNumMsg[20]="?";
    char lMsg[GTL_MESSAGE_STRING_SIZE+100];
    
    // Retrieve number of messages in the stack
    gtl_get(GTL_KEY_NUM_OF_MESSAGES_IN_STACK, lNumMsg);
    int n=0;
    int r=sscanf(lNumMsg, "%d", &n);
    if (r!=1)
        return -1;
    
    // Is stack empty?
    if (n==0)
        return 0;
    
    sprintf(lMsg, "%d messages retrieved from stack", n);
    GTL_DisplayMessage(lMsg, eDEBUG);
    
    // Retrieve and display all messages in the stack
    int     severity=0, messageID=0;
    char    gtl_string[1024]="?";
    char    message[GTL_MESSAGE_STRING_SIZE]="?"; // currently 1 message is 1024 chars
    for (int i=0; i<n; i++)
    {
        int r=gtl_command((char*)GTL_COMAND_POP_FIRST_MESSAGE);
        if (r==0)
        {
            gtl_get(GTL_KEY_CURRENT_MESSAGE, message);
            gtl_get(GTL_KEY_CURRENT_MESSAGE_SEV, gtl_string);
            sscanf(gtl_string, "%d", &severity);
            gtl_get(GTL_KEY_CURRENT_MESSAGE_ID, gtl_string);
            sscanf(gtl_string, "%d", &messageID);
            sprintf(lMsg, "%d\t%d\t%s", severity, messageID, message);
            GTL_DisplayMessage(lMsg, eGTL);
        }
        else
        {
            sprintf(lMsg, "Failed to pop message (%d)", r);
            GTL_DisplayMessage(lMsg, eGTL);
            return r;
        }
    }
    gtl_command((char*)GTL_COMMAND_CLEAR_MESSAGES_STACK);
    return 0;
}

int GTL_Get(char * key, char * value, bool flush)
{
    char lMsg[100];
    
    int r = gtl_get(key, value);
    
    if(flush)
        GTL_FlushMessages();
    
    if(r != GTL_CORE_ERR_OKAY)
    {
        sprintf(lMsg, "gtl_get(%s) failed (%d)", key, r);
        GTL_DisplayMessage(lMsg, eGTL);
    }
    
    return r;
}

int GTL_Set(char * key, char * value, bool flush)
{
    char lMsg[100];
    
    int r = gtl_set(key, value);
    
    if(flush)
        GTL_FlushMessages();
    
    if(r != GTL_CORE_ERR_OKAY)
    {
        sprintf(lMsg, "gtl_set(%s, %s) failed (%d)", key, value, r);
        GTL_DisplayMessage(lMsg, eGTL);
    }
    
    return r;
}

int GTL_Command(char * command, bool flush)
{
    char lMsg[100];
    
    int r = gtl_command(command);
    
    if(flush)
        GTL_FlushMessages();
    
    if(r != GTL_CORE_ERR_OKAY)
    {
        sprintf(lMsg, "gtl_command(%s) failed (%d)", command, r);
        GTL_DisplayMessage(lMsg, eGTL);
    }
    
    return r;
}

void GalaxyPATDlogData::
SetSummaryNeeded(bool is_needed)
{
    if (Parent != NULL)
        Parent -> SummaryNeeded = is_needed;
}

void GalaxyPATDlogData::
ResetNumTestsExecuted()
{
    if (Parent != NULL)
        Parent -> NumTestsExecuted = 0;
}

void GalaxyPATDlogData::
IncNumTestsExecuted()
{
    if (Parent != NULL) {
        for (SiteIter s1 = GetDlogSites().Begin(); !s1.End(); ++s1)
            ++(Parent -> NumTestsExecuted[*s1]);
    }
}

unsigned int GalaxyPATDlogData::
GetNumTestsExecuted(SITE site) const
{
    if (Parent != NULL)
        return Parent -> NumTestsExecuted[site];
    return 0;
}

void GalaxyPATDlogData::
SetFinishTime()
{
    if (Parent != NULL)
        Parent -> FinishTime = DlogTime;
}

const FloatS &GalaxyPATDlogData::
GetFinishTime() const
{
    if (Parent != NULL)
        return Parent -> FinishTime;
    return UTL_VOID;
}

/*
 STDFV4Stream GalaxyPATDlogData::
 GetSTDFV4Stream(bool make_private) const
 {
 if (Parent != NULL)
 return Parent -> GetSTDFV4Stream(make_private);
 return STDFV4Stream::NullSTDFV4Stream;
 }
 */

void GalaxyPATDlogData::
FormatTestDescription(StringS &str, const  StringS &user_info) const
{
    if (user_info.Valid() && (user_info.Length() > 0)) {
        str = user_info;
        return;
    }
    if (Parent != NULL) {
        const StringS &TN = GetTestName();
        if (TN.Valid() && (TN.Length() > 0)) {
            str = TN;
            const StringS &BN = GetBlockName();
            if (BN.Valid() && (BN.Length() > 0)) {
                str += "/";
                str += BN;
            }
            return;
        }
    }
    str = user_info;
}

// *****************************************************************************
// StartOfTest

class StartOfTestData : public GalaxyPATDlogData {
public:
    StartOfTestData(GalaxyPATDlog &);
    ~StartOfTestData();
    
    virtual void Format(const char *format, bool fail_only_mode, std::ostream &output);
private:
    Sites SelSites;
    StringS TesterType;
    void FormatGTL(bool fail_only_mode, std::ostream &output);
};

StartOfTestData::
StartOfTestData(GalaxyPATDlog &parent) :
GalaxyPATDlogData(DatalogMethod::StartOfTest, parent),
SelSites(SelectedSites),
TesterType(SYS.GetTestHeadType())
{
    ResetNumTestsExecuted();
}

StartOfTestData::
~StartOfTestData()
{
}

void StartOfTestData::
Format(const char *format, bool fail_only_mode, std::ostream &output)
{
    if (format != NULL) {
        if (format[0] == formats[GTL_INDEX][0])
            FormatGTL(fail_only_mode, output);
    }
}

void StartOfTestData::
FormatGTL(bool fail_only_mode, std::ostream &output)
{
    // CHECKME: (for debug only)
#if GTL_DEBUG
    MATH.Randomize(0);
#endif

    GTL_Command((char *)GTL_COMMAND_BEGINJOB);
}

static char GetCode(const char *field)
{
    if (field != NULL) {
        StringS str = TestProg.GetLotInfo(field);
        if (str.Length() > 0)
            return str[0];
    }
    return ' ';
}

DatalogData *GalaxyPATDlog::
StartOfTest(const DatalogBaseUserData *)
{
    SummaryNeeded = true;
    return new StartOfTestData(*this);
}

// *****************************************************************************
// EndOfTest

class EndOfTestData : public GalaxyPATDlogData {
public:
    EndOfTestData(GalaxyPATDlog &);
    ~EndOfTestData();
    
    virtual void Format(const char *format, bool fail_only_mode, std::ostream &output);
private:
    EndOfTestStruct EOT;
    bool Valid;
    Sites SelSites;
    void FormatGTL(bool fail_only_mode, std::ostream &output);
    void DumpLimitStruct(const LimitStruct &lim) const;
};

// ---------- Galaxy Semi Begin
void EndOfTestData::
DumpLimitStruct(const LimitStruct &lim) const
{
    int nbLimitSet = lim.GetNumberOfLimitSets();
    
    StringS lMsg;
    lMsg = StringS("Test ") + lim.GetTestNumber() + StringS(" (") + lim .GetName() + StringS(") has ") + nbLimitSet + StringS(" limit sets.");
    GTL_DisplayMessage(lMsg, eDEBUG);
    for(int i = 0; i < nbLimitSet; ++i)
        lMsg = StringS("Limit set ") + i + StringS(": LL=[") + lim.GetLowLimit(i).GetFloatM() + "]; HL=[" + lim.GetHighLimit(i).GetFloatM() + "];  BIN=" + lim.GetBin(i).GetBinNumber();
}

bool GTL_CheckPatConfig()
{
    StringS lMsg;
    
    // Make sure SPAT and DPAT columns are either -1 or in the valid range of limi sets
    int lNbLimiSets = TestProg.GetActiveLimitTable().GetNumberOfLimits();
    
    if(SPATColumnIndex < -1) {
        lMsg = StringS("SPAT value in your PAT.conf file (") + IntS(SPATColumnIndex).GetText() + StringS(") must be >= -1!");
        GTL_DisplayMessage(lMsg, eDLOG);
        return false;
    }
    
    if(SPATColumnIndex >= lNbLimiSets) {
        lMsg = StringS("SPAT value in your PAT.conf file (") + IntS(SPATColumnIndex).GetText() + StringS(") must be < nb limit sets (");
        lMsg += IntS(lNbLimiSets).GetText() + StringS(")!");
        GTL_DisplayMessage(lMsg, eDLOG);
        return false;
    }
    
    if(DPATColumnIndex < -1) {
        lMsg = StringS("DPAT value in your PAT.conf file (") + IntS(DPATColumnIndex).GetText() + StringS(") must be >= -1!");
        GTL_DisplayMessage(lMsg, eDLOG);
        return false;
    }
    
    if(DPATColumnIndex >= lNbLimiSets) {
        lMsg = StringS("DPAT value in your PAT.conf file (") + IntS(DPATColumnIndex).GetText() + StringS(") must be < nb limit sets (");
        lMsg += IntS(lNbLimiSets).GetText() + StringS(")!");
        GTL_DisplayMessage(lMsg, eDLOG);
        return false;
    }
    
    if(SPATColumnIndex >= DPATColumnIndex) {
        lMsg = StringS("SPAT value in your PAT.conf file (") + IntS(DPATColumnIndex).GetText();
        lMsg += StringS(") must be < DPAT value in your PAT.conf file (") + IntS(DPATColumnIndex).GetText() + StringS(")!");
        GTL_DisplayMessage(lMsg, eDLOG);
        return false;
    }
    
    return true;
}

// ---------- Galaxy Semi End

EndOfTestData::
EndOfTestData(GalaxyPATDlog &parent) :
GalaxyPATDlogData(DatalogMethod::EndOfTest, parent),
Valid(false),
EOT(),
SelSites(SelectedSites)
{
    // Overload DPAT limits if DPATColumnIndex > -1
    if(DPATColumnIndex > -1) {
        Valid = RunTime.GetEndOfTestData(EOT);
        LimitTable lims = TestProg.GetActiveLimitTable();
        
        // ======== Galaxy Semi Begin =======
        unsigned int    ArraysSize = 5000;
        long            TestNb[ArraysSize];
        double          LowLimits[ArraysSize];
        double          HighLimits[ArraysSize];
        unsigned int    Flags[ArraysSize];
        unsigned int    HardBins[ArraysSize];
        unsigned int    SoftBins[ArraysSize];
        unsigned int    ArrayFilledSize = 0;
        char            lMsg[1024];
        
        SitesContext context(LOADED_SITES_TYPE);
        for (SiteIter s1=LoadedSites.Begin(); !s1.End(); ++s1) {
            int status = gtl_get_dpat_limits((unsigned int)s1.GetValue() , TestNb, Flags, LowLimits, HighLimits, HardBins, SoftBins, ArraysSize, &ArrayFilledSize);
            if(status == GTL_CORE_ERR_OKAY && ArrayFilledSize) {
                sprintf(lMsg, "Received DPAT limits on site %d for %d tests", (int)s1.GetValue(), ArrayFilledSize);
                GTL_DisplayMessage(StringS(lMsg), eGTL);
                
                // Modify limits for this test/site
                GTL_ModifyLimits(s1.GetValue(), DPATColumnIndex, ArrayFilledSize, TestNb, Flags, LowLimits, HighLimits);
                
                // ===============================================
                
                /*SITE site = s1.GetValue();
                 StringS     lMsg;
                 char        lCharray[1024];
                 LimitTable  lims = TestProg.GetActiveLimitTable();
                 
                 SitesContext context(LOADED_SITES_TYPE);
                 cerr << " ColumnIndex : " << DPATColumnIndex << endl;
                 for(int i = 0; i < ArrayFilledSize; ++i) {
                 // Let's dump test ID information
                 sprintf(lCharray, "Test %d: Flags=%d; LL=%g; HL=%g", TestNb[i], Flags[i], LowLimits[i], HighLimits[i]);
                 GTL_DisplayMessage(lCharray, eDEBUG);
                 
                 // Let's get limits structure for this test
                 LimitStruct lim = lims.FindLimit(TestNb[i]);
                 
                 if (!lim.Valid()) {
                 ERR.ReportError(ERR_GENERIC_ADVISORY,"PAT Error: attempting to set non-existent limit!",IntS(TestNb[i]),site,UTL_VOID);
                 continue;
                 }
                 
                 //Check if enough columns have been configured for this test
                 if(DPATColumnIndex > lim.GetNumberOfLimitSets() - 1) {
                 ERR.ReportError(ERR_GENERIC_ADVISORY,"PAT Error: not enough columns set in you limit table file for this tesy!",IntS(TestNb[i]),site,UTL_VOID);
                 continue;
                 }
                 
                 // Get current limits
                 FloatM ll = lim.GetLowLimit(DPATColumnIndex).GetFloatM();
                 FloatM hl = lim.GetHighLimit(DPATColumnIndex).GetFloatM();
                 
                 
                 cerr << "ll[site] :" << ll[site] <<  endl;
                 if(LowLimits[i] > (-GTL_INFINITE_VALUE_FLOAT + 1.0)) {
                 // Get LL and overload with DPAT LL for this site
                 GTL_DisplayMessage("trace 1", eDEBUG);
                 ll[site] = FloatS(LowLimits[i]);
                 }
                 
                 if(HighLimits[i] < (GTL_INFINITE_VALUE_FLOAT - 1.0)) {
                 // Get HL and overload with DPAT HL for this site
                 hl[site] = FloatS(HighLimits[i]);
                 }
                 
                 // Display limits
                 lMsg = StringS("Overload   limit set ") + IntS(DPATColumnIndex).GetText() + StringS(" for test ") + lim.GetTestNumber().GetText() + StringS(": LL=[") + ll + StringS("]; HL=[") + hl + StringS("]");
                 GTL_DisplayMessage(lMsg, eDEBUG);
                 
                 // Overload limits
                 lim.SetLowLimit(DPATColumnIndex,ll);
                 lim.SetHighLimit(DPATColumnIndex,hl);
                 
                 // Display overloaded limits
                 FloatM lNewLL = lim.GetLowLimit(DPATColumnIndex).GetFloatM();
                 FloatM lNewHL = lim.GetHighLimit(DPATColumnIndex).GetFloatM();
                 lMsg = StringS("Overloaded limit set ") + IntS(DPATColumnIndex).GetText() + StringS(" for test ") + lim.GetTestNumber().GetText() + StringS(": LL=[") + lNewLL + StringS("]; HL=[") + lNewHL + StringS("]");
                 GTL_DisplayMessage(lMsg, eDEBUG);
                 }
                 // ==============================================
                 */
            }
        }
    }
    
    SetFinishTime();
}

TMResultM ModifyLimits(LimitStruct & lim, const FloatM & newll, const FloatM & newhl)
{
    TMResultM retval = TM_PASS;
    cerr << "TMResultM - Change to GTL limits : Lo=" << newll << ", Hi=" << newhl << endl;
    lim.SetLowLimit(0,newll);
    lim.SetHighLimit(0,newhl);
    FloatM gll = lim.GetLowLimit(0).GetFloatM();
    FloatM ghl = lim.GetHighLimit(0).GetFloatM();
    cerr << "TMResultM - After                 : Lo=" << gll << ", Hi=" << ghl << endl;
    
    return   retval;
}

EndOfTestData::
~EndOfTestData()
{
}

void EndOfTestData::
Format(const char *format, bool fail_only_mode, std::ostream &output)
{
    if (format != NULL) {
        if (format[0] == formats[GTL_INDEX][0])
            FormatGTL(fail_only_mode, output);
    }
}

static bool IsSelectedSite(SITE site)
// Checks to see if the site passed in is in the SelectedSites list for this run
{
    for (SiteIter s1 = SelectedSites.Begin(); !s1.End(); ++s1) {
        if (*s1 == site) return true;
    }
    return false;
}

void EndOfTestData::
FormatGTL(bool fail_only_mode, std::ostream &output)
{
    char string[1024*5];
    
    for (SiteIter s1 = LoadedSites.Begin(); !s1.End(); ++s1) {
        if (IsSelectedSite(*s1)) {
            sprintf(string, "%d", (int)EOT.SerialNumbers[*s1]);
            //                cerr << " PartId : " << string << " - HBIN: " <<  EOT.HardwareBinNumbers[*s1] << " -SBIN: " << EOT.SoftwareBinNumbers[*s1] << endl;
            gtl_set_binning((int)*s1, EOT.HardwareBinNumbers[*s1], EOT.SoftwareBinNumbers[*s1], string);
            
            
            //            int old_hbin = EOT.HardwareBinNumbers[*s1];
            //            int old_sbin = EOT.SoftwareBinNumbers[*s1];
            //            int new_hbin;
            //            int new_sbin;
            //            gtl_binning(UnsignedS(*s1), old_hbin ,old_sbin , &new_hbin, &new_sbin,"NotSure");
            //            /*if(old_hbin != new_hbin)
            //                cerr << "new_hbin :" << new_hbin <<"(" << old_hbin << ")"  << endl;
            //
            //            if(old_sbin != new_sbin)
            //                cerr << "new_sbin :" << new_sbin <<"(" << old_sbin << ")"  << endl;    */
        }
    }
    GTL_Command((char *)GTL_COMMAND_ENDJOB);
}

DatalogData *GalaxyPATDlog::
EndOfTest(const DatalogBaseUserData *)
{
    SummaryNeeded = true;
    return new EndOfTestData(*this);
}

// *****************************************************************************
// ProgramLoad

DatalogData *GalaxyPATDlog::
ProgramLoad(const DatalogBaseUserData *)
{
    return NULL;        // Not used  by GalaxyPATDlog
}

// *****************************************************************************
// ProgramUnload

DatalogData *GalaxyPATDlog::
ProgramUnload(const DatalogBaseUserData *)
{
    if (SummaryNeeded)        // insure my summary has been processed
        DoAction(GetSystemEventName(DatalogMethod::Summary));
    return NULL;
}

// *****************************************************************************
// ProgramReset

class ProgramResetData : public GalaxyPATDlogData {
public:
    ProgramResetData(GalaxyPATDlog &);
    ~ProgramResetData();
    
    virtual void Format(const char *format, bool fail_only_mode, std::ostream &output);
private:
    EndOfTestStruct EOT;
    bool Valid;
    Sites SelSites;
    void FormatGTL(bool fail_only_mode, std::ostream &output);
};

ProgramResetData::
ProgramResetData(GalaxyPATDlog &parent) :
GalaxyPATDlogData(DatalogMethod::ProgramReset, parent),
Valid(false),
EOT(),
SelSites(SelectedSites)
{
    Valid = RunTime.GetEndOfTestData(EOT);
    SetFinishTime();
}

ProgramResetData::
~ProgramResetData()
{
}

void ProgramResetData::
Format(const char *format, bool fail_only_mode, std::ostream &output)
{
    if (format != NULL) {
        if (format[0] == formats[GTL_INDEX][0])
            FormatGTL(fail_only_mode, output);
    }
}

void ProgramResetData::
FormatGTL(bool fail_only_mode, std::ostream &output)
{
}

DatalogData *GalaxyPATDlog::
ProgramReset(const DatalogBaseUserData *)
{
    SummaryNeeded = true;
    return new ProgramResetData(*this);
}

// *****************************************************************************
// Summary

class SummaryData : public GalaxyPATDlogData {
public:
    SummaryData(GalaxyPATDlog &, bool is_final = false, bool file_closing_after_summary = false);
    ~SummaryData();
    
    virtual void Format(const char *format, bool fail_only_mode, std::ostream &output);
    void BinASCII(SITE site) const;
private:
    bool IsFinalSummary;
    bool FileClosingAfterSummary;
    bool Valid;
    bool TSRValid;
    // Lot Info
    StringS FileName;
    StringS ProgName;
    StringS UserName;
    StringS DUTName;
    StringS LotID;
    StringS SublotID;
    StringS LotStat;
    StringS LotType;
    StringS LotDesc;
    StringS ProdID;
    StringS WaferID;
    StringS FabID;
    StringS LotStart;
    StringS Shift;
    StringS Operator;
    StringS TesterName;
    StringS FlowName;
    StringS DIBName;
    StringS CurLocalTime;
    StringS CurGMTime;
    StringS PHName;
    StringS SumHdr;
    // Bin Info
    BinInfoArrayStruct BinInfo;
    HWBinInfoArrayStruct HWBinInfo;
    // Pass/Fail totals
    BinCountStruct Passes;
    BinCountStruct Fails;
    TSRInfoStruct TSRInfo;
    
    
    void FormatGTL(bool fail_only_mode, std::ostream &output);
};

SummaryData::
SummaryData(GalaxyPATDlog &parent, bool is_final, bool file_closing_after_summary) :
GalaxyPATDlogData(DatalogMethod::Summary, parent),
IsFinalSummary(is_final),
FileClosingAfterSummary(file_closing_after_summary),
Valid(false),
BinInfo(),
HWBinInfo(),
Passes(),
Fails(),
FileName(),
ProgName(),
UserName(),
DUTName(),
LotID(),
SublotID(),
LotStat(),
LotType(),
LotDesc(),
ProdID(),
WaferID(),
FabID(),
LotStart(),
Shift(),
Operator(),
TesterName(),
FlowName(),
DIBName(),
CurLocalTime(),
CurGMTime(),
PHName(),
SumHdr()
{
    Valid = RunTime.GetBinInfo(BinInfo, Passes, Fails);
    if (Valid) {
        (void) RunTime.GetBinInfo(HWBinInfo);
        FileName = TestProg.GetLotInfo("TestProgFileName");
        ProgName = TestProg.GetLotInfo("ProgramName");
        UserName = TestProg.GetLotInfo("UserName");
        DUTName = TestProg.GetLotInfo("DeviceName");
        LotID = TestProg.GetLotInfo("LotID");
        SublotID = TestProg.GetLotInfo("SubLotID");
        LotStat = TestProg.GetLotInfo("LotStatus");
        LotType = TestProg.GetLotInfo("LotType");
        LotDesc = TestProg.GetLotInfo("LotDescription");
        ProdID = TestProg.GetLotInfo("ProductID");
        WaferID = TestProg.GetLotInfo("WaferID");
        FabID = TestProg.GetLotInfo("FabID");
        LotStart = TestProg.GetLotInfo("StartTime");
        Operator = TestProg.GetLotInfo("OperatorID");
        TesterName = TestProg.GetLotInfo("TesterName");
        FlowName = TestProg.GetLotInfo("ActiveFlow");
        DIBName = TestProg.GetLotInfo("ActiveLoadBoard");
        PHName = TestProg.GetLotInfo("ProberHander");
        SumHdr = TestProg.GetLotInfo("SummaryHeader");
        CurLocalTime = TestProg.GetCurrentLocalTime();
        CurGMTime = TestProg.GetCurrentGMTime();
    }
    RunTime.GetTSRInformation(TSRInfo);
    TSRValid = (TSRInfo.TestNum.GetSize() > 0) ? true : false;
}

SummaryData::
~SummaryData()
{
}

void SummaryData::
Format(const char *format, bool fail_only_mode, std::ostream &output)
{
    if (format != NULL) {
        if (format[0] == formats[GTL_INDEX][0])
            FormatGTL(fail_only_mode, output);
    }
}

void SummaryData::
FormatGTL(bool fail_only_mode, std::ostream &output)
{
}

DatalogData *GalaxyPATDlog::
Summary(const DatalogBaseUserData *udata)
{
    const DatalogSummaryInfo *sdata = dynamic_cast<const DatalogSummaryInfo *>(udata);
    SummaryNeeded = false;
    bool DoFinal = (sdata != NULL) ? (sdata -> GetPartialSummary() ? false : true) : false;
    bool FileClosingAfterSummary = sdata ? sdata->GetFileClosingAfterSummary() : false;
    return new SummaryData(*this, DoFinal, FileClosingAfterSummary);
}

// *****************************************************************************
// StartOfWafer

class StartOfWaferData : public GalaxyPATDlogData {
public:
    StartOfWaferData(GalaxyPATDlog &);
    ~StartOfWaferData();
    
    virtual void Format(const char *format, bool fail_only_mode, std::ostream &output);
private:
    bool Valid;
    WaferMap WMap;
    StringS WaferID;
    
    void FormatGTL(bool fail_only_mode, std::ostream &output);
};

StartOfWaferData::
StartOfWaferData(GalaxyPATDlog &parent) :
GalaxyPATDlogData(DatalogMethod::StartOfWafer, parent),
Valid(false),
WMap(TestProg.GetActiveWaferMap()),
WaferID(TestProg.GetLotInfo("WaferID"))
{
    Valid = (WMap.Valid() && (WaferID.Length() > 0));
}

StartOfWaferData::
~StartOfWaferData()
{
}

void StartOfWaferData::
Format(const char *format, bool fail_only_mode, std::ostream &output)
{
    if (format != NULL) {
        if (format[0] == formats[GTL_INDEX][0])
            FormatGTL(fail_only_mode, output);
    }
}

static const StringS FormatTime(const FloatS &time)
{
    if (time != UTL_VOID) {
        char buff[8192];
        time_t wtime = time;
        struct tm tm_var;
        asctime_r(gmtime_r(&wtime, &tm_var), buff);
        return buff;
    }
    return "";
}

static const StringS SafeString(const StringS &str)
{
    if (str != UTL_VOID)
        return str;
    return "";
}

void StartOfWaferData::
FormatGTL(bool fail_only_mode, std::ostream &output)
{
}

static char GetDirectionChar(const WaferDirectionS &dir)
{
    switch(dir) {
        case WAFER_DIR_LEFT:    return 'L';
        case WAFER_DIR_RIGHT:    return 'R';
        case WAFER_DIR_TOP:     return 'U';
        case WAFER_DIR_BOTTOM:    return 'D';
        default:        break;
    }
    return ' ';
}

DatalogData *GalaxyPATDlog::
StartOfWafer(const DatalogBaseUserData *)
{
    return new StartOfWaferData(*this);
}

// *****************************************************************************
// EndOfWafer

class EndOfWaferData : public GalaxyPATDlogData {
public:
    EndOfWaferData(GalaxyPATDlog &);
    ~EndOfWaferData();
    
    virtual void Format(const char *format, bool fail_only_mode, std::ostream &output);
private:
    bool Valid;
    WaferInfoStruct WaferInfo;
    
    void FormatGTL(bool fail_only_mode, std::ostream &output);
};

EndOfWaferData::
EndOfWaferData(GalaxyPATDlog &parent) :
GalaxyPATDlogData(DatalogMethod::EndOfWafer, parent),
Valid(false),
WaferInfo()
{
    Valid = RunTime.GetWaferInfo(WaferInfo);
}

EndOfWaferData::
~EndOfWaferData()
{
}

void EndOfWaferData::
Format(const char *format, bool fail_only_mode, std::ostream &output)
{
    if (format != NULL) {
        if (format[0] == formats[GTL_INDEX][0])
            FormatGTL(fail_only_mode, output);
    }
}

void EndOfWaferData::
FormatGTL(bool fail_only_mode, std::ostream &output)
{
}

DatalogData *GalaxyPATDlog::
EndOfWafer(const DatalogBaseUserData *)
{
    return new EndOfWaferData(*this);
}

// *****************************************************************************
// StartOfLot

class StartOfLotData : public GalaxyPATDlogData {
public:
    StartOfLotData(GalaxyPATDlog &);
    ~StartOfLotData();
    
    virtual void Format(const char *format, bool fail_only_mode, std::ostream &output);
private:
    Sites SelSites;
    StringS TesterType;
    void FormatGTL(bool fail_only_mode, std::ostream &output);
    bool GTL_Init();
};

StartOfLotData::
StartOfLotData(GalaxyPATDlog &parent) :
GalaxyPATDlogData(DatalogMethod::StartOfLot, parent),
SelSites(SelectedSites),
TesterType(SYS.GetTestHeadType())
{
    ResetNumTestsExecuted();
}

StartOfLotData::
~StartOfLotData()
{
}

void StartOfLotData::
Format(const char *format, bool fail_only_mode, std::ostream &output)
{
    if (format != NULL) {
        if (format[0] == formats[GTL_INDEX][0])
            FormatGTL(fail_only_mode, output);
    }
}

bool StartOfLotData::
GTL_Init()
{
    // Set some mandatory GTL keys
    // Setting the max mem for GTL messages :
    // currently a message is 1032 octet... so 30000 bytes will allow 30 messages max in the stack
    GTL_Set((char*)GTL_KEY_MAX_MESSAGES_STACK_SIZE, (char*)"90064");
    
    GTL_DisplayMessage("ReadPatConfigurationFile (./PAT.conf)", eDLOG);
    
    ifstream patFile("./PAT.conf");
    if(patFile.is_open()) {
        char buffer[255];
        patFile.getline(buffer, 255);
        while(!patFile.eof()) {
            string stringBuffer(buffer);
            if(stringBuffer.empty() == false && stringBuffer[0] != '#' ) {
                size_t pos = stringBuffer.find_first_of("=");
                if(pos != string::npos) {
                    string tag = stringBuffer.substr(0, pos);
                    //cerr << " tag : " << tag.c_str() << endl;
                    string value = stringBuffer.substr(pos + 1, stringBuffer.size() -1 );
                    //cerr << " value : " << value.c_str() << endl;
                    
                    if( tag == "SPAT") {
                        SPATColumnIndex = atoi(value.c_str());
                    }
                    else if( tag == "DPAT") {
                        DPATColumnIndex = atoi(value.c_str());
                    }
                    else if( tag == "Recipe") {
                        //cerr << tag.c_str() << ": " << value.c_str() << endl;
                        GTL_Set((char*)GTL_KEY_RECIPE_FILE, const_cast<char*>(value.c_str()));
                    }
                    else if( tag == "GTLOutputFolder") {
                        //cerr << tag.c_str() << ": " << value.c_str() << endl;
                        GTL_Set((char*)GTL_KEY_OUTPUT_FOLDER, const_cast<char*>(value.c_str()));
                    }
                    else if( tag == "GTLOutputFileName") {
                        //cerr << tag.c_str() << ": " << value.c_str() << endl;
                        GTL_Set((char*)GTL_KEY_OUTPUT_FILENAME, const_cast<char*>(value.c_str()));
                    }
                    else if( tag == "GTLConf") {
                        //cerr << tag.c_str() << ": " << value.c_str() << endl;
                        GTL_Set((char*)GTL_KEY_CONFIG_FILE, const_cast<char*>(value.c_str()));
                    }
                }
            }
            
            patFile.getline(buffer, 255);
        }
    }
    else
    {
        cerr << " file './PAT.conf' not found" << endl;
        return false;
    }
    
    return true;
}

void StartOfLotData::
FormatGTL(bool fail_only_mode, std::ostream &output)
{
    // Let's check GTL version
    char lLV[255]="?";
    gtl_get((char*)GTL_KEY_LIB_VERSION, lLV);
    
    // Display GTL version
    StringS lMsg = StringS("GTL version is ") + StringS(lLV);
    GTL_DisplayMessage(lMsg, eGTL);
    
    GTL_Init();
    LimitTable lims = TestProg.GetActiveLimitTable();
    // check if the index specified and the limit table set are matching
    //cerr << " lims.GetDefaultLimit().GetNumberOfLimitSets()): " << lims.GetDefaultLimit().GetNumberOfLimitSets() << endl;
    lMsg = StringS("SPATColumnIndex=") + IntS(SPATColumnIndex).GetText();
    GTL_DisplayMessage(lMsg, eGTL);
    lMsg = StringS("DPATColumnIndex=") + IntS(DPATColumnIndex).GetText();
    GTL_DisplayMessage(lMsg, eGTL);
    
    // Check PAT configuration
    if(!GTL_CheckPatConfig())
    {
        lMsg = StringS("PAT configuration issue. PAT will be disabled!");
        GTL_DisplayMessage(lMsg, eDLOG);
        return; // Is there another way to do that ?
    }
    
    StringS LotID = TestProg.GetLotInfo("LotID");
    StringS DevName = TestProg.GetLotInfo("DeviceName");
    
    char string[1024*5];
    GetChar(LotID,string);
    GTL_Set((char*)GTL_KEY_LOT_ID, string);
    GetChar(DevName,string);
    GTL_Set((char*)GTL_KEY_PRODUCT_ID, string);
    // CHECKME: use real values from OS
    GTL_Set((char*)GTL_KEY_TESTER_TYPE, (char*)"Unison");
    GTL_Set((char*)GTL_KEY_TESTER_NAME, (char*)"Simulation");
    
    char sites_nb_command[] = GTL_KEY_MAX_NUMBER_OF_ACTIVE_SITES;
    StringS max_sites;
    IO.Print(max_sites,"%d",IntS(LoadedSites.GetNumSites()));
    char nsites[256];
    GetChar(max_sites,nsites);
    GTL_Set(sites_nb_command, nsites);
    
    
    // CHECKME: (for debug only)
#if GTL_DEBUG
    GTL_Set((char*)GTL_KEY_RELOAD_LIMITS, (char *)"off");
#endif
    
    char sites_command[] = GTL_KEY_SITES_NUMBERS;
    StringS sites;
    for (SiteIter s1=LoadedSites.Begin(); !s1.End(); ++s1)
    {
        sites += IntS(*s1).GetText() + " ";
    }
    char sites_string[1024*5];
    GetChar(sites,sites_string);
    GTL_Set(sites_command, sites_string);
    
    GTL_Command((char *)GTL_COMMAND_OPEN);
    
    // Overload SPAT limits if SPATColumnIndex > -1
    if(SPATColumnIndex > -1) {
        Sites           current = ActiveSites;
        //LimitTable lims = TestProg.GetActiveLimitTable();
        long            ArraysSize = 5000;
        long            TestNb[ArraysSize];
        double          LowLimits[ArraysSize];
        double          HighLimits[ArraysSize];
        unsigned int    Flags[ArraysSize];
        unsigned int    HardBins[ArraysSize];
        unsigned int    SoftBins[ArraysSize];
        unsigned int    ArrayFilledSize = 0;
        char            lCharray[1024];
        
        SitesContext context(LOADED_SITES_TYPE);
        for (SiteIter s1=LoadedSites.Begin(); !s1.End(); ++s1) {
            int status = gtl_get_spat_limits((unsigned int)s1.GetValue() , TestNb, Flags, LowLimits, HighLimits, HardBins, SoftBins, ArraysSize, &ArrayFilledSize);
            if(status == GTL_CORE_ERR_OKAY && ArrayFilledSize) {
                sprintf(lCharray, "Received SPAT limits on site %d for %d tests", (int)s1.GetValue(), ArrayFilledSize);
                GTL_DisplayMessage(StringS(lCharray), eGTL);
                
                // Modify limits for this test/site
                GTL_ModifyLimits(s1.GetValue(), SPATColumnIndex, ArrayFilledSize, TestNb, Flags, LowLimits, HighLimits);
            }
        }
        RunTime.SetActiveSites(current);
    }
    
    SetFinishTime();
}

DatalogData *GalaxyPATDlog::
StartOfLot(const DatalogBaseUserData *)
{
    GTL_DisplayMessage(StringS("StartOfLot"), eDEBUG);
    SummaryNeeded = true;
    return new StartOfLotData(*this);
}

// *****************************************************************************
// EndOfLot

DatalogData *GalaxyPATDlog::
EndOfLot(const DatalogBaseUserData *)
{
    GTL_DisplayMessage(StringS("EndOfLot"), eDEBUG);
    GTL_Command((char *)GTL_COMMAND_CLOSE);
    return NULL;            // Processed as summary
}

// *****************************************************************************
// StartTestNode

class StartTestNodeData : public GalaxyPATDlogData {
public:
    StartTestNodeData(GalaxyPATDlog &);
    ~StartTestNodeData();
    
    virtual void Format(const char *format, bool fail_only_mode, std::ostream &output);
private:
};

StartTestNodeData::
StartTestNodeData(GalaxyPATDlog &parent) :
GalaxyPATDlogData(DatalogMethod::StartTestNode, parent)
{
}

StartTestNodeData::
~StartTestNodeData()
{
}

void StartTestNodeData::
Format(const char *format, bool fail_only_mode, std::ostream &output)
{
}

DatalogData *GalaxyPATDlog::
StartTestNode(const DatalogBaseUserData *)
{
    return new StartTestNodeData(*this);
}

// *****************************************************************************
// StartTestBlock

DatalogData *GalaxyPATDlog::
StartTestBlock(const DatalogBaseUserData *)
{
    return NULL;                // not used
}

// *****************************************************************************
// ParametricTest

class ParametricTestData : public GalaxyPATDlogData {
public:
    ParametricTestData(GalaxyPATDlog &, const DatalogParametric &pdata);
    ~ParametricTestData();
    
    virtual void Format(const char *format, bool fail_only_mode, std::ostream &output);
private:
    DatalogParametric PData;
    
    void FormatGTL(bool fail_only_mode, std::ostream &output);
};

ParametricTestData::
ParametricTestData(GalaxyPATDlog &parent, const DatalogParametric &pdata) :
GalaxyPATDlogData(DatalogMethod::ParametricTest, parent),
PData(pdata)
{
    IncNumTestsExecuted();
}

ParametricTestData::
~ParametricTestData()
{
}

void ParametricTestData::
Format(const char *format, bool fail_only_mode, std::ostream &output)
{
    if (format != NULL) {
        if (format[0] == formats[GTL_INDEX][0])
            FormatGTL(fail_only_mode, output);
    }
}

void ParametricTestData::
FormatGTL(bool fail_only_mode, std::ostream &output)
{
    Sites fsites = GetDlogSites();
    char comment[256];
    for (SiteIter s = fsites.Begin(); !s.End(); ++s) {
        const BasicVar &TV = PData.GetBaseSData(DatalogParametric::Test, *s);
        double tval;
        if (TV.GetType() == SV_INT)
            tval = TV.GetIntS();
        else if (TV.GetType() == SV_FLOAT)
            tval = TV.GetFloatS();
        GetChar(PData.GetComment(),comment);
        int status = gtl_test(int(*s), long (PData.GetTestID()), comment, tval);
    }
}

static const char *GetDefaultFormat(const BasicVar &var)
{
    if ((var.GetType() == SV_INT) || (var.GetType() == SV_UINT))
        return "%9.0f";
    return "%9.3f";
}

// left here just in case
/*
 void ParametricTestData::FormatSTDFV4(bool fail_only_mode, std::ostream &output)
 {
 STDFV4Stream STDF = GetSTDFV4Stream(false);
 if (STDF.Valid()) {
 STDFV4_PTR PTR;
 Sites fsites = GetDlogSites();
 const TMResultM &Res = PData.GetResult();
 if (fail_only_mode)
 (void)fsites.DisableFailingSites(Res.Equal(TM_FAIL));    // This removes anything that is not a fail due to Equal
 const StringS &units = PData.GetUnits();
 StringS real_units, str, tdesc;
 StringS testText = PData.GetComment();
 
 if (GetAppendPinName()) DatalogData::AppendPinNameToTestText(PData.GetPins(), testText);
 
 FormatTestDescription(tdesc, testText);
 double scale = PData.CalculateBaseUnitScale(units, real_units);
 if ( scale == 0.0 && !GetUnitAutoscaling() ) scale = 1.0;
 PTR.SetInfo(PData.GetTestID(), tdesc);
 PTR.SetUnits(real_units);
 for (SiteIter s1 = fsites.Begin(); !s1.End(); ++s1) {
 SITE site = *s1;
 const BasicVar &TV = PData.GetBaseSData(DatalogParametric::Test, site);
 if (TV != UTL_VOID) {
 const BasicVar &LL = PData.GetBaseSData(DatalogParametric::LowLimit, site);
 const BasicVar &HL = PData.GetBaseSData(DatalogParametric::HighLimit, site);
 double real_scale = (scale != 0.0) ? scale : PData.CalculateAutoRangeUnitScale(units, real_units, TV, LL, HL);
 if (real_scale != 0.0)
 real_scale = 1.0 / real_scale;            // STDF routine wants value, not multiplier
 const char *fmt = GetDefaultFormat(TV);
 PTR.SetContext(site);
 PTR.SetResult(Res[site], TV, real_scale, fmt);
 PTR.SetLimit(STDFV4_PTR::PTR_LO_LIMIT, LL, real_scale, fmt);
 PTR.SetLimit(STDFV4_PTR::PTR_HI_LIMIT, HL, real_scale, fmt);
 STDF.Write(PTR);
 }
 }
 }
 }
 */

DatalogData *GalaxyPATDlog::
ParametricTest(const DatalogBaseUserData *udata)
{
    const DatalogParametric *pdata = dynamic_cast<const DatalogParametric *>(udata);
    if (pdata != NULL) {
        SummaryNeeded = true;
        return new ParametricTestData(*this, *pdata);
    }
    return NULL;
}

// *****************************************************************************
// ParametricTestArray

class ParametricTestDataArray : public GalaxyPATDlogData {
public:
    ParametricTestDataArray(GalaxyPATDlog &, const DatalogParametricArray &);
    ~ParametricTestDataArray();
    
    virtual void Format(const char *format, bool fail_only_mode, std::ostream &output);
private:
    DatalogParametricArray PData;
    
    void FormatGTL(bool fail_only_mode, std::ostream &output);
};

ParametricTestDataArray::
ParametricTestDataArray(GalaxyPATDlog &parent, const DatalogParametricArray &pdata) :
GalaxyPATDlogData(DatalogMethod::ParametricTestArray, parent),
PData(pdata)
{
    IncNumTestsExecuted();
}

ParametricTestDataArray::
~ParametricTestDataArray()
{
}

void ParametricTestDataArray::
Format(const char *format, bool fail_only_mode, std::ostream &output)
{
    if (format != NULL) {
        if (format[0] == formats[GTL_INDEX][0])
            FormatGTL(fail_only_mode, output);
    }
}

void ParametricTestDataArray::
FormatGTL(bool fail_only_mode, std::ostream &output)
{
    Sites fsites = GetDlogSites();
    char comment[256];
    GetChar(PData.GetComment(),comment);
    
    const TMResultM1D &Res1D = PData.GetResults();
    const Sites &dlog_sites = GetDlogSites();
    const StringS &units = PData.GetUnits();
    StringS real_units, str, tdesc;
    FormatTestDescription(tdesc, PData.GetComment());
    const PinML &Pins = PData.GetPins();
    int npins = Pins.GetNumPins();
    int pin_indexes[npins];
    for (int i = 0; i < npins; ++i) {
        pin_indexes[i] = i;
    }
    int nvalues = PData.GetNumValues(DatalogParametricArray::Test);
    double result[nvalues];
    
    for (SiteIter s1 = dlog_sites.Begin(); !s1.End(); ++s1) {
        SITE site = *s1;
        const TMResultS1D &Res = Res1D[site];
        unsigned int test_id = PData.GetTestID();
        for (int ii = 0; ii < nvalues; ++ii) {
            BasicVar TV, LL, HL;
            PData.StuffSData(TV, DatalogParametricArray::Test, ii, site);
            PData.StuffSData(LL, DatalogParametricArray::LowLimit, ii, site);
            PData.StuffSData(HL, DatalogParametricArray::HighLimit, ii, site);
            double tval;
            if (TV.GetType() == SV_INT)
                tval = TV.GetIntS();
            else if (TV.GetType() == SV_FLOAT)
                tval = TV.GetFloatS();
            result[ii] = tval;
        }
        int status = gtl_mptest(int(*s1), long (PData.GetTestID()), comment, result, pin_indexes, long(nvalues));
    }
}

static bool PerPinLimits(const BasicVar &LL, const BasicVar &HL)
{
    if ((LL != UTL_VOID) && (LL.GetConfig() == SV_ARRAY_S1D)) {
        switch(LL.GetType()) {
            case SV_FLOAT:
            {
                const FloatS1D &sv = LL.GetFloatS1D();
                if ((sv.GetSize() > 1) && ((sv == sv[0]) == false))
                    return true;
            }
                break;
            case SV_INT:
            {
                const IntS1D &sv = LL.GetIntS1D();
                if ((sv.GetSize() > 1) && ((sv == sv[0]) == false))
                    return true;
            }
                break;
            case SV_UINT:
            {
                const UnsignedS1D &sv = LL.GetUnsignedS1D();
                if ((sv.GetSize() > 1) && ((sv == sv[0]) == false))
                    return true;
            }
                break;
            default:
                break;
        }
    }
    if ((HL != UTL_VOID) && (HL.GetConfig() == SV_ARRAY_S1D)) {
        switch(HL.GetType()) {
            case SV_FLOAT:
            {
                const FloatS1D &sv = HL.GetFloatS1D();
                if ((sv.GetSize() > 1) && ((sv == sv[0]) == false))
                    return true;
            }
                break;
            case SV_INT:
            {
                const IntS1D &sv = HL.GetIntS1D();
                if ((sv.GetSize() > 1) && ((sv == sv[0]) == false))
                    return true;
            }
                break;
            case SV_UINT:
            {
                const UnsignedS1D &sv = HL.GetUnsignedS1D();
                if ((sv.GetSize() > 1) && ((sv == sv[0]) == false))
                    return true;
            }
                break;
            default:
                break;
        }
    }
    return false;
}

static int GetArrayLength(const BasicVar &TV)
{
    if ((TV != UTL_VOID) && (TV.GetConfig() == SV_ARRAY_S1D)) {
        switch(TV.GetType()) {
            case SV_FLOAT:
            {
                const FloatS1D &sv = TV.GetFloatS1D();
                return sv.GetSize();
            }
                break;
            case SV_INT:
            {
                const IntS1D &sv = TV.GetIntS1D();
                return sv.GetSize();
            }
                break;
            case SV_UINT:
            {
                const UnsignedS1D &sv = TV.GetUnsignedS1D();
                return sv.GetSize();
            }
                break;
            default:
                break;
        }
    }
    return 0;
}

// left here just in case
/*void ParametricTestDataArray::
 FormatSTDFV4(bool fail_only_mode, std::ostream &output)
 {
 STDFV4Stream STDF = GetSTDFV4Stream(false);
 if (STDF.Valid()) {
 STDFV4_MPR MPR;
 Sites fsites = GetDlogSites();
 const TMResultM1D &Res1D = PData.GetResults();
 const Sites &dlog_sites = GetDlogSites();
 const StringS &units = PData.GetUnits();
 StringS real_units, str, tdesc;
 FormatTestDescription(tdesc, PData.GetComment());
 double scale = PData.CalculateBaseUnitScale(units, real_units);
 if ( scale == 0.0 && !GetUnitAutoscaling() ) scale = 1.0;
 MPR.SetInfo(PData.GetTestID(), tdesc);
 MPR.SetUnits(real_units);
 for (SiteIter s1 = dlog_sites.Begin(); !s1.End(); ++s1) {
 SITE site = *s1;
 const BasicVar &TV = PData.GetBaseS1DData(DatalogParametricArray::Test, site);
 const BasicVar &LL = PData.GetBaseS1DData(DatalogParametricArray::LowLimit, site);
 const BasicVar &HL = PData.GetBaseS1DData(DatalogParametricArray::HighLimit, site);
 const char *fmt = GetDefaultFormat(TV);
 double real_scale = (scale != 0.0) ? scale : PData.CalculateAutoRangeUnitScale(real_units, str, TV, LL, HL);
 if (real_scale != 0.0)
 real_scale = 1.0 / real_scale;                // STDF routine wants value, not multiplier
 if (PerPinLimits(LL, HL)) {                    // Implement as an array of PTRs
 const PinML &pins = PData.GetPins();
 int num_vals = GetArrayLength(TV);
 int num_low = GetArrayLength(LL);
 int num_high = GetArrayLength(HL);
 int num_pins = pins.GetNumPins();
 STDFV4_PTR PTR;
 PTR.SetInfo(PData.GetTestID(), tdesc);
 PTR.SetUnits(real_units);
 PTR.SetContext(site);
 BasicVar BV;
 for (int ii = 0; ii < num_vals; ii++) {
 PData.StuffSData(BV, DatalogParametricArray::Test, ii, site);
 if (BV.Valid()) {
 PTR.SetResult(Res1D[site][ii], BV, real_scale, fmt);
 BV = UTL_VOID;
 if (num_low > 0)
 PData.StuffSData(BV, DatalogParametricArray::LowLimit, (num_low == 1) ? 0 : ii, site);
 PTR.SetLimit(STDFV4_PTR::PTR_LO_LIMIT, BV, real_scale, fmt);
 BV = UTL_VOID;
 if (num_high > 0)
 PData.StuffSData(BV, DatalogParametricArray::HighLimit, (num_high == 1) ? 0 : ii, site);
 PTR.SetLimit(STDFV4_PTR::PTR_HI_LIMIT, BV, real_scale, fmt);
 STDF.Write(PTR);
 }
 }
 }
 else {
 MPR.SetContext(site);
 MPR.SetResult(PData.GetPins(), Res1D[site], TV, true, real_scale, fmt);
 MPR.SetLimit(STDFV4_MPR::MPR_LO_LIMIT, LL, real_scale, fmt);
 MPR.SetLimit(STDFV4_MPR::MPR_HI_LIMIT, HL, real_scale, fmt);
 STDF.Write(MPR);
 }
 }
 }
 }
 */

DatalogData *GalaxyPATDlog::
ParametricTestArray(const DatalogBaseUserData *udata)
{
    const DatalogParametricArray *pdata = dynamic_cast<const DatalogParametricArray *>(udata);
    if (pdata != NULL) {
        SummaryNeeded = true;
        return new ParametricTestDataArray(*this, *pdata);
    }
    return NULL;
}

// *****************************************************************************
// FunctionalTest

class FunctionalTestData : public GalaxyPATDlogData {
public:
    FunctionalTestData(GalaxyPATDlog &, const DatalogFunctional &);
    ~FunctionalTestData();
    
    virtual void Format(const char *format, bool fail_only_mode, std::ostream &output);
    const DatalogFunctional &GetFuncData() const;
    const IntS &GetMaxNumFails() const;
    const DigitalPatternInfoStruct &GetPatInfo() const;
    const DigitalPatternPinStruct &GetPatPinInfo() const;
private:
    DatalogFunctional FData;            // data passed in from Functional BIF
    IntS MaxNumFails;                // requested maximum number of fails, user can cause extra collection
    DigitalPatternInfoStruct PatInfo;        // data collected from DIGITAL driver
    DigitalPatternPinStruct PatPinInfo;        // data collected from DIGITAL driver
    void FormatGTL(bool fail_only_mode, std::ostream &output);
};

FunctionalTestData::
FunctionalTestData(GalaxyPATDlog &parent, const DatalogFunctional &fdata) :
GalaxyPATDlogData(DatalogMethod::FunctionalTest, parent),
FData(fdata),
MaxNumFails(TestProg.GetNumberOfFunctionalFails()),
PatInfo(DIGITAL.GetPatternInfo()),
PatPinInfo(DIGITAL.GetPatternPinInfo())
{
    IncNumTestsExecuted();
}

FunctionalTestData::
~FunctionalTestData()
{
}

void FunctionalTestData::
Format(const char *format, bool fail_only_mode, std::ostream &output)
{
    if ((format != NULL) && (PatInfo.NumRecords != 0)) {
        if (format[0] == formats[GTL_INDEX][0])
            FormatGTL(fail_only_mode, output);
    }
}

void FunctionalTestData::
FormatGTL(bool fail_only_mode, std::ostream &output)
{
    //int gtl_test(unsigned int SiteNb, long TestNb, char *TestName, double Result)
}

DatalogData *GalaxyPATDlog::
FunctionalTest(const DatalogBaseUserData *udata)
{
    const DatalogFunctional *fdata = dynamic_cast<const DatalogFunctional *>(udata);
    if (fdata != NULL) {
        SummaryNeeded = true;
        return new FunctionalTestData(*this, *fdata);
    }
    return NULL;
}

// *****************************************************************************
// Text

class TextData : public GalaxyPATDlogData {
public:
    TextData(GalaxyPATDlog &, const DatalogText &);
    ~TextData();
    
    virtual void Format(const char *format, bool fail_only_mode, std::ostream &output);
private:
    DatalogText TData;
    
    void FormatGTL(bool fail_only_mode, std::ostream &output);
};

TextData::
TextData(GalaxyPATDlog &parent, const DatalogText &tdata) :
GalaxyPATDlogData(DatalogMethod::Text, parent),
TData(tdata)
{
}

TextData::
~TextData()
{
}

void TextData::
Format(const char *format, bool fail_only_mode, std::ostream &output)
{
    if (format != NULL) {
        if (format[0] == formats[GTL_INDEX][0])
            FormatGTL(fail_only_mode, output);
    }
}

void TextData::
FormatGTL(bool fail_only_mode, std::ostream &output)
{
}

DatalogData *GalaxyPATDlog::
Text(const DatalogBaseUserData *udata)
{
    const DatalogText *tdata = dynamic_cast<const DatalogText *>(udata);
    if (tdata != NULL) {
        SummaryNeeded = true;
        return new TextData(*this, *tdata);
    }
    return NULL;
}

// *****************************************************************************
// Generic

class GenericData : public GalaxyPATDlogData {
public:
    GenericData(GalaxyPATDlog &, const DatalogGeneric &);
    ~GenericData();
    
    virtual void Format(const char *format, bool fail_only_mode, std::ostream &output);
private:
    DatalogGeneric GData;
    
    void FormatGTL(bool fail_only_mode, std::ostream &output);
};

GenericData::
GenericData(GalaxyPATDlog &parent, const DatalogGeneric &gdata) :
GalaxyPATDlogData(DatalogMethod::Generic, parent),
GData(gdata)
{
}

GenericData::
~GenericData()
{
}

void GenericData::
Format(const char *format, bool fail_only_mode, std::ostream &output)
{
    if (format != NULL) {
        if (format[0] == formats[GTL_INDEX][0])
            FormatGTL(fail_only_mode, output);
    }
}

void GenericData::
FormatGTL(bool fail_only_mode, std::ostream &output)
{
}

DatalogData *GalaxyPATDlog::
Generic(const DatalogBaseUserData *udata)
{
    return NULL;
}

bool GTL_ModifyLimits(const SITE site, const int ColumnIndex, const unsigned int ArraySize, const long int *TestNb, const unsigned int *Flags, const double *LowLimits, const double *HighLimits)
{
    StringS     lMsg;
    char        lCharray[1024];
    LimitTable  lims = TestProg.GetActiveLimitTable();
    
    for(int i = 0; i < ArraySize; ++i) {
        // Let's dump test ID information
        sprintf(lCharray, "Test %d: Flags=%d; LL=%g; HL=%g", TestNb[i], Flags[i], LowLimits[i], HighLimits[i]);
        GTL_DisplayMessage(lCharray, eDEBUG);
        
        // Let's get limits structure for this test
        LimitStruct lim = lims.FindLimit(TestNb[i]);
        
        if (!lim.Valid()) {
            
            ERR.ReportError(ERR_GENERIC_ADVISORY,"PAT Error: attempting to set non-existent limit!",IntS(TestNb[i]),site,UTL_VOID);
            continue;
        }
        
        //Check if enough columns have been configured for this test
        if(ColumnIndex > lim.GetNumberOfLimitSets() - 1) {
            
            ERR.ReportError(ERR_GENERIC_ADVISORY,"PAT Error: not enough columns set in you limit table file for this tesy!",IntS(TestNb[i]),site,UTL_VOID);
            continue;
        }
      
        // -----  workaround (to solve the pb of site's limit that are not
        // -----  updated) To force their update the site 1 must be update too
        FloatS lld, hld;
        if( site != SITE_1)
        {
            FloatM  ll = lim.GetLowLimit(ColumnIndex).GetFloatM();
            FloatM  hl = lim.GetHighLimit(ColumnIndex).GetFloatM();
            lld    = ll[SITE_1];
            hld    = hl[SITE_1];
            
            ll[SITE_1] = -101;
            hl[SITE_1] = 101;
            
            lim.SetLowLimit (ColumnIndex,ll);
            lim.SetHighLimit(ColumnIndex,hl);
        }
        
        // -------------------------------------
        // Get current limits
        FloatM ll = lim.GetLowLimit(ColumnIndex).GetFloatM();
        FloatM hl = lim.GetHighLimit(ColumnIndex).GetFloatM();
        
        // -----  workaround: re-set the previous value for site 1
        if( site != SITE_1)  {
            ll[SITE_1] = lld;
            hl[SITE_1] = hld;
        }
        // -----
        
        if(LowLimits[i] > (-GTL_INFINITE_VALUE_FLOAT + 1.0)) {
            // Get LL and overload with PAT LL for this site
            ll[site] = LowLimits[i];
        }
        
        if(HighLimits[i] < (GTL_INFINITE_VALUE_FLOAT - 1.0)) {
            // Get HL and overload with PAT HL for this site
            hl[site] = HighLimits[i];
        }
        
        // Display limits
        lMsg = StringS("Overload   limit set ") + IntS(ColumnIndex).GetText() + StringS(" for test ") + lim.GetTestNumber().GetText() + StringS(": LL=[") + ll + StringS("]; HL=[") + hl + StringS("]");
        GTL_DisplayMessage(lMsg, eDEBUG);
        
        // Overload limits
        lim.SetLowLimit(ColumnIndex,ll);
        lim.SetHighLimit(ColumnIndex,hl);
        
        // Display overloaded limits
        FloatM lNewLL = lim.GetLowLimit(ColumnIndex).GetFloatM();
        FloatM lNewHL = lim.GetHighLimit(ColumnIndex).GetFloatM();
        lMsg = StringS("Overloaded limit set ") + IntS(ColumnIndex).GetText() + StringS(" for test ") + lim.GetTestNumber().GetText() + StringS(": LL=[") + lNewLL + StringS("]; HL=[") + lNewHL + StringS("]");
        GTL_DisplayMessage(lMsg, eDEBUG);
    }
    
    return true;
}




