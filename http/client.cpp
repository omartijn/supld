#include "client.h"


namespace http {

    /**
     *  Constructor
     *
     *  @param  executor    The executor to use
     */
    client::client(boost::asio::any_io_executor executor) :
        _executor{ executor },
        _resolver{ executor }
    {}

    /**
     *  Retrieve the used executor
     *
     *  @return The executor used for the client
     */
    boost::asio::any_io_executor client::get_executor() const noexcept
    {
        return _executor;
    }

}
