#include <QRegExp>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include "composite.h"
#include "chart_element.h"
#include "wafermap_element.h"
#include "statistical_table.h"
#include "gqtl_log.h"
#include "engine.h"
#include "gex_constants.h"

Composite::Composite(const QString& name, const QJsonObject& jsonDescription, Component *parent, T_Component type):Component(name, jsonDescription, parent, type)
{
}

Composite::~Composite()
{
    Clear();
}

bool Composite::AddElement(Component *elt)
{
    UpdateElementsBeforeAdded(elt);
    mReportElements.push_back(elt);
    return true;
}

bool Composite::AddElement( const QJsonObject &jsonDescription)
{
    Component* lComponent = BuildReportElement(jsonDescription);
    if(lComponent)
    {
        if (lComponent->GetType() != T_SECTION && lComponent->GetType() != T_NONE)
        {
            GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString("%1 added to the Report Builder").arg(lComponent->GetName()));
            QCoreApplication::processEvents();
        }

        return AddElement(lComponent);
    }
    return false;
}

void Composite::Clear()
{
    qDeleteAll(mReportElements);
    mReportElements.clear();
    mComment.clear();
    mName.clear();
}

QJsonObject Composite::ToJson()
{
    mJsonDescription.insert("Name", mName);
    if(mComment.isEmpty() == false)
        mJsonDescription.insert("Comment", mComment);

    // -- update the inside json description if needed with the GUI input
    UpdateJson();

    QJsonArray lChilds;
    QList<Component*>::iterator lIterBegin(mReportElements.begin()), lIterEnd(mReportElements.end());
    for(int lIndex = 0;lIterBegin != lIterEnd; ++lIterBegin)
    {
        (*lIterBegin)->UpdateJson();
        lChilds.insert(lIndex++, (*lIterBegin)->ToJson());
    }

    if(mType == T_ROOT)
    {
        mJsonDescription.insert("Sections", lChilds);
    }
    else if(mType == T_SECTION)
    {
        mJsonDescription.insert("Elements", lChilds);
    }

    return mJsonDescription;
}

Component* Composite::BuildReportElement( const QJsonObject &jsonDescription)
{
    ReportElement* lComponent = 0;
    if (jsonDescription.contains("Histogram"))
    {
        QString lTitle = jsonDescription["Histogram"].toObject()["Title"].toString();
        lComponent = new ChartElement(lTitle, jsonDescription, this, T_HISTO);
    }
    else if (jsonDescription.contains("Trend"))
    {
        QString lTitle = jsonDescription["Trend"].toObject()["Title"].toString();
        lComponent = new ChartElement(lTitle, jsonDescription,  this, T_TREND);
    }
    else if (jsonDescription.contains("ProbabilityPlot"))
    {
        QString lTitle = jsonDescription["ProbabilityPlot"].toObject()["Title"].toString();
        lComponent = new ChartElement(lTitle, jsonDescription,  this, T_PROBA);
    }
    if (jsonDescription.contains("BoxPlot"))
    {
        QString lTitle = jsonDescription["BoxPlot"].toObject()["Title"].toString();
        lComponent = new ChartElement(lTitle, jsonDescription,  this, T_BOXPLOT);
    }
    else if (jsonDescription.contains("Wafermap"))
    {
        QString lTitle = jsonDescription["Wafermap"].toObject()["Title"].toString();
        lComponent = new WaferMapElement(lTitle, jsonDescription, this);
    }
    else if (jsonDescription.contains("Table"))
    {
        QString lTitle = jsonDescription["Table"].toObject()["Title"].toString();
        lComponent = new StatisticalTable(lTitle, jsonDescription, this);
    }
    else if (jsonDescription.contains("StatisticTable"))
    {

    }
    lComponent->LoadJson();
    return lComponent;
}

bool Composite::DrawSection(QString &imagePath, int imageSize, CTest *sectionTest)
{
    for (int i=0; i<mReportElements.size(); ++i)
    {
        if (mReportElements[i] ==0 || !mReportElements[i]->DrawSection(imagePath, imageSize, sectionTest))
            return false;
    }
    return true;
}

void        Composite::EraseChild(Component * toRemove)
{
    QList<Component*>::iterator lIterBegin(mReportElements.begin()), lIterEnd(mReportElements.end());
    for(;lIterBegin != lIterEnd; ++lIterBegin)
    {
        if(*lIterBegin == toRemove)
        {
            delete toRemove;
            mReportElements.erase(lIterBegin);
            break;
        }
    }
}

const QList<Component *> & Composite::GetElements() const
{
    return mReportElements;
}

const QList<Component *> & Composite::GetElementsSorted() const
{
    std::sort(mReportElements.begin(), mReportElements.end(), SortComponent());
    return mReportElements;
}


int Composite::GetElementsNumber() const
{
    return mReportElements.size();
}

