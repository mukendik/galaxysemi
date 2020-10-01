#ifndef MAPALIGNMENT_H
#define MAPALIGNMENT_H

#include "wafermap.h"
#include "pat_process_ws_private.h"

namespace GS
{
namespace Gex
{
class MapAlignment
{
public:
    MapAlignment();

    enum OpposireAxisDirAction
    {
        Reject,
        Reflect,
        NoAction
    };

    OpposireAxisDirAction   GetOppositeAxisDirAction() const;
    void                    SetOppositeAxisDirAction(OpposireAxisDirAction aAction);

    /// \fn bool PerformMapAlignment
    /// \brief This function perform the alignment of a list of maps with a reference map
    /// \param refMap: the reference map
    /// \param otherMap: the map to be aligned
    /// \param transform: the tranformation
    bool PerformMapAlignment(const CWaferMap& refMap, CWaferMap &otherMap, WaferTransform &transform);

    const QString& GetErrorMessage() const;

private:

    bool ComputeTranslation(WaferTransform &transform, QList<WaferCoordinate> refMapLocation,
                            QList<WaferCoordinate> otherMapLocation);
    int  ComputeRotation(char aRefWaferOrientation, char aOtherWaferOrientation);

    QString                 mErrorMessage;
    OpposireAxisDirAction   mAxisDirectionOpposedAction;

};

} // end namespace Gex
} // end namespace GS

#endif // MAPALIGNMENT_H
