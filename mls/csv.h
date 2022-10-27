#include <string_view>
#include <type_traits>
#include <msstl/charconv.hpp>
#include <magic_enum.hpp>


namespace mls {

    /**
     *  Data type concepts:
     *
     *  arithmetic matches on all arithmetic types
     */
    template <typename T> concept arithmetic_t = std::is_arithmetic_v<T>;
    template <typename T> concept enum_t = std::is_enum_v<T>;

    /**
     *  CSV parsing helpers
     */
    struct csv
    {
        /**
         *  Skip the leading column from a line of CSV
         *
         *  @return Whether a column was skipped
         */
        static bool skip_column(std::string_view& data) noexcept;

        /**
         *  Read a single column containing textual data
         *
         *  @return Whether a column was read
         */
        static bool read_column(std::string_view& value, std::string_view& data) noexcept;

        /**
         *  Read a single column containing numeric data
         *
         *  @return Whether a column with numeric was read
         */
        template <arithmetic_t value_type>
        static bool read_column(value_type& value, std::string_view& data) noexcept
        {
            // first read the value into a string
            std::string_view string;

            if (!read_column(string, data)) {
                return false;
            }

            // convert the value to the requested type
            auto result = msstl::from_chars(string.data(), string.data() + string.size(), value);

            // check whether the conversion succeeded
            if (result.ec != std::errc{} || result.ptr != string.data() + string.size()) {
                return false;
            }

            return true;
        }

        /**
         *  Read a single column containing enum data
         *
         *  @return Whether a column with a valid enum value was read
         */
        template <enum_t value_type>
        static bool read_column(value_type& value, std::string_view& data) noexcept
        {
            // first read the value into a string
            std::string_view string;

            if (!read_column(string, data)) {
                return false;
            }

            // try to convert the value
            if (auto enum_value = magic_enum::enum_cast<value_type>(string); enum_value.has_value()) {
                value = *enum_value;
                return true;
            }

            return false;
        }
    };

}
