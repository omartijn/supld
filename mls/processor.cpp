#include "processor.h"
#include <spdlog/spdlog.h>

namespace mls {

    boost::asio::mutable_buffer processor::prepare(std::size_t size, std::error_code&)
    {
        return _buffer.prepare(size);
    }

    void processor::commit(size_t size, std::error_code& ec)
    {
        if (size == 0) {
            return;
        }

        // store data in underlying buffer
        _buffer.commit(size);

        // create a string_view for easy searching
        // and substring operations
        std::string_view data{ 
            static_cast<const char*>(_buffer.data().data()),
            _buffer.data().size()
        };

        for (auto pos = data.find("\r\n"); pos != std::string_view::npos; pos = data.find("\r\n")) {
            // create a view of just the line
            std::string_view line{ data.substr(0, pos) };
            mls::record record;

            // remove the recorrd, as well as the newline
            data.remove_prefix(pos + 2);

            // are we processing the header
            if (!std::exchange(_header_received, true)) {
                // check whether the header matches - if fields were rearranged
                // we'd probably fail to process the data
                if (line != "radio,mcc,net,area,cell,unit,lon,lat,range,samples,changeable,created,updated,averageSignal") {
                    spdlog::error("MLS data header mismatch: {}", line);
                    ec = std::make_error_code(std::errc::bad_message);
                    return;
                }
            } else {
                // process the record data
                auto [radio, success] = record.load(line);

                // check whether the record loaded successfully
                if (!success) {
                    spdlog::warn("Invalid MLS record received: {}", line);
                    continue;
                }

                switch (radio) {
                    case mls::radio::GSM:
                        _gsm_records.push_back(record);
                        break;
                    case mls::radio::UMTS:
                        _umts_records.push_back(record);
                        break;
                    case mls::radio::LTE:
                        _lte_records.push_back(record);
                        break;
                }
            }
        }

        // consume from underlying buffer
        _buffer.consume(_buffer.size() - data.size());
    }

    std::span<const mls::record> processor::gsm_records() const & noexcept
    {
        return _gsm_records;
    }

    std::vector<mls::record>&& processor::gsm_records() && noexcept
    {
        return std::move(_gsm_records);
    }

    std::span<const mls::record> processor::umts_records() const & noexcept
    {
        return _umts_records;
    }

    std::vector<mls::record>&& processor::umts_records() && noexcept
    {
        return std::move(_umts_records);
    }

    std::span<const mls::record> processor::lte_records() const & noexcept
    {
        return _lte_records;
    }

    std::vector<mls::record>&& processor::lte_records() && noexcept
    {
        return std::move(_lte_records);
    }

}
