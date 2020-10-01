#include "result_array.h"

#include <sstream>
#include <iomanip>

result_array::result_array
    ( std::size_t column_count, std::size_t row_count ) :
    column_count_{ column_count },
    row_count_{ row_count }
{
    const char *header_prefix = "column_";
    const char *content_prefix = "content_";

    for( std::size_t i = 0; i < row_count; ++i )
    {
        std::ostringstream oss;

        result_row row{ column_count_ };

        // header row
        for( std::size_t j = 0; j < column_count_; ++j )
        {
            if( i == 0 )
            {
                oss <<
                   header_prefix <<
                   std::setfill( '0' ) <<
                   std::setw( 4 ) <<
                   j;
            }
            // remaining rows
            else
            {
                oss <<
                   content_prefix <<
                   std::setfill( '0' ) <<
                   std::setw( 4 ) <<
                   j;
            }

            row.add_cell( oss.str() );
            oss.str( {} );
        }

        rows_.push_back( row );
    }
}

result_array::result_column::result_column
    ( std::string &&content, std::size_t position_in_row ) :
    content_{ content },
    position_in_row_{ position_in_row } {}

result_array::result_row::result_row( std::size_t cell_count ) :
    cell_count_{ cell_count },
    cell_index_ { 0 }
{
    cells_.reserve( cell_count_ );
}

void result_array::result_row::add_cell( std::string &&content )
{
    cells_.push_back
        ( { std::forward< std::string >( content ), cell_index_++ } );
}
