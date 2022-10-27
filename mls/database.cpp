#include "database.h"
#include <spdlog/spdlog.h>
#include <spdlog/spdlog.h>
#include "processor.h"
#include "../http/inflater.h"

namespace {

    std::string formatted_date(std::string format, const std::chrono::system_clock::time_point& time)
    {
        std::time_t timestamp = std::chrono::system_clock::to_time_t(time);

        std::array<char, 64> buffer;
        auto size = std::strftime(buffer.data(), buffer.size(), format.c_str(), std::localtime(&timestamp));

        return { buffer.data(), size };
    }

}

namespace mls {

    database::database(http::client& client, std::string_view mls_host) :
        _client{ client },
        _refresh_timer{ _client.get_executor() },
        _url_base{ fmt::format("https://{}/export/", mls_host) }
    {
        // we start by retrieving a full data dump, the dump always has midnight
        // as the timestamp, so we floor it to get time of last midnight
        std::chrono::system_clock::time_point start = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());

        // we continue loading the incremental dumps from the next hour
        _next_refresh = start + std::chrono::hours{ 1 };

        // build the url to the full data dump to download initially
        boost::urls::url url{ _url_base + formatted_date("MLS-full-cell-export-%Y-%m-%dT%H0000.csv.gz", start) };
        load_data(url);
    }

    database::~database()
    {
        _refresh_timer.cancel();
    }

    boost::asio::any_io_executor database::get_executor() const noexcept
    {
        return _client.get_executor();
    }

    void database::load_data(boost::urls::url_view url) noexcept
    {
        spdlog::info("Retrieving {}", url.string());

        struct request_data {
            processor                       processor   {           };
            http::inflater<mls::processor>  inflater    { processor };
        };

        auto data = std::make_shared<request_data>();

        _client.download_resource(url, data->inflater, [this, data](const boost::system::error_code& ec) {
            if (ec) {
                spdlog::warn("Failed to retrieve MLS data: {}", ec.message());
                return;
            }

            spdlog::info("Processing MLS update");

            _gsm_records.merge(std::move(data->processor).gsm_records());            
            _umts_records.merge(std::move(data->processor).umts_records());
            _lte_records.merge(std::move(data->processor).lte_records());

            spdlog::info("MLS database update complete");

            // schedule next download - add 10 minutes to ensure the file becomes available
            _refresh_timer.expires_at(_next_refresh + std::chrono::minutes{ 10 });
            _refresh_timer.async_wait(std::bind(&database::on_expiry, this, std::placeholders::_1));
        });
    }

    void database::on_expiry(const boost::system::error_code& ec) noexcept
    {
        // check whether we were canceled
        if (ec == boost::asio::error::operation_aborted) {
            return spdlog::info("MLS database refresh handler stopped");
        }

        // build the url to the updated data dump to download initially
        boost::urls::url url{ _url_base + formatted_date("MLS-diff-cell-export-%Y-%m-%dT%H0000.csv.gz", _next_refresh) };
        load_data(url);

        // next refresh will be in an hour
        _next_refresh += std::chrono::hours{ 1 };
    }

}
