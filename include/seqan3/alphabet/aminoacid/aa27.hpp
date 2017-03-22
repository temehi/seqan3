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
// Author: Sara Hetzel <sara.hetzel AT fu-berlin.de>
// ============================================================================

#pragma once

#include "../alphabet.hpp"

namespace seqan3
{

struct aa27
{
    using char_type = char;
    using integral_type = uint8_t;

    // strictly typed enum, unfortunately with scope
    enum struct c_type : integral_type
    {
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        TERMINATOR,
        UNKNOWN = X
    };

    // import into local scope
    static constexpr c_type A{c_type::A};
    static constexpr c_type B{c_type::B};
    static constexpr c_type C{c_type::C};
    static constexpr c_type D{c_type::D};
    static constexpr c_type E{c_type::E};
    static constexpr c_type F{c_type::F};
    static constexpr c_type G{c_type::G};
    static constexpr c_type H{c_type::H};
    static constexpr c_type I{c_type::I};
    static constexpr c_type J{c_type::J};
    static constexpr c_type K{c_type::K};
    static constexpr c_type L{c_type::L};
    static constexpr c_type M{c_type::M};
    static constexpr c_type N{c_type::N};
    static constexpr c_type O{c_type::O};
    static constexpr c_type P{c_type::P};
    static constexpr c_type Q{c_type::Q};
    static constexpr c_type R{c_type::R};
    static constexpr c_type S{c_type::S};
    static constexpr c_type T{c_type::T};
    static constexpr c_type U{c_type::U};
    static constexpr c_type V{c_type::V};
    static constexpr c_type W{c_type::W};
    static constexpr c_type X{c_type::X};
    static constexpr c_type Y{c_type::Y};
    static constexpr c_type Z{c_type::Z};
    static constexpr c_type TERMINATOR{c_type::TERMINATOR};
    static constexpr c_type UNKNOWN{c_type::UNKNOWN};

    // the value
    c_type value;

    // implicit compatibility to inner_type
    constexpr aa27 & operator =(c_type const c)
    {
        value = c;
        return *this;
    }
    constexpr operator c_type() const
    {
        return value;
    }

    // explicit compatibility to char
    explicit constexpr operator char_type() const
    {
        return to_char();
    }
    constexpr char_type to_char() const
    {
        return value_to_char[static_cast<integral_type>(value)];
    }

    constexpr aa27 from_char(char_type const c)
    {
        value = char_to_value[c];
        return *this;
    }

    // explicit compatibility to integral
    constexpr integral_type to_integral() const
    {
        return static_cast<integral_type>(value);
    }

    constexpr aa27 from_integral(integral_type const c)
    {
        value = static_cast<c_type>(c);
        return *this;
    }

    // conversion tables
    static constexpr integral_type value_size{27};

    static constexpr char value_to_char[value_size]
    {
        'A',
        'B',
        'C',
        'D',
        'E',
        'F',
        'G',
        'H',
        'I',
        'J',
        'K',
        'L',
        'M',
        'N',
        'O',
        'P',
        'Q',
        'R',
        'S',
        'T',
        'U',
        'V',
        'W',
        'X',
        'Y',
        'Z',
        '*'
    };

    static constexpr c_type char_to_value[256]
    {
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        // *,
        c_type::TERMINATOR, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        //                                                                                   A,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::A,
        // B,               C,               D,               E,               F,               G,
        c_type::B,          c_type::C,       c_type::D,       c_type::E,       c_type::F,       c_type::G,
        // H,               I,               J,               K,               L,               M,
        c_type::H,          c_type::I,       c_type::J,       c_type::K,       c_type::L,       c_type::M,
        // N,               O,               P,               Q,               R,               S,
        c_type::N,          c_type::O,       c_type::P,       c_type::Q,       c_type::R,       c_type::S,
        // T,               U,               V,               W,               X,               Y,
        c_type::T,          c_type::U,       c_type::V,       c_type::W,       c_type::X,       c_type::Y,
        // Z,
        c_type::Z,          c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        //                  a,               b,               c,               d,               e,
        c_type::UNKNOWN,    c_type::A,       c_type::B,       c_type::C,       c_type::D,       c_type::E,
        // f,               g,               h,               i,               j,               k,
        c_type::F,          c_type::G,       c_type::H,       c_type::I,       c_type::J,       c_type::K,
        // l,               m,               n,               o,               p,               q,
        c_type::L,          c_type::M,       c_type::N,       c_type::O,       c_type::P,       c_type::Q,
        // r,               s,               t,               u,               v,               w,
        c_type::R,          c_type::S,       c_type::T,       c_type::U,       c_type::V,       c_type::W,
        // x,               y,               z
        c_type::X,          c_type::Y,       c_type::Z,       c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN,
        c_type::UNKNOWN,    c_type::UNKNOWN, c_type::UNKNOWN, c_type::UNKNOWN
    };

};

static_assert(alphabet_concept<aa27>);

}
