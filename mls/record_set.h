#pragma once

#include <optional>
#include <vector>
#include "record.h"


namespace mls {

    /**
     *  A class holding a set of records from MLS
     *  of a particular type
     */
    class record_set
    {
        public:
            /**
             *  Find a record
             *
             *  @param  key     The record key to search for
             */
            std::optional<record::value_type> find(record::key_type key) const noexcept;

            /**
             *  Merge in another set of records, existing records are
             *  updated with the new values, new values are inserted.
             *
             *  @param  set     The other set to merge in
             */
            void merge(std::vector<record>&& set) noexcept;
        private:
            std::vector<record> _records;
    };

}
