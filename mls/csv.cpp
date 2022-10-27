#include "csv.h"
#include <algorithm>


namespace mls {

    bool csv::skip_column(std::string_view& data) noexcept
    {
        // is there any data left to skip?
        if (data.empty()) {
            return false;
        }

        // find the column separator in the data
        auto separator = std::find(begin(data), end(data), ',');

        // did we find the separator?
        if (separator == end(data)) {
            // this was the last column, no more data left
            data = {};
        } else {
            // create a substring past the found element
            data.remove_prefix(std::distance(begin(data), separator) + 1);
        }

        return true;
    }

    bool csv::read_column(std::string_view& value, std::string_view& data) noexcept
    {
        // do we have any data available?
        if (data.empty()) {
            return false;
        }

        // find the separator in the data
        auto separator = std::find(begin(data), end(data), ',');

        // is this the last column?
        if (separator == end(data)) {
            value = std::string_view{ data.data(), data.size() };
            data = {};
        } else {
            std::size_t index = std::distance(begin(data), separator);

            value = std::string_view{ data.data(), index };
            data.remove_prefix(index + 1);
        }

        return true;
    }

}
