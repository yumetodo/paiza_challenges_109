#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <type_traits>
#include <codecvt>
#include <limits>
#include <string>
#include <type_traits>
#include "../3rd_party/string_split.hpp"
namespace atithmetic_cvt {
	using std::nullptr_t;
	namespace detail {
		namespace stl_wrap {
			template<typename T> struct is_char_type_old : std::false_type {};
			template<> struct is_char_type_old<char> : std::true_type {};
			template<> struct is_char_type_old<wchar_t> : std::true_type {};
			template <typename char_type> int stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, int base, int) {
				return std::stoi(s, idx, base);
			}
			template <typename char_type> long stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, int base, long) {
				return std::stol(s, idx, base);
			}
			template <typename char_type> long long stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, int base, long long) {
				return std::stoll(s, idx, base);
			}
			template <typename char_type> unsigned int stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, int base, unsigned int) {
				const unsigned long re = std::stoul(s, idx, base);
				if (std::numeric_limits<unsigned int>::max() < re) throw std::out_of_range("in function atithmetic_cvt::detail::stl_wrap::stox_impl(int)");
				return static_cast<unsigned int>(re);
			}
			template <typename char_type> unsigned long stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, int base, unsigned long) {
				return std::stoul(s, idx, base);
			}
			template <typename char_type> unsigned long long stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, int base, unsigned long long) {
				return std::stoull(s, idx, base);
			}
			template<typename T, typename char_type, std::enable_if_t<is_char_type_old<char_type>::value && std::is_integral<T>::value, nullptr_t> = nullptr>
			T stox(const std::basic_string<char_type>& s, std::size_t * idx = nullptr, int base = 10) {
				return stox_impl(s, idx, base, T{});
			}
			template <typename char_type> float stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, float) {
				return std::stof(s, idx);
			}
			template <typename char_type> double stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, double) {
				return std::stod(s, idx);
			}
			template <typename char_type> long double stox_impl(const std::basic_string<char_type>& s, std::size_t * idx, long double) {
				return std::stold(s, idx);
			}
			template<typename T, typename char_type, std::enable_if_t<is_char_type_old<char_type>::value && std::is_floating_point<T>::value, nullptr_t> = nullptr>
			inline T stox(const std::basic_string<char_type>& s, std::size_t * idx = nullptr) {
				return stox_impl(s, idx, T{});
			}
		}
		template<typename char_type, typename T>
		struct from_str_helper;
		template<typename T>
		struct from_str_helper<char, T> {
			T operator()(const std::string& s) {
				return stl_wrap::stox<T>(s);
			}
		};
		template<typename T>
		struct from_str_helper<wchar_t, T> {
			T operator()(const std::wstring& s) {
				return stl_wrap::stox<T>(s);
			}
		};
	}
	template<typename T, typename char_type, std::enable_if_t<std::is_arithmetic<T>::value, nullptr_t> = nullptr>
	inline T from_str(const std::basic_string<char_type>& s) {
		return detail::from_str_helper<char_type, T>()(s);
	}
}
namespace detail {
	template<typename T>
	struct to_arithmetic_helper {};
	template<typename T, typename char_type, std::enable_if_t<std::is_arithmetic<T>::value, std::nullptr_t> = nullptr>
	T operator| (const std::basic_string<char_type>& s, to_arithmetic_helper<T>) { return atithmetic_cvt::from_str<T>(s); }
}
template<typename T, std::enable_if_t<std::is_arithmetic<T>::value, std::nullptr_t> = nullptr>
detail::to_arithmetic_helper<T> to_arithmetic() { return{}; }

class message_db {
public:
	using value_type = std::vector<std::string>;
	using type = std::vector<value_type>;
	using id_type = std::size_t;
	using group_type = std::vector<id_type>;
	using group = std::vector<group_type>;
private:
	type data_;
	group group_;
	void push_message(id_type id, std::string message) {
		this->data_[id].push_back(std::move(message));
	}
	void push_message(id_type send_user_id, id_type recive_user_id, std::string message) {
		this->push_message(send_user_id, message);
		this->push_message(recive_user_id, std::move(message));
	}
	void push_message_group(id_type gropu_id, id_type send_user_id, std::string message) {
		const auto& g = this->group_[gropu_id];
		if (std::none_of(g.begin(), g.end(), [send_user_id](id_type id) { return send_user_id == id; })) throw std::runtime_error("no parmission to send data.");
		for (auto&& user_id : g) {
			this->push_message(user_id, message);
		}
	}
public:
	message_db() = default;
	message_db(std::size_t user_num, std::size_t group_num) : message_db() {
		this->data_.resize(user_num);
		this->group_.reserve(group_num);
	}
	void push_group_info(group_type group) {
		this->group_.push_back(std::move(group));
	}
	void push_message(bool to_group, id_type send_user_id, id_type recive_id, std::string message) {
		if (to_group) {
			this->push_message_group(recive_id - 1, send_user_id - 1, std::move(message));
		}
		else {
			push_message(send_user_id - 1, recive_id - 1, std::move(message));
		}
	}
	const value_type& get_message_line(id_type user_id) {
		return this->data_[user_id];
	}
};
int main() {
	std::string buf;
	std::getline(std::cin, buf);
	const auto n = buf | split(' ')[0] | to_arithmetic<std::size_t>();
	const auto g = buf | split(' ')[1] | to_arithmetic<std::size_t>();
	const auto m = buf | split(' ')[2] | to_arithmetic<std::size_t>();
	message_db db(n, g);
	for (std::size_t i = 0; i < g; ++i) {
		std::getline(std::cin, buf);
		const auto tmp = buf | split(' ');
		message_db::group_type tmp2;
		tmp2.reserve(tmp.size() - 1);
		for (auto it = tmp.begin() + 1; it != tmp.end(); ++it) tmp2.push_back((*it | to_arithmetic<message_db::id_type>()) - 1);
		db.push_group_info(std::move(tmp2));
	}
	for (std::size_t i = 0; i < m; ++i) {
		std::getline(std::cin, buf);
		db.push_message(
			0 != (buf | split(' ')[1] | to_arithmetic<int>()),
			buf | split(' ')[0] | to_arithmetic<std::size_t>(),
			buf | split(' ')[2] | to_arithmetic<std::size_t>(),
			buf | split(' ')[3]
		);
	}
	bool is_first = true;
	for (std::size_t i = 0; i < n; ++i) {
		if (!is_first) {
			std::cout << "--" << std::endl;
		}
		else {
			is_first = !is_first;
		}
		for (auto&& s : db.get_message_line(i)) {
			std::cout << s << std::endl;
		}
	}
}