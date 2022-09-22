#pragma once

#include "impl/streaming_download_operation.h"
#include "streaming_download_handler.h"
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/url/urls.hpp>


namespace http {

    /**
     *  HTTP client class, allowing retrieval of data
     *  available from HTTP servers
     */
    class client
    {
        public:
            /**
             *  Constructor
             *
             *  @param  executor    The executor to use
             */
            client(boost::asio::any_io_executor executor);

            /**
             *  Retrieve the used executor
             *
             *  @return The executor used for the client
             */
            boost::asio::any_io_executor get_executor() const noexcept;

            /**
             *  Download a resource
             *
             *  Note: only downloading using HTTPS is currently supported.
             *
             *  @param  url     The URL to retrieve from
             *  @param  handler The download handler that will receive the chunked data
             *  @param  token   The completion handler to signify completion/failure
             */
            template <streaming_download_handler download_handler, typename completion_token>
            auto download_resource(boost::urls::url_view url, download_handler& handler, completion_token&& token)
            {
                using handler_signature = void(const boost::system::error_code&);
                using handler_type = BOOST_ASIO_HANDLER_TYPE(completion_token, handler_signature);

                boost::asio::async_completion<completion_token, handler_signature> init{ token };
                impl::streaming_download_operation<download_handler, handler_type> session{ _resolver, url, handler, std::move(init.completion_handler) };

                return init.result.get();
            }
        private:
            boost::asio::any_io_executor    _executor;
            boost::asio::ip::tcp::resolver  _resolver;
    };

}
