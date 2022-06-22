//
// Created by maxng on 6/21/2022.
//

#ifndef REBAR_PREPROCESS_HPP
#define REBAR_PREPROCESS_HPP

#include "definitions.hpp"
#include "lexer.hpp"
#include "parser.hpp"

namespace rebar {
    struct parse_unit {
        std::string m_plaintext;
        lex_unit m_lex_unit;
        node::block m_block;

        parse_unit() noexcept = default;

        parse_unit(const parse_unit&) = delete;
        parse_unit(parse_unit&& a_unit) noexcept :
                m_plaintext(std::move(a_unit.m_plaintext)),
                m_lex_unit(std::move(a_unit.m_lex_unit)),
                m_block(std::move(a_unit.m_block)) {}

        parse_unit& operator=(const parse_unit&) = delete;
        parse_unit& operator=(parse_unit&& a_unit) noexcept {
            m_plaintext = std::move(a_unit.m_plaintext);
            m_lex_unit = std::move(a_unit.m_lex_unit);
            m_block = std::move(a_unit.m_block);
        }

        [[maybe_unused]] [[nodiscard]] std::string string_representation() const noexcept {
            std::string string;

            for (const auto& n : m_block) {
                string += n.to_string();
            }

            return string;
        }
    };

    [[nodiscard]] parse_unit parse(lexer& a_lexer, std::string a_string) noexcept {
        parse_unit unit;
        unit.m_plaintext = std::move(a_string);
        unit.m_lex_unit = std::move(a_lexer.lex(unit.m_plaintext));
        unit.m_block = parse_block(span<token>(unit.m_lex_unit.tokens()));

        return std::move(unit);
    }
}

#endif //REBAR_PREPROCESS_HPP
