///////////////////////////////////////////////////////////
// Global const structures
///////////////////////////////////////////////////////////
#define GEXMO_CONSTANTS_CPP

#include "gexmo_constants.h"

///////////////////////////////////////////////////////////
// GENERAL PURPOSE
///////////////////////////////////////////////////////////
const char* gexMoLabelTaskFrequency[] = 
{ 
	"Every 1 minute",
	"Every 2 minutes",
	"Every 3 minutes",
	"Every 4 minutes",
	"Every 5 minutes",
	"Every 10 minutes",
	"Every 15 minutes",
	"Every 30 minutes",
	"Every 1 hour",
	"Every 2 hours",
	"Every 3 hours",
	"Every 4 hours",
	"Every 5 hours",
	"Every 6 hours",
	"Every 12 hours",
	"Every 1 day",
	"Every 2 days",
	"Every 3 days",
	"Every 4 days",
	"Every 5 days",
	"Every 6 days",
	"Every 1 week",
	"Every 2 weeks",
	"Every 3 weeks",
	"Every 1 month",
	0	// Null terminator - mandatory!
};

const char* gexMoLabelTaskReportEraseFrequency[] = 
{ 
	"If older than 1 day",
	"If older than 2 days",
	"If older than 3 days",
	"If older than 4 days",
	"If older than 5 days",
	"If older than 6 days",
	"If older than 1 week",
	"If older than 2 weeks",
	"If older than 3 weeks",
	"If older than 1 month",
	"If older than 2 months",
	"If older than 3 months",
	"If older than 4 months",
	"If older than 5 months",
	"If older than 6 months",
	"Never: keep all reports on the server",

	0	// Null terminator - mandatory!
};

const char* gexMoLabelTaskFrequencyDayOfWeek[] = 
{ 
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	"Sunday",
	0	// Null terminator - mandatory!
};

const char* gexMoLabelSpecInfo[] = 
{ 
	"Data samples",
	"Cp",
	"Cpk",
	"Mean",
	"Range",
	"Sigma",
    "Yield level (passing rate in %)",
	0	// Null terminator - mandatory!
};

