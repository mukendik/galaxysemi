#ifndef GEX_TEST_CREATOR_H
#define GEX_TEST_CREATOR_H

#include <QMap>
#include "eval_exp_cexev.h"
#include "test_defines.h"


class CTest;
class CGexFileInGroup;
class GexTestToCreate;



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												CLASS GexTestCreator										//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GexTestCreator
{
public:
	GexTestCreator(CGexFileInGroup *pFileInGroup);
	~GexTestCreator();

	// Methods
    bool createTestInFileGroup(GexTestToCreate *testTocreate, CTest **ptMergedTestList, int pin = GEX_PTEST);
    void applyFormula(int iNSublot, QList<CTest *> &testInFormulat, int pin = GEX_PTEST);
    bool evaluateVars(int iSampleIndex, QList<CTest *> &testInFormulat, int pin = GEX_PTEST);
    QList<CTest*> GetTestsFromFormula();
	QMap<QString, double> m_lstVars;
    inline void setFormula(QString formula) {m_strFormula = formula;}

private:
	CTest							*m_pCTestToCreate;
	CGexFileInGroup					*m_pFileInGroup;
	QString							m_strFormula;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												CLASS ExpressionEvaluatorOnGex								//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ExpressionEvaluatorOnGex : public CExEvCtx
{
public:
	ExpressionEvaluatorOnGex(QMap<QString, double> &lstVars);
	virtual ~ExpressionEvaluatorOnGex();

	// Override to return values for variables
	virtual bool getValue(const char *cIdent, Variant& result);

private:
	QMap<QString, double> &m_lstVars;
};


#endif // GEX_TEST_CREATOR_H
