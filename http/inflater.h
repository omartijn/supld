#pragma once

#include <boost/beast/core/flat_buffer.hpp>
#include <zlib.h>

#include "../http/streaming_download_handler.h"


namespace http {

    /**
     *  Streaming download handler that
     *  performs GZIP decompression
     */
    template <streaming_download_handler download_handler>
    class inflater
    {
        public:
            /**
             *  Constructor
             *
             *  @param  handler The downstream handler to process the decompressed data
             */
            inflater(download_handler& handler) :
                _handler{ handler }
            {
                // initialize the inflate (decompress) stream, this should
                // never fail - probably indicates a memory allocation failure
                if (auto result = inflateInit2(&_stream, 15 + 16); result != Z_OK) {
                    throw std::runtime_error{ "Unable to initialize gzip stream" };
                }
            }

            /**
             *  Destructor
             */
            ~inflater()
            {
                // clean up resources
                inflateEnd(&_stream);
            }

            /**
             *  Retrieve a buffer to write compressed data to
             *
             *  @param  size    The requested buffer size
             */
            boost::asio::mutable_buffer prepare(std::size_t size, std::error_code&)
            {
                return _buffer.prepare(size);
            }

            /**
             *  Add downloaded bytes
             *
             *  @param  size    Number of bytes added
             *  @param  ec      Error code set on failure
             */
            void commit(size_t size, std::error_code& ec)
            {
                // add data to underlying buffer
                _buffer.commit(size);

                // get raw data and create a buffer to inflate to
                auto deflated = _buffer.data();
                auto inflated = _handler.prepare(deflated.size() * 10, ec);

                if (ec) {
                    return;
                }

                _stream.next_in = static_cast<unsigned char*>(deflated.data());
                _stream.next_out = static_cast<unsigned char*>(inflated.data());
                _stream.avail_in = deflated.size();
                _stream.avail_out = inflated.size();

                if (auto result = inflate(&_stream, Z_NO_FLUSH); result != Z_OK && result != Z_STREAM_END) {
                    throw std::runtime_error{ "Stream decoding failed" };
                }

                auto written = inflated.size() - _stream.avail_out;
                auto read = deflated.size() - _stream.avail_in;

                // commit inflated data and consume processed bytes
                _handler.commit(written, ec);
                _buffer.consume(read);
            }
        private:
            z_stream                    _stream{};
            boost::beast::flat_buffer   _buffer;
            download_handler&           _handler;
    };

}
