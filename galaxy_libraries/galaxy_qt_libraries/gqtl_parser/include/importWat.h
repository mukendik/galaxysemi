#ifndef IMPORTWAT_H
#define IMPORTWAT_H

#include "gqtl_global.h"
#include "parserBase.h"
#include "stdfparse.h"

namespace GS
{
    namespace Parser
    {

        typedef QMap<int, float> ValueOfSiteMap;

        class WatParameter : public ParserParameter
        {
            public:
                virtual ~WatParameter(){}

                ValueOfSiteMap  mValue;                 // Parameter values for the XXX sites locations.

                virtual float    rescale() {  return 1.; }
        };

        struct WatScaledParameter :public  WatParameter
        {
            virtual float    rescale() { return GS_POW(10.0, mResScale);}
        };

        struct WatParameterMatch: public std::binary_function<const WatParameter*,const QString&,bool>
        {
            bool operator() ( const WatParameter* watWafer, const QString& name) const
            {
                return (watWafer->GetTestName().compare(name, Qt::CaseInsensitive) == 0);
            }
        };

        class WatToStdf;
        class WatUmcToStdf;
        class WatAsmcToStdf;
        class WatSmicToStdf;
        class WatWafer
        {
            public :
                WatWafer():mWaferID(-1),mLowestSiteID(-1),mHighestSiteID(-1) {}
                ~WatWafer();

                int                     GetWaferID       () const  { return mWaferID;}		// WaferID E.g: 6
                int                     GetLowestSiteID  () const  { return mLowestSiteID;}	// Lowest SiteID found in WAT file for this wafer
                int                     GetHighestSiteID () const  { return mHighestSiteID;}

                void                    SetWaferID       (int waferID)          { mWaferID = waferID;}           // WaferID E.g: 6
                void                    SetLowestSiteID  (int lowestSiteID)     { mLowestSiteID = lowestSiteID;}	// Lowest SiteID found in WAT file for this wafer
                void                    SetHighestSiteID (int highestSiteID)    { mHighestSiteID = highestSiteID;}

                friend class WatToStdf;
                friend class WatAsmcToStdf;
                friend class WatSmicToStdf;
                friend class WatUmcToStdf;

            private :
                QList<WatParameter*>    mParameterList;	// List of Parameters in Wafer
                int                     mWaferID;		// WaferID E.g: 6
                int                     mLowestSiteID;	// Lowest SiteID found in WAT file for this wafer
                int                     mHighestSiteID;	// Highest SiteID found in WAT file for this wafer

        };

        struct WatWaferMatch: public std::binary_function<const WatWafer*,int,bool>
        {
            bool operator() ( const WatWafer* watWafer, int waferID) const
            {
                return (watWafer->GetWaferID() == waferID);
            }
        };

        class WatToStdf : public ParserBase
        {
        public:
            enum eLimitType
            {
                eLowLimit,			// Flag to specify to save the WAT Parameter LOW limit
                eHighLimit			// Flag to specify to save the WAT Parameter HIGH limit
            };

            WatToStdf();
            WatToStdf(ParserType lType, const QString& lName);
            ~WatToStdf();

            /**
             * \fn static bool	IsCompatible(const QString &FileName)
             * \brief static function called by the import all to check if the input file is compatible with the
             *         TriQuintRF format.
             * \param FileName is the input file, contains the TriQuintRF ascii file.
             * \return true if the input file is compatible with the TriQuintRF TriQuintRF format. Otherwise return false.
             */
            static bool IsCompatible(const QString &fileName);

            /**
             * \fn bool ConvertoStdf(const QString &TriQuintRFFileName, const QString &StdfFileName)
             * \brief Read the TriQuintRF ascii file.
             * \param fileName is the input file, contains the TriQuintRF file.
             * \param stdfFileName is the output file, contains the stdf file.
             * \return true if the file has been successfully read. Otherwise return false.
             */
            bool ConvertoStdf(const QString &fileName,  QString &stdfFileName);

        protected:

            QList<WatWafer*>    mWaferList;                 // List of Wafers in WAT file
            QString             mProductID;                 // ProductID
            QString             mLotID;                     // LotID
            QString             mSubLotID;                  // SubLotID
            QString             mProcessID;                 // ProcessID
            QString             mFamilyID ;                  // family ID
            QString             mSpecID;                    // SpecID
            QString             mJobName;                   // Job Name
            QString             mTestCode;                  // Step code
            QString             mTesterType;                // TesterType
            QString             mPackageId;                 // package ID
            QString             mProgRev;                   // Job rev
            QString             mOperatorName;              // Operator
            QString             mTemperature;
            long                mStartTime;                 // Startup time
            int                 mTotalParameters;


            virtual WatParameter*   CreateWatParameter(const QString& paramName, const QString& units);


            virtual bool        ParseFile                   (const QString &fileName);
            virtual bool        WriteStdfFile               (const char *stdFileName);
            virtual void        SaveParameterResult         (int waferID,int siteID, QString name, QString units, QString& value);


            void                NormalizeLimits             (WatParameter *pParameter);
            void                SaveParameterLimit          (const QString& name, QString value, int limit, QString units);
            WatParameter*       FindParameterEntry          (int iWaferID,int iSiteID,QString paramName,QString units="");
            void                WriteSBR                    (int binNumber, int totalBins, bool isPassed, int testHead = 255 /*ALL*/ , int testSite = 255/*ALL*/ );
            void                WriteHBR                    (int binNumber, int totalBins, bool isPassed, int testHead = 255 /*ALL*/ , int testSite = 255/*ALL*/ );
            void                SetMIRRecord                (GQTL_STDF::Stdf_MIR_V4 &lMIRV4Record, const char *userType, bool eTest = true);
            void                SetWIRRecord                (GQTL_STDF::Stdf_WIR_V4 &lWIRV4Record, int waferID );
            void                SetWRRRecord                (GQTL_STDF::Stdf_WRR_V4 &lWRRV4Record, int waferID, int totalBin , int totalGoodBin);

            GQTL_STDF::Stdf_SBR_V4 mSBRV4Record;
            GQTL_STDF::Stdf_HBR_V4 mHBRV4Record;
        };
    }
}


#endif // IMPORTWAT_H
