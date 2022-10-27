#include "record.h"
#include <msstl/charconv.hpp>
#include <magic_enum.hpp>

#include "csv.h"


namespace mls {

    std::pair<radio, bool> record::load(std::string_view data) noexcept
    {
        // the network column to read into
        radio network{};

        // read data from the line
        bool success =
            csv::read_column(network, data)     &&
            csv::read_column(key.mcc, data)     &&
            csv::read_column(key.mnc, data)     &&
            csv::skip_column(data)              &&
            csv::read_column(key.cell, data)    &&
            csv::skip_column(data)              &&
            csv::read_column(value.lon, data)   &&
            csv::read_column(value.lat, data);

        if (!success) {
            return { network, false };
        }

        return { network, true };
    }

}
