///////////////////////////////////////////////////////////
// Class to Import a generic CSV file (Wizard import)
///////////////////////////////////////////////////////////

#ifndef GEX_IMPORT_CSV_WIZARD_H
#define GEX_IMPORT_CSV_WIZARD_H

#include "gex_constants.h"
#include "ui_tb_import_csv_wizard_dialog.h"
#include "stdf.h"

#include <time.h>
#include <qregexp.h>

#define GEX_TB_MAXLINES_PREVIEW	100
#define GEX_TB_UNDEFINED_DIE_LOC	(int) -32768

// Test info definitions
#define	GEX_TB_TEST_LABEL_LABEL		"Label"
#define	GEX_TB_TEST_LABEL_LL		"Low L."
#define	GEX_TB_TEST_LABEL_HL		"High L."
#define	GEX_TB_TEST_LABEL_DATA		"Data"
#define	GEX_TB_TEST_LABEL_IGNORE	"Ignore"
#define	GEX_TB_TEST_LABEL_UNITS		"Units"
#define	GEX_TB_TEST_LABEL_TEST		"Test#"

// PartInfo definitions
#define	GEX_TB_PART_LABEL_TESTPF	"Device Pass/Fail flag"
#define	GEX_TB_PART_LABEL_SBIN		"Soft Bin#"
#define	GEX_TB_PART_LABEL_HBIN		"Hard Bin#"
#define	GEX_TB_PART_LABEL_DIEX		"Die X"
#define	GEX_TB_PART_LABEL_DIEY		"Die Y"
#define GEX_TB_PART_LABEL_SBLOT		"Wafer ID"
#define GEX_TB_PART_LABEL_SITE		"Site ID"
#define GEX_TB_PART_LABEL_IGNORE	"Ignore"
#define	GEX_TB_TEST_LABEL_PARAMETER	"Parameter"

// Test info flags
#define	GEX_TB_TEST_ID_LABEL		1
#define	GEX_TB_TEST_ID_LL			2
#define	GEX_TB_TEST_ID_HL			3
#define	GEX_TB_TEST_ID_DATA			4
#define	GEX_TB_TEST_ID_IGNORE		5
#define	GEX_TB_TEST_ID_UNITS		6
#define	GEX_TB_TEST_ID_TEST			7

// PartInfo flags
#define	GEX_TB_PART_ID_TESTPF		8
#define	GEX_TB_PART_ID_SBIN			9
#define	GEX_TB_PART_ID_HBIN			10
#define	GEX_TB_PART_ID_DIEX			11
#define	GEX_TB_PART_ID_DIEY			12
#define GEX_TB_PART_ID_SBLOT		13
#define GEX_TB_PART_ID_SITE			14
#define GEX_TB_PART_ID_IGNORE		15
#define GEX_TB_PART_ID_PARAMETER	16

class CGCSV_WizardtoSTDF
{
public:
    CGCSV_WizardtoSTDF();
    ~CGCSV_WizardtoSTDF();
    bool	Convert(const char *CsvFileName, const char *strFileNameSTDF, QString strParserConfig="");
    bool	isParsingConfigAvailable(QString strCsvFile,QString &strParsingConfigFile);
    QString GetLastError();
private:
    QString strLastError;	// Holds last error string during CSV->STDF convertion
    int	iLastError;			// Holds last error ID
    enum  errCodes
    {
        errNoError,			// No erro (default)
        errAbort,			// Manual abort
        errOpenFail,		// Failed Opening file
        errLicenceExpired,	// File date out of Licence window!
        errWriteSTDF		// Failed creating STDF intermediate file
    };
};

class ImportCsvWizardDialog : public QDialog, public Ui::ImportCsvWizardDialogBase
{
    Q_OBJECT

public:
    ImportCsvWizardDialog( QWidget* parent = 0, bool modal = true, Qt::WindowFlags fl = 0 );
    ~ImportCsvWizardDialog(){}
    QString	setFile(QString strCsvFileName,QString strFileNameSTDF);
    bool	ParseFile();
    bool	loadParserProfile(void);

private:
    int		iLastError;
    enum  errCodes
    {
        errNoError,			// No erro (default)
        errWriteSTDF		// Failed creating STDF intermediate file
    };
    void	clear(void);
    // can return "error..." or "ok"
    QString	LoadFileInTable();
    void	WriteMIR(void);
    int		WritePTR(long lPartID,QString strValue,QString strLabel,QString strUnits,QString strLowL,QString strHighL, bool bNeedPir);
    void	WriteWIR(void);
    void	WriteWRR(long &lTotalPartsInWafer);
    void	WritePIR(void);
    void	WritePRR(long &lTotalPartsInWafer);
    bool	WriteStdfFile(void);
    QString m_strCsvFileName;
    QString m_strFileNameSTDF;
    long	m_lEditorRows;			// Rows in table
    long	m_lEditorCols;			// Cols in table
    long	m_lStartingRow;		// Ignore all file lines lower than this.
    long	m_lStartingCol;		// Ignore all file columns lower than this.
    QRegExp	m_rSplit;			// Regular expression to hold parsing characters.
    long	m_lLabelOffset;		// Holds line (or Col) offset to the Label data
    long	m_lUnitsOffset;		// Holds line (or Col) offset to the Units data
    long	m_lTestOffset;		// Holds line (or Col) offset to the Test# data
    long	m_lLowLOffset;		// Holds line (or Col) offset to the Low Limit data
    long	m_lHighLOffset;		// Holds line (or Col) offset to the High Limit data
    long	m_lSoftBinOffset;	// Holds line (or Col) offset to the SOTFWARE bin#
    long	m_lHardBinOffset;	// Holds line (or Col) offset to the SOTFWARE bin#
    long	m_lDieXOffset;		// Holds line (or Col) offset to the DieX (X loc)
    long	m_lDieYOffset;		// Holds line (or Col) offset to the DieY (Y loc)
    long	m_lSublotOffset;	// Holds line (or Col) offset to the SubLot ID / Wafer ID
    long	m_lSiteOffset;		// Holds line (or Col) offset to the Site
    int		m_iSoftBin;
    int		m_iHardBin;
    int		m_iDieX;
    int		m_iDieY;
    int		m_iSiteID;
    long	m_lTestNumber;
    long	m_iTotalGoodBin;
    long	m_iTotalFailBin;
    long	m_iTotalTests;		// Total tests in a execution flow
    long	m_iTotalPassFailTests;	// Total failing tests in the test flow (only used if Pass/fail test columnbs defined)
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    GS::StdLib::Stdf	StdfFile;
    BYTE	m_bData;
    long	m_lPartID;
    QString m_strFirstTestInFlow;
    long	m_lTotalPartsInWafer;

    void LoadParameterIndexTable(void);
    void DumpParameterIndexTable(void);
    void UpdateParameterIndexTable(QString strParamName);
    void UpdateCellType(int row, int col, int iCellType);
    void setEnabledInteractiveControls(bool value);
    void resetPartInfoFlagForOffset(int offset = -1);
    void resetTestInfoFlagForOffset(int offset = -1);

    QString repaintItems();
    bool repaintItem(QTableWidgetItem *pQtwiItem, int nItemCol, int nItemRow);

    bool	bNewCsvParameterFound;
    QStringList pFullCsvParametersList;	// Complete list of ALL CSV parameters known.

    QString	m_strLotId;
    QString	m_strSubLotId;
    QString m_strPreviousSubLotId;
    QString	m_strOperator;
    QString	m_strTester;
    QString	m_strTestProgram;

    QString ReadLine(QTextStream& hFile);

public slots:
    void OnNext();
    void OnLoadParserProfile();
    void OnSaveParserProfile();
    void OnDelimiterChange();
    void OnStartingCoordinateChange(int);
    // could return "error..." on error else "ok"
    QString OnUpdateTableLabels();
    void OnSelectionMade(QModelIndex qmiModelIndex);

    void showEvent ( QShowEvent * event );
};

#endif
