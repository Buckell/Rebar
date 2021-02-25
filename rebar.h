#pragma once

#include <type_traits>
#include <cstring>
#include <atomic>
#include <thread>
#include <forward_list>
#include <unordered_map>

namespace rebar {
	enum class object_type : size_t {
		null = 0,
		boolean = 1,
		integer = 2,
		function = 3,
		number = 4,
		native_handle = 5,
		table = 6,
		string = 7
	};

	struct object;

	struct object_array {
		size_t count;
		object* objects;
	};

	class environment;

	using function = object(*)(environment*, object_array);

	using integer = size_t;
	using number = std::conditional_t<sizeof(void*) == 8, double, float>;

	struct internal_string {
		size_t size;
		char* buffer;

		static bool equals(const internal_string str1, const internal_string str2) {
			if (str1.size != str2.size || str1.buffer[0] != str2.buffer[0]) [[likely]] {
				return false;
			}

			for (size_t i{ 1 }; i < str1.size; i++) {
				if (str1.buffer[i] != str2.buffer[i]) [[unlikely]] {
					return false;
				}
			}

			return true;
		}
	};

	struct internal_table {
		struct pair;

		struct group;

		group* data;

		// Creates an empty table.
		static internal_table create() noexcept;

		// Frees a table.
		static void free(internal_table t) noexcept;

		// Emplaces an object (value) into the table with the given key (key).
		static void emplace(internal_table tab, const object key, const object value) noexcept;

		// Retrieves a value from the table given a key.
		// NOTE: Optimizations would be extremely beneficial for this function.
		static object get(const internal_table tab, const object key) noexcept;

		// Compares two tables for equality.
		static bool equals(const internal_table tab1, const internal_table tab2) noexcept;
	};

	struct object {
		object_type type;
		size_t data;

		inline size_t numeric_type() const noexcept {
			return reinterpret_cast<const size_t&>(type);
		}

		inline bool is_null() const noexcept {
			return type == object_type::null;
		}

		inline bool is_boolean() const noexcept {
			return type == object_type::boolean;
		}

		inline bool is_integer() const noexcept {
			return type == object_type::integer;
		}

		inline bool is_function() const noexcept {
			return type == object_type::function;
		}

		inline bool is_number() const noexcept {
			return type == object_type::number;
		}

		inline bool is_native_handle() const noexcept {
			return type == object_type::native_handle;
		}

		inline bool is_table() const noexcept {
			return type == object_type::table;
		}

		inline bool is_string() const noexcept {
			return type == object_type::string;
		}

		inline bool get_boolean() const noexcept {
			return data;
		}

		inline integer get_integer() const noexcept {
			return data;
		}

		inline function get_function() const noexcept {
			return reinterpret_cast<function>(data);
		}

		inline number get_number() const noexcept {
			return reinterpret_cast<const number&>(data);
		}

		inline void* get_native_handle() const noexcept {
			return reinterpret_cast<void*>(data);
		}

		inline internal_table get_internal_table() const noexcept {
			return reinterpret_cast<const internal_table&>(data);
		}

		inline internal_string get_string() const noexcept {
			return reinterpret_cast<const internal_string&>(data);
		}

		inline bool equals(const object o) const noexcept {
			// Compare types and return false if types are mismatched.
			if (type != o.type) [[unlikely]] {
				return false;
			}

			// All types with a corresponding integer value equal to or less than
			// object_type::native_handle can simply have their values compared.
			// Types with a greater integer value (string/table) have special 
			// comparison functions.
			if (type <= object_type::native_handle) [[likely]] {
				return data == o.data;
			} else if (is_string()) {
				return internal_string::equals(get_string(), o.get_string());
			} else {
				return internal_table::equals(get_internal_table(), o.get_internal_table());
			}
		}
	};

	struct internal_table::pair {
		object key;
		object value;
	};

	struct internal_table::group {
		size_t count;
		size_t capacity;
		pair* object_pairs;
	};

	inline internal_table internal_table::create() noexcept {
		internal_table t{ new group[134] };
		memset(t.data, 0, sizeof(group) * 134);
		return t;
	}

	void internal_table::free(internal_table t) noexcept {
		for (size_t i{ 0 }; i < 134; i++) {
			delete[] t.data[i].object_pairs;
		}

		delete[] t.data;
	}

	void internal_table::emplace(internal_table tab, const object key, const object value) noexcept {
		// Determine offset into the "hash" table. Strings are mapped by first character and every other type only has one group.
		group* g{ key.is_string() ? tab.data + key.get_string().buffer[0] : tab.data + 128 + key.numeric_type() };

		// Determine if the key already exists. Replaces the existing value if it does.
		for (size_t i{ 0 }; i < g->count; i++) {
			if (key.equals(g->object_pairs[i].key)) {
				g->object_pairs[i].value = value;
				return;
			}
		}

		// Determines if the group's capacity has been reached.
		if (g->count + 1 > g->capacity) [[unlikely]] {
			if (g->capacity == 0) {
				g->object_pairs = new pair[1];
			} else {
				pair* new_pairs{ new pair[g->capacity * 2] };
				memcpy(new_pairs, g->object_pairs, g->capacity);
				g->capacity *= 2;
				delete[] g->object_pairs;
				g->object_pairs = new_pairs;
			}
		}

		g->object_pairs[g->count++] = { key, value };
	}

	object internal_table::get(const internal_table tab, const object key) noexcept {
		// Find offset in the group array.
		group* g{ key.is_string() ? tab.data + key.get_string().buffer[0] : tab.data + 128 + key.numeric_type() };

		// Iterate over the object pairs and if the key is found, return the corresponding value.
		for (size_t i{ 0 }; i < g->count; i++) {
			if (key.equals(g->object_pairs[i].key)) {
				return g->object_pairs[i].value;
			}
		}

		// Return null if the key isn't found.
		return { object_type::null, 0 };
	}

	bool internal_table::equals(const internal_table tab1, const internal_table tab2) noexcept {
		// Iterate through all of the table groups.
		for (size_t i{ 0 }; i < 134; i++) {
			group* g1{ tab1.data + i };
			group* g2{ tab2.data + i };

			// Return false upon mismatching counts.
			if (g1->count != g2->count) {
				return false;
			}

			// Iterate through all of the contained objects/keys and do a simple comparison.
			// NOTE: This segment might benefit from some optimization.
			for (size_t ii{ 0 }; ii < g1->count; ii++) {
				pair p{ g1->object_pairs[ii] };
					
				if (!p.value.equals(get(tab2, p.key))) [[unlikely]] {
					return false;
				}
			}
		}

		return true;
	}
	
	class environment {
		internal_table global_table;

		std::atomic_bool garbage_collector_control;
		std::thread garbage_collector_thread;

		std::forward_list<internal_table*> table_allocations;
		std::forward_list<internal_string*> string_allocations;

		std::unordered_map<internal_table*, size_t> native_table_ref;
		std::unordered_map<internal_string*, size_t> native_string_ref;

	public:
		environment() noexcept : global_table(internal_table::create()), garbage_collector_control(false) {

		}

		void run_async_garbage_collector() {
			garbage_collector_control = true;
			garbage_collector_thread = std::thread(&environment::garbage_collector, this);
		}

		void halt_async_garbage_collector() {

		}

		void collect_garbage() {

		}

	private:
		std::vector<void*> garbage_run_table(internal_table table) {
			std::vector<void*> ptrs;
			
			for (size_t i{ 0 }; i < 134; i++) {
				internal_table::group* g = table.data + i;
				
				if (g->count == 0) [[likely]] {
					continue;
				}

				for (size_t ii{ 0 }; i < g->count; i++) {
					internal_table::pair object_pair;
					
				}
			}

			return std::move(ptrs);
		}

		void garbage_collector() {
			while (garbage_collector_control) {
				collect_garbage();
			}
		}
	};

	class table : private internal_table {
		environment& env;

	public:
		table(environment& env, internal_table tab) noexcept : internal_table{ tab.data }, env(env) {

		}


	};
}