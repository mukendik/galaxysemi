#ifndef GEX_TEST_TO_CREATE_H
#define GEX_TEST_TO_CREATE_H

#include <QString>
#include <QList>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												CLASS GexTestToCreate										//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GexTestToCreate
{
public:
    GexTestToCreate();
	~GexTestToCreate();

	// Setters
	void setTestName(QString strTestName);
	void setTestNumber(QString strTestNumber);
	void setTestUnit(QString strTestUnit);
	void setTestLowLimit(QString strLLimit);
	void setTestHighLimit(QString strHLimit);
	void setFormula(QString strFormula);

	QString name() {return m_strTestName;}
	QString number() {return m_strTestNumber;}
	QString unit() {return m_strTestUnit;}
	QString lowLimit() {return m_strLLimit;}
	QString highLimit() {return m_strHLimit;}
	QString formula() {return m_strFormula;}

private:
	QString m_strTestName;
	QString m_strTestNumber;
	QString m_strTestUnit;
	QString m_strLLimit;
	QString m_strHLimit;
	QString m_strFormula;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												CLASS GexTestToCreateList									//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GexTestToCreateList
{
public:
	GexTestToCreateList();
	~GexTestToCreateList();

	void addTestToCreate(GexTestToCreate *testToCreate);
	QList<GexTestToCreate *>			m_lstTestToCreate;

private:

};



#endif // GEX_TEST_TO_CREATE_H
