#ifndef CONSOLIDATION_TREE_COMMAND_H
#define CONSOLIDATION_TREE_COMMAND_H

#include <QStringList>
#include <QDateTime>
#include <QTextEdit>

class GexDbPlugin_Galaxy;

class CTAbstractCommand
{
public:

    CTAbstractCommand()      {}
    virtual ~CTAbstractCommand() {}

    const QString&          errorMessage() const    { return m_errorMessage;}
    virtual bool            execute(GexDbPlugin_Galaxy * pPlugin) = 0;
    virtual bool            isHeavyUpdate() const;

protected:

    QString                 m_errorMessage;
};

class CTUpdateSplitlotCommand : public CTAbstractCommand
{
public:

    CTUpdateSplitlotCommand();
    ~CTUpdateSplitlotCommand();

    bool                    execute(GexDbPlugin_Galaxy * pPlugin);

    void                    setTestingStage(const QString& testingStage);
    void                    setIncludedProducts(const QStringList& products);
    void                    setExcludedProducts(const QStringList& products);
    void                    setBeginDate(const QDateTime& begin);
    void                    setBeginDate(const QDate& begin);
    void                    setEndDate(const QDateTime& end);
    void                    setEndDate(const QDate& end);


private:

    QString                 m_testingStage;
    QStringList             m_includedProducts;
    QStringList             m_excludedProducts;
    QDateTime               m_beginDate;
    QDateTime               m_endDate;
};

class CTUpdateStartTimeCommand : public CTAbstractCommand
{
public:

    CTUpdateStartTimeCommand();
    ~CTUpdateStartTimeCommand();

    bool                    execute(GexDbPlugin_Galaxy * pPlugin);
    bool                    isHeavyUpdate() const;

    void                    setTestingStage(const QString& testingStage);
    void                    setStartTimeFunction(const QString& function);

private:

    QString                 m_testingStage;
    QString                 m_startTimeFunction;
};

#endif // CONSOLIDATION_TREE_COMMAND_H
