#ifndef PROD_RECIPE_FILE_H
#define PROD_RECIPE_FILE_H

// Include
#include <QString>
#include <QStringList>
#include <QList>

// FlowSource
#define	FLOW_SOURCE_MAP			0
#define	FLOW_SOURCE_STDF		1
#define	FLOW_SOURCE_MAP_STDF	2

// Source ID
#define	FLOW_SOURCE_MAP_STRING		"OLI"
#define	FLOW_SOURCE_STDF_STRING		"STDF"
#define	FLOW_SOURCE_MAP_STDF_STRING	"OLI+STDF"

class	CRecipeFlow
{
public:
	CRecipeFlow();
	void	clear(void);
	QString	strSourceName(void);			// Returns source name
	int		iSourceID(QString strName);		// Returns source ID

	QString	m_strFlow;			// Flow name
	int		m_iDataSources;		// 0=MAP, 1=STDF, 2= MAP+STDF (see FLOW_SOURCE_xxx)
	QString	m_strGoodHbins;		// Good HBINs list (eg: "1-5,1000")
	QString	m_strENG_RecipeName;// Engineering Recipe associated with this flow.
	QString	m_strENG_Version;	// Engineering Recipe version to use in this flow.
	QString m_strENG_RecipeFile;// Engineering Recipe file name.
};

class   CPROD_Recipe
{
	public:
	CPROD_Recipe();
	void	clear();			// Erase entry
	bool	loadRecipe(QString strPROD_RecipeFile,QString &strErrorMessage);
	bool	saveRecipe(QString strPROD_RecipeFile,QString &strErrorMessage);
	bool	getFlowList(int iDataSource,QStringList &strList);	// Returns list of flows using specified data source (STDF or OLI/MAP)
	QString	strFlowListDetails(QString strSeparator=" ; ");
	QString	strStdfOutputMode(void);

	QString	m_strGroup;				// Group to which this recipe belongs
	QString m_strRecipeName;		// Recipe file name (without version# nor Galaxy suffix string)
	QString m_strRecipeFile;		// Recipe file name
	QString	m_strComment;			// Recipe comment line.
	int		m_iEnabled;				// 0 if recipe is disabled.
	int     m_iVersion;				// Recipe Production version
	int     m_iBuild;				// Recipe Production build
	QList <CRecipeFlow> m_cFlows;	// List of Flows

	// Misc. global options
	void	setPatBins(int iPpat,int iGpat,int iZpat);
	int     m_iPPAT_Bin;			// PPAT overloading bin; -1 for no overload
	int     m_iGPAT_Bin;			// GPAT overloading bin; -1 for no overload
	int     m_iZPAT_Bin;			// ZPAT overloading bin; -1 for no overload
	int		m_iStdfOutput;			// 0= No STDF 1= STDF MAP only;; 2= STDF Output - Full contents;
};

#endif	// PROD_RECIPE_FILE_H
