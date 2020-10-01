#ifndef GEX_TEST_TO_UPDATE_H
#define GEX_TEST_TO_UPDATE_H

#include <QList>
#include <QString>

class GexTestToUpdate
{
public:
    GexTestToUpdate();
    ~GexTestToUpdate();

    // Setters
    void setTestName(QString strTestName);
    void setTestNumber(QString strTestNumber);
    void setTestLowLimit(QString strLLimit);
    void setTestHighLimit(QString strHLimit);
    void setSource(QString strSrc);

    QString name() {return m_strTestName;}
    QString number() {return m_strTestNumber;}
    QString lowLimit() {return m_strLLimit;}
    QString highLimit() {return m_strHLimit;}
    QString source() {return m_strSource;}

private:
    QString m_strTestName;
    QString m_strTestNumber;
    QString m_strLLimit;
    QString m_strHLimit;
    QString m_strSource;
};

class GexTestToUpdateList
{
public:
    GexTestToUpdateList(){

        ;
    }
    ~GexTestToUpdateList(){
        while (!m_lstTestToUpdate.isEmpty())
            delete m_lstTestToUpdate.takeFirst();
    }

    void addTestToUpdate(GexTestToUpdate *testToUpdate){
        m_lstTestToUpdate << testToUpdate;
    }
    bool isActivated() const{
        return m_lstTestToUpdate.count() > 0;
    }

    GexTestToUpdate *getTestToUpdate(int iTestNumber) const{
        for(int iIdx=0; iIdx<m_lstTestToUpdate.count(); ++iIdx){
            if(iTestNumber == m_lstTestToUpdate[iIdx]->number().toInt())
                return m_lstTestToUpdate[iIdx];
        }
        return 0;
    }

    QList<GexTestToUpdate *>    m_lstTestToUpdate;

private:

};

#endif // GEX_TEST_TO_UPDATE_H
