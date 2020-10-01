#ifndef GS_GEX_QTITLEFOR_H
#define GS_GEX_QTITLEFOR_H

#include <QString>
#include <QDialog>

namespace GS
{
namespace Gex
{
/**
 * \brief implementation details features
 */
namespace details
{
/**
 * @brief The empty struct, empty type
 */
struct empty {};

/**
 * @brief base class encapsulating a title that could be flagged as custom
 */
class QTitle
{
public :
  /**
   * @brief access on the title
   * @return the title
   */
  const QString & getTitle() const { return mTitle; }

  /**
   * @brief basic setter
   * @param title the title to set
   */
  void setTitle( const QString &title ) { mTitle = title; }

  /**
   * @brief access on custom status of this title
   * @return true if custom title is set, false otherwise
   */
  bool isCustomTitle() const { return mIsCustom; }

  /**
   * @brief set custom flag on
   */
  void setCustomTitleFlag() { mIsCustom = true; }

  /**
   * @brief set custom flag off
   */
  void clearCustomTitleFlag() { mIsCustom = false; }

  /**
   * @brief basic implicit constructor
   * @param title the initialized title
   */
  QTitle( const QString &title ) :
    mTitle( title ),
    mIsCustom( false ) {}

private :
  /**
   * @brief the title
   */
  QString mTitle;

  /**
   * @brief flag indicating this title is a custom one
   */
  bool mIsCustom;
};
}

/**
 * \brief class that can be used to add some titling capabilities for controls
 * This general form is undefined
 *
 * \note for each different base class used in control which need a title, you
 * have to create a specialization. In this specialization, create a delegating
 * constructor
 */
template< class = details::empty >
  class QTitleFor;

/**
 * \brief explicit specialization to use in a class having no base
 */
template<>
  class QTitleFor< details::empty > :
  public details::QTitle
  {
  public :
    /**
     * @brief simple delegating constructor
     * @param title the title to set initially
     */
    QTitleFor( const QString &title ) :
      details::QTitle( title ){}
  };

/**
 * \brief explicit specialization for a class having QDialog as base class
 */
template<>
  class QTitleFor< QDialog > :
  public QDialog, public details::QTitle
  {
  public :
    /**
     * @brief delegating constructor to base
     * @param parent parent widget of the QDialog base
     * @param f windows flag used in QDialog base
     */
    QTitleFor
      ( QWidget *parent, Qt::WindowFlags f, const QString &title = QString() ) :
      QDialog( parent, f ),
      details::QTitle( title ){}
  };
}
}

#endif // GS_GEX_QTITLEFOR_H