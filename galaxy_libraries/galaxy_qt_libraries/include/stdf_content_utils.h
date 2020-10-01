#ifndef STDF_CONTENT_UTILS_H
#define STDF_CONTENT_UTILS_H

#include <string>
#include <vector>

namespace GS
{
namespace Gex
{
    /// \brief This class groups tools acting on a stdf file content.
    class StdfContentUtils
    {
    public :

        /// \brief get all site from a stdf file
        /// \param stdfFileName Stdf file name to scan
        /// \param sites output parameter receiving all site numbers
        /// \param validOnly Flag indicating if only validation is required
        static bool GetSites( const std::string &stdfFilename, std::vector<int> &sites, bool validOnly );

        /// \brief get all site from a stdf file
        /// \param stdfFileName Stdf file name to scan
        /// \param sites output parameter receiving all site numbers
        /// \param validSites output parameter receiving valid sites only
        static bool GetSites( const std::string &stdfFilename, std::vector<int> &sites, std::vector<int> &validSite );
    };
} // namespace GEX
} // namespace GS

#endif // STDF_CONTENT_UTILS_H
