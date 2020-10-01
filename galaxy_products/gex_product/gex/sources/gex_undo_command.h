#ifndef GEX_UNDO_COMMAND_H
#define GEX_UNDO_COMMAND_H
#include <QMap>
#include <QList>
#include <QUndoCommand>
#include "ctest.h"
#include "gex_group_of_files.h"

class TestRemoveResultCommand : public QUndoCommand
{
public:
    class TestRemovedResult{
    private:
        CGexGroupOfFiles *m_poGroup;
        CTest *m_poTest;
        QList<int> m_oRunIdx;
        QMap<int, QList<int> > m_oMultiResultRunIdx;
        int m_iOutlierRemoval;
        bool m_bRemoveAssociatedParts;
    public:
        TestRemovedResult(CTest *poTest, CGexGroupOfFiles *poGroup, bool bRemoveAssociatedParts);
        ~TestRemovedResult();
        void addRemovedIdx(int iRunIdx);
        void addRemovedIdx(int iRunIdx, int iSubRunIdx);

        CTest *getTest() {
            return m_poTest;
        }

        const QList<int> &getRunIdx(){
            return m_oRunIdx;
        }

        QList<int> &getMultiResultRunIdx(int iRunIdx){
            return m_oMultiResultRunIdx[iRunIdx];
        }

        void saveTestsStats(CTest *poSrc, CTest *poDest);
        int getOutlierRemoval(){
            return m_iOutlierRemoval;
        }
        void setOutlierRemoval(int iOutlier){
            m_iOutlierRemoval = iOutlier;
        }
        CGexGroupOfFiles *getGroup(){
            return m_poGroup;
        }
        bool getRemoveAssociatedParts(){
            return m_bRemoveAssociatedParts;
        }
    };

public:
    TestRemoveResultCommand(const QString &strAction, QList<TestRemoveResultCommand::TestRemovedResult *> &oRemovedResult, QUndoCommand *poParent = 0);
    ~TestRemoveResultCommand();

    void undo();
    void redo();

private:
    void updateChart();
    void updateTest(CTest *poTest);
    void updateGroup(CGexGroupOfFiles *poGroup, QList<int> lPartOffset,bool bUndo);

private:
    QList<TestRemovedResult *>  m_oRemovedResult;
    bool m_bFirstRun;
};
#if 0
class OptionChangedCommand:  public QUndoCommand {
public:
    OptionChangedCommand(const QString &strSection, const QString &strField, const QVariant &oOld, const QVariant &oNew,QUndoCommand *poParent=0);
    ~OptionChangedCommand();
    void undo();
    void redo();
private:
     bool m_bFirstRun;
     QString m_strSection;
     QString m_strField;
     QVariant m_oOld;
     QVariant m_oNew;

};
#endif
#endif // GEX_UNDO_COMMAND_H
