//
// Created by maxng on 10/2/2022.
//

#ifndef REBAR_NATIVE_TEMPLATE_HPP
#define REBAR_NATIVE_TEMPLATE_HPP

#define REBAR_NATIVE_FUNCTION(name) ::rebar::native_template_function_definition DEF_##name {m_environment, REBAR_STRINGIFY(name), *m_table, name}; \
                                    static REBAR_FUNCTION(name)
#define REBAR_NATIVE_CLASS(name, id) name(environment& a_env) : native_template(a_env, id) {}

#define REBAR_OVERLOAD(name, ret, ...) struct OVERLOAD_##name {                            \
                                           OVERLOAD_##name(virtual_table& a_table) {       \
                                               a_table.overload_##name = overload_##name;  \
                                           }                                               \
                                       };                                                  \
                                       OVERLOAD_##name OVERLOAD_DEF_##name{*m_table};      \
                                       static ret overload_##name(environment* env, native_object self, __VA_ARGS__)

#define REBAR_OVERLOAD_NO_PARAMS(name, ret) struct OVERLOAD_##name {                            \
                                                OVERLOAD_##name(virtual_table& a_table) {       \
                                                    a_table.overload_##name = overload_##name;  \
                                                }                                               \
                                            };                                                  \
                                            OVERLOAD_##name OVERLOAD_DEF_##name{*m_table};      \
                                            static ret overload_##name(environment* env, native_object self)

#define REBAR_OVERLOAD_ASSIGNMENT()                REBAR_OVERLOAD(assignment, void, const object&)
#define REBAR_OVERLOAD_ADDITION()                  REBAR_OVERLOAD(addition, object, const object&)
#define REBAR_OVERLOAD_ADDITION_ASSIGNMENT()       REBAR_OVERLOAD(addition_assignment, void, const object&)
#define REBAR_OVERLOAD_MULTIPLICATION()            REBAR_OVERLOAD(multiplication, object, const object&)
#define REBAR_OVERLOAD_MULTIPLICATION_ASSIGNMENT() REBAR_OVERLOAD(multiplication_assignment, void, const object&)
#define REBAR_OVERLOAD_DIVISION()                  REBAR_OVERLOAD(division, object, const object&)
#define REBAR_OVERLOAD_DIVISION_ASSIGNMENT()       REBAR_OVERLOAD(division_assignment, void, const object&)
#define REBAR_OVERLOAD_SUBTRACTION()               REBAR_OVERLOAD(subtraction, object, const object&)
#define REBAR_OVERLOAD_SUBTRACTION_ASSIGNMENT()    REBAR_OVERLOAD(subtraction_assignment, void, const object&)
#define REBAR_OVERLOAD_EQUALITY()                  REBAR_OVERLOAD(equality, object, const object&)
#define REBAR_OVERLOAD_INVERSE_EQUALITY()          REBAR_OVERLOAD(inverse_equality, object, const object&)
#define REBAR_OVERLOAD_GREATER()                   REBAR_OVERLOAD(greater, object, const object&)
#define REBAR_OVERLOAD_LESSER()                    REBAR_OVERLOAD(lesser, object, const object&)
#define REBAR_OVERLOAD_GREATER_EQUALITY()          REBAR_OVERLOAD(greater_equality, object, const object&)
#define REBAR_OVERLOAD_LESSER_EQUALITY()           REBAR_OVERLOAD(lesser_equality, object, const object&)
#define REBAR_OVERLOAD_LOGICAL_OR()                REBAR_OVERLOAD(logical_or, object, const object&)
#define REBAR_OVERLOAD_LOGICAL_AND()               REBAR_OVERLOAD(logical_and, object, const object&)
#define REBAR_OVERLOAD_LOGICAL_NOT()               REBAR_OVERLOAD(logical_not, object)
#define REBAR_OVERLOAD_BITWISE_OR()                REBAR_OVERLOAD(bitwise_or, object, const object&)
#define REBAR_OVERLOAD_BITWISE_OR_ASSIGNMENT()     REBAR_OVERLOAD(bitwise_or_assignment, void, const object&)
#define REBAR_OVERLOAD_BITWISE_XOR()               REBAR_OVERLOAD(bitwise_xor, object, const object&)
#define REBAR_OVERLOAD_BITWISE_XOR_ASSIGNMENT()    REBAR_OVERLOAD(bitwise_xor_assignment, void, const object&)
#define REBAR_OVERLOAD_BITWISE_AND()               REBAR_OVERLOAD(bitwise_and, object, const object&)
#define REBAR_OVERLOAD_BITWISE_AND_ASSIGNMENT()    REBAR_OVERLOAD(bitwise_and_assignment, void, const object&)
#define REBAR_OVERLOAD_BITWISE_NOT()               REBAR_OVERLOAD(bitwise_not, object)
#define REBAR_OVERLOAD_SHIFT_RIGHT()               REBAR_OVERLOAD(shift_right, object, const object&)
#define REBAR_OVERLOAD_SHIFT_RIGHT_ASSIGNMENT()    REBAR_OVERLOAD(shift_right_assignment, void, const object&)
#define REBAR_OVERLOAD_SHIFT_LEFT()                REBAR_OVERLOAD(shift_left, object, const object&)
#define REBAR_OVERLOAD_SHIFT_LEFT_ASSIGNMENT()     REBAR_OVERLOAD(shift_left_assignment, void, const object&)
#define REBAR_OVERLOAD_EXPONENT()                  REBAR_OVERLOAD(exponent, object, const object&)
#define REBAR_OVERLOAD_EXPONENT_ASSIGNMENT()       REBAR_OVERLOAD(exponent_assignment, void, const object&)
#define REBAR_OVERLOAD_MODULUS()                   REBAR_OVERLOAD(modulus, object, const object&)
#define REBAR_OVERLOAD_MODULUS_ASSIGNMENT()        REBAR_OVERLOAD(modulus_assignment, void, const object&)
#define REBAR_OVERLOAD_LENGTH()                    REBAR_OVERLOAD_NO_PARAMS(length, object)
#define REBAR_OVERLOAD_PREFIX_INCREMENT()          REBAR_OVERLOAD_NO_PARAMS(operation_prefix_increment, void)
#define REBAR_OVERLOAD_POSTFIX_INCREMENT()         REBAR_OVERLOAD_NO_PARAMS(operation_postfix_increment, object)
#define REBAR_OVERLOAD_PREFIX_DECREMENT()          REBAR_OVERLOAD_NO_PARAMS(operation_prefix_decrement, void)
#define REBAR_OVERLOAD_POSTFIX_DECREMENT()         REBAR_OVERLOAD_NO_PARAMS(operation_postfix_decrement, object)
#define REBAR_OVERLOAD_INDEX()                     REBAR_OVERLOAD(operation_index, object&, const object&)
#define REBAR_OVERLOAD_SELECT()                    REBAR_OVERLOAD(operaion_select, object, const object&)
#define REBAR_OVERLOAD_RANGED_SELECT()             REBAR_OVERLOAD(operaion_ranged_select, object, const object&, const object&)
#define REBAR_OVERLOAD_CALL()                      REBAR_OVERLOAD_NO_PARAMS(operation_call, object)
#define REBAR_OVERLOAD_CONSTRUCT()                 REBAR_OVERLOAD_NO_PARAMS(operation_new, object)

namespace rebar {
    namespace overload_test {
        REBAR_OVERLOAD_TEST(assignment, overload_assignment, void, environment*, native_object, const object&);

        REBAR_OVERLOAD_TEST(addition, overload_addition, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(addition_assignment, overload_addition_assignment, void, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(multiplication, overload_multiplication, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(multiplication_assignment, overload_multiplication_assignment, void, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(division, overload_division, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(division_assignment, overload_division_assignment, void, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(subtraction, overload_subtraction, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(subtraction_assignment, overload_subtraction_assignment, void, environment*, native_object, const object&);

        REBAR_OVERLOAD_TEST(equality, overload_equality, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(inverse_equality, overload_inverse_equality, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(greater, overload_greater, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(lesser, overload_lesser, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(greater_equality, overload_greater_equality, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(lesser_equality, overload_lesser_equality, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(logical_or, overload_logical_or, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(logical_and, overload_logical_and, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(logical_not, overload_logical_not, object, environment*, native_object);

        REBAR_OVERLOAD_TEST(bitwise_or, overload_bitwise_or, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(bitwise_or_assignment, overload_bitwise_or_assignment, void, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(bitwise_xor, overload_bitwise_xor, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(bitwise_xor_assignment, overload_bitwise_xor_assignment, void, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(bitwise_and, overload_bitwise_and, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(bitwise_and_assignment, overload_bitwise_and_assignment, void, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(bitwise_not, overload_bitwise_not, object, environment*, native_object);

        REBAR_OVERLOAD_TEST(shift_right, overload_shift_right, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(shift_right_assignment, overload_shift_right_assignment, void, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(shift_left, overload_shift_left, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(shift_left_assignment, overload_shift_left_assignment, void, environment*, native_object, const object&);

        REBAR_OVERLOAD_TEST(exponent, overload_exponent, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(exponent_assignment, overload_exponent_assignment, void, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(modulus, overload_modulus, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(modulus_assignment, overload_modulus_assignment, void, environment*, native_object, const object&);

        REBAR_OVERLOAD_TEST(length, overload_length, object, environment*, native_object);

        REBAR_OVERLOAD_TEST(prefix_increment, overload_operation_prefix_increment, void, environment*, native_object);
        REBAR_OVERLOAD_TEST(postfix_increment, overload_operation_postfix_increment, object, environment*, native_object);
        REBAR_OVERLOAD_TEST(prefix_decrement, overload_operation_prefix_decrement, void, environment*, native_object);
        REBAR_OVERLOAD_TEST(postfix_decrement, overload_operation_postfix_decrement, object, environment*, native_object);

        REBAR_OVERLOAD_TEST(index, overload_operation_index, const object&, environment*, const object&);
        REBAR_OVERLOAD_TEST(select, overload_operaion_select, object, environment*, native_object, const object&);
        REBAR_OVERLOAD_TEST(ranged_select, overload_operaion_ranged_select, object, environment*, native_object, const object&, const object&);
        REBAR_OVERLOAD_TEST(call, overload_operation_call, const object&, environment*, native_object);
        REBAR_OVERLOAD_TEST(construct, overload_operation_new, const object&, environment*, native_object);
    }

    struct native_template_function_definition {
        native_template_function_definition(environment& a_env, std::string_view a_name, virtual_table& a_table, callable a_function);
    };

    template <typename t_internal_type>
    class native_template {
    protected:
        environment& m_environment;
        virtual_table* m_table;

    public:
        native_template(environment& a_environment, std::string_view a_identifier);
        object build_object();
    };
}

#undef REBAR_OVERLOAD
#undef REBAR_OVERLOAD_NO_PARAMS

#endif //REBAR_NATIVE_TEMPLATE_HPP
