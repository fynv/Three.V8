//
// Copyright (c) 2016-2019 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_URL_GRAMMAR_TOKEN_HPP
#define BOOST_URL_GRAMMAR_TOKEN_HPP

#include <boost/url/detail/config.hpp>
#include <boost/url/grammar/charset.hpp>
#include <boost/url/grammar/error.hpp>
#include <boost/url/grammar/parse_tag.hpp>
#include <boost/url/string_view.hpp>
#include <boost/static_assert.hpp>

namespace boost {
namespace urls {
namespace grammar {

/** Rule for 1 or more characters from a character set

    @par BNF
    @code
    token     = 1*( ch )
    @endcode
*/
template<class CharSet>
struct token
{
    using value_type = std::string;
    using reference = string_view;

    BOOST_STATIC_ASSERT(
        is_charset<CharSet>::value);

    string_view s;

    reference
    operator*() noexcept
    {
        return s;
    }

    friend
    void
    tag_invoke(
        parse_tag const&,
        char const*& it,
        char const* end,
        error_code& ec,
        token& t) noexcept
    {
        auto const start = it;
        static constexpr CharSet cs{};
        if(it == end)
        {
            ec = grammar::error::incomplete;
            return;
        }
        it = (find_if_not)(it, end, cs);
        if(it == start)
        {
            ec = grammar::error::syntax;
            return;
        }
        t.s = string_view(start, it - start);
    }
};

} // grammar
} // urls
} // boost

#endif
