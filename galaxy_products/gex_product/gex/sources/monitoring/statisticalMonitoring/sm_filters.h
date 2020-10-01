#ifndef SPM_FILTERS_H
#define SPM_FILTERS_H

#include "gex_database_filter.h"

#include <QWidget>

// type used as pointer or reference
class QGridLayout;
class QVBoxLayout;
class QPushButton;
class spm_filter_line;
struct spm_filter_line_infos;

namespace Ui {
class spm_filters;
}

/**
 * @brief widget representing various SPM filters
 */
class smFilters : public QWidget
{
  Q_OBJECT

public:
  /**
   * @brief parametric construction, needs a list of valid keys as filter
   * @param parent parent widget
   * @param filter_keys list of text filter keys
   */
  smFilters
    (QWidget *parent, const QStringList &filter_keys );

  /**
   * Destruction, liberation of resources
   */
  ~smFilters();

  /**
   * @brief reset all filter lines of this control, adding new uninitialized
   * lines
   */
  void reset_filter_lines();

  /**
   * @brief allow to manually set a list of valid keys to be used for further
   * filter line creations
   * @param keys list of text filter keys
   */
  void set_filter_keys( const QStringList &keys );

  /**
   * @brief set the testing stage of this filter
   * (wafer sort, final test, e-test)
   * @param stage the testing stage as string
   */
  void set_testing_stage( const QString &stage )
  {
    m_testing_stage = stage;
  }

  /**
   * @brief set the database logical name on which the filter applies
   * @param name name of the database recognizable by the database engine
   */
  void set_database_name( const QString &name )
  {
    m_database_name = name;
  }

  /**
   * @brief copy filter lines' content inside the specified map, avoid consuming
   * copies
   * @param destination the destination map of filter lines
   */
  void copy_filters_in( QMap< QString, QString > &destination );

  /**
   * @brief obtain a reference on a constant instance of a repository of
   * narrow filters
   * @return filter informations
   */
  QMap<QString, QString> get_filter() const;

  /**
   * @brief obtain a reference on a constant instance of a repository of
   * narrow filters
   * @return filter informations
   */
  GexDatabaseFilter get_gex_database_filter() const;

  /**
   * @brief load some filter from an external repository of narrow filters
   * @param database_filter some narrow filter to initialialize with
   */
  void load_from(const QMap<QString, QString>& database_filter , bool createItem=false);

  /**
   * @brief setter of products, transform the string to be SQL compliant
   * @param products a wildcard expression for product identifier
   */
  void set_products( const QString &products );

signals :
  /**
   * @brief emitted when reset button is pushed
   */
  void reset_requested();

private:
  /**
   * @brief Qt ui generated type usage
   */
  Ui::spm_filters *ui;

  /**
   * @brief internally set up the layout if this widget
   */
  void setup_dynamic_layout();

  /**
   * @brief add an empty default-initialized filter line in the widget
   * @return the line that has just been added
   */
  spm_filter_line * add_filter_line();

  /**
   * @brief add an initialized filter line.
   * @param key_text Key of the filter, must be in the filter key list provided
   * either in construction or by set_filter_keys method call
   * @param value_text the value text of the line
   * @return the line inserted
   */
  spm_filter_line * add_filter_line
    ( const QString &key_text, const QString &value_text , bool createItem=false);

  /**
   * @brief makes the connection of signal/slot of a specific filter line.
   * @param line the line to use in the connection
   */
  void connect_line( spm_filter_line *line );

  /**
   * @brief creates a popup window containing available values regarding the
   * filter key used and the other initialized filter lines in the widget
   * @param line reference on the control containing the value filter text;
   * target of the user selection
   */
  void pick_filter_from_live_list( spm_filter_line &line );

  /**
   * @brief left side of the widget, used to initialize the dynamic layout
   */
  QVBoxLayout *m_left_column_vbox;

  /**
   * @brief right side of the widget, used to initialize the dynamic layout
   */
  QGridLayout *m_right_column_grid;

  /**
   * @brief button sued to reset all filter lines
   */
  QPushButton *m_reset_button;
  QIcon mIcon;

  /**
   * @brief container of all filter line initialized in this widget
   */
  QList< spm_filter_line * > m_filter_lines;

  /**
   * @brief filter keys being selectable inside each filter line
   */
  QStringList m_filter_keys;

  /**
   * @brief logical name of the database on which filters apply
   */
  QString m_database_name;

  /**
   * @brief The testing stage currently selected
   */
  QString m_testing_stage;

  /**
   * @brief expression that could contains wildcards. Represents products
   * identifiers
   */
  QString m_products;

private slots :
  /**
   * @brief internally reacts on the reset button push, emitting a local signal
   */
  void on_forward_reset_request( bool );

  /**
   * @brief internally reacts on the filter key change, emitting a local signal
   * @param infos information on the filter line (key and value index/text)
   */
  void on_filter_key_index_changed( const spm_filter_line_infos &infos );

  /**
   * @brief internally reacts on the picker push, emitting a local signal
   * @param reference on the control containing the filter value
   */
  void on_line_picker_clicked( spm_filter_line &filter_line );
};

#endif // SPM_FILTERS_H
