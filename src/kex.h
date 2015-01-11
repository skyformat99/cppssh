/*
    cppssh - C++ ssh library
    Copyright (C) 2015  Chris Desjardins
    http://blog.chrisd.info cjd@chrisd.info

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _KEX_Hxx
#define _KEX_Hxx

#include "session.h"
#include "botan/botan.h"
#include <memory>

class CppsshKex
{
public:
    CppsshKex(const std::shared_ptr<CppsshSession> &session);
    bool sendInit();
    bool handleInit();
    bool sendKexDHInit();

private:
    void constructLocalKex();
    std::shared_ptr<CppsshSession> _session;
    Botan::secure_vector<Botan::byte> _localKex;
    Botan::secure_vector<Botan::byte> _remoteKex;
    std::vector<std::string> _ciphers;
    std::vector<std::string> _hmacs;
    Botan::secure_vector<Botan::byte> _e;
};

#endif
