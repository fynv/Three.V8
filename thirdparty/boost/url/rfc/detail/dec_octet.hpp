//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/url
//

#ifndef BOOST_URL_RFC_DETAIL_DEC_OCTET_HPP
#define BOOST_URL_RFC_DETAIL_DEC_OCTET_HPP

#include <boost/url/grammar/charset.hpp>
#include <boost/url/grammar/parse_tag.hpp>
#include <boost/url/rfc/charsets.hpp>

namespace boost {
namespace urls {
namespace detail {

struct dec_octet
{
    unsigned char& v;

    friend
    void
    tag_invoke(
        grammar::parse_tag const&,
        char const*& it,
        char const* const end,
        error_code& ec,
        dec_octet const& t) noexcept
    {
        if(it == end)
        {
            // expected DIGIT
            ec = grammar::error::incomplete;
            return;
        }
        if(! grammar::digit_chars(*it))
        {
            // not a digit
            ec = error::bad_digit;
            return;
        }
        unsigned v = *it - '0';
        ++it;
        if(it == end)
        {
            t.v = static_cast<
                std::uint8_t>(v);
            return;
        }
        if(! grammar::digit_chars(*it))
        {
            t.v = static_cast<
                std::uint8_t>(v);
            return;
        }
        if(v == 0)
        {
            // bad leading '0'
            ec = error::bad_leading_zero;
            return;
        }
        v = (10 * v) + *it - '0';
        ++it;
        if(it == end)
        {
            t.v = static_cast<
                std::uint8_t>(v);
            return;
        }
        if(! grammar::digit_chars(*it))
        {
            t.v = static_cast<
                std::uint8_t>(v);
            return;
        }
        if(v > 25)
        {
            // out of range
            ec = error::bad_octet;
            return;
        }
        v = (10 * v) + *it - '0';
        if(v > 255)
        {
            // out of range
            ec = error::bad_octet;
            return;
        }
        ++it;
        t.v = static_cast<
            std::uint8_t>(v);
    }
};

} // detail
} // urls
} // boost

#endif
