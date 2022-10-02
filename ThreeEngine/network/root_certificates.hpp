#pragma once

#include <string>
#include "cacert.pem.h"

inline void load_root_certificates(ssl::context& ctx, boost::system::error_code& ec)
{  

    ctx.add_certificate_authority(
        boost::asio::buffer(cacert_pem, sizeof(cacert_pem)), ec);
    if (ec)
        return;
}

inline void load_root_certificates(ssl::context& ctx)
{
    boost::system::error_code ec;
    load_root_certificates(ctx, ec);
    if (ec)
        throw boost::system::system_error{ ec };
}