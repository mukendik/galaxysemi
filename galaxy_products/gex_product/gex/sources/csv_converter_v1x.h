#ifndef CSV_CONVERTER_V1X_H
#define CSV_CONVERTER_V1X_H

#include "abstract_csv_converter.h"

class CsvConverterV1x : public AbstractCsvConverter
{
public:

    enum CsvV1Minor
    {
        CsvV1Minor0 = 0
    };

    CsvConverterV1x(CsvV1Minor minorVersion = CsvV1Minor0);
    ~CsvConverterV1x();

protected:

    QMap <int,cTestList>	m_cTestFlow;                    // Holds testing flow per testing site.

    /*! \brief Clear variables (call the abstract clear, and clear Testflow, ...)
    */
    void			Clear(void);

    void            ProcessStaticDataMPR(CShortTest * ptTestCell, long lTestNumber, int iPMRCount);	// Extract Static data Test (Multiple Parametric)
    void            WriteResultsHeader(void);

    bool            ProcessMIR(); 	// Extract MIR data
    void            ProcessSDR(void);                       // Extract Site info details (loadboard, etc...)
    void            ProcessPDR(void);                       // Extract Parametric Test Definition (STDF V3 only.)
    void            ProcessFDR(void);                       // Extract Functional Test Definition (STDF V3 only.)
    void            ProcessFTR(void);                       // Extract FTR data
    void            ProcessPTR(void);                       // Extract Test results
    void            ProcessMPR(void);                       // Extract Test results (Multiple Parametric)
    //! \brief Binning result.
    void            ProcessPRR(void);
    void            ProcessTSR(void);                       // To extract test name in case not found in PTRs
    void            ProcessWCR(void);                       // Extract wafer flat info
    void            ProcessWIR(void);                       // Extract wafer#
    void            ProcessMRR(void);                       // Extract End testing time.
    void            ProcessPMR(void);                       // Extract Pinmap info.
    void            ProcessDTR(void);                       // Extract DTR records
    void            ProcessGDR(void);                       // Extract GDR records
};
#endif // CSV_CONVERTER_V1X_H
