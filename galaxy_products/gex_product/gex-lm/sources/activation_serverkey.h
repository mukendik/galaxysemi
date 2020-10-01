#include <QString>

#include "read_system_info.h"
#include "ui_activation_serverkey_dialogbase.h"

#define GEX_T(string) (char *)(string)

class LicenseFile
{
public:
	LicenseFile(const QString & strUserHome, const QString & strApplicationDir);
	~LicenseFile();

	bool				IsCorrectLicenseFile(void);
	void				CreateLicenseRequestFile(int nPackageID, QString & strUserName, QString & strProductID);
	
private:
	void				GetUserFolder(QString &strPath);
	bool				LoadLicenseFile(void);
	void				WriteCryptedFile(char *szString);	// Encrypt+write string to file
	void				WriteCryptedFile(long lData);		// Encrypt+write (long)  to file
	bool				ReadFileLine(char *szLine,int iMaxChar,FILE *hFile);
	void				ReadCryptedFile(char *szString);
	void				ReadCryptedFile(long *lData);
	bool				CheckDisabledSupportPerpetualLicense(QString strCurrentLicense);

	FILE				*hCryptedFile;		// Used to create license request file.
	unsigned long		uChecksum;
	QString				m_strUserHome;
	QString				m_strApplicationDir;

	// Info used in PC+Unix
	QString				strLicenseeName;
	ReadSystemInfo		cSystemInfo;
};

class ActivationServerKeyDialog : public QDialog, public Ui::ActivationServerKeyDialogBase
{
    Q_OBJECT

public:
	ActivationServerKeyDialog(const QString & strUserHome, const QString & strApplicationDir, QWidget* parent = 0);

private:
	QString				m_strUserHome;
	QString				m_strApplicationDir;

protected slots:
    virtual void		on_buttonNext_clicked();		// 'Next button': implicit signal/slot connection
};

