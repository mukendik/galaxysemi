#ifndef TEST_DEFINES_H
#define TEST_DEFINES_H

#define	GEX_UNITS			8		// Max. string length for units (truncated if bigger)
#define GEX_TEST_LABEL		20		// Max chars. to build a TestLabel string!
#define GEX_LIMIT_LABEL		20		// Max chars. to build a test limit string!
#define	TEST_HISTOSIZE		18		// Define Histogram size (number of cells to draw histogram)

#define	C_INFINITE		(double) 9e99
#define F_INFINITE      (double) 3.4028194099033679e+38
#define	C_NO_CP_CPK		(double) -9.9e99

// Definitions used to classify test type (Parametric, MParametric,...)
#define	GEX_PTEST		-1							// Parametric test
#define	GEX_MPTEST		-2							// Multi-parametric test
#define	GEX_FTEST		-3							// Functional test
#define GEX_UNKNOWNTEST	-4							// Unknown Test Type
#define GEX_INVALIDTEST	-5							// Invalid Test Type
#define	GEX_MINTEST		0L							// Lowest test allowed
#define	GEX_MAXTEST		2147483647lu					// Highest test allowed
#define	GEX_MINPINMAP	1							// Lowest Pinmap index allowed
#define	GEX_MAXPINMAP	32767						// Highest Pinmap index allowed
#define GEX_PINMAP_NOLOWLIMIT (GEX_MINPINMAP-1)		// Tells we do not filter on the pinmap low value
#define GEX_PINMAP_NOHIGHLIMIT (GEX_MAXPINMAP+1)	// Tells we do not filter on the pinmap high value

///////////////////////////////////////////////////////////////////////////////
// bLimitFlag details
// Mask
// 0x01 Bit 0 = 1 for No Low Test Limit
// 0x02 Bit 1 = 1 for No High Test Limit
// 0x04 Bit 2 = 1 for Limits already defined
// 0x08 Bit 3 = 1 for Low test limit is NOT strict
// 0x10 Bit 4 = 1 for High test limit is NOT strict
// 0x20 Bit 5 = 1 for No Low Spec Limit
// 0x40 Bit 6 = 1 for No High Spec Limit

// Masks
#define CTEST_LIMITFLG_NOLTL		0x01	// 0x01 Bit 0 = 1 for No Low Test Limit
#define CTEST_LIMITFLG_NOHTL		0x02	// 0x02 Bit 1 = 1 for No High Test Limit
#define CTEST_LIMITFLG_GOTTL		0x04	// 0x04 Bit 2 = 1 for Limits already defined
#define CTEST_LIMITFLG_LTLNOSTRICT	0x08	// 0x08 Bit 3 = 1 for Low test limit is NOT strict
#define CTEST_LIMITFLG_HTLNOSTRICT	0x10	// 0x10 Bit 4 = 1 for High test limit is NOT strict
#define CTEST_LIMITFLG_NOLSL		0x20	// 0x20 Bit 5 = 1 for No Low Spec Limit
#define CTEST_LIMITFLG_NOHSL		0x40	// 0x40 Bit 6 = 1 for No High Spec Limit

// Merging Rules
#define TEST_MERGE_NUMBER           0
#define TEST_MERGE_NAME             1
#define TEST_NO_MERGE               2

#endif // TEST_DEFINES_H
