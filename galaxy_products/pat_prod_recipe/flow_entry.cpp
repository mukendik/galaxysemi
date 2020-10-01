#include <QtGui>

#include "patprodrecipe.h"



FlowDefinition::FlowDefinition(QStringList strFlowsAvailable,QStringList strEngRecipes,QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);

	// Flow list
	ui.comboBoxFlow->clear();
	ui.comboBoxFlow->addItems(strFlowsAvailable);

	// ENG Recipes list
	ui.comboBoxENG_Recipe->clear();
	ui.comboBoxENG_Recipe->addItems(strEngRecipes);

	// Connect signals: Recipe
	QObject::connect(ui.pushButtonOk, SIGNAL(clicked()),	this, SLOT(onOk(void)));
	QObject::connect(ui.pushButtonCancel, SIGNAL(clicked()),	this, SLOT(reject(void)));
}

void	FlowDefinition::onOk()
{
	// Ensure recipe name selected.
	accept();
}

void	FlowDefinition::push_GUI(CRecipeFlow cFlow)
{
	ui.comboBoxFlow->setCurrentIndex(ui.comboBoxFlow->findText(cFlow.m_strFlow));
	ui.comboBoxSource->setCurrentIndex(cFlow.m_iDataSources);
	ui.lineEditGoodHbins->setText(cFlow.m_strGoodHbins);

	QString strString = cFlow.m_strENG_RecipeName + " - V" + cFlow.m_strENG_Version;
	strString += "  [File: " + cFlow.m_strENG_RecipeFile + "]";
	int index = ui.comboBoxENG_Recipe->findText(strString);
	if(index<=0)
		index = 0;
	ui.comboBoxENG_Recipe->setCurrentIndex(index);
}

void	FlowDefinition::pull_GUI(CRecipeFlow &cFlow)
{
	cFlow.m_strFlow = ui.comboBoxFlow->currentText();
	cFlow.m_iDataSources = ui.comboBoxSource->currentIndex();
	cFlow.m_strGoodHbins = ui.lineEditGoodHbins->text();

	// Parse string: "<recipe name> - Vx.y [File: <recipe file]"
	QString strString = ui.comboBoxENG_Recipe->currentText();
	QString strSection;
	cFlow.m_strENG_RecipeName = strString.section(" - V",0,0);
	strSection = strString.section(" - V",1);
	strSection = strSection.section("[",0,0).simplified();
	if(strSection.isEmpty())
		strSection = "1.0";
	cFlow.m_strENG_Version = strSection;
	cFlow.m_strENG_RecipeFile = strString.section("File:",1).simplified();
	cFlow.m_strENG_RecipeFile = cFlow.m_strENG_RecipeFile.replace("]","");	// Remove ending ']'
}
