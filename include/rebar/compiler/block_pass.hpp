//
// Created by maxng on 8/21/2022.
//

#ifndef REBAR_BLOCK_PASS_HPP
#define REBAR_BLOCK_PASS_HPP

#include "../compiler.hpp"

#include "macro.hpp"

namespace rebar {
    void compiler::perform_block_pass(function_context& ctx, const node::block& a_block) {
        // SYNCHRONIZE CHANGES WITH PERFORM_NODE_PASS - CASE FOR_DECLARATION

        function_context::pass_control pass(ctx);

        auto& cc = ctx.assembler;
        ctx.local_variable_list.emplace_back();
        ctx.constant_tables.emplace_back();

        ctx.block_local_offsets.emplace_back(0);

        ctx.if_stack.push_back(std::nullopt);

        for (size_t i = 0; i < a_block.size(); ++i) {
            perform_node_pass(ctx, a_block[i], output_side::lefthand, i + 1 < a_block.size() ? std::optional<const node*>(&a_block[i + 1]) : std::nullopt);
        }

        auto& locals_table = ctx.local_variable_list.back();

        if (!locals_table.empty()) {
            auto block_local_stack_offset = ctx.block_local_offsets.back();

            ctx.local_stack_position -= block_local_stack_offset;

            REBAR_CC_DEBUG("Perform garbage collection. (Offset: %d - %d Objects)", ctx.local_stack_position, locals_table.size());

            // Dereference and garbage collect locals.
            asmjit::x86::Mem locals_stack(ctx.locals_stack);
            locals_stack.addOffset(ctx.local_stack_position * sizeof(object));

            cc.lea(ctx.identifier, locals_stack);
            cc.mov(ctx.lhs_type, locals_table.size());

            REBAR_CODE_GENERATION_GUARD({
                asmjit::InvokeNode* invoke_node;
                cc.invoke(&invoke_node, object::block_dereference, asmjit::FuncSignatureT<void, object*, size_t>(platform_call_convention));
                invoke_node->setArg(0, ctx.identifier);
                invoke_node->setArg(1, ctx.lhs_type);
            })

            pass.set_flags(pass_flag::clobber_identifier | pass_flag::clobber_left);
        }

        ctx.if_stack.pop_back();
        ctx.block_local_offsets.pop_back();
        ctx.local_variable_list.pop_back();
        ctx.constant_tables.pop_back();
    }
}

#endif //REBAR_BLOCK_PASS_HPP
