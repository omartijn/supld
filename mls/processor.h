#pragma once

#include <system_error>
#include <span>

#include "record.h"

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/asio/buffer.hpp>


namespace mls {

    /**
     *  Process streaming data and generate records
     */
    class processor
    {
        public:
            /**
             *  Get a buffer to write chunks to
             *
             *  @param  size    The requested size of the buffer
             *  @param  ec      The error code to set on failure
             *  @return The write buffer to use
             */
            boost::asio::mutable_buffer prepare(std::size_t size, std::error_code&);

            /**
             *  Process received bytes and create records
             *
             *  @param  size    The number of bytes that were received
             *  @param  ec      The error code to set on failure
             */
            void commit(size_t size, std::error_code& ec);

            /**
             *  Get the records for cell towers using
             *  the GSM signal
             *
             *  @return The GSM signal tower records
             */
            std::span<const mls::record> gsm_records() const & noexcept;
            std::vector<mls::record>&& gsm_records() && noexcept;

            /**
             *  Get the records for cell towers using
             *  the UMTS signal
             *
             *  @return The UMTS signal tower records
             */
            std::span<const mls::record> umts_records() const & noexcept;
            std::vector<mls::record>&& umts_records() && noexcept;

            /**
             *  Get the records for cell towers using
             *  the LTE signal
             *
             *  @return The LTE signal tower records
             */
            std::span<const mls::record> lte_records() const & noexcept;
            std::vector<mls::record>&& lte_records() && noexcept;
        private:
            boost::beast::flat_buffer _buffer;
            std::vector<mls::record>  _gsm_records;
            std::vector<mls::record>  _umts_records;
            std::vector<mls::record>  _lte_records;
            bool                      _header_received{};
    };

}
