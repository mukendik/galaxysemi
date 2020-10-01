#ifndef CSV_CONVERTER_V2X_H
#define CSV_CONVERTER_V2X_H

#include "abstract_csv_converter.h"

class CsvConverterV2x : public AbstractCsvConverter
{
public:

    enum CsvV2Minor
    {
        CsvV2Minor0 = 0,
        CsvV2Minor1 = 1
    };

    CsvConverterV2x(CsvV2Minor minorVersion);
    ~CsvConverterV2x();

private:

    /*! \brief Extract Static data Test (Multiple Parametric)
    */
    void ProcessStaticDataMPR(CShortTest * ptTestCell, long lTestNumber, int iPMRCount, int iResultCount,
                              bool bValidResult, unsigned short bSite);
    /*! \brief Write Results header
    */
    void WriteResultsHeader(void);

    /*! \brief returns the test units according to the scaling options
    */
    QString	scaleTestUnits(int nResScal, const QString& strTestUnits) const;

    /*! \brief returns the test results for a given run according to the scaling otpions
    */
    double	scaleTestResult(int nResScal, double dResult) const;

    bool    ProcessMIR(); 	// Extract MIR data
    void    ProcessSDR(void);                       // Extract Site info details (loadboard, etc...)
    void    ProcessPDR(void);                       // Extract Parametric Test Definition (STDF V3 only.)
    void    ProcessFDR(void);                       // Extract Functional Test Definition (STDF V3 only.)
    void    ProcessFTR(void);                       // Extract FTR data
    void    ProcessPTR(void);                       // Extract Test results
    void    ProcessMPR(void);                       // Extract Test results (Multiple Parametric)
    //! \brief Binning result.
    void    ProcessPRR(void);
    void    ProcessTSR(void);                       // To extract test name in case not found in PTRs
    void    ProcessWCR(void);                       // Extract wafer flat info
    void    ProcessWIR(void);                       // Extract wafer#
    void    ProcessMRR(void);                       // Extract End testing time.
    void    ProcessPMR(void);                       // Extract Pinmap info.
    void    ProcessDTR(void);                       // Extract DTR records.
    bool    ProcessHBR(void);                       // Extract HARD Bin results
    bool	ProcessSBR(void);                       // Extract SOFT Bin results
};

#endif // CSV_CONVERTER_V2X_H
