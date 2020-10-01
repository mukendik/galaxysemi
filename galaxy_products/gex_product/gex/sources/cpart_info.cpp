#include "cpart_info.h"
#include "gex_algorithms.h"

int CPartInfo::comparePartID(const CPartInfo *pOther)
{
    return algorithms::gexCompareIntegerString(strPartID, pOther->strPartID);
}


int CPartInfo::compareXY(const CPartInfo *pOther)
{
    int nCompare = iDieX - pOther->iDieX;

    if (nCompare == 0)
        return (iDieY - pOther->iDieY);

    return nCompare;
}


void CPartInfo::setPartID(QString strPID){
    strPartID = strPID;
}
QString CPartInfo::getPartID(){
    return strPartID;
}


int CPartInfo::GetDieX() const
{
    return iDieX;
}

int CPartInfo::GetDieY() const
{
    return iDieY;
}
