#include "interactive_wizard.h"


namespace Gex
{

IInteractiveWizard::IInteractiveWizard(unsigned int testNumber, QString testName, int pinNumber, T_WIZARD type):
    mIndex(-1),
    mTestCell(0),
    mTestName(testName),
    mTestNumber(testNumber),
    mPinNumber(pinNumber),
    mType(type)
{
}

IInteractiveWizard::~IInteractiveWizard()
{
}


InteractiveChartsWizard::InteractiveChartsWizard(unsigned int testNumber, QString testName, int pinNumber)
    :IInteractiveWizard(testNumber, testName, pinNumber, T_CHARTS)
{
}

InteractiveChartsWizard::~InteractiveChartsWizard()
{
}


InteractiveTableWizard::InteractiveTableWizard(unsigned int testNumber, QString testName, int pinNumber)
    :IInteractiveWizard(testNumber, testName, pinNumber, T_TABLE)
{
}

InteractiveTableWizard::~InteractiveTableWizard()
{
}

InteractiveWafer3DWizard::InteractiveWafer3DWizard(unsigned int testNumber, QString testName, int pinNumber)
    :IInteractiveWizard(testNumber, testName, pinNumber, T_WAFER3D)
{
}

InteractiveWafer3DWizard::~InteractiveWafer3DWizard()
{
}

}
