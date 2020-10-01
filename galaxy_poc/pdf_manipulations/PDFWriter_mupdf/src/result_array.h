#ifndef RESULT_ARRAY_H
#define RESULT_ARRAY_H

#include <vector>
#include <list>
#include <string>
#include <initializer_list>

class result_array
{
    class result_row;

    class result_column
    {
        friend class result_array::result_row;

        std::string content_;
        std::size_t position_in_row_;

        result_column
            ( std::string &&content, std::size_t position_in_row );
    };

    class result_row
    {
        friend class result_array;

        std::size_t cell_count_;
        std::size_t cell_index_;

        std::vector< result_column > cells_;

        result_row( std::size_t cell_count );

        void add_cell( std::string &&content );

    public :
        auto begin() { return cells_.begin(); }
        auto begin() const { return cells_.begin(); }

        auto end() { return cells_.end(); }
        auto end() const { return cells_.end(); }
    };

    std::size_t column_count_;
    std::size_t row_count_;

    std::list< result_row > rows_;

public :
    result_array( std::size_t column_count, std::size_t row_count );

    auto begin() { return rows_.begin(); }
    auto begin() const { return rows_.begin(); }

    auto end() { return rows_.end(); }
    auto end() const { return rows_.end(); }
};

#endif // RESULT_ARRAY_H
