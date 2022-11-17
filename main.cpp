#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

#include <boost/program_options.hpp>

#include "http/client.h"
#include "mls/database.h"

int main(int argc, const char** argv)
{
    // note: we want to work with UTC timestamps, but the version of libc++ I have (14), does
    // not yet have std::chrono::utc_clock, so we explicitly set our timezone to UTC and use
    // std::chrono::system_clock instead. note: POSIX only
    setenv("TZ", "/usr/share/zoneinfo/UTC", 1);

    // setup command-line arguments for executable
    boost::program_options::options_description program_options { "Allowed options for supld"               };
    boost::program_options::variables_map       variables       {                                           };

    // define the supported options
    program_options.add_options()
        ("help",                                                            "Get program usage instructions"    )
        ("listen-address",  boost::program_options::value<std::string>(),   "The address to listen on"          )
        ("listen-port",     boost::program_options::value<uint16_t>(),      "The port to listen on"             )
        ("mls-host",        boost::program_options::value<std::string>(),   "The hostname to download MLS data" );

    // parse command-line input
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, program_options),
        variables
    );

    // check whether the user requested help
    if (variables.count("help")) {
        std::cout << program_options << "\n";
        return 0;
    }

    // define defaults to use if not given
    std::string     listen_host { "0.0.0.0"                         };
    std::uint16_t   listen_port { 8080                              };
    std::string     mls_host    { "d2koia3g127518.cloudfront.net"   };

    if (variables.count("listen-address")) {
        listen_host = variables["listen-address"].as<std::string>();
    }

    if (variables.count("listen-port")) {
        listen_port = variables["listen-port"].as<std::uint16_t>();
    }

    if (variables.count("mls-host")) {
        mls_host = variables["mls-host"].as<std::string>();
    }

    boost::asio::io_context                     io_context      {                                       };
    http::client                                client          { io_context.get_executor()             };
    mls::database                               database        { client, std::move(mls_host)           };

    io_context.run();
    return 0;
}
