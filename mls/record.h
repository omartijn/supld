#pragma once

#include <string_view>
#include <algorithm>
#include <compare>

#include "radio.h"


namespace mls {

    struct record
    {
        /**
         *  Load a row of CSV data into the record
         *
         *  @param  data    A row from the MLS datafile
         *  @return The radio type, and whether the record was loaded successfully
         */
        std::pair<radio, bool> load(std::string_view data) noexcept;

        /**
         *  The key type to use for looking up a record
         */
        struct key_type {
            auto operator<=>(const key_type&) const = default;

            uint16_t mcc;   // the mobile country code
            uint16_t mnc;   // mobile network code / service provider
            uint32_t cell;  // the identifier for the cellular tower
        };

        /**
         *  The position information for this record
         */
        struct value_type {
            float lon;      // the longitude at the towers location
            float lat;      // the latitude at the towers location
        };

        key_type    key;
        value_type  value;
    };

}
