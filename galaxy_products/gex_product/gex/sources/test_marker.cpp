#include "test_marker.h"

bool TestMarker::IsMarkerOfType(const TestMarkerType MarkerType)
{
    if((MarkerType & PatNearLimits) && ((strLabel=="-N") || (strLabel=="+N")))
        return true;
    if((MarkerType & PatMediumLimits) && ((strLabel=="-M") || (strLabel=="+M")))
        return true;
    if((MarkerType & PatFarLimits) && ((strLabel=="-F") || (strLabel=="+F")))
        return true;
    if((MarkerType & PatRelaxedLimits) && (strLabel.endsWith(QString(" relaxed"))))
        return true;
    return false;
}
