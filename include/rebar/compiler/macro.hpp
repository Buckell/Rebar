//
// Created by maxng on 8/27/2022.
//

#ifndef REBAR_COMPILER_MACRO_HPP
#define REBAR_COMPILER_MACRO_HPP

#define REBAR_CC_DEBUG(...) if constexpr (debug_mode) {   \
                                cc.commentf("[DEBUG] " __VA_ARGS__); \
                            }

#endif //REBAR_COMPILER_MACRO_HPP
