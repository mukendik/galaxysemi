#ifndef CPART_INFO_H
#define CPART_INFO_H

#include "stdf.h"
#include <QString>
#include <QList>
class CTest;

//////////////////////////////////////////////////////
// // Structure to hold part info: DieXY, Exec time, Tests executed, partID, etc...
//////////////////////////////////////////////////////
class	CPartInfo
{
public:
    long			lPartNumber;
    unsigned short m_site;
    BYTE			bHead;
    bool			bPass;
    int				iSoftBin;
    int				iHardBin;
    int				iDieX;
    int				iDieY;
    unsigned int	iFailTestNumber;
    float			fFailingValue;
    float			fValue;
    int				iValue;
    int				iFailTestPinmap;
    int				iTestsExecuted;
    long			lExecutionTime;
    QString			strPartRepairInfo;
    QList<CTest*>	lstFailedTestGoodResult;
    QList<CTest*>	lstPassedTestBadResult;
protected:
    QString			strPartID;
    QString			strPartText;
public:
    void setPartID(QString strPID);
    QString getPartID();
    inline void setPartText(QString strPIDText){
        strPartText = strPIDText;
    }
    inline QString getPartText(){
        return strPartText;
    }

    // Compart PartID of two parts
    int             compareXY(const CPartInfo * pOther);

    // Compare XY coordinates of two parts
    int             comparePartID(const CPartInfo * pOther);
    int             GetDieX() const;
    int             GetDieY() const;
};

#endif // CPART_INFO_H
