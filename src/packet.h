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
#ifndef _PACKET_Hxx
#define _PACKET_Hxx

#include "botan/botan.h"
#include <cstdint>
#include <cstddef>

class CppsshPacket
{
public:
    CppsshPacket(Botan::secure_vector<Botan::byte>* data);

    void addVectorField(const Botan::secure_vector<Botan::byte>& vec);
    void addVector(const Botan::secure_vector<Botan::byte>& vec);
    void addString(const std::string& str);
    void addInt(const uint32_t var);
    void addByte(const uint8_t ch);
    void addBigInt(const Botan::BigInt& bn);
    static void bn2vector(Botan::secure_vector<Botan::byte>& result, const Botan::BigInt& bi);

    CppsshPacket& operator=(Botan::secure_vector<Botan::byte>* encryptedPacket);
    uint32_t getPacketLength() const;
    uint32_t getCryptoLength() const;
    Botan::byte getPadLength() const;
    Botan::byte getCommand() const;
    Botan::secure_vector<Botan::byte>::iterator getPayloadBegin() const;
    Botan::secure_vector<Botan::byte>::iterator getPayloadEnd() const;

    bool getString(Botan::secure_vector<Botan::byte>& result);
    bool getString(std::string& result);
    bool getBigInt(Botan::BigInt& result);
    uint32_t getInt();

    void copy(const Botan::secure_vector<Botan::byte>& src);
    void clear();
    size_t size() const;

private:
    Botan::secure_vector<Botan::byte>* _data;
};

// This class makes a copy of just the header bytes, for quick error checking etc...
class CppsshPacketHeader : public CppsshPacket
{
public:
    CppsshPacketHeader(const Botan::secure_vector<Botan::byte>& data);
    ~CppsshPacketHeader();
private:
    Botan::secure_vector<Botan::byte> _buf;
};

#endif

