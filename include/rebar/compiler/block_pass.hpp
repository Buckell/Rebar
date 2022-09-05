//
// Created by maxng on 8/21/2022.
//

#ifndef REBAR_BLOCK_PASS_HPP
#define REBAR_BLOCK_PASS_HPP

#include "../compiler.hpp"

namespace rebar {
    void compiler::perform_block_pass(function_context& a_ctx, const node::block& a_block, pass_flags a_flags) {
        auto& cc = a_ctx.assembler;
        a_ctx.local_variable_list.emplace_back();

        a_ctx.block_local_offsets.emplace_back(0);

        for (const auto& node : a_block) {
            perform_node_pass(a_ctx, node);
        }

        auto& locals_table = a_ctx.local_variable_list.back();

        if (!locals_table.empty()) {
            auto block_local_stack_offset = a_ctx.block_local_offsets.back();

            a_ctx.local_stack_position -= block_local_stack_offset;

            if constexpr (debug_mode) {
                cc.commentf("Perform garbage collection. (Offset: %d - %d Objects)", a_ctx.local_stack_position, locals_table.size());
            }

            // Dereference and garbage collect locals.
            asmjit::x86::Mem locals_stack(a_ctx.locals_stack);
            locals_stack.addOffset(a_ctx.local_stack_position * sizeof(object));

            cc.lea(a_ctx.identifier, locals_stack);
            cc.mov(a_ctx.lhs_type, locals_table.size());

            asmjit::InvokeNode* invoke_node;
            cc.invoke(&invoke_node, object::block_dereference, asmjit::FuncSignatureT<void, object*, size_t>(platform_call_convention));
            invoke_node->setArg(0, a_ctx.identifier);
            invoke_node->setArg(1, a_ctx.lhs_type);
        }

        a_ctx.block_local_offsets.pop_back();
        a_ctx.local_variable_list.pop_back();
    }
}

#endif //REBAR_BLOCK_PASS_HPP
