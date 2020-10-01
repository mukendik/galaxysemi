#ifndef GEXSTRING_DIALOG_H
#define GEXSTRING_DIALOG_H

#include <QVBoxLayout>
#include "ui_getstring_dialog.h"

#if defined unix || __MACH__
#define F_WINDOW_Flags (QWidget::WStyle_Customize | QWidget::WStyle_NormalBorder | QWidget::WStyle_Title | QWidget::WStyle_SysMenu)
#else
	// Under PC: do not show the system menu...so no 'About Qt' can be seen !
#define F_WINDOW_Flags (QWidget::WStyle_Customize | QWidget::WStyle_NormalBorder)
#endif

/////////////////////////////////////////////////////////////////////////////

class GetStringDialog : public QDialog, public Ui::GetStringDialogBase
{
	Q_OBJECT

public:
	GetStringDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
	void setDialogText(const char * Title, const char * szFieldName, const char * szFieldValue, const char * szToolTip, bool bAllowPickPath = false, bool bBrowseFoldersOnly = true);
	void setDialogText(QString strTitle,QString strWelcomeText,QString strString1,QString &strValue1,QString strToolTip1,QString strString2,QString &strValue2,QString strToolTip2);
	void getString(char *szString) ;
	QString getString(int iStringID) ;
    void addConditions(QWidget *poWidget);
    void setProhibatedList(const QStringList &oProhibitedName);

private:
	bool bBrowseFoldersOnly;
    QStringList m_oProhibitedName;
public slots:
	void OnBrowse();
    void checkGroupName(const QString & );
};

class CDoubleInputDialog : public QDialog
{
	Q_OBJECT

public:
	CDoubleInputDialog(QWidget* parent, QString initial_value, double bottom, double top)
		: QDialog(parent),
		 m_vbl(this), m_ok_button("ok", this), m_dv(bottom, top, 3, &m_le)
	{
		setLayout(&m_vbl);
        //setCaption(QString("between %1 and %2").arg(bottom).arg(top)); // Qt3
        setWindowTitle(QString("between %1 and %2").arg(bottom).arg(top));
		m_label.setText(QString("Please enter a number between %1 and %2...").arg(bottom).arg(top));
		m_label.setAlignment(Qt::AlignHCenter);
		m_vbl.addWidget(&m_label);
		setModal(true);
		m_le.setText(initial_value);
		m_vbl.addWidget(&m_le);
		m_dv.setBottom(bottom);
		m_dv.setTop(top);
		m_dv.setLocale(QLocale::C);
		m_le.setValidator(&m_dv);
        m_ok_button.setText("OK");
		m_vbl.addWidget(&m_ok_button);
		QObject::connect(&m_ok_button, SIGNAL(clicked()), this, SLOT(accept()));
		QObject::connect(&m_le, SIGNAL(textChanged(QString)), this, SLOT(OnLineEditTextChanged(QString)));
		// Select text in line edit and set focus
		m_le.selectAll();
		m_le.setFocus();
	};
	~CDoubleInputDialog() { };
	QLabel m_label;
	QLineEdit m_le;
	QVBoxLayout m_vbl;
	QPushButton m_ok_button;
	QDoubleValidator m_dv;
protected slots:
	void	OnLineEditTextChanged(QString s)
	{
		int pos=0;
		if (m_dv.validate(s,pos)==QValidator::Acceptable)
		{
			m_ok_button.setEnabled(true);
		}
		else
		{
			m_ok_button.setEnabled(false);
		}
	};
};


#endif // GEXSTRING_DIALOG_H
