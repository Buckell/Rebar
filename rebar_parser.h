#include <variant>
#include <vector>
#include <type_traits>
#include <string_view>
#include <map>
#include <span>
#include <forward_list>
#include <optional>

namespace rebar {
	using integer = size_t;
	using number = std::conditional_t<sizeof(size_t) == 8, double, float>;

	enum class token_type : size_t {
		separator,
		identifier,
		string,
		number,
		integer
	};

	enum class separator_type : size_t {
		space,
		assignment,
		addition,
		addition_assignment,
		multiplication,
		multiplication_assignment,
		division,
		division_assignment,
		subtraction,
		subtraction_assignment,
		increment,
		decrement,
		open_group,
		close_group,
		open_selector,
		close_selector,
		open_scope,
		close_scope,
		equality,
		inverse_equality,
		greater,
		lesser,
		greater_equality,
		lesser_equality,
		logical_or,
		logical_and,
		logical_not,
		bitwise_or,
		bitwise_or_assignment,
		bitwise_xor,
		bitwise_xor_assignment,
		bitwise_and,
		bitwise_and_assignment,
		bitwise_not,
		shift_right,
		shift_right_assignment,
		shift_left,
		shift_left_assignment,
		exponent,
		exponent_assignment,
		modulus,
		modulus_assignment,
		seek,
		ternary,
		end_statement,
		dot,
		list,
		direct
	};

	struct token {
		token_type type;
		std::variant<separator_type, std::string_view, integer, number> data;
	
		token(const token_type type, const separator_type sep) noexcept : type(type), data(sep) {}
		token(const token_type type, const std::string_view string) noexcept : type(type), data(string) {}
		token(const token_type type, const integer i) noexcept : type(type), data(i) {}
		token(const token_type type, const number n) noexcept : type(type), data(n) {}

		[[nodiscard]] inline bool is_separator() const noexcept {
			return type == token_type::separator;
		}

		[[nodiscard]] inline bool is_identifier() const noexcept {
			return type == token_type::identifier;
		}

		[[nodiscard]] inline bool is_string() const noexcept {
			return type == token_type::string;
		}

		[[nodiscard]] inline bool is_number() const noexcept {
			return type == token_type::number;
		}

		[[nodiscard]] inline bool is_integer() const noexcept {
			return type == token_type::integer;
		}

		[[nodiscard]] inline separator_type get_separator() const noexcept {
			return std::get<separator_type>(data);
		}

		[[nodiscard]] inline std::string_view get_identifier() const noexcept {
			return std::get<std::string_view>(data);
		}

		[[nodiscard]] inline std::string_view get_string() const noexcept {
			return std::get<std::string_view>(data);
		}

		[[nodiscard]] inline number get_number() const noexcept {
			return std::get<number>(data);
		}

		[[nodiscard]] inline integer get_integer() const noexcept {
			return std::get<integer>(data);
		}

		[[nodiscard]] inline bool operator==(const separator_type separator) const noexcept {
			return is_separator() && get_separator() == separator;
		}

		[[nodiscard]] inline bool operator==(const std::string string) const noexcept {
			return (is_separator() || is_identifier()) && get_string() == string;
		}

		[[nodiscard]] inline bool operator==(const number num) const noexcept {
			return is_number() && get_number() == num;
		}

		[[nodiscard]] inline bool operator==(const integer num) const noexcept {
			return is_integer() && get_integer() == num;
		}
	};

	struct separator {
		std::string_view representation;
		separator_type type_enum;
		size_t precedence;
	};

	class lexer {
		std::vector<separator> separators;

	public:
		lexer() noexcept {
			separators.emplace_back(" ", separator_type::space, 0);
			separators.emplace_back("\n", separator_type::space, 0);
			separators.emplace_back("\t", separator_type::space, 0);
			separators.emplace_back("=", separator_type::assignment, 6);
			separators.emplace_back("+", separator_type::addition, 8);
			separators.emplace_back("+=", separator_type::addition_assignment, 0);
			separators.emplace_back("*", separator_type::multiplication, 10);
			separators.emplace_back("*=", separator_type::multiplication_assignment, 0);
			separators.emplace_back("-", separator_type::subtraction, 7);
			separators.emplace_back("-=", separator_type::subtraction_assignment, 0);
			separators.emplace_back("/", separator_type::division, 9);
			separators.emplace_back("/=", separator_type::division_assignment, 0);
			separators.emplace_back("++", separator_type::increment, 0);
			separators.emplace_back("--", separator_type::decrement, 0);
			separators.emplace_back("(", separator_type::open_group, 0);
			separators.emplace_back(")", separator_type::close_group, 0);
			separators.emplace_back("[", separator_type::open_selector, 0);
			separators.emplace_back("]", separator_type::close_selector, 0);
			separators.emplace_back("}", separator_type::open_scope, 0);
			separators.emplace_back("{", separator_type::close_scope, 0);
			separators.emplace_back("}", separator_type::close_scope, 0);
			separators.emplace_back("==", separator_type::equality, 0);
			separators.emplace_back("!=", separator_type::inverse_equality, 0);
			separators.emplace_back(">", separator_type::greater, 2);
			separators.emplace_back("<", separator_type::lesser, 2);
			separators.emplace_back(">=", separator_type::greater_equality, 2);
			separators.emplace_back("<=", separator_type::lesser_equality, 2);
			separators.emplace_back("||", separator_type::logical_or, 15);
			separators.emplace_back("&&", separator_type::logical_and, 15);
			separators.emplace_back("!", separator_type::logical_not, 0);
			separators.emplace_back("|", separator_type::bitwise_or, 13);
			separators.emplace_back("|=", separator_type::bitwise_or_assignment, 0);
			separators.emplace_back(">|", separator_type::bitwise_xor, 13);
			separators.emplace_back(">|=", separator_type::bitwise_xor_assignment, 0);
			separators.emplace_back("&", separator_type::bitwise_and, 13);
			separators.emplace_back("&=", separator_type::bitwise_and_assignment, 0);
			separators.emplace_back("~", separator_type::bitwise_not, 0);
			separators.emplace_back(">>", separator_type::shift_right, 13);
			separators.emplace_back(">>=", separator_type::shift_right_assignment, 0);
			separators.emplace_back("<<", separator_type::shift_left, 13);
			separators.emplace_back("<<=", separator_type::shift_left_assignment, 0);
			separators.emplace_back("^", separator_type::exponent, 20);
			separators.emplace_back("^=", separator_type::exponent_assignment, 0);
			separators.emplace_back("%", separator_type::modulus, 13);
			separators.emplace_back("%=", separator_type::modulus_assignment, 0);
			separators.emplace_back(":", separator_type::seek, 30);
			separators.emplace_back("?", separator_type::ternary, 25);
			separators.emplace_back(";", separator_type::end_statement, 0);
			separators.emplace_back(".", separator_type::dot, 30);
			separators.emplace_back(",", separator_type::list, 0);
			separators.emplace_back("->", separator_type::direct, 25);
		}

		[[nodiscard]] std::vector<token> operator()(const std::string_view string) const noexcept {
			std::vector<token> tokens;

			bool string_mode{ false };
			bool escape_mode{ false };

			int64_t string_start_index{ -1 };
			int64_t identifier_start_index{ -1 };

			size_t scan_index{ 0 };
			while (scan_index < string.size()) {
				if (string_mode) {
					if (escape_mode) {
						escape_mode = false;
					} else {
						if (string[scan_index] == '"') {
							if (identifier_start_index != -1) {
								tokens.emplace_back(token_type::string, string.substr(static_cast<size_t>(identifier_start_index), static_cast<size_t>(scan_index - identifier_start_index)));
								identifier_start_index = -1;
							}

							string_mode = false;
							tokens.emplace_back(token_type::string, string.substr(static_cast<size_t>(identifier_start_index), static_cast<size_t>(scan_index - identifier_start_index)));
							scan_index++;
							string_start_index = -1;
							continue;
						}

						escape_mode = string[scan_index] == '\\';
					}

					scan_index++;
					continue;
				}

				if (string[scan_index] == '"') {
					string_mode = true;
					string_start_index = static_cast<int64_t>(++scan_index);
					continue;
				}

				std::string_view current_separator_index;
				separator_type current_separator;

				for (const separator& sep : separators) {
					if (sep.representation.size() > current_separator_index.size() && scan_index + sep.representation.size() - 1 < string.size() && string.substr(scan_index, sep.representation.size()) == sep.representation) {
						current_separator_index = sep.representation;
						current_separator = sep.type_enum;
					}
				}

				if (!current_separator_index.empty()) {
					if (identifier_start_index != -1) {
						tokens.emplace_back(token_type::identifier, string.substr(static_cast<size_t>(identifier_start_index), static_cast<size_t>(scan_index - identifier_start_index)));
						identifier_start_index = -1;
					}

					tokens.emplace_back(token_type::separator, current_separator);

					scan_index += current_separator_index.size();
				} else {
					identifier_start_index = static_cast<int64_t>(identifier_start_index < 0 ? scan_index : identifier_start_index);
					scan_index++;
				}
			}

			if (identifier_start_index > 0) {
				tokens.emplace_back(token_type::identifier, string.substr(static_cast<size_t>(identifier_start_index), static_cast<size_t>(scan_index - identifier_start_index)));
			}

			tokens.erase(std::remove_if(tokens.begin(), tokens.end(), [](const token& t) noexcept -> bool {
				return t == separator_type::space;
			}), tokens.cend());

			return std::move(tokens);
		}
	};

	class unit {
		enum class node_type : size_t {
			token,
			argument_list,
			group,
			selector,
			ranged_selector,
			block,
			statement
		};

		struct node {
			using argument_list = std::vector<node>;
			using group = std::vector<node>;
			using selector = std::vector<node>;
			using ranged_selector = std::pair<std::vector<node>, std::vector<node>>;
			using block = std::vector<node>;
			using statement = std::vector<node>;

			node_type type;
			std::variant<token, std::vector<node>, ranged_selector> data;

			node(const token t) noexcept : type(node_type::token), data(t) {}
			node(const node_type type, std::vector<node> nodes) noexcept : type(type), data(std::move(nodes)) {}
			node(ranged_selector sel) noexcept : type(node_type::ranged_selector), data(std::move(sel)) {}
			node(const node& n) noexcept : type(n.type), data(n.data) {}
			node(node&& n) noexcept : type(n.type), data(std::move(n.data)) {}
		};

		std::string plaintext;
		std::vector<token> tokens;
		node::block block;
	
	public:
		unit(std::string string) noexcept : plaintext(std::move(string)) {
			const static lexer lex;
			tokens = lex(plaintext);
			
			block = parse_block(tokens);
		}

	private:
		[[nodiscard]] node::ranged_selector parse_ranged_selector(const std::span<token> selector_tokens) const noexcept {
			size_t selector_increment{ 0 };

			for (size_t i{ 0 }; i < selector_tokens.size(); i++) {
				const token& t{ selector_tokens[i] };

				if (t == separator_type::open_selector) {
					selector_increment++;
				} else if (t == separator_type::close_selector) {
					selector_increment--;
				} else if (selector_increment == 0 && t == separator_type::seek) {
					return node::ranged_selector(parse_group(selector_tokens.subspan(0, i - 1)), parse_group(selector_tokens.subspan(i + 1)));
				}
			}
		}

		[[nodiscard]] node::argument_list parse_arguments(const std::span<token> argument_tokens) const noexcept {
			std::vector<node> argument_nodes;
			size_t group_increment{ 0 };
			size_t last_index{ 0 };

			for (size_t i{ 0 }; i < argument_tokens.size(); i++) {
				const token& t{ argument_tokens[i] };

				if (t == separator_type::open_group) {
					group_increment++;
				} else if (t == separator_type::close_group) {
					group_increment--;
				} else if (group_increment == 0 && t == separator_type::list) {
					if (i - last_index == 2) {
						argument_nodes.emplace_back(argument_tokens[i - 1]);
					} else {
						argument_nodes.emplace_back(parse_group(argument_tokens.subspan(++last_index, i - last_index)));
					}

					last_index = i;
				}
			}
		}

		[[nodiscard]] node::group parse_group(const std::span<token> group_tokens) const noexcept {
			std::vector<node> group_nodes;

			bool group_parse{ false };
			bool argument_parse{ false };
			size_t group_index{ 0 };
			size_t group_increment{ 0 };

			bool selector_parse{ false };
			bool ranged_selector_parse{ false };
			size_t selector_index{ 0 };
			size_t selector_increment{ 0 };

			bool block_parse{ false };
			size_t block_index{ 0 };
			size_t block_increment{ 0 };

			for (size_t i{ 0 }; i < group_tokens.size(); i++) {
				const token& t{ group_tokens[i] };

				if (group_parse) {
					if (t == separator_type::open_group) {
						group_increment++;
					} else if (t == separator_type::close_group) {
						group_increment--;
					} else if (t == separator_type::list && group_increment == 1) {
						argument_parse = true;
					}

					if (group_increment == 0) {
						if (argument_parse) {
							group_nodes.emplace_back(node_type::group, parse_arguments(group_tokens.subspan(group_index, i - group_index)));
							argument_parse = false;
						} else {
							group_nodes.emplace_back(node_type::group, parse_group(group_tokens.subspan(group_index, i - group_index)));
						}

						group_parse = false;
					}
				} else if (selector_parse) {
					if (t == separator_type::open_selector) {
						selector_increment++;
					} else if (t == separator_type::close_selector) {
						selector_increment--;
					} else if (t == separator_type::seek && selector_increment == 1) {
						ranged_selector_parse = true;
					}

					if (selector_increment == 0) {
						if (ranged_selector_parse) {
							group_nodes.emplace_back(parse_ranged_selector(group_tokens.subspan(selector_index, i - selector_index)));
							ranged_selector_parse = false;
						} else {
							group_nodes.emplace_back(node_type::selector, parse_group(group_tokens.subspan(selector_index, i - selector_index)));
						}

						selector_parse = false;
					}
				} else if (block_parse) {
					if (t == separator_type::open_scope) {
						block_increment++;
					} else if (t == separator_type::close_scope) {
						block_increment--;
					}

					if (block_increment == 0) {
						group_nodes.emplace_back(node_type::block, parse_block(group_tokens.subspan(block_index, i - block_index)));
						block_parse = false;
					}
				} else {
					if (t == separator_type::open_group) {
						group_parse = true;
						group_index = i + 1;
						group_increment++;
					} else if (t == separator_type::open_selector) {
						selector_parse = true;
						selector_index = i + 1;
						selector_increment++;
					} else if (t == separator_type::open_scope) {
						block_parse = true;
						block_index = i + 1;
						block_increment++;
					} else {
						group_nodes.emplace_back(t);
					}
				}
			}
		}
		
		[[nodiscard]] node::block parse_block(const std::span<token> block_tokens) const noexcept {
			std::vector<node> block_nodes;

			size_t statement_begin_index{ 0 };

			for (size_t i{ 0 }; i < block_tokens.size(); i++) {
				const token& t{ block_tokens[i] };

				if (t == separator_type::end_statement) {
					block_nodes.emplace_back(node_type::statement, parse_group(block_tokens.subspan(statement_begin_index, i - statement_begin_index)));
				}
			}

			return std::move(block_nodes);
		}
	};
}