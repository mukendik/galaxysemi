#include "pat_option_reticle.h"

PATOptionReticle::PATOptionReticle()
    : mBadBinsReticleList("0;2-65535")
{
    mRuleName = "";
    mReticleColor   = QColor(255,0,255);
    mReticleEnabled = true;
    // GDBN wafermap source : STDF Soft/Hardbin, Prober map
    mReticle_WafermapSource = GEX_PAT_WAFMAP_SRC_SOFTBIN;
    mReticleYieldThreshold = 50.0;
    mReticleSBin = mReticleHBin = 144;
    mReticleMaskName = "-None-";

    // Corner activation
    mRule = REPEATING_PATTERNS;
    mActivatedCorners = static_cast<ActivatedCorners>(CORNER_TOP_LEFT|CORNER_TOP_RIGHT|CORNER_BOTTOM_RIGHT|CORNER_BOTTOM_LEFT);
    mXInk       = 1;
    mYInk       = 1;
    mDiagInk    = 1;
    mXOffDiag   = 0;
    mYOffDiag   = 0;
    mIgnoreDiagBadDies = false;

    // Defectivity Check
    mFieldSelection = ALL_RETICLE_FIELDS;
    mFieldThreshold = 50.0;
}

PATOptionReticle::PATOptionReticle(const PATOptionReticle &rule)
    : QObject(rule.parent())
{
    *this = rule;
}

PATOptionReticle& PATOptionReticle::operator=(const PATOptionReticle& rule)
{
    if (this != &rule)
    {
        mRuleName = rule.mRuleName;
        mReticleEnabled = rule.mReticleEnabled;
        mReticleSBin = rule.mReticleSBin;
        mReticleHBin = rule.mReticleHBin;
        mReticleColor = rule.mReticleColor;
        mReticle_WafermapSource = rule.mReticle_WafermapSource;
        mReticleYieldThreshold = rule.mReticleYieldThreshold;

        mBadBinsReticleList = rule.mBadBinsReticleList;
        mReticleMaskName = rule.mReticleMaskName;

        mRule = rule.mRule;
        mActivatedCorners = rule.mActivatedCorners;
        mXInk = rule.mXInk;
        mYInk = rule.mYInk;
        mDiagInk = rule.mDiagInk;
        mXOffDiag = rule.mXOffDiag;
        mYOffDiag = rule.mYOffDiag;
        mIgnoreDiagBadDies  = rule.mIgnoreDiagBadDies;
        mFieldCoordinates   = rule.mFieldCoordinates;
        mFieldSelection     = rule.mFieldSelection;
        mFieldThreshold     = rule.mFieldThreshold;
    }

    return *this;
}

PATOptionReticle::~PATOptionReticle()
{
}

int PATOptionReticle::GetXInk() const
{
    return mXInk;
}

int PATOptionReticle::GetYInk() const
{
    return mYInk;
}

int PATOptionReticle::GetDiagInk() const
{
    return mDiagInk;
}

int PATOptionReticle::GetXOffDiag() const
{
    return mXOffDiag;
}

int PATOptionReticle::GetYOffDiag() const
{
    return mYOffDiag;
}

PATOptionReticle::Rule PATOptionReticle::GetRule() const
{
    return mRule;
}

QString PATOptionReticle::GetRuleString() const
{
    if (mRule == CORNER)
        return "corner_rule";
    else if (mRule == REPEATING_PATTERNS)
        return "repeating_patterns";
    else
        return "step_defectivity_check";
}

bool PATOptionReticle::IsActivatedCorner(const ActivatedCorners &corner) const
{
    return (mActivatedCorners & corner);
}

void PATOptionReticle::SetActivatedCorners(const ActivatedCorners& activatedCorners)
{
    mActivatedCorners = activatedCorners;
}

void PATOptionReticle::AddActivatedCorner(PATOptionReticle::ActivatedCorners corner)
{
    mActivatedCorners |= corner;
}

QString PATOptionReticle::FieldCoordinatesToString(const QList<QPair<int, int> > &coordinates)
{
    QString lCoordinates;

    for(int lIdx = 0; lIdx < coordinates.count(); ++lIdx)
    {
        lCoordinates += "(" + QString::number(coordinates.at(lIdx).first) + ",";
        lCoordinates += QString::number(coordinates.at(lIdx).second) + ")";
    }

    return lCoordinates;
}

QList<QPair<int, int> > PATOptionReticle::FieldCoordinatesFromString(const QString &coordinates, bool* ok)
{
    QList<QPair<int, int> > lCoordinates;
    QRegExp lCoordRE("^[\\(\\-?\\d+,\\-?\\d+\\)]*\\(\\-?\\d+,\\-?\\d+\\)$");
    QString lFieldCoords = coordinates;
    QString lCoord;
    int     lXCoord;
    int     lYCoord;

    if (lCoordRE.exactMatch(coordinates))
    {
        int lIdx = lFieldCoords.indexOf(")");

        while (lIdx > 0)
        {
            // Get the coordinate without parentheses
            lCoord  = lFieldCoords.left(lIdx).remove(0,1);

            // Extract X and Y coordinates
            lXCoord = lCoord.section(',', 0, 0).toInt();
            lYCoord = lCoord.section(',', 1, 1).toInt();

            // Add coordinaes to the field coordinates list
            lCoordinates.append(QPair<int,int>(lXCoord, lYCoord));

            if (ok)
                *ok = true;

            // Remove the analyzed coordinate from the string
            lFieldCoords = lFieldCoords.mid(lIdx+1);

            // Find next closing parenthese
            lIdx = lFieldCoords.indexOf(")");
        }
    }
    else
    {
        if (ok)
            *ok = false;
    }

    return lCoordinates;
}

QString PATOptionReticle::GetRuleName() const
{
    return mRuleName;
}

void PATOptionReticle::SetRuleName(const QString &value)
{
    mRuleName = value;
}

void PATOptionReticle::SetIgnoreDiagonalBadDies(bool value)
{
    mIgnoreDiagBadDies = value;
}

void PATOptionReticle::SetFieldSelection(PATOptionReticle::FieldSelection selection)
{
    mFieldSelection = selection;
}

void PATOptionReticle::SetFieldCoordinates(const QList<QPair<int, int> > &coordinates)
{
    mFieldCoordinates = coordinates;
}

void PATOptionReticle::SetFieldThreshold(double threshold)
{
    mFieldThreshold = threshold;
}

bool PATOptionReticle::IsReticleEnabled() const
{
    return mReticleEnabled;
}

void PATOptionReticle::SetReticleEnabled(bool value)
{
    mReticleEnabled = value;
}
int PATOptionReticle::GetReticleSBin() const
{
    return mReticleSBin;
}

void PATOptionReticle::SetReticleSBin(int value)
{
    mReticleSBin = value;
}
int PATOptionReticle::GetReticleHBin() const
{
    return mReticleHBin;
}

void PATOptionReticle::SetReticleHBin(int value)
{
    mReticleHBin = value;
}
QColor PATOptionReticle::GetReticleColor() const
{
    return mReticleColor;
}

void PATOptionReticle::SetReticleColor(const QColor &value)
{
    mReticleColor = value;
}
int PATOptionReticle::GetReticle_WafermapSource() const
{
    return mReticle_WafermapSource;
}

void PATOptionReticle::SetReticle_WafermapSource(int value)
{
    mReticle_WafermapSource = value;
}
double PATOptionReticle::GetReticleYieldThreshold() const
{
    return mReticleYieldThreshold;
}

void PATOptionReticle::SetReticleYieldThreshold(double value)
{
    mReticleYieldThreshold = value;
}

bool PATOptionReticle::IgnoreDiagonalBadDies() const
{
    return mIgnoreDiagBadDies;
}

PATOptionReticle::FieldSelection PATOptionReticle::GetFieldSelection() const
{
    return mFieldSelection;
}

const QList<QPair<int,int> > &PATOptionReticle::GetFieldCoordinates() const
{
    return mFieldCoordinates;
}

double PATOptionReticle::GetFieldThreshold() const
{
    return mFieldThreshold;
}

const GS::QtLib::Range& PATOptionReticle::GetBadBinsReticleList() const
{
    return mBadBinsReticleList;
}

void PATOptionReticle::SetBadBinsReticleList(const GS::QtLib::Range &value)
{
    mBadBinsReticleList = value;
}

QString PATOptionReticle::GetReticleMaskName() const
{
    return mReticleMaskName;
}

void PATOptionReticle::SetReticleMaskName(const QString &value)
{
    mReticleMaskName = value;
}

void PATOptionReticle::SetXInk(const int value)
{
    mXInk = value;
}

void PATOptionReticle::SetYInk(const int value)
{
    mYInk = value;
}
void PATOptionReticle::SetDiagInk(const int value)
{
    mDiagInk = value;
}
void PATOptionReticle::SetXOffDiag(const int value)
{
    mXOffDiag = value;
}
void PATOptionReticle::SetYOffDiag(const int value)
{
    mYOffDiag = value;
}
void PATOptionReticle::SetRule(const Rule value)
{
    mRule = value;
}

void PATOptionReticle::SetRule(const QString& value)
{
    if (value == "corner_rule")
        mRule = CORNER;
    else if (value == "repeating_patterns")
        mRule = REPEATING_PATTERNS;
    else
        mRule = STEP_DEFECTIVITY_CHECK;
}
