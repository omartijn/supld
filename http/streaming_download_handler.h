#pragma once

#include <boost/asio/buffer.hpp>
#include <system_error>
#include <concepts>


/**
 *  This concept defines the requirements for a streaming download handler,
 *  which will receive chunks of the requested resource until it is complete.
 */
template <typename type>
concept streaming_download_handler = requires(type value, std::size_t size, std::error_code ec)
{
    /**
     *  Get a buffer to write chunks to
     *
     *  @param  size    The requested size of the buffer
     *  @param  ec      The error code to set on failure
     *  @return The write buffer to use
     */
    {value.prepare(size, ec)} -> std::same_as<boost::asio::mutable_buffer>;

    /**
     *  Commit received bytes to the buffer
     *
     *  @param  size    The number of bytes that were received
     *  @param  ec      The error code to set on failure
     */
    {value.commit(size, ec)} -> std::same_as<void>;
};
