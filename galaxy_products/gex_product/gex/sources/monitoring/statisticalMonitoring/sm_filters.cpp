#include "sm_filters.h"
#include "ui_sm_filters.h"
#include "sm_filter_line.h"
#include "pickfilter_dialog.h"
#include "gex_shared.h"

#include <QGridLayout>
#include <QBoxLayout>
#include <QPushButton>

smFilters::smFilters(QWidget *parent, const QStringList &filter_keys) :
    QWidget(parent),
    ui(new Ui::spm_filters),
    m_left_column_vbox( 0 ),
    m_right_column_grid( 0 ),
    m_reset_button( 0 ),
    m_filter_keys( filter_keys )
{
    ui->setupUi(this);

    setup_dynamic_layout();
}

smFilters::~smFilters()
{
    delete ui;
}

void smFilters::setup_dynamic_layout()
{
    // will contain filter lines
    m_left_column_vbox = new QVBoxLayout();

    // will contain buttons
    m_right_column_grid = new QGridLayout();

    ui->grid->addLayout( m_left_column_vbox, 0, 0 );
    ui->grid->addLayout( m_right_column_grid, 0, 1 );
    ui->grid->setAlignment( m_left_column_vbox, Qt::AlignTop );
    ui->grid->setAlignment( m_right_column_grid, Qt::AlignTop );


    QPixmap pixmap(":/gex/icons/housekeeping.png");
    mIcon = QIcon(pixmap);
    m_reset_button = new QPushButton(mIcon, "Reset all filters" );
    QObject::connect
            (
                m_reset_button, SIGNAL(clicked(bool)),
                this, SLOT(on_forward_reset_request(bool)),
                Qt::UniqueConnection
                );

    m_left_column_vbox->addWidget( m_reset_button );

    // re adjust the size
    adjustSize();
}

void smFilters::set_filter_keys( const QStringList &keys )
{
    // replacing valid keys
    m_filter_keys = keys;
}

void smFilters::load_from( const QMap<QString, QString> &database_filter , bool createItem)
{
    // if some narrow filters has been loaded from DB, load these
    if( ! database_filter.isEmpty() )
    {
        qDeleteAll( m_filter_lines );
        m_filter_lines.clear();

        QMap< QString, QString >::const_iterator iter = database_filter.begin();

        for( ; iter != database_filter.end(); ++iter)
        {
            // add an initialized filter line
            spm_filter_line *line =
                    add_filter_line( iter.key(), iter.value() , createItem);

            // connect all signal/slots of the newly created line
            connect_line( line );
        }

        // add a new uninitialized filter line for the user and connect its
        // signal/slots
        connect_line( add_filter_line() );
    }
    else
    {
        // add 3 default filter lines inside the collection, and connect these
        connect_line( add_filter_line() );
        connect_line( add_filter_line() );
        connect_line( add_filter_line() );
    }
}

void smFilters::copy_filters_in( QMap< QString, QString > &destination  )
{
    destination.clear();

    QList< spm_filter_line * >::const_iterator it = m_filter_lines.begin();

    for( ;it != m_filter_lines.end(); ++it )
    {
        const spm_filter_line_infos &line_infos = ( *it )->get_infos();

        if( ! line_infos.get_value_infos().get_text().isEmpty() )
        {
            destination.insert
                    (
                        line_infos.get_key_infos().get_text(),
                        line_infos.get_value_infos().get_text()
                        );
        }
    }
}

GexDatabaseFilter smFilters::get_gex_database_filter() const
{
    GexDatabaseFilter db_filter;
    db_filter.bOfflineQuery = false;
    db_filter.strDatabaseLogicalName = m_database_name;
    db_filter.strDataTypeQuery = m_testing_stage;

    QList< spm_filter_line * >::const_iterator it = m_filter_lines.begin();

    for( ;it != m_filter_lines.end(); ++it )
    {
        const spm_filter_line_infos &line_infos = ( *it )->get_infos();

        if( ! line_infos.get_value_infos().get_text().isEmpty() )
            db_filter.addNarrowFilter
                    (
                        line_infos.get_key_infos().get_text(),
                        line_infos.get_value_infos().get_text()
                        );
    }

    // RVO
    return db_filter;
}

void smFilters::reset_filter_lines()
{
    // remove all filter lines set by the user
    qDeleteAll( m_filter_lines );
    m_filter_lines.clear();

    // add 3 new uninitialized line and connect these
    connect_line( add_filter_line() );
    connect_line( add_filter_line() );
    connect_line( add_filter_line() );
}

spm_filter_line *smFilters::add_filter_line
( const QString &key_text, const QString &value_text , bool createItem)
{
    // create a new uninitialized line...
    spm_filter_line *line = add_filter_line();

    // ... and set its content
    line->set_line_with( key_text, value_text , createItem );

    return line;
}

void smFilters::connect_line( spm_filter_line *line )
{
    // takes care of signal exposed by the spm_filter_line control

    // index of key changed
    QObject::connect
            (
                line, SIGNAL(filter_key_index_changed(spm_filter_line_infos)),
                this, SLOT(on_filter_key_index_changed(spm_filter_line_infos)),
                Qt::UniqueConnection
                );

    // value picker clicked
    QObject::connect
            (
                line, SIGNAL(picker_value_clicked(spm_filter_line&)),
                this, SLOT(on_line_picker_clicked(spm_filter_line&)),
                Qt::UniqueConnection
                );
}

spm_filter_line *smFilters::add_filter_line()
{
    // create a new instance of the spm_filter_line control and add it in the
    // container
    spm_filter_line *line = new spm_filter_line();
    line->append_filter_keys( m_filter_keys );
    m_filter_lines.append( line );

    // add in the widget (left part)
    m_left_column_vbox->addWidget( line );

    return line;
}

void smFilters::on_line_picker_clicked( spm_filter_line &filter_line )
{
    // pop up with requested values
    pick_filter_from_live_list( filter_line );
}

void smFilters::set_products( const QString &products )
{
    m_products = products;
}

void smFilters::pick_filter_from_live_list( spm_filter_line &line )
{
    // create a database filter
    GexDatabaseFilter dbFilter = get_gex_database_filter();
    dbFilter.setQueryFilter( line.get_infos().get_key_infos().get_text() );

    // adding a static product id
    dbFilter.addNarrowFilter
            ( gexLabelFilterChoices[ GEX_QUERY_FILTER_PRODUCT ], m_products );

    // Extract only consolidated data
    dbFilter.bConsolidatedData = true;

    // Fill Filter list with relevant strings, forbidding wildcar usage
    PickFilterDialog pick_filter;
    pick_filter.fillList( dbFilter, false );

    // Prompt dialog box, let user pick Filter string from the list
    if(pick_filter.exec() != 1)
        return;	// User 'Abort'

    QString selection = pick_filter.filterList();

    // set the value filter text
    line.set_value_text( selection );
}

void smFilters::on_filter_key_index_changed
( const spm_filter_line_infos &infos )
{
    // this first element is not a valid key
    if( infos.get_key_infos().get_index() > 0 )
    {
        bool add_new_filter_line = true;

        foreach (spm_filter_line *line, m_filter_lines)
        {
            add_new_filter_line &= line->valid_key_selected();
            if( ! add_new_filter_line )
                break;
        }

        // only if all key are set, create a new uninitialized line
        if( add_new_filter_line )
            connect_line( add_filter_line() );
    }
}

void smFilters::on_forward_reset_request( bool )
{
    // forwarding a custom signal
    emit reset_requested();
}
