module;
#include"asio.hpp"
export module asio;
export namespace asio
{

//Properties

    //Customisation Points

    // using asio::prefer;
    // using asio::query;
    // using asio::require;
    // using asio::require_concept;

    //Traits

    using asio::can_prefer;
    using asio::can_query;
    using asio::can_require;
    using asio::can_require_concept;
    using asio::is_nothrow_prefer;
    using asio::is_nothrow_query;
    using asio::is_nothrow_require;
    using asio::is_nothrow_require_concept;
    using asio::prefer_result;
    using asio::query_result;
    using asio::require_result;
    using asio::require_concept_result;

//Execution

    namespace execution
    {

        //Class Templates
        using execution::any_executor;

        //Classes

        using execution::bad_executor;
        using execution::invocable_archetype;
        using execution::receiver_invocation_error;

        //Properties
        
        using execution::allocator_t;
        using execution::blocking_t;
        using execution::blocking_adaptation_t;
        using execution::bulk_guarantee_t;
        using execution::context_t;
        using execution::context_as_t;
        using execution::mapping_t;
        using execution::occupancy_t;
        using execution::outstanding_work_t;
        using execution::prefer_only;
        using execution::relationship_t;

        //Customisation Points
        
        //using execution::bulk_execute;
        //using execution::connect;
        //using execution::execute;
        //using execution::schedule;
        //using execution::set_done;
        //using execution::set_error;
        //using execution::set_value;
        //using execution::start;
        //using execution::submit;

        //Property Objects

        using execution::allocator;
        using execution::blocking;
        using execution::blocking_adaptation;
        using execution::bulk_guarantee;
        using execution::context;
        using execution::context_as;
        using execution::mapping;
        using execution::occupancy;
        using execution::outstanding_work;
        using execution::relationship;

        //Type Traits

        using execution::can_bulk_execute;
        using execution::can_connect;
        using execution::can_execute;
        using execution::can_schedule;
        using execution::can_set_done;
        using execution::can_set_error;
        using execution::can_set_value;
        using execution::can_start;
        using execution::can_submit;
        using execution::connect_result;
        using execution::is_executor;
        using execution::is_executor_of;
        using execution::is_nothrow_receiver_of;
        using execution::is_receiver;
        using execution::is_receiver_of;
        using execution::is_sender;
        using execution::is_sender_to;
        using execution::is_typed_sender;
        using execution::sender_traits;
        


    }

    



//Core

    //Classes
    using asio::any_io_executor;
    using asio::bad_executor;
    using asio::cancellation_signal;
    using asio::cancellation_slot;
    using asio::cancellation_state;
    using asio::cancellation_type;
    using asio::coroutine;
    using asio::detached_t;
    using asio::error_code;
    using asio::execution_context;
    using asio::executor;
    using asio::executor_arg_t;
    using asio::invalid_service_owner;
    using asio::io_context;
    using asio::multiple_exceptions;
    using asio::service_already_exists;
    using asio::static_thread_pool;
    using asio::system_context;
    using asio::system_error;
    using asio::system_executor;
    using asio::this_coro::executor_t;
    using asio::thread;
    using asio::thread_pool;
    //using asio::yield_context;

    //Free Functions

    using asio::add_service;
    //experimental
    using asio::asio_handler_allocate;
    using asio::asio_handler_deallocate;
    using asio::asio_handler_invoke;
    using asio::asio_handler_is_continuation;
    using asio::async_compose;
    using asio::async_initiate;
    using asio::bind_cancellation_slot;
    using asio::bind_executor;
    //using asio::co_spawn;
    using asio::dispatch;
    using asio::defer;
    using asio::get_associated_allocator;
    using asio::get_associated_cancellation_slot;
    using asio::get_associated_executor;
    using asio::has_service;
    using asio::make_strand;
    using asio::make_work_guard;
    using asio::post;
    using asio::redirect_error;
    //using asio::spawn;
    using asio::this_coro::reset_cancellation_state;
    using asio::this_coro::throw_if_cancelled;
    using asio::use_service;

    //Class Templates

    using asio::async_completion;
    //using asio::awaitable;
    using asio::basic_io_object;
    using asio::basic_system_executor;
    //using asio::basic_yield_context;
    using asio::cancellation_filter;
    using asio::cancellation_slot_binder;
    //using asio::experimental::deferred_t;
    using asio::executor_binder;
    using asio::executor_work_guard;
    // using asio::experimental::append_t;
    // using asio::experimental::as_single_t;
    // using asio::experimental::as_tuple_t;
    // using asio::experimental::basic_channel;
    // using asio::experimental::basic_concurrent_channel;
    // using asio::experimental::channel_traits;
    // using asio::experimental::coro;
    // using asio::experimental::parallel_group;
    // using asio::experimental::prepend_t;
    // using asio::experimental::promise;
    // using asio::experimental::use_coro_t;
    // using asio::experimental::wait_for_all;
    // using asio::experimental::wait_for_one;
    // using asio::experimental::wait_for_one_error;
    // using asio::experimental::wait_for_one_success;
    using asio::redirect_error_t;
    using asio::strand;
    // using asio::use_awaitable_t;
    using asio::use_future_t;

    //Special Values
    
    using asio::detached;
    using asio::executor_arg;
    // //using asio::experimental::deferred;
    // //using asio::experimental::use_coro;
    namespace this_coro
    {
        using this_coro::cancellation_state;
        using this_coro::executor;
    }
    using asio::use_future;
    // using asio::use_awaitable;

    //Error Codes

    namespace error
    {   
        using error::basic_errors;
        using error::netdb_errors;
        using error::addrinfo_errors;
        using error::misc_errors;
    }

    //Type Traits


    using asio::associated_allocator;
    using asio::associated_cancellation_slot;
    using asio::associated_executor;
    using asio::associator;
    using asio::async_result;
    using asio::default_completion_token;
    using asio::is_executor;
    using asio::uses_executor;

//Buffers and Buffer-Oriented Operations 

    //Classes

    using asio::const_buffer;
    using asio::mutable_buffer;
    using asio::const_buffers_1;
    using asio::mutable_buffers_1;
    using asio::const_registered_buffer;
    using asio::mutable_registered_buffer;
    using asio::null_buffers;
    using asio::streambuf;
    using asio::registered_buffer_id;

    //Class Templates

    using asio::basic_streambuf;
    using asio::buffer_registration;
    using asio::buffered_read_stream;
    using asio::buffered_stream;
    using asio::buffered_write_stream;
    using asio::buffers_iterator;
    using asio::dynamic_string_buffer;
    using asio::dynamic_vector_buffer;

    //Free Functions

    using asio::async_read;
    using asio::async_read_at;
    using asio::async_read_until;
    using asio::async_write;
    using asio::async_write_at;
    using asio::buffer;
    using asio::buffer_cast;
    using asio::buffer_copy;
    using asio::buffer_size;
    using asio::buffer_sequence_begin;
    using asio::buffer_sequence_end;
    using asio::buffers_begin;
    using asio::buffers_end;
    using asio::dynamic_buffer;
    using asio::read;
    using asio::read_at;
    using asio::read_until;
    using asio::register_buffers;
    using asio::transfer_all;
    using asio::transfer_at_least;
    using asio::transfer_exactly;
    using asio::write;
    using asio::write_at;

    //Type Traits

    using asio::is_const_buffer_sequence;
    using asio::is_dynamic_buffer;
    using asio::is_dynamic_buffer_v1;
    using asio::is_dynamic_buffer_v2;
    using asio::is_match_condition;
    using asio::is_mutable_buffer_sequence;
    using asio::is_read_buffered;
    using asio::is_write_buffered;

//Networking

    //Classes

    namespace generic
    {
        using generic::datagram_protocol;
        using generic::raw_protocol;
        using generic::seq_packet_protocol;
        using generic::stream_protocol;
    }

    namespace ip
    {
        using ip::address;
        using ip::address_v4;
        using ip::address_v4_iterator;
        using ip::address_v4_range;
        using ip::address_v6;
        using ip::address_v6_iterator;
        using ip::address_v6_range;
        using ip::bad_address_cast;
        using ip::icmp;
        using ip::network_v4;
        using ip::network_v6;
        using ip::resolver_base;
        using ip::resolver_query_base;
        using ip::tcp;
        using ip::udp;
        using ip::v4_mapped_t;
    }

    using asio::socket_base;

    //Free Functions

    using asio::async_connect;
    using asio::connect;

    namespace ip
    {
        using ip::host_name;
        using ip::make_address;
        using ip::make_address_v4;
        using ip::make_address_v6;
        using ip::make_network_v4;
        using ip::make_network_v6;
    }

    //Class Templates

    
    using asio::basic_datagram_socket;
    using asio::basic_raw_socket;
    using asio::basic_seq_packet_socket;
    using asio::basic_socket;
    using asio::basic_socket_acceptor;
    using asio::basic_socket_iostream;
    using asio::basic_socket_streambuf;
    using asio::basic_stream_socket;

    namespace generic
    {
        using generic::basic_endpoint;
    }

    namespace ip
    {
        using ip::basic_endpoint;
        using ip::basic_resolver;
        using ip::basic_resolver_entry;
        using ip::basic_resolver_iterator;
        using ip::basic_resolver_results;
        using ip::basic_resolver_query;
    }

    //Sockets Options

    namespace ip
    {
        namespace multicast
        {
            using multicast::enable_loopback;
            using multicast::hops;
            using multicast::join_group;
            using multicast::leave_group;
            using multicast::outbound_interface;
        }
        namespace unicast
        {
            using unicast::hops;
        }
        using ip::v6_only;
    }

    using asio::socket_base;

//Timers

    //Classes

    //using asio::deadline_timer;
    using asio::high_resolution_timer;
    using asio::steady_timer;
    using asio::system_timer;

    //Class Templates

    //using asio::basic_deadline_timer;
    using asio::basic_waitable_timer;
    //using asio::time_traits;
    using asio::wait_traits;

//SSL

    // namespace ssl
    // {
    //     //Classes
    //     using ssl::context;
    //     using ssl::context_base;
    //     using ssl::host_name_verification;
    //     using ssl::rfc2818_verification;
    //     using ssl::stream_base;
    //     using ssl::verify_context;

    //     //Class Templates
    //     using ssl::stream;

    //     //Error Codes
    //     namespace error
    //     {
    //         using error::stream_errors;
    //     }
    // }

//Serial Ports

    //Classes

    using asio::serial_port;
    using asio::serial_port_base;

    //Class templates

    using asio::basic_serial_port;

//Signal Handling

    //Classes

    using asio::signal_set;

    //Class Templates

    using asio::basic_signal_set;

//Files and Pipes

//POSIX-specific

//Windows-specific

//ADL
    namespace error
    {
        using error::make_error_code;
    }

}

//std specific

export namespace std
{
    using std::hash;
    using std::uses_allocator;
    using std::is_error_code_enum;
}