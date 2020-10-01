#include "stdf_content_utils.h"
#include "stdfparse.h"
#include "stdf_head_and_site_number_decipher.h"
#include "gqtl_log.h"

bool GS::Gex::StdfContentUtils::GetSites(const std::string &lSTDFFilename, std::vector<int> &lSites, bool lValidOnly)
{
    std::vector<int> lTmpSites;

    if (lValidOnly)
        return GS::Gex::StdfContentUtils::GetSites(lSTDFFilename, lTmpSites, lSites);
    else
        return GS::Gex::StdfContentUtils::GetSites(lSTDFFilename, lSites, lTmpSites);
}


//////////////////////////////////////////////////
/// 1/ Read the SDR if exists and get valid sites
/// 2/ Read all PRR to get sites one by one.
//////////////////////////////////////////////////
bool GS::Gex::StdfContentUtils::GetSites(const std::string& lSTDFFilename, std::vector<int>& lAllSites, std::vector<int>& lValidSite)
{
    int lSiteNum=0;
    std::vector<int> lSdrSites;

    // Define STDF file to read
    if(lSTDFFilename.compare("") != 0)
    {
        // Reset Sites list
        lAllSites.clear();
        lValidSite.clear();

        GQTL_STDF::StdfParse parser;
        int recordType;

        if( ! parser.Open( lSTDFFilename.c_str() ) ) return false;

        for( int status = parser.LoadNextRecord( &recordType );
             status == GS::StdLib::Stdf::NoError;
             status = parser.LoadNextRecord( &recordType ) )
        {
            if( recordType == GQTL_STDF::Stdf_Record::Rec_SDR )
            {
                GQTL_STDF::Stdf_SDR_V4 lSDRRecord;
                if(parser.ReadRecord(&lSDRRecord) == false)
                {
                    return false;
                }
                if (lSDRRecord.m_u1SITE_CNT == 0)
                    continue;
                for (int i=0; i<lSDRRecord.m_u1SITE_CNT; ++i)
                {
                    lSdrSites.push_back(lSDRRecord.m_ku1SITE_NUM[i]);
                }
            }
            // decode a GDR to know if we are using the SDR high count feature
            if( recordType == GQTL_STDF::Stdf_Record::Rec_GDR )
            {
                GQTL_STDF::Stdf_GDR_V4 gdr;
                parser.ReadRecord(&gdr);

                // modify a deciphering mode for site number if necessary
                GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInParser(gdr, parser);
            }

            if( recordType == GQTL_STDF::Stdf_Record::Rec_PRR )
            {
                GQTL_STDF::Stdf_PRR_V4 prr_record;
                parser.ReadRecord( &prr_record );

                int lHardBin    = -1;
                int lSoftBin    = -1;

                lSiteNum = GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn(parser, prr_record);

                lHardBin = prr_record.m_u2HARD_BIN;
                lSoftBin = prr_record.m_u2SOFT_BIN;

                // Only use sites of parts with valid binnings
                // In some files, the first PIR/PRR are only used to have the definition of tests
                if(lHardBin <= 32767)
                {
                    // Insert site in list if not already in!
                    if (std::find(lValidSite.begin(), lValidSite.end(), lSiteNum) == lValidSite.end())
                    {
                        lValidSite.push_back(lSiteNum);
                    }

                    if (std::find(lAllSites.begin(), lAllSites.end(), lSiteNum) == lAllSites.end())
                    {
                        lAllSites.push_back(lSiteNum);
                    }
                }
                else if(lSoftBin <= 32767)
                {
                    // if HBIN invalid, take SBIN for all sites
                    if (std::find(lAllSites.begin(), lAllSites.end(), lSiteNum) == lAllSites.end())
                    {
                        lAllSites.push_back(lSiteNum);
                    }
                }
            }
        }

        // If we cannot find any valid sites from PRR records, use the sites read from SDR if any
        if(lAllSites.empty() && !lSdrSites.empty())
        {
            lAllSites = lSdrSites;
            lValidSite = lSdrSites;
        }

        // Sort list
        std::sort(lValidSite.begin(), lValidSite.end());
        std::sort(lAllSites.begin(), lAllSites.end());
    }

    return true;
}
