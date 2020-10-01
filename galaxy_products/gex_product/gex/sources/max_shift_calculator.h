#ifndef MAX_SHIFT_CALCULATOR_H
#define MAX_SHIFT_CALCULATOR_H

class CGexReport;

class MaxShiftCalculator
{
public :
    explicit MaxShiftCalculator( CGexReport *aReport );

    void ComputeMaxShiftInReferenceGroupOfFiles();

private :
    CGexReport *mUnderlyingReport;
};

#endif // MAX_SHIFT_CALCULATOR_H
