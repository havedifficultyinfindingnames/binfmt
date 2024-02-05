#include <string>
#include <fast_io.h>
#include <fast_io_legacy.h>
#include <fast_io_dsal/vector.h>

namespace fast_io
{
	enum class width_print_status_code : ::std::uint8_t
	{
		content_unprinted,
		leading_sign,
		filling_left,
		content_printed,
		filling_right,
	};
	template <::std::integral char_type, typename T>
	struct width_left_static_status_t
	{
		using enum width_print_status_code;
		constexpr static ::std::size_t print_reserve_size_T{ print_reserve_size(io_reserve_type<char_type, T>) };
		char_type buffer[print_reserve_size_T];
		char_type* ptr;
		::std::size_t fill_size;
		width_print_status_code code{ content_unprinted };
		context_print_result<char_type*> print_context_define(width_t<scalar_placement::left, T> t, char_type* begin, char_type* end)
		{
			if (code == content_unprinted)
			{
				if (end - begin < print_reserve_size_T)
				{
					ptr = print_reserve_define(::fast_io::io_reserve_type<char_type, T>, buffer, t.reference);
					auto printed_size{ static_cast<::std::size_t>(ptr - buffer) };
					fill_size = printed_size > t.width ? 0 : t.width - printed_size;
					return { begin, false };
				}
				else
				{
					auto newptr{ print_reserve_define(::fast_io::io_reserve_type<char_type, T>, begin, t.reference) };
					auto printed_size{ static_cast<::std::size_t>(newptr - begin) };
					fill_size = printed_size > t.width ? 0 : t.width - printed_size;
					if (auto remained_size{ static_cast<::std::size_t>(end - newptr) }; fill_size <= remained_size)
					{
						return { static_cast<char_type*>(::fast_io::freestanding::my_memset(newptr, ' ', fill_size)), true };
					}
					else
					{
						::fast_io::freestanding::my_memset(newptr, ' ', remained_size);
						fill_size -= remained_size;
						code = filling_right;
						return { end, false };
					}
				}
			}
			else if (code == filling_right)
			{
				if (auto remained_size{ static_cast<::std::size_t>(end - begin) }; fill_size <= remained_size)
				{
					return { static_cast<char_type*>(::fast_io::freestanding::my_memset(begin, '\0', fill_size)), true };
				}
				else
				{
					::fast_io::freestanding::my_memset(begin, ' ', remained_size);
					fill_size -= remained_size;
					return { end, false };
				}
			}
			else
			{
				// __builtin_unreachable();
			}
		}
	};
	template <::std::integral char_type, scalar_placement flags, typename T>
	auto print_context_type(io_reserve_type_t<char_type, width_t<flags, T>>)
	{
		if constexpr (flags == scalar_placement::left)
		{
			if constexpr (reserve_printable<char_type, T>)
			{
				return io_type_t<width_left_static_status_t<char_type, T>>{};
			}
		}
	}
}

namespace fast_io
{
	using binfmt_size_t = ::std::uint_least32_t;
	enum class binfmt_kinds : binfmt_size_t
	{
		no_format = 0,
		prefix = 1 << 0,
		align = 1 << 1,
		fill = 1 << 2,
		width = 1 << 3,
		precision = 1 << 4,
		type = 1 << 5,
		runtime_prefix = (prefix << 16) + prefix,
		runtime_align = (align << 16) + align,
		runtime_fill = (fill << 16) + fill,
		runtime_width = (width << 16) + width,
		runtime_precision = (precision << 16) + precision,
		runtime_type = (type << 16) + type,
	};
	inline constexpr binfmt_kinds operator&(binfmt_kinds lhs, binfmt_kinds rhs)
	{
		using underlying_type = ::std::underlying_type_t<binfmt_kinds>;
		return static_cast<binfmt_kinds>(static_cast<underlying_type>(lhs) & static_cast<underlying_type>(rhs));
	}
	inline constexpr binfmt_kinds operator&=(binfmt_kinds& lhs, binfmt_kinds rhs)
	{
		return lhs = lhs & rhs;
	}
	inline constexpr binfmt_kinds operator|(binfmt_kinds lhs, binfmt_kinds rhs)
	{
		using underlying_type = ::std::underlying_type_t<binfmt_kinds>;
		return static_cast<binfmt_kinds>(static_cast<underlying_type>(lhs) | static_cast<underlying_type>(rhs));
	}
	inline constexpr binfmt_kinds operator|=(binfmt_kinds& lhs, binfmt_kinds rhs)
	{
		return lhs = lhs | rhs;
	}
	enum class binfmt_align : ::std::uint_least8_t { internal, left, right, center };
	enum class binfmt_prefix : ::std::uint_least8_t
	{
		space = 1,
		minus = 2,
		plus = 3,
		hash = 4,
	};
	inline constexpr binfmt_prefix operator&(binfmt_prefix lhs, binfmt_prefix rhs)
	{
		using underlying_type = ::std::underlying_type_t<binfmt_prefix>;
		return static_cast<binfmt_prefix>(static_cast<underlying_type>(lhs) & static_cast<underlying_type>(rhs));
	}
	inline constexpr binfmt_prefix operator&=(binfmt_prefix& lhs, binfmt_prefix rhs)
	{
		return lhs = lhs & rhs;
	}
	inline constexpr binfmt_prefix operator|(binfmt_prefix lhs, binfmt_prefix rhs)
	{
		using underlying_type = ::std::underlying_type_t<binfmt_prefix>;
		return static_cast<binfmt_prefix>(static_cast<underlying_type>(lhs) | static_cast<underlying_type>(rhs));
	}
	inline constexpr binfmt_prefix operator|=(binfmt_prefix& lhs, binfmt_prefix rhs)
	{
		return lhs = lhs & rhs;
	}

	using binfmt = vector<::std::byte>;

	namespace details
	{
		template <::std::integral T>
		inline constexpr typename vector<::std::byte>::iterator byte_insert(vector<::std::byte>& v, typename vector<::std::byte>::const_iterator where, T t) noexcept
		{
			t = ::fast_io::little_endian(t);
			auto arr{ ::std::bit_cast<freestanding::array<::std::byte, sizeof(T)>>(t) };
			return v.insert_range(where, arr);
		}
		template <::std::integral T>
		inline constexpr void byte_insert_end(vector<::std::byte>& v, T t) noexcept
		{
			t = ::fast_io::little_endian(t);
			auto arr{ ::std::bit_cast<freestanding::array<::std::byte, sizeof(T)>>(t) };
			v.append_range(arr);
		}
		template <::std::integral T>
		inline constexpr typename vector<::std::byte>::iterator byte_write_unsafe(typename vector<::std::byte>::iterator itr, T t) noexcept
		{
			t = ::fast_io::little_endian(t);
			auto arr{ ::std::bit_cast<freestanding::array<::std::byte, sizeof(T)>>(t) };
			return ::fast_io::freestanding::nonoverlapped_bytes_copy_n(arr.begin(), sizeof(T), itr);
		}
		template <::std::integral T>
		inline constexpr typename vector<::std::byte>::const_iterator byte_read_unsafe(typename vector<::std::byte>::const_iterator itr, T& t) noexcept
		{
			freestanding::array<::std::byte, sizeof(T)> arr;
			::fast_io::freestanding::nonoverlapped_bytes_copy_n(itr, sizeof(T), arr.begin());
			t = ::fast_io::little_endian(::std::bit_cast<T>(arr));
			return itr + sizeof(T);
		}
		template <typename T>
		inline constexpr void byte_append_range(vector<::std::byte>& v, T const* begin, T const* end) noexcept
		{
			auto size{ static_cast<::std::size_t>(end - begin) * sizeof(T) };
			v.reserve(v.size() + size);
			compile_time_type_punning_copy_n(begin, size, v.end());
			v.imp.curr_ptr += size;
		}

		enum class index_state : ::std::uint_least8_t { unknown, automatic, manual };

		template <::std::integral char_type>
		struct libfmt_format_parser
		{
		public:
			binfmt retval;
			[[nodiscard]] constexpr char_type const* parse_literal(char_type const* begin, char_type const* end)
			{
				buffer.clear();
				begin = parse_literal_internal(begin, end);
				if (!buffer.empty())
				{
					byte_insert_end(retval, ::std::uint_least8_t{ 0 });
					retval.append_range(buffer);
				}
				return begin;
			}
			[[nodiscard]] constexpr char_type const* parse_format(char_type const* begin, char_type const* end)
			{
				buffer.clear();
				++begin;
				if (begin == end)
				{
					terminate(begin, end);
				}
				begin = parse_index(begin, end);
				if (begin == end)
				{
					terminate(begin, end);
				}
				fmt_kinds = binfmt_kinds::no_format;
				byte_insert_end(buffer, static_cast<::std::underlying_type_t<binfmt_kinds>>(fmt_kinds));
				if (*begin == char_literal_v<u8':', char_type>)
				{
					++begin;
					begin = maybe_parse_format(begin, end);
				}
				if (begin == end || *begin != char_literal_v<u8'}', char_type>)
				{
					terminate(begin, end);
				}
				++begin;
				return begin;
			}
		private:
			binfmt buffer;
			binfmt_size_t index{};
			binfmt_kinds fmt_kinds;
			binfmt_size_t fill_content;
			index_state idx_st{ index_state::unknown };
			binfmt_align align;
			binfmt_prefix prefix;
		private:
			[[nodiscard]] constexpr char_type const* parse_literal_internal(char_type const* begin, char_type const* end)
			{
				auto curr{ begin };
				while (curr != end)
				{
					if (*curr == char_literal_v<u8'}', char_type>) [[unlikely]]
						{
							terminate(begin, end);
						}
					else if (*curr == char_literal_v<u8'{', char_type>) [[unlikely]]
						{
							break;
						}
					else [[likely]]
						{
							++curr;
						}
				}
				if (auto len{ static_cast<binfmt_size_t>(static_cast<::std::size_t>(curr - begin) * sizeof(char_type)) }; len != 0)
				{
					if (buffer.empty())
					{
						byte_insert_end(buffer, len);
					}
					else
					{
						auto retbeg{ buffer.begin() };
						binfmt_size_t tmp{};
						byte_read_unsafe(retbeg, tmp);
						tmp += len;
						byte_write_unsafe(retbeg, tmp);
					}
					byte_append_range(buffer, begin, curr);
				}
				if (auto curr1{ curr + 1 }; curr1 < end && *curr1 == char_literal_v<u8'{', char_type>) [[unlikely]]
					{
						return parse_literal_internal(curr1, end);
					}
					return curr;
			}
			[[nodiscard]] constexpr char_type const* parse_index(char_type const* begin, char_type const* end)
			{
				if (idx_st == index_state::unknown)
				{
					if (char_category::is_c_digit(*begin))
					{
						idx_st = index_state::manual;
					}
					else
					{
						idx_st = index_state::automatic;
					}
				}
				if (idx_st == index_state::manual)
				{
					auto alias{ scan_alias_define(io_alias, index) };
					auto [ptr, code] {scan_contiguous_define(io_reserve_type<char_type, decltype(alias)>, begin, end, alias)};
					if (code != parse_code::ok)
					{
						terminate(begin, end);
					}
					byte_insert_end(buffer, index);
					begin = ptr;
				}
				else
				{
					byte_insert_end(buffer, index);
					++index;
				}
				return begin;
			}
			[[nodiscard]] constexpr char_type const* maybe_parse_format(char_type const* begin, char_type const* end)
			{
				begin = maybe_parse_align(begin, end);
				begin = maybe_parse_sign(begin, end);
				begin = maybe_parse_hash(begin, end);
				begin = maybe_parse_zero(begin, end);
				begin = maybe_parse_width(begin, end);
				begin = maybe_parse_precision(begin, end);
				begin = maybe_parse_type(begin, end);
				return begin;
			}
			[[nodiscard]] constexpr char_type const* maybe_parse_align(char_type const* begin, char_type const* end)
			{
				constexpr auto maybe_length{ 4u / sizeof(char_type) + 1 };
				auto maybe_end{ static_cast<::std::size_t>(end - begin) < maybe_length ? end : begin + maybe_length };
				align = static_cast<binfmt_align>(-1);
				char_type const* cp_end{}; // need not to be initialized, however msvc complains
				for (auto ptr{ begin }; ptr != maybe_end; ++ptr)
				{
					switch (*ptr)
					{
					case char_literal_v<u8'<', char_type>:
						align = binfmt_align::left;
						break;
					case char_literal_v<u8'^', char_type>:
						align = binfmt_align::center;
						break;
					case char_literal_v<u8'>', char_type>:
						align = binfmt_align::right;
						break;
					default:
						continue;
					}
					cp_end = ptr;
					break;
				}
				if (align == static_cast<binfmt_align>(-1))
				{
					return begin;
				}
				fmt_kinds |= binfmt_kinds::align;
				if (cp_end != begin)
				{
					fmt_kinds |= binfmt_kinds::fill;
					::fast_io::freestanding::my_memcpy(&fill_content, begin, cp_end - begin);
				}
				else
				{
					fill_content = 0;
				}
				return cp_end + 1;
			}
			[[nodiscard]] constexpr char_type const* maybe_parse_sign(char_type const* begin, char_type const* end)
			{
				prefix = static_cast<binfmt_prefix>(-1);
				if (begin == end)
				{
					terminate(begin, end);
				}
				switch (*begin)
				{
				case char_literal_v<u8' ', char_type>:
					prefix = binfmt_prefix::space;
					++begin;
					break;
				case char_literal_v<u8'-', char_type>:
					prefix = binfmt_prefix::minus;
					++begin;
					break;
				case char_literal_v<u8'+', char_type>:
					prefix = binfmt_prefix::plus;
					++begin;
					break;
				}
				return begin;
			}
			[[nodiscard]] constexpr char_type const* maybe_parse_hash(char_type const* begin, char_type const* end)
			{
				if (begin == end)
				{
					terminate(begin, end);
				}
				if (*begin == char_literal_v<u8'#', char_type>)
				{
					prefix |= binfmt_prefix::hash;
					++begin;
				}
				if (prefix != static_cast<binfmt_prefix>(-1))
				{
					fmt_kinds |= binfmt_kinds::prefix;
					binfmt_size_t temp{ static_cast<::std::underlying_type_t<binfmt_kinds>>(prefix) };
					byte_insert_end(buffer, temp);
				}
				return begin;
			}
			[[nodiscard]] constexpr char_type const* maybe_parse_zero(char_type const* begin, char_type const* end)
			{
				if (begin == end)
				{
					terminate(begin, end);
				}
				if (align == static_cast<binfmt_align>(-1))
				{
					if (*begin == char_literal_v<u8'0', char_type>)
					{
						align = binfmt_align::internal;
						fmt_kinds |= binfmt_kinds::align | binfmt_kinds::fill;
						binfmt_size_t tmp1{ static_cast<::std::underlying_type_t<binfmt_kinds>>(align) };
						binfmt_size_t tmp2{ char_literal_v<u8'0', char_type> };
						byte_insert_end(buffer, tmp1);
						byte_insert_end(buffer, tmp2);
						++begin;
					}
				}
				else
				{
					binfmt_size_t tmp{ static_cast<::std::underlying_type_t<binfmt_kinds>>(align) };
					byte_insert_end(buffer, tmp);
					if (fill_content != 0)
					{
						byte_insert_end(buffer, fill_content);
					}
				}
				return begin;
			}
			[[nodiscard]] constexpr char_type const* maybe_parse_width(char_type const* begin, char_type const* end)
			{
				if (begin == end)
				{
					terminate(begin, end);
				}
				if (*begin == char_literal_v<u8'{', char_type>)
				{
					fmt_kinds |= binfmt_kinds::runtime_width;
					begin = parse_index(begin, end) + 1;
					if (begin == end || *begin != char_literal_v<u8'}', char_type>)
					{
						terminate(begin, end);
					}
					++begin;
				}
				else
				{
					binfmt_size_t width;
					auto alias{ scan_alias_define(io_alias, width) };
					auto [ptr, code] {scan_contiguous_define(io_reserve_type<char_type, decltype(alias)>, begin, end, alias)};
					if (code != parse_code::ok)
					{
						return begin;
					}
					fmt_kinds |= binfmt_kinds::width;
					byte_insert_end(buffer, width);
					begin = ptr;
				}
				return begin;
			}
			[[nodiscard]] constexpr char_type const* maybe_parse_precision(char_type const* begin, char_type const* end)
			{
				if (begin == end)
				{
					terminate(begin, end);
				}
				if (*begin != char_literal_v<u8'.', char_type>)
				{
					return begin;
				}
				++begin;
				if (begin == end)
				{
					terminate(begin, end);
				}
				if (*begin == char_literal_v<u8'{', char_type>)
				{
					fmt_kinds |= binfmt_kinds::runtime_precision;
					begin = parse_index(begin, end);
					if (*begin != char_literal_v<u8'}', char_type>)
					{
						terminate(begin, end);
					}
					++begin;
				}
				else
				{
					binfmt_size_t precision;
					auto alias{ scan_alias_define(io_alias, precision) };
					auto [ptr, code] {scan_contiguous_define(io_reserve_type<char_type, decltype(alias)>, begin, end, alias)};
					if (code != parse_code::ok)
					{
						return begin;
					}
					fmt_kinds |= binfmt_kinds::precision;
					byte_insert_end(buffer, precision);
					begin = ptr;
				}
				return begin;
			}
			[[nodiscard]] constexpr char_type const* maybe_parse_type(char_type const* begin, char_type const* end)
			{
				if (begin == end)
				{
					terminate(begin, end);
				}
				binfmt_size_t tmp;
				switch (*begin++)
				{
				case char_literal_v<u8's', char_type>:
					tmp = 1;
					break;
				case char_literal_v<u8'b', char_type>:
					tmp = 2;
					break;
				case char_literal_v<u8'B', char_type>:
					tmp = 3;
					break;
				case char_literal_v<u8'c', char_type>:
					tmp = 4;
					break;
				case char_literal_v<u8'd', char_type>:
					tmp = 5;
					break;
				case char_literal_v<u8'o', char_type>:
					tmp = 6;
					break;
				case char_literal_v<u8'x', char_type>:
					tmp = 7;
					break;
				case char_literal_v<u8'X', char_type>:
					tmp = 8;
					break;
				case char_literal_v<u8'a', char_type>:
					tmp = 9;
					break;
				case char_literal_v<u8'A', char_type>:
					tmp = 10;
					break;
				case char_literal_v<u8'e', char_type>:
					tmp = 11;
					break;
				case char_literal_v<u8'E', char_type>:
					tmp = 12;
					break;
				case char_literal_v<u8'f', char_type>:
					tmp = 13;
					break;
				case char_literal_v<u8'F', char_type>:
					tmp = 14;
					break;
				case char_literal_v<u8'g', char_type>:
					tmp = 15;
					break;
				case char_literal_v<u8'G', char_type>:
					tmp = 16;
					break;
				case char_literal_v<u8'p', char_type>:
					tmp = 17;
					break;
				case char_literal_v<u8'?', char_type>:
					tmp = 18;
					break;
				case char_literal_v<u8'%', char_type>:
					return maybe_parse_time(begin, end);
				default:
					clearup();
					return --begin;
				}
				fmt_kinds |= binfmt_kinds::type;
				clearup();
				byte_insert_end(retval, tmp);
				return begin;
			}
			[[nodiscard]] constexpr char_type const* maybe_parse_time(char_type const* begin, char_type const* end)
			{
				if (begin == end)
				{
					terminate(begin, end);
				}
				if ((fmt_kinds & binfmt_kinds::prefix) != static_cast<binfmt_kinds>(0))
				{
					terminate(begin, end);
				}
				while (begin != end && *begin != char_literal_v<u8'}', char_type>)
				{
					binfmt_size_t tmp;
					switch (*begin)
					{
					case char_literal_v<u8'a', char_type>:
						tmp = 129;
						break;
					case char_literal_v<u8'A', char_type>:
						tmp = 130;
						break;
					case char_literal_v<u8'b', char_type>:
					case char_literal_v<u8'h', char_type>:
						tmp = 131;
						break;
					case char_literal_v<u8'B', char_type>:
						tmp = 132;
						break;
					case char_literal_v<u8'c', char_type>:
						tmp = 133;
						break;
					case char_literal_v<u8'C', char_type>:
						tmp = 134;
						break;
					case char_literal_v<u8'd', char_type>:
						tmp = 135;
						break;
					case char_literal_v<u8'D', char_type>:
						tmp = 136;
						break;
					case char_literal_v<u8'e', char_type>:
						tmp = 137;
						break;
					case char_literal_v<u8'E', char_type>:
						++begin;
						if (begin == end)
						{
							terminate(begin, end);
						}
						switch (*begin)
						{
						case char_literal_v<u8'c', char_type>:
							tmp = 138;
							break;
						case char_literal_v<u8'C', char_type>:
							tmp = 139;
							break;
						case char_literal_v<u8'x', char_type>:
							tmp = 140;
							break;
						case char_literal_v<u8'X', char_type>:
							tmp = 141;
							break;
						case char_literal_v<u8'y', char_type>:
							tmp = 142;
							break;
						case char_literal_v<u8'Y', char_type>:
							tmp = 143;
							break;
						case char_literal_v<u8'z', char_type>:
							tmp = 144;
							break;
						default:
							terminate(begin, end);
						}
						break;
					case char_literal_v<u8'F', char_type>:
						tmp = 145;
						break;
					case char_literal_v<u8'g', char_type>:
						tmp = 146;
						break;
					case char_literal_v<u8'G', char_type>:
						tmp = 147;
						break;
					case char_literal_v<u8'H', char_type>:
						tmp = 148;
						break;
					case char_literal_v<u8'I', char_type>:
						tmp = 149;
						break;
					case char_literal_v<u8'j', char_type>:
						tmp = 150;
						break;
					case char_literal_v<u8'm', char_type>:
						tmp = 151;
						break;
					case char_literal_v<u8'M', char_type>:
						tmp = 152;
						break;
					case char_literal_v<u8'O', char_type>:
						++begin;
						if (begin == end)
						{
							terminate(begin, end);
						}
						switch (*begin)
						{
						case char_literal_v<u8'd', char_type>:
							tmp = 153;
							break;
						case char_literal_v<u8'e', char_type>:
							tmp = 154;
							break;
						case char_literal_v<u8'H', char_type>:
							tmp = 155;
							break;
						case char_literal_v<u8'I', char_type>:
							tmp = 156;
							break;
						case char_literal_v<u8'm', char_type>:
							tmp = 157;
							break;
						case char_literal_v<u8'M', char_type>:
							tmp = 158;
							break;
						case char_literal_v<u8'S', char_type>:
							tmp = 159;
							break;
						case char_literal_v<u8'u', char_type>:
							tmp = 160;
							break;
						case char_literal_v<u8'U', char_type>:
							tmp = 161;
							break;
						case char_literal_v<u8'V', char_type>:
							tmp = 162;
							break;
						case char_literal_v<u8'w', char_type>:
							tmp = 163;
							break;
						case char_literal_v<u8'W', char_type>:
							tmp = 164;
							break;
						case char_literal_v<u8'y', char_type>:
							tmp = 165;
							break;
						case char_literal_v<u8'z', char_type>:
							tmp = 144;
							break;
						default:
							terminate(begin, end);
						}
						break;
					case char_literal_v<u8'p', char_type>:
						tmp = 166;
						break;
					case char_literal_v<u8'q', char_type>:
						tmp = 167;
						break;
					case char_literal_v<u8'Q', char_type>:
						tmp = 168;
						break;
					case char_literal_v<u8'r', char_type>:
						tmp = 169;
						break;
					case char_literal_v<u8'R', char_type>:
						tmp = 170;
						break;
					case char_literal_v<u8'S', char_type>:
						tmp = 171;
						break;
					case char_literal_v<u8'T', char_type>:
						tmp = 172;
						break;
					case char_literal_v<u8'u', char_type>:
						tmp = 173;
						break;
					case char_literal_v<u8'U', char_type>:
						tmp = 174;
						break;
					case char_literal_v<u8'V', char_type>:
						tmp = 175;
						break;
					case char_literal_v<u8'w', char_type>:
						tmp = 176;
						break;
					case char_literal_v<u8'W', char_type>:
						tmp = 177;
						break;
					case char_literal_v<u8'x', char_type>:
						tmp = 178;
						break;
					case char_literal_v<u8'X', char_type>:
						tmp = 179;
						break;
					case char_literal_v<u8'y', char_type>:
						tmp = 180;
						break;
					case char_literal_v<u8'Y', char_type>:
						tmp = 181;
						break;
					case char_literal_v<u8'z', char_type>:
						tmp = 182;
						break;
					case char_literal_v<u8'Z', char_type>:
						tmp = 183;
						break;
					case char_literal_v<u8't', char_type>:
					case char_literal_v<u8'n', char_type>:
					case char_literal_v<u8'%', char_type>:
						begin = parse_timeformat_literal(begin, end, true);
						continue;
					default:
						terminate(begin, end);
					}
					fmt_kinds |= binfmt_kinds::type;
					clearup();
					byte_insert_end(retval, tmp);
					begin = parse_timeformat_literal(begin + 1, end);
				}
				return begin;
			}
			[[nodiscard]] constexpr char_type const* parse_timeformat_literal(char_type const* begin, char_type const* end, bool begin_with_escape = false)
			{
				if (begin == end)
				{
					terminate(begin, end);
				}
				auto curr{ begin };
				if (begin_with_escape)
				{
					++curr;
				}
				while (curr != end)
				{
					if (*curr == char_literal_v<u8'%', char_type>) [[unlikely]]
						{
							break;
						}
					else if (*curr == char_literal_v<u8'}', char_type>) [[unlikely]]
						{
							break;
						}
					else
					{
						++curr;
					}
				}
				auto len{ static_cast<::std::size_t>(curr - begin) * sizeof(char_type) };
				if (len == 0)
				{
					return begin;
				}
				byte_insert_end(retval, ::std::uint_least8_t{ 0 });
				byte_insert_end(retval, len);
				byte_append_range(retval, begin, curr);
				if (curr == end)
				{
					terminate(begin, end);
				}
				++curr;
				if (curr != end)
				{
					switch (*curr)
					{
					case char_literal_v<u8't', char_type>:
					case char_literal_v<u8'n', char_type>:
					case char_literal_v<u8'%', char_type>:
						return parse_timeformat_literal(curr, end, true);
					default:
						break;
					}
				}
				return curr;
			}
			constexpr void clearup() noexcept
			{
				byte_write_unsafe(buffer.begin() + sizeof(binfmt_size_t), static_cast<::std::underlying_type_t<decltype(fmt_kinds)>>(fmt_kinds));
				byte_insert_end(retval, ::std::uint_least8_t{ 1 });
				retval.append_range(buffer);
			}
			[[noreturn]] void terminate(char_type const* begin, char_type const* end)
			{
				perr("Format string error (at \"", mnp::os_c_str(begin, end - begin), "\"). \n");
				fast_terminate();
			}
		};

	}
	template <::std::ranges::contiguous_range R>
		requires ::std::integral<::std::ranges::range_value_t<R>>
	[[nodiscard]] inline constexpr binfmt parse_libformat_str(R&& r)
	{
		using char_type = ::std::ranges::range_value_t<R>;
		details::libfmt_format_parser<char_type> p;
		auto begin{ ::std::to_address(::std::ranges::cbegin(r)) };
		auto end{ ::std::to_address(::std::ranges::cend(r)) };
		while (begin != end)
		{
			begin = p.parse_literal(begin, end) - begin + begin;
			if (begin == end)
			{
				break;
			}
			begin = p.parse_format(begin, end) - begin + begin;
			if (begin == end)
			{
				break;
			}
		}
		return p.retval;
	}

	template <typename T>
	struct binfmt_apply_t
	{
		T value;
		binfmt_kinds kinds;
		binfmt_size_t args[6];
	};
	template <>
	struct binfmt_apply_t<void>
	{
		binfmt_kinds kinds;
		binfmt_size_t args[6];
	};
	template <typename T>
	inline constexpr binfmt_apply_t<::std::remove_cvref_t<T>> binfmt_apply(T&& t, binfmt_apply_t<void> const& fmt) noexcept
	{
		binfmt_apply_t<::std::remove_cvref_t<T>> retval{ t, fmt.kinds, {} };
		freestanding::my_memcpy(retval.args, fmt.args, 6 * sizeof(binfmt_size_t));
		return retval;
	}
	//template <::std::integral char_type, typename T>
	//struct binfmt_apply_state_t
	//{
	//	context_print_result<char_type*> print_context_define(binfmt_apply_t<T> t, char_type* begin, char_type* end) noexcept
	//	{
	//		return { begin, true };
	//	}
	//};
	//template <::std::integral char_type, typename T>
	//inline constexpr io_type_t<binfmt_apply_state_t<char_type, T>> print_context_type(io_reserve_type_t<char_type, binfmt_apply_t<T>>) noexcept
	//{
	//	return {};
	//}
	template <typename O, typename T>
	inline constexpr void print_define(io_reserve_type_t<typename O::output_char_type, binfmt_apply_t<T>>, O stm, binfmt_apply_t<T> const& t) noexcept
	{
		using char_type = typename O::output_char_type;
		// fast path
		if (t.kinds == binfmt_kinds::no_format)
		{
			return operations::print_freestanding<false>(stm, t.value);
		}

		if ((t.kinds & binfmt_kinds::width) != static_cast<binfmt_kinds>(0))
		{
			auto align{ scalar_placement::none };
			char_type fill_content;
			if ((t.kinds & binfmt_kinds::align) != static_cast<binfmt_kinds>(0))
			{
				switch (static_cast<binfmt_align>(t.args[1]))
				{
					using enum binfmt_align;
				case internal:
					align = scalar_placement::internal;
					break;
				case left:
					align = scalar_placement::left;
					break;
				case right:
					align = scalar_placement::right;
					break;
				case center:
					align = scalar_placement::middle;
					break;
				}
			}
			if ((t.kinds & binfmt_kinds::fill) != static_cast<binfmt_kinds>(0))
			{
				fill_content = static_cast<char_type>(t.args[2]);
			}
			else
			{
				fill_content = char_literal_v<u8' ', char_type>;
			}
			//if ((t.kinds & binfmt_kinds::precision) != static_cast<binfmt_kinds>(0))
			//{
			//fast_io::mnp::decimal
			//}
			//if ((t.kinds & binfmt_kinds::type) != static_cast<binfmt_kinds>(0))
			//{
			//	switch (t.args[5])
			//	{
			//	default:
			//		break;
			//	}
			//}
			return operations::print_freestanding<false>(stm, mnp::width(align, t.value, t.args[3], fill_content));
		}
		return operations::print_freestanding<false>(stm, t.value);
	}
	template <typename T>
	inline constexpr binfmt_size_t get_nth_integer(binfmt_size_t which, T&& t) noexcept
	{
		if (which)
		{
			fast_terminate();
		}
		else
		{
			return static_cast<binfmt_size_t>(t);
		}
	}
	template <typename T, typename ...Args>
	inline constexpr binfmt_size_t get_nth_integer(binfmt_size_t which, T&& t, Args&& ...args) noexcept
	{
		if (which)
		{
			return get_nth_integer(which - 1, ::fast_io::freestanding::forward<Args>(args)...);
		}
		else
		{
			return static_cast<binfmt_size_t>(t);
		}
	}
	template <typename O, typename T>
	inline constexpr void print_1st(O stm, binfmt_apply_t<void> fmt, T&& t) noexcept
	{
		using applied_type = binfmt_apply_t<::std::remove_cvref_t<T>>;
		applied_type applied(binfmt_apply(t, fmt));
		operations::print_freestanding<false>(stm, applied);
	}
	template <typename O, typename T>
	inline constexpr void print_nth(O&& stm, ::std::size_t which, binfmt_apply_t<void> fmt, T&& t) noexcept
	{
		if (which)
		{
			fast_terminate();
		}
		else
		{
			return print_1st(stm, fmt, ::fast_io::freestanding::forward<T>(t));
		}
	}
	template <typename O, typename T, typename ...Args>
	inline constexpr void print_nth(O stm, ::std::size_t which, binfmt_apply_t<void> fmt, T&& t, Args&& ...args) noexcept
	{
		if (which)
		{
			return print_nth(stm, which - 1, fmt, ::fast_io::freestanding::forward<Args>(args)...);
		}
		else
		{
			return print_1st(stm, fmt, ::fast_io::freestanding::forward<T>(t));
		}
	}
	template <::std::integral char_type, typename T>
	inline constexpr context_print_result<char_type*> print_context_1st(char_type* begin, char_type* end, T&& t, binfmt_apply_t<void> fmt) noexcept
	{
		using applied_type = binfmt_apply_t<::std::remove_cvref_t<T>>;
		applied_type applied(binfmt_apply(t, fmt));
		typename ::std::remove_cvref_t<decltype(print_context_type(io_reserve_type<char_type, applied_type>))>::type state;
		return state.print_context_define(applied, begin, end);
	}
	template <::std::integral char_type, typename T, typename ...Args>
	inline constexpr context_print_result<char_type*> print_context_nth(char_type* begin, char_type* end, ::std::size_t which, T&& t, Args&& ...args, binfmt_apply_t<void> fmt) noexcept
	{
		if (which)
		{
			return print_context_nth(begin, end, which - 1, ::fast_io::freestanding::forward<Args>(args)..., fmt);
		}
		else
		{
			return print_context_1st(begin, end, ::fast_io::freestanding::forward<T>(t), fmt);
		}
	}
	template <::std::integral char_type, typename T>
	inline constexpr context_print_result<char_type*> print_context_nth(char_type* begin, char_type* end, ::std::size_t which, T&& t, binfmt_apply_t<void> fmt) noexcept
	{
		if (which)
		{
			fast_terminate();
		}
		else
		{
			return print_context_1st(begin, end, ::fast_io::freestanding::forward<T>(t), fmt);
		}
	}
	template <typename T, typename ...Args>
	inline constexpr void binfmt_print(T stm, binfmt const& fmt, Args ...args) noexcept
	{
		using char_type = typename T::output_char_type;
		//static_assert(context_printable<char_type, binfmt_apply_t<Args>> && ...);
		auto begin{ fmt.begin() };
		auto const end{ fmt.end() };
		while (begin != end)
		{
			if (*begin == ::std::byte{ 0 })
			{
				++begin;
				if (static_cast<::std::size_t>(end - begin) < sizeof(binfmt_size_t)) [[unlikely]]
					{
						fast_terminate();
					}
					binfmt_size_t len;
					begin = details::byte_read_unsafe(begin, len) - begin + begin;
					if (static_cast<::std::size_t>(end - begin) < len * sizeof(char_type)) [[unlikely]]
						{
							fast_terminate();
						}
						details::local_operator_new_array_ptr<char_type> ptr(len);
						details::my_memcpy(ptr.get(), begin, len * sizeof(char_type));
						::fast_io::operations::decay::write_all_decay(stm, ptr.get(), ptr.get() + len);
						begin += len;
			}
			else if (*begin == ::std::byte{ 1 })
			{
				++begin;
				if (static_cast<::std::size_t>(end - begin) < 2 * sizeof(binfmt_size_t)) [[unlikely]]
					{
						fast_terminate();
					}
					binfmt_size_t index;
					::std::underlying_type_t<binfmt_kinds> fmt_kinds;
					begin = details::byte_read_unsafe(begin, index) - begin + begin;
					begin = details::byte_read_unsafe(begin, fmt_kinds) - begin + begin;
					binfmt_apply_t<void> fmt{ static_cast<binfmt_kinds>(fmt_kinds), {} };
					auto num{ ::std::popcount(fmt_kinds & 0x0000ffff) };
					if (static_cast<::std::size_t>(end - begin) < num * sizeof(binfmt_size_t)) [[unlikely]]
						{
							fast_terminate();
						}
						::std::size_t i{};
						for (;;)
						{
							auto num{ ::std::countr_zero(fmt_kinds) };
							i += num;
							if (i >= 6)
							{
								break;
							}
							fmt_kinds >>= num;
							binfmt_size_t arg;
							begin = details::byte_read_unsafe(begin, arg) - begin + begin;
							if (fmt_kinds & 0x00010000)
							{
								fmt.args[i] = get_nth_integer(arg, args...);
							}
							else
							{
								fmt.args[i] = arg;
							}
							fmt_kinds &= ::std::numeric_limits<binfmt_size_t>::max() - 1;
						}
						//constexpr ::std::size_t reserved_size{ 32u };
						//char_type buffer[reserved_size];
						//char_type* buffered{ buffer + reserved_size };
						//for (;;)
						//{
						//	auto [resit, done] = print_context_nth(buffer, buffered, index, args..., fmt);
						//	::fast_io::operations::decay::write_all_decay(stm, buffer, resit);
						//	if (done)
						//	{
						//		break;
						//	}
						//}
						print_nth(stm, index, fmt, args...);
			}
			else
			{
				fast_terminate();
			}
		}
	}

}

#if 0
extern "C" int LLVMFuzzerTestOneInput(char* s, size_t n)
{
	try
	{
		auto v{ fast_io::parse_libformat_str(std::ranges::subrange(s, s + n)) };
	}
	catch (...) {}
	return 0;
}
#else

#include <ranges>
#include <iostream>
#include <format>

template <std::size_t N>
void test(char const(&s)[N])
try {
	auto v = fast_io::parse_libformat_str(s);
}
catch (...) {}

int main() {
	test("\2037{:{");
	//fast_io::binfmt_print(fast_io::c_stdout(), fast_io::parse_libformat_str("{:>#{}d}\n"), 2, 3);
	//std::print(std::cout, "{:>{}.{}}\n", 2, 3, 4);
	//fast_io::io::print(fast_io::mnp::right(fast_io::mnp::hex0x(123), 5));
}
#endif


/* universal format string rule
* ufs := array<element>
* element := 0 str | 1 arg
* str := sizeof(string) string
* arg := index format_kinds format_arguments[length = popcount(format_kinds)]
*/

/* format kinds
*      0 => no format
* 1 << 0 => prefix
* 1 << 1 => align
* 1 << 2 => fill
* 1 << 3 => width
* 1 << 4 => precision
* 1 << 5 => type
* _ << 16 | _ => runtime version
*/

/* format arguments
* align: left right middle
* prefix: + - space # (in bitwise)
* required types: int float str time
* type formats: according to the table
*/
