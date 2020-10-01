#ifndef TEST_H
#define TEST_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QList>
#include <QVariant>
#include <stdio.h>
#include <stdlib.h>

#define	GEX_UNITS			8
#define	TEST_HISTOSIZE		18
#define GEX_TEST_LABEL		20
#define GEX_LIMIT_LABEL		20
typedef unsigned char	BYTE;

class Test : QObject
{
    Q_OBJECT
    QMap <QString, bool> m_oPatMarkerEnable;
    QMap <QString, QVariant > mProperties;
    BYTE	m_cSiteCount[256];
    QList<int> pSamplesInSublots;
    QString strTestName;
    char	szTestUnits[GEX_UNITS];		// Test Units.
    char	szTestUnitsIn[GEX_UNITS];	// Input condition Test Units (Multi-parametric test)
    int	lHistogram[TEST_HISTOSIZE];	// To compute histogram data.
    // Arrays to store displayed info...avoids to rebuild them for each report page
    char	szTestLabel[GEX_TEST_LABEL];
    char	szLowL[GEX_LIMIT_LABEL];
    char	szHighL[GEX_LIMIT_LABEL];
    char	szLowSpecL[GEX_LIMIT_LABEL];
    char	szHighSpecL[GEX_LIMIT_LABEL];

    double mDoubles[10000];
    int mInts[10000];

public:
    Test():QObject() {}
};

#endif // TEST_H
