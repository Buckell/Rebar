//
// Created by maxng on 8/22/2022.
//

#ifndef REBAR_EXT_HPP
#define REBAR_EXT_HPP

#include "../environment.hpp"

#include "object_ext.hpp"

namespace rebar {
    object* _ext_index_global(environment* env, size_t object_type, size_t object_data) {
        return &env->global_table()[object{ (type)object_type, object_data }];
    }

    void _ext_reference_object(size_t object_type, size_t object_data) {
        object o((type)object_type, object_data);

        switch (o.object_type()) {
            case type::string:
                o.get_string().reference();
                break;
            case type::table:
                ++(o.get_table().m_reference_count);
                break;
            case type::array:
                o.get_array().reference();
                break;
            case type::native_object:
                o.get_native_object().reference();
                break;
            default:
                break;
        }
    }

    void _ext_dereference_object(size_t object_type, size_t object_data) {
        object o((type)object_type, object_data);

        switch (o.object_type()) {
            case type::string:
                o.get_string().dereference();
                break;
            case type::table:
                if (--(o.get_table().m_reference_count) == 0) {
                    delete &o.get_table();
                }

                break;
            case type::array:
                o.get_array().dereference();
                break;
            case type::native_object:
                o.get_native_object().dereference();
                break;
            default:
                break;
        }
    }

    void _ext_output_object_data(size_t type, size_t data, size_t annotation) {
        std::cout << "[" << std::dec << annotation << "] " << type << " " << std::hex << data << std::endl;
    }

    table* _ext_allocate_table() {
        return new table;
    }
}

#endif //REBAR_EXT_HPP
