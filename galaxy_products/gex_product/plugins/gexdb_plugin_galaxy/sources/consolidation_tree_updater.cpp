#include "consolidation_tree_updater.h"
#include "consolidation_tree_query_engine_p.h"
#include "consolidation_tree_defines.h"
#include "consolidation_tree_query_filter.h"
#include "consolidation_tree_period.h"
#include "consolidation_tree_commands.h"
#include <gqtl_log.h>
#include "gexdb_plugin_galaxy.h"

#include <QStringList>
#include <QDate>
#include <QTextEdit>
#include <QMessageBox>

ConsolidationTreeUpdater::ConsolidationTreeUpdater()
    : m_refData(NULL), m_curData(NULL)
{

}

ConsolidationTreeUpdater::~ConsolidationTreeUpdater()
{
    flushCommands();
}

bool ConsolidationTreeUpdater::prepare(const ConsolidationTreeData& refData, const ConsolidationTreeData& curData)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Consolidation Tree Updater [prepare]: starting");

    // flush commands
    flushCommands();

    // reset error message
    m_errorMessage.clear();

    // Check reference consolidation tree data is valid
    if (refData.isValid() == false)
    {
        m_errorMessage = "Consolidation Tree Updater [prepare]: failed - reference tree is not valid";
        GSLOG(SYSLOG_SEV_WARNING, m_errorMessage.toLatin1().data());

        return false;
    }

    // Check modified consolidation tree data is valid
    if (curData.isValid() == false)
    {
        m_errorMessage = "Consolidation Tree Updater [prepare]: failed - modified tree is not valid";
        GSLOG(SYSLOG_SEV_WARNING, m_errorMessage.toLatin1().data());

        return false;
    }

    // Copy consolidation trees
    m_refData = refData;
    m_curData = curData;

    // check changes at Wafer Sort level
    doTestingStage("Wafer Sort");
    doTestingStage("Final Test");

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Consolidation Tree Updater [prepare]: done [%1 command(s) to execute]")
          .arg( m_commands.count()).toLatin1().constData());

    return true;
}

bool ConsolidationTreeUpdater::execute(GexDbPlugin_Galaxy *pPlugin)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Consolidation Tree Updater [execute]: starting");

    bool success = false;

    m_errorMessage.clear();

    if (pPlugin)
    {
        for(int idx = 0; idx < m_commands.count(); ++idx)
        {
            if (m_commands.at(idx)->isHeavyUpdate())
            {
                QMessageBox msgBox;
//                if (pPlugin-
//                    pGexSkin->applyPalette(&msgBox);

                QString strMessage = "Your GexDB";
                strMessage+= " consolidated table needs to be re-generated due to your changes in the consolidation tree.\n";
                strMessage+= "This may take several minutes to a few hours depending on the size of your splitlot table and the performance of your database server.\n";
                strMessage+= "\nDo you confirm ? ";

                msgBox.setWindowTitle("Updating Consolidated Table");
                msgBox.setText(strMessage);
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Cancel);

                msgBox.setButtonText( QMessageBox::Yes, "Update" );
                msgBox.setButtonText( QMessageBox::Cancel, "&Cancel" );

                // If not, do nothing
                if(msgBox.exec() == QMessageBox::Cancel)
                {
                    pPlugin->InsertIntoUpdateLog("  DB update cancelled\n");

                    return success;
                }
                break;
            }
        }

        success = true;

        while (m_commands.isEmpty() == false && success)
        {
            CTAbstractCommand * pCommand = m_commands.takeFirst();

            // Execute command
            if (pCommand->execute(pPlugin) == false)
            {
                success = false;
                m_errorMessage = "Consolidation Tree updater [execute]: failed - " + pCommand->errorMessage();
                GSLOG(SYSLOG_SEV_WARNING, m_errorMessage.toLatin1().data());
            }

            delete pCommand;
        }
    }
    else
    {
        m_errorMessage = "Consolidation Tree update: Database plugin is not instantiated";
        GSLOG(SYSLOG_SEV_WARNING, m_errorMessage.toLatin1().data());
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Consolidation Tree Updater [execute]: %1")
          .arg( (success) ? "success" : "failed").toLatin1().constData());

    return success;
}

const QString &ConsolidationTreeUpdater::errorMessage() const
{
    return m_errorMessage;
}

void ConsolidationTreeUpdater::flushCommands()
{
    while (m_commands.isEmpty() == false)
        delete m_commands.takeFirst();
}

bool ConsolidationTreeUpdater::doProduct()
{
    foreach(m_currentProduct, m_products)
    {
        QList<CTPeriod> listPeriod = createConsolidationPeriods();

        foreach(const CTPeriod& period, listPeriod)
        {
            if (doTestCondition(period) == false)
                doSet(period);
        }
    }

    return true;
}

bool ConsolidationTreeUpdater::doTestingStage(const QString &testingStage)
{
    m_currentTestingStage   = testingStage;
    m_products              = retrieveProducts();

    // Check for modified products having a consolidation entry
    doProduct();

    // check for modified products based on default rules
    doDefault();

    // check modified options
    doOptions();

    return true;
}

bool ConsolidationTreeUpdater::doDefault()
{
    m_currentProduct.clear();
    m_hasTestCondition = false;

    doSet(CTPeriod(), true);

    return true;
}

bool ConsolidationTreeUpdater::doTestCondition(const CTPeriod &period)
{
    ConsolidationTreeQueryEnginePrivate qEngineOld(m_refData);
    ConsolidationTreeQueryEnginePrivate qEngineNew(m_curData);

    QString         strPeriod = period.beginDate().toString(Qt::ISODate) + "|" + period.endDate().toString(Qt::ISODate);
    CTQueryFilter   qFilter;

    qFilter.add(CTQueryFilter::FilterTestingStage,  m_currentTestingStage);
    qFilter.add(CTQueryFilter::FilterProductID,     m_currentProduct);
    qFilter.add(CTQueryFilter::FilterDate,          strPeriod);

    QList<QDomElement> listOldElement = qEngineOld.findConditionElement(qFilter);
    QList<QDomElement> listNewElement = qEngineNew.findConditionElement(qFilter);

    //
    m_hasTestCondition = (listOldElement.count() > 0);

    if (listOldElement.count() != listNewElement.count())
    {
        CTUpdateSplitlotCommand  * pCommand = new CTUpdateSplitlotCommand();

        pCommand->setTestingStage(m_currentTestingStage);
        pCommand->setIncludedProducts(QStringList(m_currentProduct));
        pCommand->setBeginDate(period.beginDate());
        pCommand->setEndDate(period.endDate());

        m_commands.append(pCommand);

        return true;
    }

    foreach(const QDomElement& oldConditon, listOldElement)
    {
        bool valid = false;

        foreach(const QDomElement& newCondition, listNewElement)
        {
            if (oldConditon.attribute(CT_XML_ATTR_CONDITION_NAME).toLower() == newCondition.attribute(CT_XML_ATTR_CONDITION_NAME).toLower() &&
                oldConditon.attribute(CT_XML_ATTR_SPLITLOT_FIELD).toLower() == newCondition.attribute(CT_XML_ATTR_SPLITLOT_FIELD).toLower())
            {
                valid = true;

                QStringList allowedValueOld = oldConditon.attribute(CT_XML_ATTR_ALLOWED_VALUES).toLower().split("|");
                QStringList allowedValueNew = newCondition.attribute(CT_XML_ATTR_ALLOWED_VALUES).toLower().split("|");

                foreach(const QString& value, allowedValueOld)
                {
                    if (allowedValueNew.contains(value) == false)
                        valid = false;
                }
            }
        }

        if (!valid)
        {
            CTUpdateSplitlotCommand  * pCommand = new CTUpdateSplitlotCommand();

            pCommand->setTestingStage(m_currentTestingStage);
            pCommand->setIncludedProducts(QStringList(m_currentProduct));
            pCommand->setBeginDate(period.beginDate());
            pCommand->setEndDate(period.endDate());

            m_commands.append(pCommand);

            return true;
        }
    }

    return false;
}

bool ConsolidationTreeUpdater::doSet(const CTPeriod &period, bool defaultOnly)
{
    QList<CTPeriod> listPeriod = createSetPeriods(period, defaultOnly);

    foreach(const CTPeriod& period, listPeriod)
    {
        doRules(period, defaultOnly);
    }

    return true;
}

bool ConsolidationTreeUpdater::doRules(const CTPeriod &period, bool defaultOnly)
{
    ConsolidationTreeQueryEnginePrivate qEngineOld(m_refData);
    ConsolidationTreeQueryEnginePrivate qEngineNew(m_curData);

    if (defaultOnly)
    {
        qEngineOld.setRuleSources(ConsolidationTreeQueryEnginePrivate::FromDefault);
        qEngineNew.setRuleSources(ConsolidationTreeQueryEnginePrivate::FromDefault);
    }

    QString         strPeriod = period.beginDate().toString(Qt::ISODate) + "|" + period.endDate().toString(Qt::ISODate);
    CTQueryFilter   qFilter;

    qFilter.add(CTQueryFilter::FilterTestingStage,  m_currentTestingStage);
    if (m_currentProduct.isEmpty() == false)
        qFilter.add(CTQueryFilter::FilterProductID,     m_currentProduct);
    qFilter.add(CTQueryFilter::FilterDate,          strPeriod);

    QList<QDomElement> listOldElement = qEngineOld.findRuleElement(qFilter);
    QList<QDomElement> listNewElement = qEngineNew.findRuleElement(qFilter);

    if (listOldElement.count() != listNewElement.count())
    {
        CTUpdateSplitlotCommand  * pCommand = new CTUpdateSplitlotCommand();

        pCommand->setTestingStage(m_currentTestingStage);
        if (m_currentProduct.isEmpty())
            pCommand->setExcludedProducts(m_products);
        else
            pCommand->setIncludedProducts(QStringList(m_currentProduct));

        pCommand->setBeginDate(period.beginDate());
        pCommand->setEndDate(period.endDate());

        m_commands.append(pCommand);

        return true;
    }

    foreach(const QDomElement& oldElement, listOldElement)
    {
        bool valid = false;

        if ((m_hasTestCondition && oldElement.attribute(CT_XML_ATTR_GROUP_BY).toLower() == CT_XML_VALUE_GROUP_BY_NONE) ||
                (!m_hasTestCondition && oldElement.attribute(CT_XML_ATTR_GROUP_BY).toLower() != CT_XML_VALUE_GROUP_BY_NONE))
            continue;

        foreach(const QDomElement& newElement, listNewElement)
        {
            if ((m_hasTestCondition && newElement.attribute(CT_XML_ATTR_GROUP_BY).toLower() == CT_XML_VALUE_GROUP_BY_NONE) ||
                    (!m_hasTestCondition && newElement.attribute(CT_XML_ATTR_GROUP_BY).toLower() != CT_XML_VALUE_GROUP_BY_NONE))
                continue;

            if ((oldElement.attribute(CT_XML_ATTR_NAME).toLower() == newElement.attribute(CT_XML_ATTR_NAME).toLower()) &&
                (oldElement.attribute(CT_XML_ATTR_ALGO).toLower() == newElement.attribute(CT_XML_ATTR_ALGO).toLower()) &&
                (oldElement.attribute(CT_XML_ATTR_DATA_TYPE).toLower() == newElement.attribute(CT_XML_ATTR_DATA_TYPE).toLower()) &&
                (oldElement.attribute(CT_XML_ATTR_GROUP_BY).toLower() == newElement.attribute(CT_XML_ATTR_GROUP_BY).toLower()) &&
                (oldElement.attribute(CT_XML_ATTR_STORED_RESULTS).toLower() == newElement.attribute(CT_XML_ATTR_STORED_RESULTS).toLower()))
            {
                valid = true;
            }
        }

        if (!valid)
        {
            CTUpdateSplitlotCommand  * pCommand = new CTUpdateSplitlotCommand();

            pCommand->setTestingStage(m_currentTestingStage);
            if (m_currentProduct.isEmpty())
                pCommand->setExcludedProducts(m_products);
            else
                pCommand->setIncludedProducts(QStringList(m_currentProduct));
            pCommand->setBeginDate(period.beginDate());
            pCommand->setEndDate(period.endDate());

            m_commands.append(pCommand);

            return true;
        }
    }

    return false;
}

bool ConsolidationTreeUpdater::doOptions()
{
    ConsolidationTreeQueryEnginePrivate qEngineOld(m_refData);
    ConsolidationTreeQueryEnginePrivate qEngineNew(m_curData);
    QStringList                         options;
    CTQueryFilter                       qFilter;

    qFilter.add(CTQueryFilter::FilterTestingStage,  m_currentTestingStage);

    options = retrieveOptions();

    foreach(const QString& option, options)
    {
        qFilter.add(CTQueryFilter::FilterOption,  option);

        QList<QDomElement>  listOldElement = qEngineOld.findOption(qFilter);
        QList<QDomElement>  listNewElement = qEngineNew.findOption(qFilter);

        foreach(const QDomElement& oldOption, listOldElement)
        {
            bool found = false;

            foreach(const QDomElement& newOption, listNewElement)
            {
                if (oldOption.tagName() == newOption.tagName())
                {
                    found = true;

                    if (oldOption.text() != newOption.text())
                    {
                        CTUpdateStartTimeCommand  * pCommand = new CTUpdateStartTimeCommand();

                        pCommand->setTestingStage(m_currentTestingStage);
                        pCommand->setStartTimeFunction(newOption.text());

                        m_commands.append(pCommand);

                        return true;
                    }
                }
            }

            if (!found)
            {
                CTUpdateStartTimeCommand  * pCommand = new CTUpdateStartTimeCommand();

                pCommand->setTestingStage(m_currentTestingStage);
                pCommand->setStartTimeFunction("min_start_time");

                m_commands.append(pCommand);

                return true;
            }
        }
    }

    return true;
}

QStringList ConsolidationTreeUpdater::retrieveProducts()
{
    ConsolidationTreeQueryEnginePrivate qEngineOld(m_refData);
    ConsolidationTreeQueryEnginePrivate qEngineNew(m_curData);
    QList<QDomElement>                  productsElement;
    CTQueryFilter                       qFilter;
    QStringList                         products;

    qFilter.add(CTQueryFilter::FilterTestingStage,  m_currentTestingStage);

    // Find old products list
    productsElement = qEngineOld.findGroupElement(qFilter);

    foreach(const QDomElement& group, productsElement)
    {
        products.append(group.attribute(CT_XML_ATTR_PRODUCTS_ID).split("|"));
    }

    // Find new products list
    productsElement = qEngineNew.findGroupElement(qFilter);

    foreach(const QDomElement& group, productsElement)
    {
        QStringList tmpProducts = group.attribute(CT_XML_ATTR_PRODUCTS_ID).split("|");

        foreach(const QString& product, tmpProducts)
        {
            if (products.contains(product) == false)
                products.append(product);
        }
    }

    return products;
}

QStringList ConsolidationTreeUpdater::retrieveOptions()
{
    ConsolidationTreeQueryEnginePrivate qEngineOld(m_refData);
    ConsolidationTreeQueryEnginePrivate qEngineNew(m_curData);
    QList<QDomElement>                  optionsElement;
    CTQueryFilter                       qFilter;
    QStringList                         options;

    qFilter.add(CTQueryFilter::FilterTestingStage,  m_currentTestingStage);

    // Find old products list
    optionsElement = qEngineOld.findOption(qFilter);

    foreach(const QDomElement& option, optionsElement)
        options.append(option.tagName());

    // Find new products list
    optionsElement = qEngineNew.findOption(qFilter);

    foreach(const QDomElement& option, optionsElement)
    {
        if (options.contains(option.tagName()) == false)
            options.append(option.tagName());
    }

    return options;
}


QList<CTPeriod> ConsolidationTreeUpdater::createConsolidationPeriods()
{
    ConsolidationTreeQueryEnginePrivate qEngineOld(m_refData);
    ConsolidationTreeQueryEnginePrivate qEngineNew(m_curData);

    CTQueryFilter   qFilter;

    qFilter.add(CTQueryFilter::FilterTestingStage,  m_currentTestingStage);
    qFilter.add(CTQueryFilter::FilterProductID,     m_currentProduct);

    QList<QDomElement> listOldElement = qEngineOld.findConsolidationElement(qFilter);
    QList<QDomElement> listNewElement = qEngineNew.findConsolidationElement(qFilter);

    QList<CTPeriod> listPeriod;

    QList<CTPeriod> oldPeriods;
    QList<CTPeriod> newPeriods;

    foreach(const QDomElement& oldElement, listOldElement)
    {
        QDate       begin = QDate::fromString(oldElement.attribute(CT_XML_ATTR_BEGIN_DATE, ""), Qt::ISODate);
        QDate       end   = QDate::fromString(oldElement.attribute(CT_XML_ATTR_END_DATE, ""), Qt::ISODate);
        CTPeriod    period(begin, end);

        oldPeriods.append(period);
    }

    foreach(const QDomElement& newElement, listNewElement)
    {
        QDate       begin = QDate::fromString(newElement.attribute(CT_XML_ATTR_BEGIN_DATE, ""), Qt::ISODate);
        QDate       end   = QDate::fromString(newElement.attribute(CT_XML_ATTR_END_DATE, ""), Qt::ISODate);
        CTPeriod    period(begin, end);

        newPeriods.append(period);
    }

    // Sort period by chronological order
    qSort(oldPeriods);
    qSort(newPeriods);

    listPeriod = createPeriods(oldPeriods, newPeriods, CTPeriod());

    return listPeriod;
}

QList<CTPeriod> ConsolidationTreeUpdater::createPeriods(const QList<CTPeriod> &oldPeriods, const QList<CTPeriod> &newPeriods, const CTPeriod &filterPeriod)
{
    QList<CTPeriod> listPeriod;
    QList<CTPeriod> tmpPeriods;
    QDate           beginDate = filterPeriod.beginDate();
    QDate           endDate   = filterPeriod.endDate();

    if (oldPeriods.count() > 0)
    {
        CTPeriod  period;

        // Check there is no overlap between periods
        for (int idx = 0; idx < oldPeriods.count(); idx++)
        {
            period = oldPeriods.at(idx);

            if ((beginDate.isNull() && beginDate < period.beginDate()) && period.contains(beginDate) == false)
            {
                CTPeriod interPeriod;

                interPeriod.setBeginDate(beginDate);
                interPeriod.setEndDate(period.beginDate().addDays(-1));

                tmpPeriods.append(interPeriod);
            }

            if (period.endDate().isNull() == false)
                beginDate = period.endDate().addDays(1);

            tmpPeriods.append(period);
        }

        if ((period.endDate().isNull() == false && endDate.isNull()) ||
            (period.endDate().isNull() == false && endDate.isNull() == false && period.endDate() < endDate))
        {
            CTPeriod interPeriod;

            interPeriod.setBeginDate(period.endDate().addDays(1));
            interPeriod.setEndDate(endDate);

            tmpPeriods.append(interPeriod);
        }
    }
    else
        tmpPeriods.append(CTPeriod(beginDate, endDate));

    beginDate   = filterPeriod.beginDate();
    endDate     = filterPeriod.endDate();

    QListIterator<CTPeriod> itTmpPeriods(tmpPeriods);
    QListIterator<CTPeriod> itNewPeriods(newPeriods);
    CTPeriod                newPeriod;
    CTPeriod                tmpPeriod;
    bool                    endTmpPeriod = false;

    if (itNewPeriods.hasNext())
        newPeriod = itNewPeriods.next();

    while (itTmpPeriods.hasNext())
    {
        tmpPeriod       = itTmpPeriods.next();
        endTmpPeriod    = false;

        while((beginDate <= tmpPeriod.endDate() || tmpPeriod.endDate().isNull()) && endTmpPeriod == false)
        {
            if ((beginDate > newPeriod.endDate()) && (newPeriod.endDate().isNull() == false))
            {
                if (itNewPeriods.hasNext())
                    newPeriod = itNewPeriods.next();
            }

            if (beginDate < newPeriod.beginDate())
            {
                if (newPeriod.beginDate() < tmpPeriod.endDate())
                {
                    listPeriod.append(CTPeriod(beginDate, newPeriod.beginDate().addDays(-1)));

                    beginDate = newPeriod.beginDate();
                }
                else
                {
                    listPeriod.append(CTPeriod(beginDate, tmpPeriod.endDate()));

                    if (tmpPeriod.endDate().isNull() == false)
                        beginDate = tmpPeriod.endDate().addDays(1);
                    else
                        endTmpPeriod = true;
                }
            }
            else
            {
                if (newPeriod.endDate().isNull() == false && newPeriod.endDate() < tmpPeriod.endDate())
                {
                    listPeriod.append(CTPeriod(beginDate, newPeriod.endDate()));

                    beginDate = newPeriod.endDate().addDays(1);
                }
                else
                {
                    listPeriod.append(CTPeriod(beginDate, tmpPeriod.endDate()));

                    if (tmpPeriod.endDate().isNull() == false)
                        beginDate = tmpPeriod.endDate().addDays(1);
                    else
                        endTmpPeriod = true;
                }
            }
        }
    }

    return listPeriod;
}

QList<CTPeriod> ConsolidationTreeUpdater::createSetPeriods(const CTPeriod &period, bool defaultOnly)
{
    ConsolidationTreeQueryEnginePrivate qEngineOld(m_refData);
    ConsolidationTreeQueryEnginePrivate qEngineNew(m_curData);

    if (defaultOnly)
    {
        qEngineOld.setRuleSources(ConsolidationTreeQueryEnginePrivate::FromDefault);
        qEngineNew.setRuleSources(ConsolidationTreeQueryEnginePrivate::FromDefault);
    }

    QString         strPeriod = period.beginDate().toString(Qt::ISODate) + "|" + period.endDate().toString(Qt::ISODate);
    CTQueryFilter   qFilter;

    qFilter.add(CTQueryFilter::FilterTestingStage,  m_currentTestingStage);
    qFilter.add(CTQueryFilter::FilterProductID,     m_currentProduct);
    qFilter.add(CTQueryFilter::FilterDate,          strPeriod);

    QList<QDomElement> listOldElement = qEngineOld.findSetElement(qFilter);
    QList<QDomElement> listNewElement = qEngineNew.findSetElement(qFilter);

    QList<CTPeriod> listPeriod;
    QList<CTPeriod> oldPeriods;
    QList<CTPeriod> newPeriods;

    foreach(const QDomElement& oldElement, listOldElement)
    {
        QDate       begin = QDate::fromString(oldElement.attribute(CT_XML_ATTR_BEGIN_DATE, period.beginDate().toString(Qt::ISODate)), Qt::ISODate);
        QDate       end   = QDate::fromString(oldElement.attribute(CT_XML_ATTR_END_DATE, period.endDate().toString(Qt::ISODate)), Qt::ISODate);
        CTPeriod    oldPeriod(begin, end);

        oldPeriods.append(oldPeriod);
    }

    foreach(const QDomElement& newElement, listNewElement)
    {
        QDate       begin = QDate::fromString(newElement.attribute(CT_XML_ATTR_BEGIN_DATE, period.beginDate().toString(Qt::ISODate)), Qt::ISODate);
        QDate       end   = QDate::fromString(newElement.attribute(CT_XML_ATTR_END_DATE, period.endDate().toString(Qt::ISODate)), Qt::ISODate);
        CTPeriod    newPeriod(begin, end);

        newPeriods.append(newPeriod);
    }

    // Sort period by chronological order
    qSort(oldPeriods);
    qSort(newPeriods);

    listPeriod = createPeriods(oldPeriods, newPeriods, period);

    return listPeriod;
}
