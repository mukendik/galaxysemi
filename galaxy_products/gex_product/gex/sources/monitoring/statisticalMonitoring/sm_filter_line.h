#ifndef SPM_FILTER_LINE_H
#define SPM_FILTER_LINE_H

#include <QWidget>

namespace Ui {
class spm_filter_line;
}

/**
 * @brief a derived of QPair, representing a filter line element information
 * used in some signal/slot connections. Designed to create constant instances
 */
struct spm_filter_line_element_infos :
  public QPair< const int, const QString >
  {
  public :
    /**
     * @brief convenient type alias
     */
    typedef QPair< const int, const QString > base_type;

    /**
     * @brief parametric construction, initializing constant members
     * @param index index of the element in the list
     * @param text text value of the element
     */
    spm_filter_line_element_infos( const int index, const QString text );

    /**
     * @brief simple accessor on the index of the element in the list
     * @return the index in the list
     */
    const int get_index() const;

    /**
     * @brief simple accessor on the text value of the element in the list
     * @return the text representation of this element
     */
    const QString & get_text() const;
  };

/**
 * @brief Derived of a QPair representing a filter line information, containing
 * both key and value information of a filter line
 */
struct spm_filter_line_infos :
  public QPair
  < const spm_filter_line_element_infos, const spm_filter_line_element_infos >
  {
  public :
    /**
     * @brief convenient alias
     */
    typedef
      QPair
      <
        const spm_filter_line_element_infos,
        const spm_filter_line_element_infos
      >
      base_type;

    /**
     * @brief parametric construction initializing constant members
     * @param filter_key_infos informations on the key of a filter line
     * @param filter_value_infos informations on the value of a filter line
     */
    spm_filter_line_infos
      (
        const spm_filter_line_element_infos &filter_key_infos,
        const spm_filter_line_element_infos &filter_value_infos
      );

    /**
     * @brief simple accessor on the key informations
     * @return key informations
     */
    const spm_filter_line_element_infos & get_key_infos() const;

    /**
     * @brief simple accessor on the value informations
     * @return value informations
     */
    const spm_filter_line_element_infos & get_value_infos() const;
  };

/**
 * @brief The spm_filter_line class represents a part of an overall filter.
 */
class spm_filter_line : public QWidget
{
  Q_OBJECT

public:
  /**
   * @brief classical widget construction
   * @param parent the supposed parent of the widget at construction
   */
  explicit spm_filter_line(QWidget *parent = 0);

  /**
   * @brief Destruction, freeing some resources
   */
  ~spm_filter_line();

  /**
   * @brief resets the current line, setting the current index of both key and
   * value combo to 0
   */
  void reset();

  /**
   * @brief indicates if a valid key is selected.
   * @return true if the index of the selected key is greater than 0, false
   * otherwise
   */
  bool valid_key_selected() const;

  /**
   * @brief specifies all the keys that are selectable as keys
   * @param keys key list considered as valid
   */
  void append_filter_keys( const QStringList &keys );

  /**
   * @brief accessor on the text representation of the key filter
   * @return the text representation of the key filter
   */
  QString key_text() const;

  /**
   * @brief accessor on the text representation of the value filter
   * @return the text representation of the value filter
   */
  QString value_text() const;

  /**
   * @brief modify the text representation of the filter value
   * @param text new text representation of the filter value
   */
  void set_value_text( const QString &text );

  /**
   * @brief modifies the text representation of both key and value filter
   * @param key_text new text representation of the key
   * @param value_text new text representation of the value
   */
  void set_line_with( const QString &key_text, const QString &value_text , bool createItem=false);

  /**
   * @brief extracts and exposes this filter line information about key and
   * value
   * @return key and value filter informations
   */
  const spm_filter_line_infos get_infos() const;

private:
  /**
   * @brief Qt ui resources
   */
  Ui::spm_filter_line *ui;

private slots :
  /**
   * @brief reacts on some signals exposed by inner controls
   */
  void on_filter_key_activated( QString );

  /**
   * @brief reacts on some signals exposed by inner controls
   */
  void on_filter_value_activated( QString );

  /**
   * @brief reacts on some signals exposed by inner controls
   */
  void on_picker_value_clicked();

  /**
   * @brief reacts on some signals exposed by inner controls
   */
  void on_filter_key_index_changed( int );

  /**
   * @brief reacts on some signals exposed by inner controls
   */
  void on_filter_value_text_changed( QString );

signals :
  /**
   * @brief signal emitted by some private slots, exposing informations on this
   * filter line
   * @param infos filter line informations
   */
  void filter_key_activated( const spm_filter_line_infos &infos );

  /**
   * @brief signal emitted by some private slots, exposing informations on this
   * filter line
   * @param infos filter line informations
   */
  void filter_value_activated( const spm_filter_line_infos &infos );

  /**
   * @brief signal emitted by some private slots, exposing informations on this
   * filter line
   * @param filter_line the control itself, mutable externally
   * @todo study a more safe way to do that
   */
  void picker_value_clicked( spm_filter_line &filter_line );

  /**
   * @brief signal emitted by some private slots, exposing informations on this
   * filter line
   * @param infos filter line informations
   */
  void filter_key_index_changed( const spm_filter_line_infos &infos );

  /**
   * @brief signal emitted by some private slots, exposing informations on this
   * filter line
   * @param infos filter line informations
   */
  void filter_value_text_changed( const spm_filter_line_infos &infos );
};

#endif // SPM_FILTER_LINE_H
