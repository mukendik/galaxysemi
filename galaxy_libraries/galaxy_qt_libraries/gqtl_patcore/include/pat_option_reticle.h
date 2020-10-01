#ifndef PAT_OPTION_RETICLE
#define PAT_OPTION_RETICLE

#include <QString>
#include <QColor>
#include <gqtl_utils.h>
#include "pat_defines.h"

class PATOptionReticle : public QObject
{
    Q_OBJECT

public:
    /// \brief Constructor
    PATOptionReticle();
    PATOptionReticle(const PATOptionReticle& rule);
    PATOptionReticle&  operator=(const PATOptionReticle& rule);
    /// \brief Destructor
    virtual ~PATOptionReticle();


    /// \var mReticleSizeSource
    enum ReticleSizeSource
    {
        RETICLE_SIZE_FIXED = 0,
        RETICLE_SIZE_FILE = 1
    };

    enum ActivatedCornersFlag
    {
        NO_CORNER = 0x00,
        CORNER_TOP_LEFT = 0x01,
        TOP = 0x02,
        CORNER_TOP_RIGHT = 0x04,
        RIGHT = 0x08,
        CORNER_BOTTOM_RIGHT = 0x10,
        BOTTOM = 0x20,
        CORNER_BOTTOM_LEFT = 0x40,
        LEFT = 0x80,
        ALL_CORNER = 0xFF
    };

    Q_DECLARE_FLAGS(ActivatedCorners, ActivatedCornersFlag)

    enum Rule
    {
        REPEATING_PATTERNS = 0,
        CORNER = 1,
        STEP_DEFECTIVITY_CHECK = 2
    };

    enum FieldSelection
    {
        ALL_RETICLE_FIELDS = 0,
        LIST_RETICLE_FIELDS,
        EDGE_RETICLE_FIELDS
    };


    /// ******** GETTERS ********
    const GS::QtLib::Range& GetBadBinsReticleList() const;
    QString                 GetReticleMaskName() const;
    QString                 GetRuleName() const;
    QColor                  GetReticleColor() const;
    Rule                    GetRule() const;
    QString                 GetRuleString() const;
    double                  GetReticleYieldThreshold() const;
    bool                    IsReticleEnabled() const;
    int                     GetXInk() const;
    int                     GetYInk() const;
    int                     GetDiagInk() const;
    int                     GetXOffDiag() const;
    int                     GetYOffDiag() const;
    int                     GetReticleSBin() const;
    int                     GetReticleHBin() const;
    int                     GetReticle_WafermapSource() const;
    bool                    IgnoreDiagonalBadDies() const;
    FieldSelection          GetFieldSelection() const;
    const QList<QPair<int, int> >& GetFieldCoordinates() const;
    double                  GetFieldThreshold() const;
    /// \fn bool IsActivatedCorner(const ActivatedCorners corner) const;
    /// \brief check if the corner is activated
    /// \param corner: an enum to the corner to check
    /// \return true if the corner is activated. Otherwise return false.
    bool                IsActivatedCorner(const ActivatedCorners& corner) const;

    /// ******* SETTERS ********
    void                SetXInk(const int);
    void                SetYInk(const int);
    void                SetDiagInk(const int);
    void                SetXOffDiag(const int);
    void                SetYOffDiag(const int);
    void                SetRule(const Rule);
    void                SetRule(const QString& value);
    void                SetReticleEnabled(bool value);
    void                SetReticleSBin(int value);
    void                SetReticleHBin(int value);
    void                SetReticleColor(const QColor &value);
    void                SetReticle_WafermapSource(int value);
    void                SetReticleYieldThreshold(double value);
    void                SetBadBinsReticleList(const GS::QtLib::Range& value);
    void                SetReticleMaskName(const QString &value);
    void                SetRuleName(const QString &value);
    void                SetIgnoreDiagonalBadDies(bool value);
    void                SetFieldSelection(FieldSelection selection);
    void                SetFieldCoordinates(const QList<QPair<int,int> >& coordinates);
    void                SetFieldThreshold(double threshold);
    /// \fn void SetActivatedCorner(const ActivatedCorners corner, const bool activated);
    /// \brief Set the activation status for a corner
    /// \param corner: an enum to the corner to check
    /// \param activated: the activation status
    void                SetActivatedCorners(const PATOptionReticle::ActivatedCorners &activatedCorners);
    void                AddActivatedCorner(ActivatedCorners corner);

    static QString                 FieldCoordinatesToString(const QList<QPair<int,int> >& coordinates);
    static QList<QPair<int,int> >  FieldCoordinatesFromString(const QString& coordinates, bool *ok = NULL);

private:

    GS::QtLib::Range    mBadBinsReticleList;	///< List of Bad bins to consider when computing Reticle yield.
    QColor              mReticleColor;			///< Color for the fail bin due to reticle errors.
    Rule                mRule;                  ///<
    bool                mReticleEnabled;		///< =true if rule enabled.
    int                 mReticleSBin;			///< SoftBin to assign to bad reticle dies identified
    int                 mReticleHBin;			///< HardBin to assign to bad reticle dies identified
    int                 mReticle_WafermapSource;///< GDBN wafermap source: STDF (soft/hard bin), prober map
    QString             mRuleName;              ///< holds rule name

    // Repeating Pattern options
    double              mReticleYieldThreshold;	///< Yield level under which reticle die location rejected all over the wafer
    QString             mReticleMaskName;

    // Defictivity Check options
    FieldSelection          mFieldSelection;
    QList<QPair<int,int> >  mFieldCoordinates;
    double                  mFieldThreshold;

    // Corner rule options
    ActivatedCorners    mActivatedCorners;      ///< An int which contains the list of the ActivatedCorners.
                                                ///< Each bit represents a flag to the corresonding corner
    int                 mXInk;                  ///< holds X inking size
    int                 mYInk;                  ///< holds Y inking size
    int                 mDiagInk;               ///< holds diagonal inking size
    int                 mXOffDiag;              ///< holds X inking size from diagonal
    int                 mYOffDiag;              ///< holds Y inking size from diagonal
    bool                mIgnoreDiagBadDies;     ///< holds if diagonal bad dies have to be ignored or not
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PATOptionReticle::ActivatedCorners)


#endif // PAT_OPTION_RETICLE

