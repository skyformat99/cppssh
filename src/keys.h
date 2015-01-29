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
#ifndef _KEYS_Hxx
#define _KEYS_Hxx

#include "session.h"
#include "crypto.h"
#include <string>

class CppsshKeys
{
public:
    CppsshKeys(std::shared_ptr<CppsshSession> session)
        : _session(session),
        _keyAlgo(hostkeyMethods::MAX_VALS)

    {

    }
    bool getKeyPairFromFile(const std::string& privKeyFileName);
    const Botan::secure_vector<Botan::byte>& generateSignature(const Botan::secure_vector<Botan::byte>& sessionID, const Botan::secure_vector<Botan::byte>& signingData);
    Botan::secure_vector<Botan::byte> generateRSASignature(const Botan::secure_vector<Botan::byte>& sessionID, const Botan::secure_vector<Botan::byte>& signingData);
    Botan::secure_vector<Botan::byte> generateDSASignature(const Botan::secure_vector<Botan::byte>& sessionID, const Botan::secure_vector<Botan::byte>& signingData);

    hostkeyMethods getKeyAlgo()
    {
        return _keyAlgo;
    }
    const Botan::secure_vector<Botan::byte>& getPublicKeyBlob()
    {
        return _publicKeyBlob;
    }
private:
    bool isKey(const Botan::secure_vector<Botan::byte>& buf, std::string header, std::string footer);
    bool getRSAKeys(Botan::secure_vector<Botan::byte> buf);
    bool getDSAKeys(Botan::secure_vector<Botan::byte> buf);
    static Botan::secure_vector<Botan::byte>::const_iterator findEndOfLine(const Botan::secure_vector<Botan::byte>& privateKey, const std::string& lineHeader);
    static Botan::secure_vector<Botan::byte>::const_iterator findKeyBegin(const Botan::secure_vector<Botan::byte>& privateKey, const std::string& header);
    static Botan::secure_vector<Botan::byte>::const_iterator findKeyEnd(const Botan::secure_vector<Botan::byte>& privateKey, const std::string& footer);

    static const std::string HEADER_DSA;
    static const std::string FOOTER_DSA;
    static const std::string HEADER_RSA;
    static const std::string FOOTER_RSA;
    static const std::string PROC_TYPE;
    static const std::string DEK_INFO;

    std::shared_ptr<CppsshSession> _session;
    hostkeyMethods _keyAlgo;
    std::shared_ptr<Botan::RSA_PrivateKey> _rsaPrivateKey;
    std::shared_ptr<Botan::DSA_PrivateKey> _dsaPrivateKey;
    Botan::secure_vector<Botan::byte> _publicKeyBlob;
    Botan::secure_vector<Botan::byte> _signature;
};

#endif

