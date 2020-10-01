#include "sm_filter_line.h"
#include "ui_sm_filter_line.h"

#include "gex_shared.h"

// eurk...
extern const char *gexLabelFilterChoices[];

spm_filter_line::spm_filter_line(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::spm_filter_line)
{
  ui->setupUi(this);

  // insert the first element in the filter keys, descriptive element
  ui->combo_key->addItem( gexLabelFilterChoices[GEX_QUERY_FILTER_NONE] );

  // reacts on key index changing
  QObject::connect
    (
      ui->combo_key, SIGNAL(activated(QString)),
      this, SLOT(on_filter_key_activated(QString)),
      Qt::UniqueConnection
    );

  // reacts on value text changing
  QObject::connect
    (
      ui->combo_value, SIGNAL(activated(QString)),
      this, SLOT(on_filter_value_activated(QString)),
      Qt::UniqueConnection
    );

  // reacts on key index changing
  QObject::connect
    (
      ui->combo_key, SIGNAL(currentIndexChanged(int)),
      this, SLOT(on_filter_key_index_changed(int)),
      Qt::UniqueConnection
    );

  // reacts on picker value clicking
  QObject::connect
    (
      ui->picker_value, SIGNAL(clicked()),
      this, SLOT(on_picker_value_clicked()),
      Qt::UniqueConnection
    );

  // reacts on value text changing
  QObject::connect
    (
      ui->combo_value, SIGNAL(currentTextChanged(QString)),
      this, SLOT(on_filter_value_text_changed(QString)),
      Qt::UniqueConnection
    );
}

spm_filter_line::~spm_filter_line()
{
  delete ui;
}

void spm_filter_line::on_filter_value_text_changed( QString )
{
  emit filter_value_text_changed( get_infos() );
}

void spm_filter_line::on_filter_key_activated( QString )
{
  // forwarding...
  emit filter_key_activated( get_infos() );
}

void spm_filter_line::on_filter_value_activated( QString )
{
  // forwarding...
  emit filter_value_activated( get_infos() );
}

void spm_filter_line::set_value_text( const QString &text )
{
  // add the value inside the control's list only if it doesn't already exist
  if( ui->combo_value->findText( text, Qt::MatchFixedString ) == -1 )
    ui->combo_value->addItem( text );

  // set the current text
  ui->combo_value->setCurrentText( text );
}

void spm_filter_line::on_picker_value_clicked()
{
  // forwarding...
  emit picker_value_clicked( *this );
}

QString spm_filter_line::key_text() const
{
  return ui->combo_key->currentText();
}

QString spm_filter_line::value_text() const
{
  return ui->combo_value->currentText();
}

void spm_filter_line::on_filter_key_index_changed( int )
{
  // reset the value field as it could be irrelevant regarding the new key and
  // cause an error in the SQL query
  ui->combo_value->setCurrentText( "" );

  const spm_filter_line_infos &infos = get_infos();

  // only valid elements activate the filter value combo & picker
  ui->combo_value->setEnabled( infos.get_key_infos().get_index() > 0 );
  ui->picker_value->setEnabled( infos.get_key_infos().get_index() > 0 );

  // forwarding...
  emit filter_key_index_changed( infos );
}

void spm_filter_line::reset()
{
  ui->combo_key->setCurrentIndex( 0 );
  ui->combo_value->setCurrentText( "" );
}

bool spm_filter_line::valid_key_selected() const
{
  return ui->combo_key->currentIndex() > 0;
}

void spm_filter_line::set_line_with
  ( const QString &key_text, const QString &value_text , bool createItem)
{
  int index = -1;
  if( ( index = ui->combo_key->findText( key_text ) ) != -1 )
  {
    // initialization is done only if key exists in the list of keys
    ui->combo_key->setCurrentIndex( index );
    ui->combo_value->addItem( value_text );
  }
  else if(createItem)
  {
      ui->combo_key->insertItem(ui->combo_key->count(),key_text);
      ui->combo_key->setCurrentIndex( ui->combo_key->count()-1 );
      ui->combo_value->addItem( value_text );
  }
}

void spm_filter_line::append_filter_keys( const QStringList &keys )
{
  // insert a list of keys AFTER the first element
  ui->combo_key->insertItems( 1, keys );
}

spm_filter_line_element_infos::spm_filter_line_element_infos
  ( const int index, const QString text ) :
  base_type( index, text ) {}

const spm_filter_line_infos spm_filter_line::get_infos() const
{
  const spm_filter_line_element_infos key_infos
    ( ui->combo_key->currentIndex(), ui->combo_key->currentText() );

  const spm_filter_line_element_infos value_infos
    ( ui->combo_value->currentIndex(), ui->combo_value->currentText() );

  // RVO
  return spm_filter_line_infos( key_infos, value_infos );
}

// implementation of filter infos sub set
const int spm_filter_line_element_infos::get_index() const
{ return first; }

const QString & spm_filter_line_element_infos::get_text() const
{ return second; }

spm_filter_line_infos::spm_filter_line_infos
  (
    const spm_filter_line_element_infos &key_infos,
    const spm_filter_line_element_infos &value_infos
  ) :
  base_type( key_infos, value_infos ) {}

const spm_filter_line_element_infos & spm_filter_line_infos::
  get_key_infos() const
{ return first; }

const spm_filter_line_element_infos & spm_filter_line_infos::
  get_value_infos() const
{ return second; }
