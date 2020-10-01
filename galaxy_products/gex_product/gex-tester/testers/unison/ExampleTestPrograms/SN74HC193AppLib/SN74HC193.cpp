#include <Unison.h>
#include <iostream>
#include <cmath>
using namespace std;

TMResultM ReturnValue(TMResultM val)
{
    return val;
}
void SetPowerSupply(PinML pins, FloatM value, FloatM range)
{
    VI.Initialize(pins);
    VI.Connect(pins, VI_TO_DUT, VI_MODE_REMOTE);
    VI.Gate(pins, VI_GATE_ON);
    VI.SetMeasureIRange(pins, 1.0);
    VI.SetClampsI(pins, 1.0);
    VI.ForceV(pins, value, range);
    TIME.Wait(5e-3);
}

void SetDigitalLevels(PinML pins, FloatM vil, FloatM vih, FloatM vol, FloatM voh, FloatM vref)
{
    DIGITAL.SetVihVil(pins, vih, vil);
    DIGITAL.SetVohVol(pins, voh, vol);
    DIGITAL.SetVref(pins, vref);
}

void SetDriversLow(PinML pins)
{
    DIGITAL.SetDriveMode(pins, DIGITAL_DRV_MODE_OFF);
    DIGITAL.SetLoadMode(pins, DIGITAL_LD_MODE_OFF);
}

void RestoreDrivers(PinML pins)
{
    DIGITAL.SetDriveMode(pins, DIGITAL_DRV_MODE_PATTERN);
    DIGITAL.SetLoadMode(pins, DIGITAL_LD_MODE_OFF);
}

void Disconnect(PinML pins)
{
    DIGITAL.Disconnect(pins, DIGITAL_DCL_TO_DUT);
    
    cout << "The following pins: " << pins << " have been disconnected from the tester.";
    
}

FloatM AddMyValues(FloatM a, FloatM b)
{
	return a+b;
}

void StartClock()
{
	TIME.StartTimer();
}


TMResultM StopClock(FloatS MinTimeLimit, FloatS MaxTimeLimit, FloatS sim_value)
{
	FloatS meas_Result;
  
  if(RunTime.ConnectedToTester())
    meas_Result = TIME.StopTimer();
  else
    meas_Result = sim_value;
  
  TMResultM TestTime = DLOG.Value(meas_Result, MinTimeLimit, MaxTimeLimit, "s",
    "Elapsed Time", UTL_VOID, UTL_VOID, 0, ER_PASS, true);
  
  return (TestTime);
}

TMResultM DatalogItem(FloatM val, LimitStruct lim)
{
    TMResultM retval = DLOG.ValueLS(val,lim);
    return retval;
}

// This gets a single random value, per site if you use it in a cell. 
// Type this in the cell: 
// SingleRandomValue(0.1, 2.0) 
//SingleRandomValue(Site(0.38, -0.52), Site(0.42, -0.48), Site(0.1, 0.1), Site(20, 20), Site(0.5, 0.5), Site(0.3, 0.0))
FloatS SingleRandomValue(const FloatS low, const FloatS high, const FloatS outlier_percent, const FloatS outlier_factor, const FloatS bimodal_cursor, const FloatS bimodal_shift) 
{ 
    // adds up 12 numbers from regular Rnd() 
    // subtracts 6, divides by 6 
    // result is not exactly Gaussian but it's close enough for 
    // our typical "fake simulation data" applications 
    // magic, and pretty fast 
    const int n = 12; 
    FloatS retval=0.0; 
    
    //MATH.Randomize(0);
    for (int i=0; i<n; ++i) { 
        retval += MATH.Rnd(); 
    } 
    
    retval = (retval-FloatS(n/2))/6.0*(high-low)+(high+low)/2.0;
    
    // Should we simulate bi-modal
    if(bimodal_shift > 0.0F)
    { 
        //MATH.Randomize(0);
        FloatS lRnd = MATH.Rnd()-0.5F;
        if(lRnd < bimodal_cursor)
            retval = retval + bimodal_shift; 
        else
            retval = retval - bimodal_shift; 
    } 
    
    // Should we simulate outliers
    if(outlier_percent > 0.0F)
    { 
        //MATH.Randomize(0);
        FloatS lRnd = MATH.Rnd()-0.5F;
        if((lRnd >= 0.0F) && (lRnd <= outlier_percent/2.0F))
            retval = retval + outlier_factor*fabs(high-low);
        else if((lRnd < 0.0F) && (lRnd >= -outlier_percent/2.0F))
            retval = retval - outlier_factor*fabs(high-low);
    } 
    
    return retval;
} 

// This gets a per-pin random value, per site if you use it in a cell 
// Type this in the cell: 
// PerPinRandomValue(NumPins(allins),-2mA,2mA) 
// Replace allins by the pin list used in the test 
// You'll usually find this is a cell called "TestPins" 

FloatS1D PerPinRandomValue(const IntS n_pins, const FloatS low, const FloatS high) 
{ 
    if (n_pins == 0) return UTL_VOID; 
    
    FloatS1D retval(n_pins,0.0); 
    
    for (IntS x = 0; x < n_pins; ++x ) 
        //retval[x] = SingleRandomValue(low,high,0,0.0F); 
        retval[x] = low+(high-low)/2;
    return retval; 
} 
