//
// Created by maxng on 8/22/2022.
//

#ifndef REBAR_EXT_HPP
#define REBAR_EXT_HPP

#include "../environment.hpp"
#include "../compiler.hpp"

#include "object_ext.hpp"

namespace rebar {
    object* _ext_index_global(environment* env, size_t object_type, size_t object_data) {
        return &env->global_table()[object{ (type)object_type, object_data }];
    }

    void _ext_reference_object(object* obj) {
        switch (obj->object_type()) {
            case type::string:
                obj->get_string().reference();
                break;
            case type::table:
                ++(obj->get_table().m_reference_count);
                break;
            case type::array:
                obj->get_array().reference();
                break;
            case type::native_object:
                obj->get_native_object().reference();
                break;
            default:
                break;
        }
    }

    void _ext_dereference_object(object* obj) {
        switch (obj->object_type()) {
            case type::string:
                obj->get_string().dereference();
                break;
            case type::table:
                if (--(obj->get_table().m_reference_count) == 0) {
                    delete &obj->get_table();
                }

                break;
            case type::array:
                obj->get_array().dereference();
                break;
            case type::native_object:
                obj->get_native_object().dereference();
                break;
            default:
                break;
        }
    }

    void _ext_output_object_data(size_t type, size_t data) {
        std::cout << "[OUTPUT] " << type << " " << std::hex << data << std::endl;
    }

    rtable* _ext_allocate_table() {
        return new rtable;
    }

    void _ext_allocate_array(object* ret, size_t a_size) {
        *ret = array(a_size);
        ret->get_array().vector_reference().resize(a_size);
    }
}

#endif //REBAR_EXT_HPP
