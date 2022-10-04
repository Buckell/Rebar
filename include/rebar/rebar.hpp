//
// Copyright (C) 2021, Max Goddard.
// All rights reserved.
//

#ifndef INCLUDE_REBAR_H
#define INCLUDE_REBAR_H

#pragma once

#include "array.hpp"
#include "array_impl.hpp"
#include "compiler.hpp"
#include "compiler/block_pass.hpp"
#include "compiler/compiler_impl.hpp"
#include "compiler/expression_pass.hpp"
#include "compiler/ext.hpp"
#include "compiler/function_compile.hpp"
#include "compiler/load_items.hpp"
#include "compiler/misc.hpp"
#include "compiler/node_pass.hpp"
#include "compiler/preliminary_scan.hpp"
#include "definitions.hpp"
#include "environment.hpp"
#include "exception.hpp"
#include "exception_impl.hpp"
#include "function.hpp"
#include "function_impl.hpp"
#include "interpreter.hpp"
#include "interpreter_impl.hpp"
#include "lexer.hpp"
#include "library.hpp"
#include "native_object.hpp"
#include "native_object_impl.hpp"
#include "native_template.hpp"
#include "native_template_impl.hpp"
#include "object.hpp"
#include "object_impl.hpp"
#include "operator_precedence.hpp"
#include "optional.hpp"
#include "parser.hpp"
#include "preprocess.hpp"
#include "provider.hpp"
#include "span.hpp"
#include "string.hpp"
#include "string_impl.hpp"
#include "table.hpp"
#include "token.hpp"
#include "utility.hpp"
#include "zip.hpp"

#endif // #ifndef INCLUDE_REBAR_H
