// FIXME: This is a workaround implementation for std module
// to ease the use of modules. Please replace this with the official std module
// when that is available.
module;
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <functional>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <mutex>
#include <new>
#include <optional>
#include <queue>
#include <ranges>
#include <ratio>
#include <semaphore>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-module-identifier"
#endif

export module std;

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

export namespace std {
  using std::vector;
  using std::operator==;
  using std::operator<=>;
  using std::swap;
  using std::erase;
  using std::erase_if;
  namespace pmr {
    using std::pmr::vector;
  }
  using std::hash;
}


export namespace std {
  using std::incrementable_traits;
  using std::iter_difference_t;
  using std::indirectly_readable_traits;
  using std::iter_value_t;
  using std::iterator_traits;
  using std::iter_reference_t;
  using std::iter_rvalue_reference_t;
  using std::indirectly_readable;
  using std::iter_common_reference_t;
  using std::indirectly_writable;
  using std::weakly_incrementable;
  using std::incrementable;
  using std::input_or_output_iterator;
  using std::sentinel_for;
  using std::disable_sized_sentinel_for;
  using std::sized_sentinel_for;
  using std::input_iterator;
  using std::output_iterator;
  using std::forward_iterator;
  using std::bidirectional_iterator;
  using std::random_access_iterator;
  using std::contiguous_iterator;
  using std::indirectly_unary_invocable;
  using std::indirectly_regular_unary_invocable;
  using std::indirect_unary_predicate;
  using std::indirect_binary_predicate;
  using std::indirect_equivalence_relation;
  using std::indirect_strict_weak_order;
  using std::indirect_result_t;
  using std::projected;
  using std::indirectly_movable;
  using std::indirectly_movable_storable;
  using std::indirectly_copyable;
  using std::indirectly_copyable_storable;
  using std::indirectly_swappable;
  using std::indirectly_comparable;
  using std::permutable;
  using std::mergeable;
  using std::sortable;
  using std::bidirectional_iterator_tag;
  using std::contiguous_iterator_tag;
  using std::forward_iterator_tag;
  using std::input_iterator_tag;
  using std::output_iterator_tag;
  using std::random_access_iterator_tag;
  using std::advance;
  using std::distance;
  using std::next;
  using std::prev;
  namespace ranges {
    using std::ranges::advance;
    using std::ranges::distance;
    using std::ranges::next;
    using std::ranges::prev;
  }
  using std::reverse_iterator;
  using std::operator==;
  using std::operator!=;
  using std::operator<;
  using std::operator>;
  using std::operator<=;
  using std::operator>=;
  using std::operator<=>;
  using std::operator-;
  using std::operator+;
  using std::make_reverse_iterator;
  using std::back_insert_iterator;
  using std::back_inserter;
  using std::front_insert_iterator;
  using std::front_inserter;
  using std::insert_iterator;
  using std::inserter;
  using std::move_iterator;
  using std::make_move_iterator;
  using std::move_sentinel;
  using std::common_iterator;
  using std::default_sentinel;
  using std::default_sentinel_t;
  using std::counted_iterator;
  using std::unreachable_sentinel;
  using std::unreachable_sentinel_t;
  using std::istream_iterator;
  using std::ostream_iterator;
  using std::istreambuf_iterator;
  using std::ostreambuf_iterator;
  using std::begin;
  using std::cbegin;
  using std::cend;
  using std::crbegin;
  using std::crend;
  using std::end;
  using std::rbegin;
  using std::rend;
  using std::empty;
  using std::size;
  using std::ssize;
  using std::data;
  using std::iterator;
}


export namespace std {
  using std::size_t;
  using std::memchr;
  using std::memcmp;
  using std::memcpy;
  using std::memmove;
  using std::memset;
  using std::strcat;
  using std::strchr;
  using std::strcmp;
  using std::strcoll;
  using std::strcpy;
  using std::strcspn;
  using std::strerror;
  using std::strlen;
  using std::strncat;
  using std::strncmp;
  using std::strncpy;
  using std::strpbrk;
  using std::strrchr;
  using std::strspn;
  using std::strstr;
  using std::strtok;
  using std::strxfrm;
}
// Workaround to ease the use.
export {
  using ::size_t;
  using ::memchr;
  using ::memcmp;
  using ::memcpy;
  using ::memmove;
  using ::memset;
  using ::strcat;
  using ::strchr;
  using ::strcmp;
  using ::strcoll;
  using ::strcpy;
  using ::strcspn;
  using ::strerror;
  using ::strlen;
  using ::strncat;
  using ::strncmp;
  using ::strncpy;
  using ::strpbrk;
  using ::strrchr;
  using ::strspn;
  using ::strstr;
  using ::strtok;
  using ::strxfrm;
}


export namespace std {
  using std::FILE;
  using std::fpos_t;
  using std::size_t;
  using std::clearerr;
  using std::fclose;
  using std::feof;
  using std::ferror;
  using std::fflush;
  using std::fgetc;
  using std::fgetpos;
  using std::fgets;
  using std::fopen;
  using std::fprintf;
  using std::fputc;
  using std::fputs;
  using std::fread;
  using std::freopen;
  using std::fscanf;
  using std::fseek;
  using std::fsetpos;
  using std::ftell;
  using std::fwrite;
  using std::getc;
  using std::getchar;
  using std::perror;
  using std::printf;
  using std::putc;
  using std::putchar;
  using std::puts;
  using std::remove;
  using std::rename;
  using std::rewind;
  using std::scanf;
  using std::setbuf;
  using std::setvbuf;
  using std::snprintf;
  using std::sprintf;
  using std::sscanf;
  using std::tmpfile;
  using std::tmpnam;
  using std::ungetc;
  using std::vfprintf;
  using std::vfscanf;
  using std::vprintf;
  using std::vscanf;
  using std::vsnprintf;
  using std::vsprintf;
  using std::vsscanf;
}


export namespace std {
  using std::variant;
  using std::variant_alternative;
  using std::variant_npos;
  using std::variant_size;
  using std::variant_size_v;
  using std::get;
  using std::get_if;
  using std::holds_alternative;
  using std::variant_alternative_t;
  using std::operator==;
  using std::operator!=;
  using std::operator<;
  using std::operator>;
  using std::operator<=;
  using std::operator>=;
  using std::operator<=>;
  using std::visit;
  using std::monostate;
  using std::swap;
  using std::bad_variant_access;
  using std::hash;
}


export namespace std {
  using std::invoke;
  using std::reference_wrapper;
  using std::cref;
  using std::ref;
  using std::divides;
  using std::minus;
  using std::modulus;
  using std::multiplies;
  using std::negate;
  using std::plus;
  using std::equal_to;
  using std::greater;
  using std::greater_equal;
  using std::less;
  using std::less_equal;
  using std::not_equal_to;
  using std::compare_three_way;
  using std::logical_and;
  using std::logical_not;
  using std::logical_or;
  using std::bit_and;
  using std::bit_not;
  using std::bit_or;
  using std::bit_xor;
  using std::identity;
  using std::not_fn;
  using std::bind_front;
  using std::is_bind_expression;
  using std::is_bind_expression_v;
  using std::is_placeholder;
  using std::is_placeholder_v;
  using std::bind;
  namespace placeholders {
    using std::placeholders::_1;
    using std::placeholders::_10;
    using std::placeholders::_2;
    using std::placeholders::_3;
    using std::placeholders::_4;
    using std::placeholders::_5;
    using std::placeholders::_6;
    using std::placeholders::_7;
    using std::placeholders::_8;
    using std::placeholders::_9;
  }
  using std::mem_fn;
  using std::bad_function_call;
  using std::function;
  using std::swap;
  using std::operator==;
  using std::default_searcher;
  using std::boyer_moore_searcher;
  using std::boyer_moore_horspool_searcher;
  using std::hash;
  namespace ranges {
    using std::ranges::equal_to;
    using std::ranges::greater;
    using std::ranges::greater_equal;
    using std::ranges::less;
    using std::ranges::less_equal;
    using std::ranges::not_equal_to;
  }
}


export namespace std {
  using std::pointer_traits;
  using std::to_address;
  using std::align;
  using std::assume_aligned;
  using std::allocator_arg;
  using std::allocator_arg_t;
  using std::uses_allocator;
  using std::uses_allocator_v;
  using std::uses_allocator_construction_args;
  using std::make_obj_using_allocator;
  using std::uninitialized_construct_using_allocator;
  using std::allocator_traits;
  using std::allocator;
  using std::operator==;
  using std::addressof;
  using std::uninitialized_default_construct;
  using std::uninitialized_default_construct_n;
  namespace ranges {
    using std::ranges::uninitialized_default_construct;
    using std::ranges::uninitialized_default_construct_n;
  }
  using std::uninitialized_value_construct;
  using std::uninitialized_value_construct_n;
  namespace ranges {
    using std::ranges::uninitialized_value_construct;
    using std::ranges::uninitialized_value_construct_n;
  }
  using std::uninitialized_copy;
  using std::uninitialized_copy_n;
  namespace ranges {
    using std::ranges::uninitialized_copy;
    using std::ranges::uninitialized_copy_result;
    using std::ranges::uninitialized_copy_n;
    using std::ranges::uninitialized_copy_n_result;
  }
  using std::uninitialized_move;
  using std::uninitialized_move_n;
  namespace ranges {
    using std::ranges::uninitialized_move;
    using std::ranges::uninitialized_move_result;
    using std::ranges::uninitialized_move_n;
    using std::ranges::uninitialized_move_n_result;
  }
  using std::uninitialized_fill;
  using std::uninitialized_fill_n;
  namespace ranges {
    using std::ranges::uninitialized_fill;
    using std::ranges::uninitialized_fill_n;
  }
  using std::construct_at;
  namespace ranges {
    using std::ranges::construct_at;
  }
  using std::destroy;
  using std::destroy_at;
  using std::destroy_n;
  namespace ranges {
    using std::ranges::destroy;
    using std::ranges::destroy_at;
    using std::ranges::destroy_n;
  }
  using std::default_delete;
  using std::unique_ptr;
  using std::make_unique;
  using std::make_unique_for_overwrite;
  using std::operator<;
  using std::operator>;
  using std::operator<=;
  using std::operator>=;
  using std::operator<=>;
  using std::operator<<;
  using std::bad_weak_ptr;
  using std::shared_ptr;
  using std::allocate_shared;
  using std::allocate_shared_for_overwrite;
  using std::make_shared;
  using std::make_shared_for_overwrite;
  using std::swap;
  using std::const_pointer_cast;
  using std::dynamic_pointer_cast;
  using std::reinterpret_pointer_cast;
  using std::static_pointer_cast;
  using std::get_deleter;
  using std::weak_ptr;
  using std::owner_less;
  using std::enable_shared_from_this;
  using std::hash;
  using std::atomic_is_lock_free;
  using std::atomic_load;
  using std::atomic_load_explicit;
  using std::atomic_store;
  using std::atomic_store_explicit;
  using std::atomic_exchange;
  using std::atomic_exchange_explicit;
  using std::atomic_compare_exchange_strong;
  using std::atomic_compare_exchange_strong_explicit;
  using std::atomic_compare_exchange_weak;
  using std::atomic_compare_exchange_weak_explicit;
}


export namespace std {
  using std::cerr;
  using std::cin;
  using std::clog;
  using std::cout;
  using std::wcerr;
  using std::wcin;
  using std::wclog;
  using std::wcout;
}

export namespace std {
  using std::ifstream;
}

export namespace std {
  namespace ranges {
    using std::ranges::in_found_result;
    using std::ranges::in_fun_result;
    using std::ranges::in_in_out_result;
    using std::ranges::in_in_result;
    using std::ranges::in_out_out_result;
    using std::ranges::in_out_result;
    using std::ranges::min_max_result;
  }
  using std::all_of;
  namespace ranges {
    using std::ranges::all_of;
  }
  using std::any_of;
  namespace ranges {
    using std::ranges::any_of;
  }
  using std::none_of;
  namespace ranges {
    using std::ranges::none_of;
  }
  using std::for_each;
  namespace ranges {
    using std::ranges::for_each;
    using std::ranges::for_each_result;
  }
  using std::for_each_n;
  namespace ranges {
    using std::ranges::for_each_n_result;
    using std::ranges::for_each_n;
  }
  using std::find;
  using std::find_if;
  using std::find_if_not;
  namespace ranges {
    using std::ranges::find;
    using std::ranges::find_if;
    using std::ranges::find_if_not;
  }
  namespace ranges {
  }
  using std::find_end;
  namespace ranges {
    using std::ranges::find_end;
  }
  using std::find_first_of;
  namespace ranges {
    using std::ranges::find_first_of;
  }
  using std::adjacent_find;
  namespace ranges {
    using std::ranges::adjacent_find;
  }
  using std::count;
  using std::count_if;
  namespace ranges {
    using std::ranges::count;
    using std::ranges::count_if;
  }
  using std::mismatch;
  namespace ranges {
    using std::ranges::mismatch_result;
    using std::ranges::mismatch;
  }
  using std::equal;
  namespace ranges {
    using std::ranges::equal;
  }
  using std::is_permutation;
  namespace ranges {
    using std::ranges::is_permutation;
  }
  using std::search;
  namespace ranges {
    using std::ranges::search;
  }
  using std::search_n;
  namespace ranges {
    using std::ranges::search_n;
  }
  namespace ranges {
  }
  using std::copy;
  namespace ranges {
    using std::ranges::copy;
    using std::ranges::copy_result;
  }
  using std::copy_n;
  namespace ranges {
    using std::ranges::copy_n;
    using std::ranges::copy_n_result;
  }
  using std::copy_if;
  namespace ranges {
    using std::ranges::copy_if;
    using std::ranges::copy_if_result;
  }
  using std::copy_backward;
  namespace ranges {
    using std::ranges::copy_backward;
    using std::ranges::copy_backward_result;
  }
  using std::move;
  namespace ranges {
    using std::ranges::move;
    using std::ranges::move_result;
  }
  using std::move_backward;
  namespace ranges {
    using std::ranges::move_backward;
    using std::ranges::move_backward_result;
  }
  using std::swap_ranges;
  namespace ranges {
    using std::ranges::swap_ranges;
    using std::ranges::swap_ranges_result;
  }
  using std::iter_swap;
  using std::transform;
  namespace ranges {
    using std::ranges::binary_transform_result;
    using std::ranges::unary_transform_result;
    using std::ranges::transform;
  }
  using std::replace;
  using std::replace_if;
  namespace ranges {
    using std::ranges::replace;
    using std::ranges::replace_if;
  }
  using std::replace_copy;
  using std::replace_copy_if;
  namespace ranges {
    using std::ranges::replace_copy;
    using std::ranges::replace_copy_if;
    using std::ranges::replace_copy_if_result;
    using std::ranges::replace_copy_result;
  }
  using std::fill;
  using std::fill_n;
  namespace ranges {
    using std::ranges::fill;
    using std::ranges::fill_n;
  }
  using std::generate;
  using std::generate_n;
  namespace ranges {
    using std::ranges::generate;
    using std::ranges::generate_n;
  }
  using std::remove;
  using std::remove_if;
  namespace ranges {
    using std::ranges::remove;
    using std::ranges::remove_if;
  }
  using std::remove_copy;
  using std::remove_copy_if;
  namespace ranges {
    using std::ranges::remove_copy;
    using std::ranges::remove_copy_if;
    using std::ranges::remove_copy_if_result;
    using std::ranges::remove_copy_result;
  }
  using std::unique;
  namespace ranges {
    using std::ranges::unique;
  }
  using std::unique_copy;
  namespace ranges {
    using std::ranges::unique_copy;
    using std::ranges::unique_copy_result;
  }
  using std::reverse;
  namespace ranges {
    using std::ranges::reverse;
  }
  using std::reverse_copy;
  namespace ranges {
    using std::ranges::reverse_copy;
    using std::ranges::reverse_copy_result;
  }
  using std::rotate;
  namespace ranges {
    using std::ranges::rotate;
  }
  using std::rotate_copy;
  namespace ranges {
    using std::ranges::rotate_copy;
    using std::ranges::rotate_copy_result;
  }
  using std::sample;
  namespace ranges {
    using std::ranges::sample;
  }
  using std::shuffle;
  namespace ranges {
    using std::ranges::shuffle;
  }
  using std::shift_left;
  namespace ranges {
  }
  using std::shift_right;
  namespace ranges {
  }
  using std::sort;
  namespace ranges {
    using std::ranges::sort;
  }
  using std::stable_sort;
  namespace ranges {
    using std::ranges::stable_sort;
  }
  using std::partial_sort;
  namespace ranges {
    using std::ranges::partial_sort;
  }
  using std::partial_sort_copy;
  namespace ranges {
    using std::ranges::partial_sort_copy;
    using std::ranges::partial_sort_copy_result;
  }
  using std::is_sorted;
  using std::is_sorted_until;
  namespace ranges {
    using std::ranges::is_sorted;
    using std::ranges::is_sorted_until;
  }
  using std::nth_element;
  namespace ranges {
    using std::ranges::nth_element;
  }
  using std::lower_bound;
  namespace ranges {
    using std::ranges::lower_bound;
  }
  using std::upper_bound;
  namespace ranges {
    using std::ranges::upper_bound;
  }
  using std::equal_range;
  namespace ranges {
    using std::ranges::equal_range;
  }
  using std::binary_search;
  namespace ranges {
    using std::ranges::binary_search;
  }
  using std::is_partitioned;
  namespace ranges {
    using std::ranges::is_partitioned;
  }
  using std::partition;
  namespace ranges {
    using std::ranges::partition;
  }
  using std::stable_partition;
  namespace ranges {
    using std::ranges::stable_partition;
  }
  using std::partition_copy;
  namespace ranges {
    using std::ranges::partition_copy;
    using std::ranges::partition_copy_result;
  }
  using std::partition_point;
  namespace ranges {
    using std::ranges::partition_point;
  }
  using std::merge;
  namespace ranges {
    using std::ranges::merge;
    using std::ranges::merge_result;
  }
  using std::inplace_merge;
  namespace ranges {
    using std::ranges::inplace_merge;
  }
  using std::includes;
  namespace ranges {
    using std::ranges::includes;
  }
  using std::set_union;
  namespace ranges {
    using std::ranges::set_union;
    using std::ranges::set_union_result;
  }
  using std::set_intersection;
  namespace ranges {
    using std::ranges::set_intersection;
    using std::ranges::set_intersection_result;
  }
  using std::set_difference;
  namespace ranges {
    using std::ranges::set_difference;
    using std::ranges::set_difference_result;
  }
  using std::set_symmetric_difference;
  namespace ranges {
    using std::ranges::set_symmetric_difference_result;
    using std::ranges::set_symmetric_difference;
  }
  using std::push_heap;
  namespace ranges {
    using std::ranges::push_heap;
  }
  using std::pop_heap;
  namespace ranges {
    using std::ranges::pop_heap;
  }
  using std::make_heap;
  namespace ranges {
    using std::ranges::make_heap;
  }
  using std::sort_heap;
  namespace ranges {
    using std::ranges::sort_heap;
  }
  using std::is_heap;
  namespace ranges {
    using std::ranges::is_heap;
  }
  using std::is_heap_until;
  namespace ranges {
    using std::ranges::is_heap_until;
  }
  using std::min;
  namespace ranges {
    using std::ranges::min;
  }
  using std::max;
  namespace ranges {
    using std::ranges::max;
  }
  using std::minmax;
  namespace ranges {
    using std::ranges::minmax_result;
    using std::ranges::minmax;
  }
  using std::min_element;
  namespace ranges {
    using std::ranges::min_element;
  }
  using std::max_element;
  namespace ranges {
    using std::ranges::max_element;
  }
  using std::minmax_element;
  namespace ranges {
    using std::ranges::minmax_element_result;
    using std::ranges::minmax_element;
  }
  using std::clamp;
  namespace ranges {
    using std::ranges::clamp;
  }
  using std::lexicographical_compare;
  namespace ranges {
    using std::ranges::lexicographical_compare;
  }
  using std::lexicographical_compare_three_way;
  using std::next_permutation;
  namespace ranges {
    using std::ranges::next_permutation_result;
    using std::ranges::next_permutation;
  }
  using std::prev_permutation;
  namespace ranges {
    using std::ranges::prev_permutation_result;
    using std::ranges::prev_permutation;
  }
}


export namespace std {
  using std::mutex;
  using std::recursive_mutex;
  using std::timed_mutex;
  using std::recursive_timed_mutex;
  using std::adopt_lock_t;
  using std::defer_lock_t;
  using std::try_to_lock_t;
  using std::adopt_lock;
  using std::defer_lock;
  using std::try_to_lock;
  using std::lock_guard;
  using std::scoped_lock;
  using std::unique_lock;
  using std::swap;
  using std::lock;
  using std::try_lock;
  using std::once_flag;
  using std::call_once;
}


export namespace std {
  using std::condition_variable;
  using std::condition_variable_any;
  using std::notify_all_at_thread_exit;
  using std::cv_status;
}


export namespace std {
  using std::memory_order;
  using std::memory_order_acq_rel;
  using std::memory_order_acquire;
  using std::memory_order_consume;
  using std::memory_order_relaxed;
  using std::memory_order_release;
  using std::memory_order_seq_cst;
  using std::kill_dependency;
  using std::atomic_ref;
  using std::atomic;
  using std::atomic_compare_exchange_strong;
  using std::atomic_compare_exchange_strong_explicit;
  using std::atomic_compare_exchange_weak;
  using std::atomic_compare_exchange_weak_explicit;
  using std::atomic_exchange;
  using std::atomic_exchange_explicit;
  using std::atomic_is_lock_free;
  using std::atomic_load;
  using std::atomic_load_explicit;
  using std::atomic_store;
  using std::atomic_store_explicit;
  using std::atomic_fetch_add;
  using std::atomic_fetch_add_explicit;
  using std::atomic_fetch_and;
  using std::atomic_fetch_and_explicit;
  using std::atomic_fetch_or;
  using std::atomic_fetch_or_explicit;
  using std::atomic_fetch_sub;
  using std::atomic_fetch_sub_explicit;
  using std::atomic_fetch_xor;
  using std::atomic_fetch_xor_explicit;
  using std::atomic_notify_all;
  using std::atomic_notify_one;
  using std::atomic_wait;
  using std::atomic_wait_explicit;
  using std::atomic_bool;
  using std::atomic_char;
  using std::atomic_char16_t;
  using std::atomic_char32_t;
  using std::atomic_char8_t;
  using std::atomic_int;
  using std::atomic_llong;
  using std::atomic_long;
  using std::atomic_schar;
  using std::atomic_short;
  using std::atomic_uchar;
  using std::atomic_uint;
  using std::atomic_ullong;
  using std::atomic_ulong;
  using std::atomic_ushort;
  using std::atomic_wchar_t;
  using std::atomic_int16_t;
  using std::atomic_int32_t;
  using std::atomic_int64_t;
  using std::atomic_int8_t;
  using std::atomic_uint16_t;
  using std::atomic_uint32_t;
  using std::atomic_uint64_t;
  using std::atomic_uint8_t;
  using std::atomic_int_least16_t;
  using std::atomic_int_least32_t;
  using std::atomic_int_least64_t;
  using std::atomic_int_least8_t;
  using std::atomic_uint_least16_t;
  using std::atomic_uint_least32_t;
  using std::atomic_uint_least64_t;
  using std::atomic_uint_least8_t;
  using std::atomic_int_fast16_t;
  using std::atomic_int_fast32_t;
  using std::atomic_int_fast64_t;
  using std::atomic_int_fast8_t;
  using std::atomic_uint_fast16_t;
  using std::atomic_uint_fast32_t;
  using std::atomic_uint_fast64_t;
  using std::atomic_uint_fast8_t;
  using std::atomic_intmax_t;
  using std::atomic_intptr_t;
  using std::atomic_ptrdiff_t;
  using std::atomic_size_t;
  using std::atomic_uintmax_t;
  using std::atomic_uintptr_t;
  using std::atomic_signed_lock_free;
  using std::atomic_unsigned_lock_free;
  using std::atomic_flag;
  using std::atomic_flag_clear;
  using std::atomic_flag_clear_explicit;
  using std::atomic_flag_test;
  using std::atomic_flag_test_and_set;
  using std::atomic_flag_test_and_set_explicit;
  using std::atomic_flag_test_explicit;
  using std::atomic_flag_notify_all;
  using std::atomic_flag_notify_one;
  using std::atomic_flag_wait;
  using std::atomic_flag_wait_explicit;
  using std::atomic_signal_fence;
  using std::atomic_thread_fence;
  using std::atomic_init;
}


export namespace std {
  using std::coroutine_traits;
  using std::coroutine_handle;
  using std::operator==;
  using std::operator<=>;
  using std::hash;
  using std::noop_coroutine;
  using std::noop_coroutine_handle;
  using std::noop_coroutine_promise;
  using std::suspend_always;
  using std::suspend_never;
}


export namespace std {
  using std::max_align_t;
  using std::nullptr_t;
  using std::ptrdiff_t;
  using std::size_t;
  using std::byte;
  using std::operator<<=;
  using std::operator<<;
  using std::operator>>=;
  using std::operator>>;
  using std::operator|=;
  using std::operator|;
  using std::operator&=;
  using std::operator&;
  using std::operator^=;
  using std::operator^;
  using std::operator~;
  using std::to_integer;
}
// Ease the use
export {
  using ::max_align_t;
  using ::ptrdiff_t;
  using ::size_t;
}


export namespace std {
  using std::bad_exception;
  using std::current_exception;
  using std::exception;
  using std::exception_ptr;
  using std::get_terminate;
  using std::make_exception_ptr;
  using std::nested_exception;
  using std::rethrow_exception;
  using std::rethrow_if_nested;
  using std::set_terminate;
  using std::terminate;
  using std::terminate_handler;
  using std::throw_with_nested;
  using std::uncaught_exception;
  using std::uncaught_exceptions;
}


export namespace std {
  using std::swap;
  using std::exchange;
  using std::forward;
  using std::move;
  using std::move_if_noexcept;
  using std::as_const;
  using std::declval;
  using std::cmp_equal;
  using std::cmp_not_equal;
  using std::cmp_greater;
  using std::cmp_greater_equal;
  using std::cmp_less;
  using std::cmp_less_equal;
  using std::in_range;
  using std::index_sequence;
  using std::integer_sequence;
  using std::make_index_sequence;
  using std::make_integer_sequence;
  using std::index_sequence_for;
  using std::pair;
  using std::operator==;
  using std::operator<=>;
  using std::make_pair;
  using std::tuple_element;
  using std::tuple_size;
  using std::get;
  using std::piecewise_construct;
  using std::piecewise_construct_t;
  using std::in_place;
  using std::in_place_t;
  using std::in_place_type;
  using std::in_place_type_t;
  using std::in_place_index;
  using std::in_place_index_t;
  namespace rel_ops {
    using rel_ops::operator!=;
    using rel_ops::operator>;
    using rel_ops::operator<=;
    using rel_ops::operator>=;
  }
}


export namespace std {
  namespace ranges {
    using std::ranges::range;
    using std::ranges::begin;
    using std::ranges::end;
    using std::ranges::cbegin;
    using std::ranges::cend;
    using std::ranges::rbegin;
    using std::ranges::rend;
    using std::ranges::crbegin;
    using std::ranges::crend;
    using std::ranges::size;
    using std::ranges::ssize;
    using std::ranges::empty;
    using std::ranges::data;
    using std::ranges::cdata;
    using std::ranges::enable_borrowed_range;
    using std::ranges::borrowed_range;
    using std::ranges::iterator_t;
    using std::ranges::range_common_reference_t;
    using std::ranges::range_difference_t;
    using std::ranges::range_reference_t;
    using std::ranges::range_rvalue_reference_t;
    using std::ranges::range_size_t;
    using std::ranges::range_value_t;
    using std::ranges::sentinel_t;
    using std::ranges::disable_sized_range;
    using std::ranges::sized_range;
    using std::ranges::enable_view;
    using std::ranges::view;
    using std::ranges::view_base;
    using std::ranges::bidirectional_range;
    using std::ranges::common_range;
    using std::ranges::contiguous_range;
    using std::ranges::forward_range;
    using std::ranges::input_range;
    using std::ranges::output_range;
    using std::ranges::random_access_range;
    using std::ranges::viewable_range;
    using std::ranges::view_interface;
    using std::ranges::subrange;
    using std::ranges::subrange_kind;
    using std::ranges::get;
  }
  using std::ranges::get;
  namespace ranges {
    using std::ranges::dangling;
    using std::ranges::borrowed_iterator_t;
    using std::ranges::borrowed_subrange_t;
    using std::ranges::empty_view;
    namespace views {
      using std::ranges::views::empty;
    }
    using std::ranges::single_view;
    namespace views {
      using std::ranges::views::single;
    }
    using std::ranges::iota_view;
    namespace views {
      using std::ranges::views::iota;
    }
    using std::ranges::basic_istream_view;
    using std::ranges::istream_view;
    using std::ranges::wistream_view;
    namespace views {
      using std::ranges::views::istream;
    }
    namespace views {
      using std::ranges::views::all;
      using std::ranges::views::all_t;
    }
    using std::ranges::ref_view;
    using std::ranges::owning_view;
    using std::ranges::filter_view;
    namespace views {
      using std::ranges::views::filter;
    }
    using std::ranges::transform_view;
    namespace views {
      using std::ranges::views::transform;
    }
    using std::ranges::take_view;
    namespace views {
      using std::ranges::views::take;
    }
    using std::ranges::take_while_view;
    namespace views {
      using std::ranges::views::take_while;
    }
    using std::ranges::drop_view;
    namespace views {
      using std::ranges::views::drop;
    }
    using std::ranges::drop_while_view;
    namespace views {
      using std::ranges::views::drop_while;
    }
    using std::ranges::join_view;
    namespace views {
      using std::ranges::views::join;
    }
    using std::ranges::lazy_split_view;
    using std::ranges::split_view;
    namespace views {
      using std::ranges::views::lazy_split;
      using std::ranges::views::split;
    }
    namespace views {
      using std::ranges::views::counted;
    }
    using std::ranges::common_view;
    namespace views {
      using std::ranges::views::common;
    }
    using std::ranges::reverse_view;
    namespace views {
      using std::ranges::views::reverse;
    }
    using std::ranges::elements_view;
    using std::ranges::keys_view;
    using std::ranges::values_view;
    namespace views {
      using std::ranges::views::elements;
      using std::ranges::views::keys;
      using std::ranges::views::values;
    }
  }
  namespace views = ranges::views;
  using std::tuple_element;
  using std::tuple_size;
}


export namespace std {
  using std::ratio;
  using std::ratio_add;
  using std::ratio_divide;
  using std::ratio_multiply;
  using std::ratio_subtract;
  using std::ratio_equal;
  using std::ratio_greater;
  using std::ratio_greater_equal;
  using std::ratio_less;
  using std::ratio_less_equal;
  using std::ratio_not_equal;
  using std::ratio_equal_v;
  using std::ratio_greater_equal_v;
  using std::ratio_greater_v;
  using std::ratio_less_equal_v;
  using std::ratio_less_v;
  using std::ratio_not_equal_v;
  using std::atto;
  using std::centi;
  using std::deca;
  using std::deci;
  using std::exa;
  using std::femto;
  using std::giga;
  using std::hecto;
  using std::kilo;
  using std::mega;
  using std::micro;
  using std::milli;
  using std::nano;
  using std::peta;
  using std::pico;
  using std::tera;
}


export namespace std {
  using std::float_denorm_style;
  using std::float_round_style;
  using std::numeric_limits;
}


export namespace std {
  using std::counting_semaphore;
  using std::binary_semaphore;
}


export namespace std {
  using std::domain_error;
  using std::invalid_argument;
  using std::length_error;
  using std::logic_error;
  using std::out_of_range;
  using std::overflow_error;
  using std::range_error;
  using std::runtime_error;
  using std::underflow_error;
}


export namespace std {
  using std::div_t;
  using std::ldiv_t;
  using std::lldiv_t;
  using std::size_t;
  using std::_Exit;
  using std::abort;
  using std::at_quick_exit;
  using std::atexit;
  using std::exit;
  using std::quick_exit;
  using std::getenv;
  using std::system;
  using std::aligned_alloc;
  using std::calloc;
  using std::free;
  using std::malloc;
  using std::realloc;
  using std::atof;
  using std::atoi;
  using std::atol;
  using std::atoll;
  using std::strtod;
  using std::strtof;
  using std::strtol;
  using std::strtold;
  using std::strtoll;
  using std::strtoul;
  using std::strtoull;
  using std::mblen;
  using std::mbstowcs;
  using std::mbtowc;
  using std::wcstombs;
  using std::wctomb;
  using std::bsearch;
  using std::qsort;
  using std::rand;
  using std::srand;
  using std::abs;
  using std::labs;
  using std::llabs;
  using std::div;
  using std::ldiv;
  using std::lldiv;
}


export namespace std {
  namespace chrono {
    using std::chrono::duration;
    using std::chrono::time_point;
  }
  using std::common_type;
  namespace chrono {
    using std::chrono::treat_as_floating_point;
    using std::chrono::treat_as_floating_point_v;
    using std::chrono::duration_values;
    using std::chrono::operator+;
    using std::chrono::operator-;
    using std::chrono::operator*;
    using std::chrono::operator/;
    using std::chrono::operator%;
    using std::chrono::operator==;
    using std::chrono::operator<;
    using std::chrono::operator>;
    using std::chrono::operator<=;
    using std::chrono::operator>=;
    using std::chrono::operator<=>;
    using std::chrono::ceil;
    using std::chrono::duration_cast;
    using std::chrono::floor;
    using std::chrono::round;
    using std::chrono::operator<<;
    using std::chrono::days;
    using std::chrono::hours;
    using std::chrono::microseconds;
    using std::chrono::milliseconds;
    using std::chrono::minutes;
    using std::chrono::months;
    using std::chrono::nanoseconds;
    using std::chrono::seconds;
    using std::chrono::weeks;
    using std::chrono::years;
    using std::chrono::time_point_cast;
    using std::chrono::abs;
    using std::chrono::system_clock;
    using std::chrono::sys_days;
    using std::chrono::sys_seconds;
    using std::chrono::sys_time;
    using std::chrono::file_clock;
    using std::chrono::file_time;
    using std::chrono::steady_clock;
    using std::chrono::high_resolution_clock;
    using std::chrono::local_days;
    using std::chrono::local_seconds;
    using std::chrono::local_t;
    using std::chrono::local_time;
    using std::chrono::last_spec;
    using std::chrono::day;
    using std::chrono::month;
    using std::chrono::year;
    using std::chrono::weekday;
    using std::chrono::weekday_indexed;
    using std::chrono::weekday_last;
    using std::chrono::month_day;
    using std::chrono::month_day_last;
    using std::chrono::month_weekday;
    using std::chrono::month_weekday_last;
    using std::chrono::year_month;
    using std::chrono::year_month_day;
    using std::chrono::year_month_day_last;
    using std::chrono::year_month_weekday;
    using std::chrono::year_month_weekday_last;
    using std::chrono::hh_mm_ss;
    using std::chrono::is_am;
    using std::chrono::is_pm;
    using std::chrono::make12;
    using std::chrono::make24;
  }
  using std::formatter;
  namespace chrono {
    using std::chrono::last;
    using std::chrono::Friday;
    using std::chrono::Monday;
    using std::chrono::Saturday;
    using std::chrono::Sunday;
    using std::chrono::Thursday;
    using std::chrono::Tuesday;
    using std::chrono::Wednesday;
    using std::chrono::April;
    using std::chrono::August;
    using std::chrono::December;
    using std::chrono::February;
    using std::chrono::January;
    using std::chrono::July;
    using std::chrono::June;
    using std::chrono::March;
    using std::chrono::May;
    using std::chrono::November;
    using std::chrono::October;
    using std::chrono::September;
  }
}
export namespace std::inline literals::inline chrono_literals {
  using std::literals::chrono_literals::operator""h;
  using std::literals::chrono_literals::operator""min;
  using std::literals::chrono_literals::operator""s;
  using std::literals::chrono_literals::operator""ms;
  using std::literals::chrono_literals::operator""us;
  using std::literals::chrono_literals::operator""ns;
  using std::literals::chrono_literals::operator""d;
  using std::literals::chrono_literals::operator""y;
}


export namespace std {
  using std::char_traits;
  using std::basic_string;
  using std::operator+;
  using std::operator==;
  using std::operator<=>;
  using std::swap;
  using std::operator>>;
  using std::operator<<;
  using std::getline;
  using std::erase;
  using std::erase_if;
  using std::string;
  using std::u16string;
  using std::u32string;
  using std::u8string;
  using std::wstring;
  using std::stod;
  using std::stof;
  using std::stoi;
  using std::stol;
  using std::stold;
  using std::stoll;
  using std::stoul;
  using std::stoull;
  using std::to_string;
  using std::to_wstring;
  namespace pmr {
    using std::pmr::basic_string;
    using std::pmr::string;
    using std::pmr::u16string;
    using std::pmr::u32string;
    using std::pmr::u8string;
    using std::pmr::wstring;
  }
  using std::hash;
  inline namespace literals {
    inline namespace string_literals {
      using std::literals::string_literals::operator""s;
    }
  }
}


export namespace std {
  using std::array;
  using std::operator==;
  using std::operator<=>;
  using std::swap;
  using std::to_array;
  using std::get;
  using std::tuple_element;
  using std::tuple_size;
}


export namespace std {
  using std::optional;
  using std::nullopt;
  using std::nullopt_t;
  using std::bad_optional_access;
  using std::operator==;
  using std::operator!=;
  using std::operator<;
  using std::operator>;
  using std::operator<=;
  using std::operator>=;
  using std::operator<=>;
  using std::swap;
  using std::make_optional;
  using std::hash;
}


export namespace std {
  using std::same_as;
  using std::derived_from;
  using std::convertible_to;
  using std::common_reference_with;
  using std::common_with;
  using std::floating_point;
  using std::integral;
  using std::signed_integral;
  using std::unsigned_integral;
  using std::assignable_from;
  using std::swappable;
  using std::swappable_with;
  using std::destructible;
  using std::constructible_from;
  using std::default_initializable;
  using std::move_constructible;
  using std::copy_constructible;
  using std::equality_comparable;
  using std::equality_comparable_with;
  using std::totally_ordered;
  using std::totally_ordered_with;
  using std::copyable;
  using std::movable;
  using std::regular;
  using std::semiregular;
  using std::invocable;
  using std::regular_invocable;
  using std::predicate;
  using std::relation;
  using std::equivalence_relation;
  using std::strict_weak_order;
}


export namespace std {
  using std::int8_t;
  using std::int16_t;
  using std::int32_t;
  using std::int64_t;
  using std::int_fast16_t;
  using std::int_fast32_t;
  using std::int_fast64_t;
  using std::int_fast8_t;
  using std::int_least16_t;
  using std::int_least32_t;
  using std::int_least64_t;
  using std::int_least8_t;
  using std::intmax_t;
  using std::intptr_t;
  using std::uint8_t;
  using std::uint16_t;
  using std::uint32_t;
  using std::uint64_t;
  using std::uint_fast16_t;
  using std::uint_fast32_t;
  using std::uint_fast64_t;
  using std::uint_fast8_t;
  using std::uint_least16_t;
  using std::uint_least32_t;
  using std::uint_least64_t;
  using std::uint_least8_t;
  using std::uintmax_t;
  using std::uintptr_t;
}
// FIXME: Ease the use.
export {
  using ::int8_t;
  using ::int16_t;
  using ::int32_t;
  using ::int64_t;
  using ::int_fast16_t;
  using ::int_fast32_t;
  using ::int_fast64_t;
  using ::int_fast8_t;
  using ::int_least16_t;
  using ::int_least32_t;
  using ::int_least64_t;
  using ::int_least8_t;
  using ::intmax_t;
  using ::intptr_t;
  using ::uint8_t;
  using ::uint16_t;
  using ::uint32_t;
  using ::uint64_t;
  using ::uint_fast16_t;
  using ::uint_fast32_t;
  using ::uint_fast64_t;
  using ::uint_fast8_t;
  using ::uint_least16_t;
  using ::uint_least32_t;
  using ::uint_least64_t;
  using ::uint_least8_t;
  using ::uintmax_t;
  using ::uintptr_t;
}


export namespace std {
  using std::queue;
  using std::operator==;
  using std::operator!=;
  using std::operator<;
  using std::operator>;
  using std::operator<=;
  using std::operator>=;
  using std::operator<=>;
  using std::swap;
  using std::uses_allocator;
  using std::priority_queue;
}


export namespace std {
  using std::bad_alloc;
  using std::bad_array_new_length;
  using std::destroying_delete;
  using std::destroying_delete_t;
  using std::align_val_t;
  using std::nothrow;
  using std::nothrow_t;
  using std::get_new_handler;
  using std::new_handler;
  using std::set_new_handler;
  using std::launder;
}
export {
  using ::operator new;
  using ::operator delete;
  using ::operator new[];
  using ::operator delete[];
}


export namespace std {
  using std::tuple;
  using std::ignore;
  using std::forward_as_tuple;
  using std::make_tuple;
  using std::tie;
  using std::tuple_cat;
  using std::apply;
  using std::make_from_tuple;
  using std::tuple_element;
  using std::tuple_size;
  using std::get;
  using std::tuple_element_t;
  using std::operator==;
  using std::operator<=>;
  using std::uses_allocator;
  using std::swap;
  using std::tuple_size_v;
}


export namespace std {
  using std::integral_constant;
  using std::bool_constant;
  using std::false_type;
  using std::true_type;
  using std::is_array;
  using std::is_class;
  using std::is_enum;
  using std::is_floating_point;
  using std::is_function;
  using std::is_integral;
  using std::is_lvalue_reference;
  using std::is_member_function_pointer;
  using std::is_member_object_pointer;
  using std::is_null_pointer;
  using std::is_pointer;
  using std::is_rvalue_reference;
  using std::is_union;
  using std::is_void;
  using std::is_arithmetic;
  using std::is_compound;
  using std::is_fundamental;
  using std::is_member_pointer;
  using std::is_object;
  using std::is_reference;
  using std::is_scalar;
  using std::is_abstract;
  using std::is_aggregate;
  using std::is_const;
  using std::is_empty;
  using std::is_final;
  using std::is_polymorphic;
  using std::is_standard_layout;
  using std::is_trivial;
  using std::is_trivially_copyable;
  using std::is_volatile;
  using std::is_bounded_array;
  using std::is_signed;
  using std::is_unbounded_array;
  using std::is_unsigned;
  using std::is_constructible;
  using std::is_copy_constructible;
  using std::is_default_constructible;
  using std::is_move_constructible;
  using std::is_assignable;
  using std::is_copy_assignable;
  using std::is_move_assignable;
  using std::is_swappable;
  using std::is_swappable_with;
  using std::is_destructible;
  using std::is_trivially_constructible;
  using std::is_trivially_copy_constructible;
  using std::is_trivially_default_constructible;
  using std::is_trivially_move_constructible;
  using std::is_trivially_assignable;
  using std::is_trivially_copy_assignable;
  using std::is_trivially_destructible;
  using std::is_trivially_move_assignable;
  using std::is_nothrow_constructible;
  using std::is_nothrow_copy_constructible;
  using std::is_nothrow_default_constructible;
  using std::is_nothrow_move_constructible;
  using std::is_nothrow_assignable;
  using std::is_nothrow_copy_assignable;
  using std::is_nothrow_move_assignable;
  using std::is_nothrow_swappable;
  using std::is_nothrow_swappable_with;
  using std::is_nothrow_destructible;
  using std::has_virtual_destructor;
  using std::has_unique_object_representations;
  using std::alignment_of;
  using std::extent;
  using std::rank;
  using std::is_base_of;
  using std::is_convertible;
  using std::is_nothrow_convertible;
  using std::is_same;
  using std::is_invocable;
  using std::is_invocable_r;
  using std::is_nothrow_invocable;
  using std::is_nothrow_invocable_r;
  using std::add_const;
  using std::add_cv;
  using std::add_volatile;
  using std::remove_const;
  using std::remove_cv;
  using std::remove_volatile;
  using std::add_const_t;
  using std::add_cv_t;
  using std::add_volatile_t;
  using std::remove_const_t;
  using std::remove_cv_t;
  using std::remove_volatile_t;
  using std::add_lvalue_reference;
  using std::add_rvalue_reference;
  using std::remove_reference;
  using std::add_lvalue_reference_t;
  using std::add_rvalue_reference_t;
  using std::remove_reference_t;
  using std::make_signed;
  using std::make_unsigned;
  using std::make_signed_t;
  using std::make_unsigned_t;
  using std::remove_all_extents;
  using std::remove_extent;
  using std::remove_all_extents_t;
  using std::remove_extent_t;
  using std::add_pointer;
  using std::remove_pointer;
  using std::add_pointer_t;
  using std::remove_pointer_t;
  using std::basic_common_reference;
  using std::common_reference;
  using std::common_type;
  using std::conditional;
  using std::decay;
  using std::enable_if;
  using std::invoke_result;
  using std::remove_cvref;
  using std::type_identity;
  using std::underlying_type;
  using std::unwrap_ref_decay;
  using std::unwrap_reference;
  using std::common_reference_t;
  using std::common_type_t;
  using std::conditional_t;
  using std::decay_t;
  using std::enable_if_t;
  using std::invoke_result_t;
  using std::remove_cvref_t;
  using std::type_identity_t;
  using std::underlying_type_t;
  using std::unwrap_ref_decay_t;
  using std::unwrap_reference_t;
  using std::void_t;
  using std::conjunction;
  using std::disjunction;
  using std::negation;
  using std::is_array_v;
  using std::is_class_v;
  using std::is_enum_v;
  using std::is_floating_point_v;
  using std::is_function_v;
  using std::is_integral_v;
  using std::is_lvalue_reference_v;
  using std::is_member_function_pointer_v;
  using std::is_member_object_pointer_v;
  using std::is_null_pointer_v;
  using std::is_pointer_v;
  using std::is_rvalue_reference_v;
  using std::is_union_v;
  using std::is_void_v;
  using std::is_arithmetic_v;
  using std::is_compound_v;
  using std::is_fundamental_v;
  using std::is_member_pointer_v;
  using std::is_object_v;
  using std::is_reference_v;
  using std::is_scalar_v;
  using std::has_unique_object_representations_v;
  using std::has_virtual_destructor_v;
  using std::is_abstract_v;
  using std::is_aggregate_v;
  using std::is_assignable_v;
  using std::is_bounded_array_v;
  using std::is_const_v;
  using std::is_constructible_v;
  using std::is_copy_assignable_v;
  using std::is_copy_constructible_v;
  using std::is_default_constructible_v;
  using std::is_destructible_v;
  using std::is_empty_v;
  using std::is_final_v;
  using std::is_move_assignable_v;
  using std::is_move_constructible_v;
  using std::is_nothrow_assignable_v;
  using std::is_nothrow_constructible_v;
  using std::is_nothrow_copy_assignable_v;
  using std::is_nothrow_copy_constructible_v;
  using std::is_nothrow_default_constructible_v;
  using std::is_nothrow_destructible_v;
  using std::is_nothrow_move_assignable_v;
  using std::is_nothrow_move_constructible_v;
  using std::is_nothrow_swappable_v;
  using std::is_nothrow_swappable_with_v;
  using std::is_polymorphic_v;
  using std::is_signed_v;
  using std::is_standard_layout_v;
  using std::is_swappable_v;
  using std::is_swappable_with_v;
  using std::is_trivial_v;
  using std::is_trivially_assignable_v;
  using std::is_trivially_constructible_v;
  using std::is_trivially_copy_assignable_v;
  using std::is_trivially_copy_constructible_v;
  using std::is_trivially_copyable_v;
  using std::is_trivially_default_constructible_v;
  using std::is_trivially_destructible_v;
  using std::is_trivially_move_assignable_v;
  using std::is_trivially_move_constructible_v;
  using std::is_unbounded_array_v;
  using std::is_unsigned_v;
  using std::is_volatile_v;
  using std::alignment_of_v;
  using std::extent_v;
  using std::rank_v;
  using std::is_base_of_v;
  using std::is_convertible_v;
  using std::is_invocable_r_v;
  using std::is_invocable_v;
  using std::is_nothrow_convertible_v;
  using std::is_nothrow_invocable_r_v;
  using std::is_nothrow_invocable_v;
  using std::is_same_v;
  using std::conjunction_v;
  using std::disjunction_v;
  using std::negation_v;
  using std::is_constant_evaluated;
  using std::aligned_storage;
  using std::aligned_storage_t;
  using std::aligned_union;
  using std::aligned_union_t;
  using std::is_pod;
  using std::is_pod_v;
}


export namespace std {
  using std::thread;
  using std::swap;
  using std::jthread;
  namespace this_thread {
    using std::this_thread::get_id;
    using std::this_thread::sleep_for;
    using std::this_thread::sleep_until;
    using std::this_thread::yield;
  }
  using std::operator==;
  using std::operator<=>;
  using std::operator<<;
  using std::hash;
}

#if defined(__GLIBCXX__) || defined(__GLIBCPP__)
export namespace __gnu_cxx {
    using __gnu_cxx::operator==;
    using __gnu_cxx::operator-;
}
#endif
