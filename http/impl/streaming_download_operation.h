#pragma once

#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/async_base.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/url/urls.hpp>

#include "../streaming_download_handler.h"


namespace http::impl {

    /**
     *  Operation handler for the intermittent
     *  operations involved in downloading a resource
     */
    template <streaming_download_handler download_handler, class handler_type>
    class streaming_download_operation :
        public boost::beast::stable_async_base<handler_type, boost::asio::ip::tcp::resolver::executor_type>,
        public boost::asio::coroutine
    {
        public:
            streaming_download_operation(boost::asio::ip::tcp::resolver& resolver, boost::urls::url_view url, download_handler& handler, handler_type&& completion_token) :
                boost::beast::stable_async_base<handler_type, boost::asio::ip::tcp::resolver::executor_type>{ std::forward<handler_type>(completion_token), resolver.get_executor() },
                _data{ boost::beast::allocate_stable<data>(*this, resolver.get_executor(), url, handler) }
            {
                boost::asio::ip::tcp::resolver::query query{ url.encoded_host(), "https" };
                resolver.async_resolve(query, std::move(*this));
            }

            /**
             *  Resolve completion
             */
            void operator()(const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::results_type results)
            {
                if (ec) {
                    return this->complete_now(ec);
                }
    
                boost::asio::async_connect(_data.stream.lowest_layer(), results, std::move(*this));
            }

            /**
             *  Connection completion
             */
            void operator()(const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&)
            {
                if (ec) {
                    return this->complete_now(ec);
                }

                _data.stream.async_handshake(data::ssl_stream::handshake_type::client, std::move(*this));
            }

            /**
             *  TLS handshake completion
             */
            void operator()(const boost::system::error_code& ec)
            {
                if (ec) {
                    return this->complete_now(ec);
                }

                boost::beast::http::async_write(_data.stream, _data.request, std::move(*this));
            }

            /**
             *  Request write, header and body read
             */
            void operator()(const boost::system::error_code& ec, std::size_t bytes_transferred)
            {
                if (ec) {
                    return this->complete_now(ec);
                }

                boost::asio::mutable_buffer buffer;
                std::error_code rec;

                BOOST_ASIO_CORO_REENTER(*this) {
                    buffer = _data.buffer.prepare(4096);

                    _data.response.get().body().data = buffer.data();
                    _data.response.get().body().size = buffer.size();

                    BOOST_ASIO_CORO_YIELD boost::beast::http::async_read_header(_data.stream, _data.data_buffer, _data.response, std::move(*this));

                    _data.buffer.consume(bytes_transferred);
                    buffer = _data.handler.prepare(4096, rec);

                    if (rec) {
                        return this->complete_now(rec);
                    }

                    _data.response.get().body().data = buffer.data();
                    _data.response.get().body().size = buffer.size();

                    while (bytes_transferred != 0 & _data.response.content_length_remaining().value_or(1) != 0) {
                        BOOST_ASIO_CORO_YIELD boost::beast::http::async_read_some(_data.stream, _data.data_buffer, _data.response, std::move(*this));

                        _data.handler.commit(bytes_transferred, rec);

                        if (rec) {
                            return this->complete_now(rec);
                        }

                        buffer = _data.handler.prepare(4096, rec);
                        _data.response.get().body().data = buffer.data();
                        _data.response.get().body().size = buffer.size();

                        if (rec) {
                            return this->complete_now(rec);
                        }
                    }

                    this->complete_now(boost::system::error_code{});
                }
            }
        private:
            struct data
            {
                data(boost::asio::any_io_executor executor, boost::urls::url_view url, download_handler& handler) :
                    context{ boost::asio::ssl::context::method::tlsv1_client },
                    stream{ executor, context },
                    request{ boost::beast::http::verb::get, url.encoded_path(), 11 },
                    handler{ handler }
                {
                    request.set(boost::beast::http::field::host, url.encoded_host());
                    request.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    
                    response.body_limit(std::numeric_limits<std::uint64_t>::max());
                }

                using ssl_stream = boost::asio::ssl::stream<
                    boost::asio::ip::tcp::socket
                >;

                boost::asio::ssl::context   context;
                ssl_stream                  stream;
                boost::beast::http::request<boost::beast::http::empty_body> request;
                boost::beast::http::response_parser<boost::beast::http::buffer_body> response;
                boost::beast::flat_buffer   buffer;
                download_handler&           handler;
                boost::beast::flat_buffer   data_buffer;
            };

            data& _data;
    };

}
