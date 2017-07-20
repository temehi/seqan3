// ============================================================================
//                 SeqAn - The Library for Sequence Analysis
// ============================================================================
//
// Copyright (c) 2006-2017, Knut Reinert & Freie Universitaet Berlin
// Copyright (c) 2016-2017, Knut Reinert & MPI Molekulare Genetik
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Knut Reinert or the FU Berlin nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL KNUT REINERT OR THE FU BERLIN BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ============================================================================
// Author: David Weese <david.weese@fu-berlin.de>
// Author: Rene Rahn <rene.rahn AT fu-berlin.de>
// ==========================================================================

#pragma once

#include <cctype>
#include <cstring>
#include <string>
#include <tuple>
#include <stdexcept>
#include <algorithm>

#include <range/v3/utility/iterator_traits.hpp>

#include <seqan3/core/concept/core.hpp>
#include <seqan3/core/concept/iterator.hpp>
#include <seqan3/range/concept.hpp>
#include <seqan3/range/container/concept.hpp>
#include <seqan3/alphabet/concept.hpp>
#include <seqan3/io/detail/container_chunk_adaptor_iterator.hpp>
#include <seqan3/io/detail/stream_iterator.hpp>

namespace seqan3::detail
{

template <typename iterator_t, typename sentinel_t>
constexpr bool is_valid_input_impl = input_iterator_concept<std::remove_reference_t<iterator_t>> &&
                                     sentinel_concept<std::remove_reference_t<sentinel_t>,
                                                      std::remove_reference_t<iterator_t>>;

template <typename out_t, typename in_t>
constexpr bool is_valid_output_impl = output_iterator_concept<std::remove_reference_t<out_t>,
                                                              ranges::value_type_t<std::remove_reference_t<in_t>>>;

template <typename t>
constexpr bool is_chunked = std::is_base_of_v<chunk_decorator<std::remove_reference_t<t>>, std::remove_reference_t<t>>;

template <typename t>
constexpr bool is_ignore_impl = std::is_same_v<std::decay_t<t>, std::decay_t<decltype(std::ignore)>>;

template <typename t, typename ...args>
constexpr bool is_predicate_impl = predicate_concept<std::remove_reference_t<t>,
                                                     ranges::value_type_t<std::remove_reference_t<args>>...>;

// ----------------------------------------------------------------------------
// Functor assert_functor
// ----------------------------------------------------------------------------

template <typename functor_t, typename exception_t, typename context_t = decltype(std::ignore), bool RETURN_VALUE = false>
struct assert_functor
{
    functor_t func;

    assert_functor() {}

    assert_functor(functor_t & func) :
    func(func)
    {}

    std::string escape_char(unsigned char val)
    {
        if (val <= '\r')
        {
            static const char * const escapeCodes[14] = {
                "\\0",  "\\1",  "\\2",  "\\3",  "\\4",  "\\5",  "\\6",  "\\a",
                "\\b",  "\\t",  "\\n",  "\\v",  "\\f",  "\\r" };
            return std::string(escapeCodes[val]);
        }
        else if (' ' <= val && val < 128u)
            return std::string() + (char)val;
        else
        {
            char buffer[6]; // 5 + 1, e.g. "\0xff" + trailing zero
            snprintf(buffer, 6, "\\%#2x", (unsigned)val);
            return std::string(buffer);
        }
    }

    template <typename value_t>
    bool operator() (value_t const & val)
    {
        if (/*SEQAN_UNLIKELY(*/!func(val))/*)*/
            throw exception_t(std::string("Unexpected character '") + escape_char(val) + "' found. " +
                              get_exception_message(func, context_t()));
        return RETURN_VALUE;
    }
};

// ----------------------------------------------------------------------------
// Functor or_functor
// ----------------------------------------------------------------------------

template <typename functor1_t, typename functor2_t>
struct or_functor
{
    functor1_t func1;
    functor2_t func2;

    or_functor()
    {}

    or_functor(functor1_t const & func1, functor2_t const & func2) :
        func1(func1), func2(func2)
    {}

    template <typename value_t>
    bool operator() (value_t const & val)
    {
        return func1(val) || func2(val);
    }

    template <typename value_t>
    bool operator() (value_t const & val) const
    {
        return func1(val) || func2(val);
    }
};

// ----------------------------------------------------------------------------
// Metafunction exception_message
// ----------------------------------------------------------------------------

template <typename T, typename TSpec = void>
struct exception_message
{
    static const std::string VALUE;
};

template <typename T, typename TSpec>
const std::string exception_message<T, TSpec>::VALUE;

// ----------------------------------------------------------------------------
// Function get_exception_message()
// ----------------------------------------------------------------------------

template <typename functor_t, typename context_t>
inline std::string const &
get_exception_message(functor_t const &, context_t const &)
{
    return exception_message<functor_t, context_t>::VALUE;
}

// ----------------------------------------------------------------------------
// Exception parse exceptions()
// ----------------------------------------------------------------------------

using  runtime_error = std::runtime_error;

struct parse_error : runtime_error
{
    template <typename string_t>
    parse_error(string_t const & message) : runtime_error(message)
    {}
};

struct unexpected_end_error : parse_error
{

    unexpected_end_error() : parse_error("Unexpected end of input.")
    {}
};

// ----------------------------------------------------------------------------
// Exception empty_field_error
// ----------------------------------------------------------------------------

struct empty_field_error : parse_error
{
    empty_field_error(std::string fieldName) : parse_error(fieldName + " field was empty.")
    {}
};

// ============================================================================
// Functors
// ============================================================================

// ----------------------------------------------------------------------------
// Functor is_in_alphabet
// ----------------------------------------------------------------------------

template <typename value_t>
   requires alphabet_concept<value_t>
struct is_in_alphabet
{
    template <typename in_value_t>
    bool operator() (in_value_t const & in_val) const
    {
        value_t val{};
        from_char(val, in_val);
        return val == std::toupper(in_val);
    }

    template <typename in_value_t>
        requires alphabet_concept<in_value_t>
    bool operator() (in_value_t const & in_val) const
    {
        value_t val{};
        from_integral(val, to_integral(in_val));
        return val == in_val;
    }

    bool operator() (value_t const &) const
    {
        return true;
    }
};

// ----------------------------------------------------------------------------
// Functor is_in_range
// ----------------------------------------------------------------------------

template <char FIRST_CHAR, char LAST_CHAR>
struct is_in_range
{
    template <typename value_t>
    bool operator() (value_t const & val) const
    {
        return FIRST_CHAR <= val && val <= LAST_CHAR;
    }
};

template <char FIRST_CHAR, char LAST_CHAR, typename context_t>
struct exception_message<is_in_range<FIRST_CHAR, LAST_CHAR>, context_t>
{
    static const std::string VALUE;
};

template <char FIRST_CHAR, char LAST_CHAR, typename context_t>
const std::string exception_message<is_in_range<FIRST_CHAR, LAST_CHAR>, context_t>::VALUE =
std::string("Character in range'") + FIRST_CHAR + "' to '" + LAST_CHAR + "' expected.";

// ----------------------------------------------------------------------------
// Functor equals_char
// ----------------------------------------------------------------------------

template <char VALUE>
struct equals_char
{
    template <typename value_t>
    bool operator() (value_t const & val) const
    {
        return val == VALUE;
    }
};

template <char CHAR, typename context_t>
struct exception_message<equals_char<CHAR>, context_t>
{
    static const std::string VALUE;
};

template <char CHAR, typename context_t>
const std::string exception_message<equals_char<CHAR>, context_t>::VALUE = std::string("Character '") + CHAR + "' expected.";

// ----------------------------------------------------------------------------
// Functor EqualsDynamicValue
// ----------------------------------------------------------------------------

template <typename value_t>
struct equals_dynamic_value
{
    value_t val;

    equals_dynamic_value(value_t const & val) :
        val(val)
    {}

    template <typename value2_t>
    bool operator() (value2_t const & v) const
    {
        return v == val;
    }
};

template <typename value_t, typename context_t>
inline std::string const &
get_exception_message(equals_dynamic_value<value_t> const & func, context_t const &)
{
    return std::string("Character '") + func.val + "' expected.";
}

// ----------------------------------------------------------------------------
// Composite Functors
// ----------------------------------------------------------------------------
// Don't use isblank() or isspace() as it they seem to be slower than our functors (due to inlining)

typedef equals_char<'\t'>                                          is_tab;
typedef equals_char<' '>                                           is_space;
typedef or_functor<is_space, is_tab>                               is_blank;
typedef or_functor<equals_char<'\n'>, equals_char<'\r'> >          is_newline;
typedef or_functor<is_blank, is_newline>                           is_whitespace;
typedef is_in_range<'!', '~'>                                      is_graph;
typedef or_functor<is_in_range<'a', 'z'>, is_in_range<'A', 'Z'> >  is_alpha;
typedef is_in_range<'0', '9'>                                      is_digit;
typedef or_functor<is_alpha, is_digit>                             is_alpha_num;

struct always_true
{
    template <typename ...args>
    inline bool operator()(args const &&...)
    {
        return true;
    }
};

struct always_false
{
    template <typename ...args>
    inline bool operator()(args const &&...)
    {
        return false;
    }
};

// ============================================================================
// Functions
// ============================================================================

// ----------------------------------------------------------------------------
// Function put()                                                        [Iter]
// ----------------------------------------------------------------------------

template <typename value_t, typename output_t>
inline void
put(value_t && val,
    output_t && o_iter)
{
    *o_iter = std::forward<value_t>(val);
    ++o_iter;
}

// ----------------------------------------------------------------------------
// Function _write(); Element-wise
// ----------------------------------------------------------------------------

template <typename input_t, typename integral_t, typename output_t>
inline void
write(input_t && i_iter,
      integral_t const n,
      output_t && o_iter)
    requires input_iterator_concept<std::remove_reference_t<input_t>> && is_valid_output_impl<output_t, input_t>
{
    std::copy_n(std::forward<input_t>(i_iter), n, std::forward<output_t>(o_iter));
}

// ----------------------------------------------------------------------------
// Function _write(); Chunked
// ----------------------------------------------------------------------------

// How can we enable chunking for pointers?
// We can get a direction iterator for an pointer type.

// TODO(rrahn): Add later
template <typename input_t, typename integral_t, typename output_t>
inline void
write(input_t && i_iter,
      integral_t n,
      output_t && o_iter)
    requires input_iterator_concept<std::remove_reference_t<input_t>> && is_chunked<input_t> &&
             is_valid_output_impl<output_t, input_t> && is_chunked<output_t>
{
    using target_size_type = ranges::size_type_t<output_t>;

    using ranges::size;
    using ranges::begin;

    while (n != static_cast<integral_t>(0))
    {
        auto ichunk = i_iter.get_chunk();
        auto ochunk = o_iter.get_chunk();

        target_size_type min_chunk_size = std::min(static_cast<target_size_type>(size(ichunk)),
                                                    static_cast<target_size_type>(size(ochunk)));

        if (/*SEQAN_UNLIKELY*/(min_chunk_size == 0u))
        {
            i_iter.next_chunk(n);
            o_iter.next_chunk(n);
            ichunk = i_iter.get_chunk();
            ochunk = o_iter.get_chunk();
            min_chunk_size = std::min(static_cast<target_size_type>(size(ichunk)),
            static_cast<target_size_type>(size(ochunk)));
            if (/*SEQAN_UNLIKELY*/(min_chunk_size == 0u))
            {
                std::copy_n(std::forward<input_t>(i_iter), n, std::forward<output_t>(o_iter));  // fall back to no-chunking version.
                return;
            }
        }

        if (min_chunk_size > static_cast<target_size_type>(n))
            min_chunk_size = static_cast<target_size_type>(n);

        std::copy_n(begin(ichunk), min_chunk_size, begin(ochunk));

        i_iter.advance_chunk(min_chunk_size);
        o_iter.advance_chunk(min_chunk_size);
        n -= min_chunk_size;
    }
}

// chunked, target is pointer (e.g. read_raw_pod)
template <typename input_t, typename integral_t, typename output_t>
inline void
write(input_t && i_iter,
      integral_t n,
      output_t && o_ptr)
    requires input_iterator_concept<std::remove_reference_t<input_t>> && is_chunked<input_t> &&
             std::is_pointer_v<std::remove_reference_t<output_t>>
{
    // we need the size type of an iterator?
    using source_size_t = ranges::size_type_t<input_t>;

    using ranges::size;
    using ranges::begin;

    while (n != static_cast<integral_t>(0))
    {
        auto ichunk = i_iter.get_chunk();
        source_size_t chunk_size = size(ichunk);

        if (/*SEQAN_UNLIKELY*/(chunk_size == 0u))
        {
            next_chunk(i_iter, n);
            ichunk = i_iter.get_chunk();
            source_size_t chunk_size = size(ichunk);
            if (/*SEQAN_UNLIKELY*/(chunk_size == 0u))
            {
                std::copy_n(std::forward<input_t>(i_iter), n, std::forward<output_t>(o_ptr));
                return;
            }
        }

        if (chunk_size > static_cast<source_size_t>(n))
            chunk_size = static_cast<source_size_t>(n);

        std::copy_n(begin(ichunk), chunk_size, std::forward<output_t>(o_ptr));

        i_iter.advance_chunk(chunk_size);                          // advance input iterator
        o_ptr += chunk_size;
        n -= chunk_size;
    }
}

// chunked, source is pointer (e.g. readRawPod)
template <typename input_t, typename integral_t, typename output_t>
inline void
write(input_t && i_ptr,
      integral_t n,
      output_t && o_iter)
    requires std::is_pointer_v<std::remove_reference_t<input_t>> &&
             is_valid_output_impl<output_t, input_t> && is_chunked<output_t>
{
    using output_size_t = ranges::size_type_t<output_t>;

    while (n != static_cast<integral_t>(0))
    {
        auto ochunk = get_chunk(o_iter);
        output_size_t chunk_size = size(ochunk);

        if (/*SEQAN_UNLIKELY*/(chunk_size == 0u))
        {
            next_chunk(o_iter, n);
            ochunk = get_chunk(o_iter);
            chunk_size = size(ochunk);
            if (/*SEQAN_UNLIKELY*/(chunk_size == 0u))
            {
                std::copy_n(std::forward<input_t>(i_ptr), n, std::forward<output_t>(o_iter));
               return;
            }
        }

        if (chunk_size > static_cast<output_size_t>(n))
            chunk_size = static_cast<output_size_t>(n);

        std::copy_n(std::forward<input_t>(i_ptr), chunk_size, begin(ochunk));

        i_ptr += chunk_size;                      // advance input iterator
        advance_chunk(o_iter, chunk_size);
        n -= chunk_size;
   }
}

// TODO(rrahn): Do we really need this?
//template <typename TOValue, typename TIValue, typename TSize>
//inline SEQAN_FUNC_ENABLE_IF(And< Is<CharConcept<TOValue> >,
//                            Is<CharConcept<TIValue> > >, void)
//write(TOValue * &optr, TIValue *iptr, TSize n)
//{
//    std::memcpy(optr, iptr, n);
//    optr += n;
//}

// TODO(rrahn): not genric in sense of char and wchar or even bigger char.
// template <typename in_value_t, typename integral_t, typename out_value_t>
//     requires std::is_same_v<std::make_unsigned_t<std::decay_t<in_value_t>>, unsigned char> &&
//              std::is_same_v<std::make_unsigned_t<std::decay_t<out_value_t>>, unsigned char>
// inline void
// write(in_value_t * & iptr, integral_t const n, out_value_t * optr)
// {
//     std::memcpy(optr, iptr, n);
//     iptr += n;
// }

// write for more complex values (defer to write of iterator value)
// used for Strings of Pairs
//template <typename input_t, typename output_t, typename integral_t>
////inline SEQAN_FUNC_ENABLE_IF(Or<Is<OutputStreamConcept<TTarget> >, Is<ContainerConcept<TTarget> > >, void)
////inline SEQAN_FUNC_ENABLE_IF(And<
////                            Is<IntegerConcept<TSize> >,
////                            Not< Is<Convertible<typename Value<TTarget>::Type,
////                            typename Value<TFwdIterator>::Type> > > >, void)
//
//// not input::value_type not convertible with output_t::value_type
//inline void
//write(input_t  & i_iter,
//      output_t && o_iter,
//      integral_t n)
//{
//    for (; n > static_cast<TSize>(0); --n, ++i_iter)
//    {
//        put(*i_iter, std::forward<output_t>(o_iter));
//        put(' ', std::forward<output_t>(o_iter));
//    }
//}

template <typename input_t, typename integral_t, typename target_t>
inline void
write(input_t && input,
      integral_t const n,
      target_t & output)
    requires input_iterator_concept<std::remove_reference_t<input_t>> &&
             !iterator_concept<target_t> &&
             assignable_concept<ranges::value_type_t<target_t> &, ranges::value_type_t<std::remove_reference_t<input_t>>>
{
    write(std::forward<input_t>(input), n, make_preferred_output_iterator(output));
}

// ----------------------------------------------------------------------------
// Function write(TContainer) but not container of container
// ----------------------------------------------------------------------------

template <typename container_t, typename target_t>
inline void
write(container_t const & input,
      target_t & output)
    requires sized_range_concept<container_t> &&
             assignable_concept<ranges::value_type_t<target_t>&, ranges::value_type_t<container_t>>
{
    using std::size;
    auto rng = make_preferred_input_iterator_range(input);
    write(std::get<0>(rng), size(input), output);
}

// ----------------------------------------------------------------------------
// Function read_impl(); Element-wise
// ----------------------------------------------------------------------------
template <typename input_t, typename in_sent_t,
          typename output_t,
          typename stop_predicate_t,
          typename ignore_predicate_t>
inline void
read_impl(input_t && i_iter,
          in_sent_t && i_sentinel,
          output_t && output_it,
          stop_predicate_t && stop_func,
          ignore_predicate_t && ignore_func)
    requires is_valid_input_impl<input_t, in_sent_t> &&
             is_valid_output_impl<output_t, input_t> &&
             is_predicate_impl<stop_predicate_t, input_t> &&
             is_predicate_impl<ignore_predicate_t, input_t>
{
    using value_type = ranges::value_type_t<std::remove_reference_t<input_t>>;
    typename std::remove_const<value_type>::type val;
    for (; i_iter != i_sentinel; ++i_iter)
    {
        if (/*SEQAN_UNLIKELY(*/stop_func(val = *i_iter))/*)*/
            return;
        if (/*SEQAN_LIKELY(*/!ignore_func(val))/*)*/
            put(val, std::forward<output_t>(output_it));
    }
}

// ----------------------------------------------------------------------------
// Function read_impl(); Chunked
// ----------------------------------------------------------------------------

template <typename input_t, typename in_sent_t,
          typename output_t,
          typename stop_predicate_t,
          typename ignore_predicate_t>
inline void
read_impl(input_t && i_iter,
       in_sent_t && i_sentinel,
       output_t && o_iter,
       stop_predicate_t && stop_func,
       ignore_predicate_t && ignore_func)
    requires is_valid_input_impl<input_t, in_sent_t>  && is_chunked<input_t> &&
             is_valid_output_impl<output_t, input_t> && is_chunked<output_t> &&
             is_predicate_impl<stop_predicate_t, input_t> &&
             is_predicate_impl<ignore_predicate_t, input_t>
{
    using ranges::v3::begin;
    using ranges::v3::end;

    auto ochunk = o_iter.get_chunk();
    auto optr = begin(ochunk);

    for (; i_iter != i_sentinel;)
    {
        auto ichunk = i_iter.get_chunk();
        auto iptr = begin(ichunk);
        for (; iptr != end(ichunk); ++iptr)
        {
            if (/*SEQAN_UNLIKELY*/(stop_func(*iptr)))
            {
                i_iter.advance_chunk(iptr - begin(ichunk));   // advance input iterator
                o_iter.advance_chunk(optr - begin(ochunk));   // extend target string size
                o_iter.trim_trailing();
                return;
            }
            if (/*SEQAN_UNLIKELY*/(ignore_func(*iptr)))
                continue;
            // construct values in reserved memory
            if (/*SEQAN_UNLIKELY*/(optr == end(ochunk)))  // in case chunk is full.
            {
                // do a pbump or for container do a reserve.
                o_iter.advance_chunk(optr - begin(ochunk));  // pbump or set
                o_iter.next_chunk(end(ichunk) - iptr);       // reserve vector or reload buffer.
                // std::cout << "get_chunk: " << std::endl;
                ochunk = o_iter.get_chunk();
                optr = begin(ochunk);
                // SEQAN_ASSERT(optr < ochunk.end);
            }

            put(*iptr, optr);
        }
        i_iter.advance_chunk(iptr - begin(ichunk)); // advance input iterator
    }
    o_iter.advance_chunk(optr - begin(ochunk));
    o_iter.trim_trailing();
}

// ----------------------------------------------------------------------------
// Function ignore_impl(); Element-wise
// ----------------------------------------------------------------------------
template <typename input_t,
          typename in_sent_t,
          typename stop_predicate_t>
inline void
ignore_impl(input_t && i_iter,
            in_sent_t && i_sentinel,
            stop_predicate_t && stop_func)
    requires is_valid_input_impl<input_t, in_sent_t> &&
             is_predicate_impl<stop_predicate_t, input_t>
{
    for (; i_iter != i_sentinel; ++i_iter)
    {
        if (/*SEQAN_UNLIKELY(*/stop_func(*i_iter))/*)*/
            return;
    }
}

// ----------------------------------------------------------------------------
// Function ignore_impl(); Chunked
// ----------------------------------------------------------------------------

template <typename input_t, typename in_sent_t,
          typename stop_predicate_t>
inline void
ignore_impl(input_t && i_iter,
            in_sent_t && i_sentinel,
            stop_predicate_t && stop_func)
    requires is_valid_input_impl<input_t, in_sent_t>  && is_chunked<input_t> &&
             is_predicate_impl<stop_predicate_t, input_t>
{
    using ranges::v3::begin;
    using ranges::v3::end;

    for (; i_iter != i_sentinel;)
    {
        auto ichunk = i_iter.get_chunk();
        auto iptr = begin(ichunk);
        for (; iptr != end(ichunk); ++iptr)
        {
            if (/*SEQAN_UNLIKELY*/(stop_func(*iptr)))
            {
                i_iter.advance_chunk(iptr - begin(ichunk));   // advance input iterator
                return;
            }
        }
        i_iter.advance_chunk(iptr - begin(ichunk)); // advance input iterator
    }
}

// ----------------------------------------------------------------------------
// Function read_until()
// ----------------------------------------------------------------------------

template <typename input_t,
          typename in_sent_t,
          typename output_t,
          typename stop_predicate_t,
          typename ignore_predicate_t = always_false>
inline void
read_until(input_t && i_iter,
           in_sent_t && i_sentinel,
           output_t && o_iter,
           stop_predicate_t && stop_func,
           ignore_predicate_t && ignore_func = always_false{})
requires is_valid_input_impl<input_t, in_sent_t> &&
         is_valid_output_impl<output_t, input_t> || is_ignore_impl<output_t> &&
         is_predicate_impl<stop_predicate_t, input_t> &&
         is_predicate_impl<ignore_predicate_t, input_t>
{
    if constexpr (std::is_same_v<std::decay_t<output_t>, std::decay_t<decltype(std::ignore)>>)
        ignore_impl(i_iter, i_sentinel, stop_func);
    else
        read_impl(i_iter, i_sentinel, o_iter, stop_func, ignore_func);
}
// ----------------------------------------------------------------------------
// Function read_n()
// ----------------------------------------------------------------------------

template <typename input_t, typename in_sent_t,
          typename output_t,
          typename integral_t,
          typename ignore_predicate_t = always_false>
inline void
read_n(input_t && i_iter,
       in_sent_t && i_sentinel,
       output_t && o_iter,
       integral_t n = 1,
       ignore_predicate_t && ignore_func = always_false{})
 requires is_valid_input_impl<input_t, in_sent_t> &&
          is_valid_output_impl<output_t, input_t> || is_ignore_impl<output_t> &&
          integral_concept<integral_t> &&
          is_predicate_impl<ignore_predicate_t, input_t>
{
    auto stop_func = [n](auto const &) mutable {return n-- == 0; };
    read_until(std::forward<input_t>(i_iter), std::forward<input_t>(i_sentinel),
               std::forward<output_t>(o_iter),
               stop_func,
               std::forward<ignore_predicate_t>(ignore_func));
}

// ----------------------------------------------------------------------------
// Function read_line()
// ----------------------------------------------------------------------------
template <typename input_t,
          typename in_sent_t,
          typename output_t,
          typename ignore_predicate_t = always_false>
inline void
read_line(input_t && i_iter,
          in_sent_t && i_sentinel,
          output_t && o_iter,
          ignore_predicate_t && ignore_func = always_false{})
    requires is_valid_input_impl<input_t, in_sent_t> &&
             is_valid_output_impl<output_t, input_t> || is_ignore_impl<output_t> &&
             is_predicate_impl<ignore_predicate_t, input_t>
{
    read_until(std::forward<input_t>(i_iter), std::forward<input_t>(i_sentinel),
               std::forward<output_t>(o_iter),
               is_newline{},
               std::forward<ignore_predicate_t>(ignore_func));
    read_until(std::forward<input_t>(i_iter),
               std::forward<input_t>(i_sentinel),
               std::ignore,
               std::not_fn(is_newline{}));          // skip next line character.
}

// ----------------------------------------------------------------------------
// Function read_one()       just one char
// ----------------------------------------------------------------------------
template <typename input_t, typename in_sent_t,
          typename output_t,
          typename ignore_predicate_t = always_false>
inline void
read_one(input_t && i_iter,
         in_sent_t && i_sentinel,
         output_t && o_iter,
         ignore_predicate_t && ignore_func = always_false{})
    requires is_valid_input_impl<input_t, in_sent_t> &&
             is_valid_output_impl<output_t, input_t> || is_ignore_impl<output_t> &&
             is_predicate_impl<ignore_predicate_t, input_t>
{
    read_n(std::forward<input_t>(i_iter), std::forward<in_sent_t>(i_sentinel),
           std::forward<output_t>(o_iter),
           1,
           std::forward<ignore_predicate_t>(ignore_func));
}

// ----------------------------------------------------------------------------
// Function read_raw_pod()
// ----------------------------------------------------------------------------

template <typename input_t, typename value_t>
inline void
read_raw_pod(input_t & i_iter, value_t && value)
{
    write(i_iter, static_cast<char*>(&value), sizeof(value_t));
}

// ----------------------------------------------------------------------------
// Function writeWrappedString()
// ----------------------------------------------------------------------------

//template <typename input_range_t, typename output_t, typename integral_t>
//inline void
//write_wrapped(input_range_t & in,
//              output_t && o_iter,
//              integral_t const line_length)
//{
//    // TODO(rrahn): Global Metafunction.
//    using size_type = input_range_t::size_type;
//
//    auto i_iter = in_begin(in);
//    size_type chars_left = std::size(in);
//    size_type chars_per_line;
//    size_type line_length_ = (line_length == 0)? maxValue<size_type>() : line_length;
//    do
//    {
//        chars_per_line = std::min(chars_left, line_length_);
//        write(i_iter, std::forward<output_t>(o_iter), chars_per_line);
//        put('\n', std::forward<output_t>(o_iter));
//        chars_left -= chars_per_line;
//    }
//    while (chars_left != 0);
//}

//// ----------------------------------------------------------------------------
//// Function findFirst()
//// ----------------------------------------------------------------------------
//
//template <typename TContainer, typename functor_t>
//inline typename Position<TContainer>::Type
//findFirst(TContainer const &cont, functor_t const &func)
//{
//    typename Iterator<TContainer const, Rooted>::Type iter = begin(cont, Rooted());
//    skipUntil(iter, func);
//    return iter - begin(cont, Rooted());
//}
//
//template <typename TContainer>
//inline typename Position<TContainer>::Type
//findFirst(TContainer const &cont, typename Value<TContainer>::Type const &val)
//{
//    EqualsDynamicValue<typename Value<TContainer>::Type> func(val);
//    return findFirst(cont, func);
//}
//
//// ----------------------------------------------------------------------------
//// Function findLast()
//// ----------------------------------------------------------------------------
//
//template <typename TContainer, typename functor_t>
//inline typename Position<TContainer>::Type
//findLast(TContainer const &cont, functor_t const &func)
//{
//    typedef ModifiedString<TContainer const, ModReverse> TRevContainer;
//
//    SEQAN_CONCEPT_ASSERT((IntegerConcept<typename Position<TContainer>::Type>));
//
//    // search from back to front
//    TRevContainer rev(cont);
//    typename Iterator<TRevContainer, Rooted>::Type iter = begin(rev, Rooted());
//    skipUntil(iter, func);
//
//    if (atEnd(iter))
//        return -1;
//
//    return host(iter) - begin(cont, Rooted());
//}
//
//template <typename TContainer>
//inline typename Position<TContainer>::Type
//findLast(TContainer const &cont, typename Value<TContainer>::Type const &val)
//{
//    EqualsDynamicValue<typename Value<TContainer>::Type> func(val);
//    return findLast(cont, func);
//}
//
//// ----------------------------------------------------------------------------
//// Function cropAfterFirst(); crop after first occurrence (including it)
//// ----------------------------------------------------------------------------
//
//template <typename TContainer, typename functor_t>
//inline void
//cropAfterFirst(TContainer &cont, functor_t const &func)
//{
//    resize(cont, findFirst(cont, func));
//}
//
//// ----------------------------------------------------------------------------
//// Function cropAfterLast(); crop after last occurrence (excluding it)
//// ----------------------------------------------------------------------------
//
//template <typename TContainer, typename functor_t>
//inline void
//cropAfterLast(TContainer &cont, functor_t const &func)
//{
//    resize(cont, findLast(cont, func) + 1);
//}
//
//// ----------------------------------------------------------------------------
//// Function cropBeforeFirst(); crop before first occurrence (excluding it)
//// ----------------------------------------------------------------------------
//
//template <typename TContainer, typename functor_t>
//inline void
//cropBeforeFirst(TContainer &cont, functor_t const &func)
//{
//    erase(cont, 0, findFirst(cont, func));
//}
//
//// ----------------------------------------------------------------------------
//// Function cropBeforeLast(); crop before first occurrence (including it)
//// ----------------------------------------------------------------------------
//
//template <typename TContainer, typename functor_t>
//inline void
//cropBeforeLast(TContainer &cont, functor_t const &func)
//{
//    erase(cont, 0, findLast(cont, func) + 1);
//}
//// ----------------------------------------------------------------------------
//// Function cropOuter(); crop after last occurrence (excluding it)
//// ----------------------------------------------------------------------------
//
//template <typename TContainer, typename functor_t>
//inline void
//cropOuter(TContainer &cont, functor_t const &func)
//{
//    cropAfterLast(cont, NotFunctor<functor_t>(func));
//    cropBeforeFirst(cont, NotFunctor<functor_t>(func));
//}

// --------------------------------------------------------------------------
// Function strSplit()
// --------------------------------------------------------------------------

/*!
 * @fn StringSet#strSplit
 * @brief Split a sequence using a delimiter and append the results to a target string set
 *
 * @signature void strSplit(result, sequence[, sep[, allowEmptyStrings[, maxSplit]]]);
 *
 * @param[out] result           The resulting string set (can be any ContainerOfContainer, also STL)
 * @param[in]  sequence         The sequence to split.
 * @param[in]  sep              The splitter to use (default <tt>' '</tt>).
 * @param[in]  allowEmptyString Whether or not to allow empty strings (<tt>bcd gitool</tt>, defaults to <tt>true</tt> iff
 *                              <tt>sep</tt> is given).
 * @param[in]  maxSplit         The maximal number of split operations to do if given.
 */

//template <typename sequence_t, typename functor_t, typename TSize>
//    requires forward_range_concept<sequence_t> &&
//
//inline auto
//split_by(TResult & result,
//         TSequence const & sequence,
//         functor_t const & sep,
//         bool const allowEmptyStrings,
//         TSize maxSplit)
//{
//    typedef typename Iterator<TSequence const, Standard>::Type TIter;
//    typedef std::conditional_t<Is<StlContainerConcept<TResult>>::VALUE,
//    TSequence,
//    decltype(infix(sequence, 0, 1))> TResultValue;
//
//    TIter itBeg = begin(sequence, Standard());
//    TIter itEnd = end(sequence, Standard());
//    TIter itFrom = itBeg;
//
//    if (maxSplit == 0)
//    {
//        appendValue(result, sequence);
//        return;
//    }
//
//    for (TIter it = itBeg; it != itEnd; ++it)
//        if (sep(getValue(it)))
//        {
//            if (allowEmptyStrings || itFrom != it)
//            {
//                appendValue(result, static_cast<TResultValue>(infix(sequence, itFrom - itBeg, it - itBeg)));
//                if (--maxSplit == 0)
//                {
//                    if (!allowEmptyStrings)
//                    {
//                        while (it != itEnd && sep(getValue(it)))
//                            ++it;
//                    }
//                    else
//                        ++it;
//
//                    if (it != itEnd)
//                        appendValue(result, static_cast<TResultValue>(infix(sequence, itFrom - itBeg, it - itBeg)));
//
//                        return;
//                }
//            }
//            itFrom = it + 1;
//        }
//
//    if (allowEmptyStrings || itFrom != itEnd)
//        appendValue(result, static_cast<TResultValue>(infix(sequence, itFrom - itBeg, itEnd - itBeg)));
//}
//
//template <typename TResult, typename TSequence, typename functor_t>
//inline SEQAN_FUNC_ENABLE_IF(And<Is<ContainerConcept<TResult> >,
//                            Is<ContainerConcept<typename Value<TResult>::Type > > >, void)
//strSplit(TResult & result, TSequence const & sequence, functor_t const & sep, bool const allowEmptyStrings)
//{
//    strSplit(result, sequence, sep, allowEmptyStrings, maxValue<typename Size<TSequence>::Type>());
//}
//
//template <typename TResult, typename TSequence, typename functor_t>
//inline SEQAN_FUNC_ENABLE_IF(And<Is<ContainerConcept<TResult> >,
//                            Is<ContainerConcept<typename Value<TResult>::Type > > >, void)
//strSplit(TResult & result, TSequence const & sequence, functor_t const & sep)
//{
//    strSplit(result, sequence, sep, true);
//}
//
//template <typename TResult, typename TSequence>
//inline SEQAN_FUNC_ENABLE_IF(And<Is<ContainerConcept<TResult> >,
//                            Is<ContainerConcept<typename Value<TResult>::Type > > >, void)
//strSplit(TResult & result, TSequence const & sequence)
//{
//    strSplit(result, sequence, equals_char<' '>(), false);
//}
}  // namespace seqan3::detail