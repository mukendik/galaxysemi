/******************************************************************************!
 * \file map_merge_api.h
 ******************************************************************************/
#ifndef MAP_MERGE_API_H
#define MAP_MERGE_API_H

#include <QObject>
#include <QString>
#include <QList>

//class CWaferMap;
#include "wafermap.h"
class CBinning;

namespace GS
{
namespace Gex
{

/******************************************************************************!
 * \class MapMergeApi
 ******************************************************************************/
class MapMergeApi : public QObject
{
    Q_OBJECT

public:
    /*!
     * \fn MapMergeApi
     * \brief Constructor
     */
    MapMergeApi(QObject* parent = 0);
    /*!
     * \fn ~MapMergeApi
     * \brief Destructor
     */
    virtual ~MapMergeApi();
    /*!
     * \fn AddMap
     * \brief Appends the map to the list of maps to be merged
              The insertion order will define the map precedence
     * \return ok when succeed, otherwise returns error string
     */
    Q_INVOKABLE const QString AddMap(const QString& filename,
                                     const QString& refLocation);
    /*!
     * \fn SetMergeBinRule
     * \brief Specifies the name of the JS hook function to
              call when merging a die on the map
     * \return ok when succeed, otherwise returns error string
     */
    Q_INVOKABLE const QString SetMergeBinRule(const QString& mergeBinHook);
    /*!
     * \fn Clear
     * \brief Clear all parameters
     * \return void
     */
    Q_INVOKABLE const QString Clear();
    /*!
     * \fn Merge
     * \brief Runs the map merge
     * \return ok when succeed, otherwise returns error string
     */
    Q_INVOKABLE const QString Merge(bool autoAlignment,
                                    bool hardBin = true);
    /*!
     * \fn Export
     * \brief Export the merged map in the specified map format
              (G85, E142, etc.) into the specified file name
     * \return ok when succeed, otherwise returns error string
     */
    Q_INVOKABLE const QString Export(const QString& mapFormat,
                                     const QString& mapFileName);
    /*!
     * \fn GetErrorMessage
     * \brief Error message from merge function
     */
    Q_INVOKABLE const QString& GetErrorMessage() const;

private:
    Q_DISABLE_COPY(MapMergeApi)

    /*!
     * \fn LoadMap
     */
    bool LoadMap(const QString& lFilename,
                 CWaferMap& map,
                 CBinning** bins,
                 bool hardBin);

    /*!
     * \var mMapList
     */
    QList<QString> mMapList;
    /*!
     * \var mRefLoc
     */
    QList<QString> mRefLoc;
    /*!
     * \var mMergeBinRuleFile
     */
    QString mMergeBinRuleFile;
    /*!
     * \var mMergeBinRule
     */
    QString mMergeBinRule;
    /*!
     * \var mErrorMessage
     * \brief Error message from merge function
     */
    QString mErrorMessage;
    /*!
     * \var mOutputMap
     */
    CWaferMap mOutputMap;
    /*!
     * \var mOutputBin
     */
    CBinning* mOutputBin;
};

}
}
#endif
