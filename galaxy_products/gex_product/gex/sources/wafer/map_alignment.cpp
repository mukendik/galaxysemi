#include "map_alignment.h"
#include "wafer_transform.h"
#include "pat_info.h"
#include "gqtl_log.h"


namespace GS
{
namespace Gex
{


MapAlignment::MapAlignment()
    : mAxisDirectionOpposedAction(MapAlignment::NoAction)
{
}

MapAlignment::OpposireAxisDirAction MapAlignment::GetOppositeAxisDirAction() const
{
    return mAxisDirectionOpposedAction;
}

void MapAlignment::SetOppositeAxisDirAction(MapAlignment::OpposireAxisDirAction aAction)
{
    mAxisDirectionOpposedAction = aAction;
}

bool MapAlignment::
PerformMapAlignment(const CWaferMap &refMap,
                                       CWaferMap &otherMap,
                                       WaferTransform& transform)
{
    if (refMap.GetReferenceDieLocation().count() == 0)
    {
        mErrorMessage = "Cannot align maps: No reference die location for reference map";

        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());

        return false;
    }

    if (otherMap.GetReferenceDieLocation().count() == 0)
    {
        mErrorMessage = "Cannot align maps: No reference die location for other map";

        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());

        return false;
    }

    if (refMap.GetReferenceDieLocation().count() != otherMap.GetReferenceDieLocation().count())
    {
        mErrorMessage = "Cannot align maps: Maps does not contain the same number of reference die locations";

        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());

        return false;
    }

    for (int lIdx = 0; lIdx < refMap.GetReferenceDieLocation().count(); ++lIdx)
    {
        // Check that both maps have a valid reference die
        if (refMap.GetReferenceDieLocation().at(lIdx).IsValid() == false)
        {
            mErrorMessage = "Cannot align maps: STDF reference location is invalid";

            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());

            return false;
        }

        if (otherMap.GetReferenceDieLocation().at(lIdx).IsValid() == false)
        {
            mErrorMessage = "Cannot align maps: External reference location is invalid";

            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());

            return false;
        }
    }

    if (refMap.cWaferFlat_Active == ' ')
    {
        mErrorMessage = "Cannot align maps: STDF map orientation not defined";

        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());

        return false;
    }

    if (otherMap.cWaferFlat_Active == ' ')
    {
        mErrorMessage = "Cannot align maps: External map orientation not defined";

        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());

        return false;
    }

    // Set the origin point for the transformation
    transform.SetOrigin(GS::Gex::WaferCoordinate(0,0));

    // Need to have both orientation normalized in order to compare them
    char lRefWaferActive    = CWaferMap::ToggleOrientation(refMap.cWaferFlat_Active,
                                                           refMap.GetPosXDirection(),
                                                           refMap.GetPosYDirection());
    char lOtherWaferActive  = CWaferMap::ToggleOrientation(otherMap.cWaferFlat_Active,
                                                           otherMap.GetPosXDirection(),
                                                           otherMap.GetPosYDirection());

    // Only check axis direction when required
    if (mAxisDirectionOpposedAction != MapAlignment::NoAction)
    {
        if (mAxisDirectionOpposedAction == MapAlignment::Reject)
        {
            if (refMap.GetPosXDirection() != otherMap.GetPosXDirection())
            {
                // Axis direction don't match between maps
                mErrorMessage = "Cannot align maps, X Axis direction are not the same";

                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());

                return false;
            }

            if (refMap.GetPosYDirection() != otherMap.GetPosYDirection())
            {
                // Axis direction don't match between maps
                mErrorMessage = "Cannot align maps, Y Axis direction are not the same";

                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());

                return false;
            }
        }
        else if (mAxisDirectionOpposedAction == MapAlignment::Reflect)
        {
            if (refMap.GetPosXDirection() != otherMap.GetPosXDirection() &&
                    refMap.GetPosYDirection() == otherMap.GetPosYDirection())
            {
                transform.Reflect(false, true);

                if (otherMap.cWaferFlat_Active == 'L')
                {
                    lOtherWaferActive = CWaferMap::ToggleOrientation('R',
                                                                     otherMap.GetPosXDirection(),
                                                                     otherMap.GetPosYDirection());
                }
                else if (otherMap.cWaferFlat_Active == 'R')
                {
                    lOtherWaferActive = CWaferMap::ToggleOrientation('L',
                                                                     otherMap.GetPosXDirection(),
                                                                     otherMap.GetPosYDirection());
                }
            }
            else if (refMap.GetPosYDirection() != otherMap.GetPosYDirection() &&
                     refMap.GetPosXDirection() == otherMap.GetPosXDirection())
            {
                transform.Reflect(true, false);

                if (otherMap.cWaferFlat_Active == 'U')
                {
                    lOtherWaferActive = CWaferMap::ToggleOrientation('D',
                                                                     otherMap.GetPosXDirection(),
                                                                     otherMap.GetPosYDirection());
                }
                else if (otherMap.cWaferFlat_Active == 'D')
                {
                    lOtherWaferActive = CWaferMap::ToggleOrientation('U',
                                                                     otherMap.GetPosXDirection(),
                                                                     otherMap.GetPosYDirection());
                }
            }
        }
        else
        {
            mErrorMessage = "Unknown Axis Opposed Direction command";

            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());

            return false;
        }
    }

    // Compute the rotation shift
    transform.Rotate(ComputeRotation(lRefWaferActive, lOtherWaferActive));

    // Compute the translation to apply to the external map
    if (ComputeTranslation(transform, refMap.GetReferenceDieLocation(), otherMap.GetReferenceDieLocation()) == false)
    {
        // if there is more than one reference die location, flip the wafer map based on the notch location and
        // perform another check for translation
        if (refMap.GetReferenceDieLocation().count() > 1)
        {
            // Apply a transformation either on X or Y axis to flip the wafer map
            if (otherMap.cWaferFlat_Active == 'U' || otherMap.cWaferFlat_Active == 'D')
                transform.Reflect(false, true);
            else
                transform.Reflect(true, false);

            // Check the translation is the same for all reference die
            if (ComputeTranslation(transform, refMap.GetReferenceDieLocation(), otherMap.GetReferenceDieLocation()) == false)
            {
                // Unable to find a macthing translation, alignment is rejected
                mErrorMessage = "Cannot align maps using multiple reference die, no common translation found between maps";

                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
        }
        else
        {
            // Unable to find a macthing translation, alignment is rejected
            mErrorMessage = "Cannot align maps using single reference die, no common translation found between maps";

            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());

            return false;
        }
    }
    else if (refMap.GetReferenceDieLocation().count() > 1)
    {
        // If we are in a situation were we have multiple refence die location, we must make sure we are not in a corner
        // case where the map can be aligned whatever the wafermap is flipped or not
        WaferTransform lTmpTransform(transform);

        // Apply a transformation either on X or Y axis to flip the wafer map
        if (otherMap.cWaferFlat_Active == 'U' || otherMap.cWaferFlat_Active == 'D')
            lTmpTransform.Reflect(false, true);
        else
            lTmpTransform.Reflect(true, false);

        // Check the translation is the same for all reference die
        if (ComputeTranslation(lTmpTransform, refMap.GetReferenceDieLocation(), otherMap.GetReferenceDieLocation()) == true)
        {
            // We also find a matching translation after flipping the wafermap, we are in a situation where the reference
            // die location are perfectly symetric. We can align the wafer maps whatever the side is flipped the external
            // map.
            // Reject the map alignment
            mErrorMessage = "Cannot align maps, unable to identify how to flip the wafermap due to symetric reference die location.";

            GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
            return false;
        }
    }

    // Apply wafermap transformation
    if (otherMap.Transform(transform) == false)
    {
        mErrorMessage = "Failed to transform the external map";
        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
        return false;
    }

    return true;
}

const QString &MapAlignment::GetErrorMessage() const
{
    return mErrorMessage;
}

bool MapAlignment::ComputeTranslation(WaferTransform &transform,
                                      QList<WaferCoordinate> refMapLocation,
                                      QList<WaferCoordinate> otherMapLocation)
{
    for (int lIdx = 0; lIdx < otherMapLocation.count(); ++lIdx)
    {
        otherMapLocation[lIdx] = transform.Map(otherMapLocation.at(lIdx));
    }

    // Order ref map location and other map location, so we can get the distance between each die``
    qSort(refMapLocation);
    qSort(otherMapLocation);

    // Get the translation between the first reference die location for each map
    WaferCoordinate lTranslation = refMapLocation.first() - otherMapLocation.first();

    // Check if translation is the same for all reference die location
    for (int lIdx = 1; lIdx < refMapLocation.count(); ++lIdx)
    {
        if (lTranslation != (refMapLocation[lIdx] - otherMapLocation[lIdx]))
        {
            // All reference die do not have the same translation, unable to compute the translation.
            return false;
        }
    }

    // Set the translation to apply
    transform.Translate(lTranslation.GetX(), lTranslation.GetY());

    return true;
}

int MapAlignment::ComputeRotation(char aRefWaferOrientation, char aOtherWaferOrientation)
{
    // Compute rotation in degree between both wafer orientation
    int lRotation = CWaferMap::OrientationToDegree(aRefWaferOrientation) -
            CWaferMap::OrientationToDegree(aOtherWaferOrientation);

    // Compute how many 90° rotation to make
    lRotation /= 90;

    return lRotation;
}

}
}
