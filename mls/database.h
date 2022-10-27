#pragma once

#include <string>
#include <vector>
#include <chrono>

#include "record_set.h"
#include "../http/client.h"
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/system_timer.hpp>


namespace mls {

    class database
    {
        public:
            /**
             *  Constructor
             *
             *  @param  client  The client to use for retrieving the MLS data
             *  @param  host    The hostname where the files are available
             */
            database(http::client& client, std::string_view mls_host);
            ~database();

            /**
             *  Retrieve the executor we are running in
             *
             *  @return The executor for asynchronous multiplexing
             */
            boost::asio::any_io_executor get_executor() const noexcept;
        private:
            /**
             *  Load MLS data and merge it into the database
             *
             *  @param  url     The URL to download from
             */
            void load_data(boost::urls::url_view url) noexcept;

            /**
             *  Handle the timer expiring, initiates hourly update
             *
             *  @param  ec      The error code from the expiring timer
             */
            void on_expiry(const boost::system::error_code& ec) noexcept;

            http::client&                           _client;
            boost::asio::system_timer               _refresh_timer;
            std::chrono::system_clock::time_point   _next_refresh;
            std::string                             _url_base;
            record_set                              _gsm_records;
            record_set                              _umts_records;
            record_set                              _lte_records;
    };

}
