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
#if !defined(WIN32) && !defined(__MINGW32__)
#   include <arpa/inet.h>
#else
#   include <Winsock2.h>
#endif

#include "crypto.h"
#include "packet.h"
#include "impl.h"

#include <string>
#include <sstream>

CppsshCrypto::CppsshCrypto(const std::shared_ptr<CppsshSession> &session)
    : _session(session),
    _encryptBlock(0),
    _decryptBlock(0),
    _inited(false),
    _c2sMacMethod(HMAC_MD5),
    _s2cMacMethod(HMAC_MD5),
    _kexMethod(DH_GROUP1_SHA1),
    _hostkeyMethod(SSH_DSS),
    _c2sCryptoMethod(AES128_CBC),
    _s2cCryptoMethod(AES128_CBC)
{
}

bool CppsshCrypto::encryptPacket(Botan::secure_vector<Botan::byte> &crypted, Botan::secure_vector<Botan::byte> &hmac, const Botan::secure_vector<Botan::byte> &packet, uint32_t seq)
{
    Botan::secure_vector<Botan::byte> macStr;

    _encrypt->start_msg();
    _encrypt->write(packet);
    _encrypt->end_msg();
    
    crypted = _encrypt->read_all(_encrypt->message_count() - 1);

    if (_hmacOut != NULL)
    {
        CppsshPacket mac(&macStr);
        mac.addInt(seq);
        macStr += packet;
        hmac = _hmacOut->process(macStr);
    }

    return true;
}

bool CppsshCrypto::decryptPacket(Botan::secure_vector<Botan::byte> &decrypted, const Botan::secure_vector<Botan::byte> &packet, uint32_t len)
{
    uint32_t pLen = packet.size();

    if (len % _decryptBlock)
    {
        len = len + (len % _decryptBlock);
    }

    if (len > pLen)
    {
        len = pLen;
    }

    _decrypt->process_msg(packet);
    decrypted = _decrypt->read_all(_decrypt->message_count() - 1);
    return true;
}

uint32_t CppsshCrypto::getMacDigestLen(uint32_t method)
{
    switch (method)
    {
    case HMAC_SHA1:
        return 20;

    case HMAC_MD5:
        return 16;

    case HMAC_NONE:
        return 0;

    default:
        return 0;
    }
}

void CppsshCrypto::computeMac(Botan::secure_vector<Botan::byte> &hmac, const Botan::secure_vector<Botan::byte> &packet, uint32_t seq)
{
    Botan::secure_vector<Botan::byte> macStr;

    if (_hmacIn)
    {
        CppsshPacket mac(&macStr);
        mac.addInt(seq);
        macStr += packet;
        hmac = _hmacIn->process(macStr);
    }
    else
    {
        hmac.clear();
    }
}

void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}
bool CppsshCrypto::agree(Botan::secure_vector<Botan::byte> &result, const std::vector<std::string>& local, const Botan::secure_vector<Botan::byte> &remote)
{
    bool ret = false;
    std::vector<std::string>::const_iterator it;
    std::vector<std::string>::const_iterator agreedAlgo;
    std::vector<std::string> remoteVec;
    std::string remoteStr((char*)remote.data(), 0, remote.size());

    split(remoteStr, ',', remoteVec);
    
    for (it = local.begin(); it != local.end(); it++)
    {
        agreedAlgo = std::find(remoteVec.begin(), remoteVec.end(), *it);
        if (agreedAlgo != remoteVec.end())
        {
            result = Botan::secure_vector<Botan::byte>((*agreedAlgo).begin(), (*agreedAlgo).end());
            ret = true;
            break;
        }
    }
    return ret;
}

bool CppsshCrypto::negotiatedKex(const Botan::secure_vector<Botan::byte> &kexAlgo)
{
    bool ret = false;
    if (equal(kexAlgo.begin(), kexAlgo.end(), "diffie-hellman-group1-sha1") == true)
    {
        _kexMethod = DH_GROUP1_SHA1;
        ret = true;
    }
    else if (equal(kexAlgo.begin(), kexAlgo.end(), "diffie-hellman-group14-sha1") == true)
    {
        _kexMethod = DH_GROUP14_SHA1;
        ret = true;
    }
    if (ret == false)
    {
        //ne7ssh::errors()->push(_session->getSshChannel(), "KEX algorithm: '%B' not defined.", &kexAlgo);
    }
    return ret;
}

bool CppsshCrypto::negotiatedHostkey(const Botan::secure_vector<Botan::byte> &hostkeyAlgo)
{
    bool ret = false;
    if (equal(hostkeyAlgo.begin(), hostkeyAlgo.end(), "ssh-dss") == true)
    {
        _hostkeyMethod = SSH_DSS;
        ret = true;
    }
    else if (equal(hostkeyAlgo.begin(), hostkeyAlgo.end(), "ssh-rsa") == true)
    {
        _hostkeyMethod = SSH_RSA;
        ret = true;
    }
    if (ret == false)
    {
        //ne7ssh::errors()->push(_session->getSshChannel(), "KEX algorithm: '%B' not defined.", &kexAlgo);
    }
    return ret;
}

bool CppsshCrypto::negotiatedCrypto(const Botan::secure_vector<Botan::byte> &cryptoAlgo, cryptoMethods* cryptoMethod)
{
    bool ret = false;
    if (equal(cryptoAlgo.begin(), cryptoAlgo.end(), "3des-cbc") == true)
    {
        *cryptoMethod = TDES_CBC;
        ret = true;
    }
    else if (equal(cryptoAlgo.begin(), cryptoAlgo.end(), "aes128-cbc") == true)
    {
        *cryptoMethod = AES128_CBC;
        ret = true;
    }
    else if (equal(cryptoAlgo.begin(), cryptoAlgo.end(), "aes192-cbc") == true)
    {
        *cryptoMethod = AES192_CBC;
        ret = true;
    }
    else if (equal(cryptoAlgo.begin(), cryptoAlgo.end(), "aes256-cbc") == true)
    {
        *cryptoMethod = AES256_CBC;
        ret = true;
    }
    else if (equal(cryptoAlgo.begin(), cryptoAlgo.end(), "blowfish-cbc") == true)
    {
        *cryptoMethod = BLOWFISH_CBC;
        ret = true;
    }
    else if (equal(cryptoAlgo.begin(), cryptoAlgo.end(), "cast128-cbc") == true)
    {
        *cryptoMethod = CAST128_CBC;
        ret = true;
    }
    else if ((equal(cryptoAlgo.begin(), cryptoAlgo.end(), "twofish-cbc") == true) || (equal(cryptoAlgo.begin(), cryptoAlgo.end(), "twofish256-cbc") == true))
    {
        *cryptoMethod = TWOFISH_CBC;
        ret = true;
    }
    if (ret == false)
    {
        //ne7ssh::errors()->push(_session->getSshChannel(), "Cryptographic algorithm: '%B' not defined.", &cryptoAlgo);
    }
    return ret;
}

bool CppsshCrypto::negotiatedCryptoC2s(const Botan::secure_vector<Botan::byte> &cryptoAlgo)
{
    return negotiatedCrypto(cryptoAlgo, &_c2sCryptoMethod);
}

bool CppsshCrypto::negotiatedCryptoS2c(const Botan::secure_vector<Botan::byte> &cryptoAlgo)
{
    return negotiatedCrypto(cryptoAlgo, &_s2cCryptoMethod);
}

bool CppsshCrypto::negotiatedMac(const Botan::secure_vector<Botan::byte> &macAlgo, macMethods* macMethod)
{
    bool ret = false;
    if (equal(macAlgo.begin(), macAlgo.end(), "hmac-sha1") == true)
    {
        *macMethod = HMAC_SHA1;
        ret = true;
    }
    else if (equal(macAlgo.begin(), macAlgo.end(), "hmac-md5") == true)
    {
        *macMethod = HMAC_MD5;
        ret = true;
    }
    else if (equal(macAlgo.begin(), macAlgo.end(), "none") == true)
    {
        *macMethod = HMAC_NONE;
        ret = true;
    }
    if (ret == false)
    {
        //ne7ssh::errors()->push(_session->getSshChannel(), "HMAC algorithm: '%B' not defined.", &macAlgo);
    }
    return ret;
}

bool CppsshCrypto::negotiatedMacC2s(const Botan::secure_vector<Botan::byte> &macAlgo)
{
    return negotiatedMac(macAlgo, &_c2sMacMethod);
}

bool CppsshCrypto::negotiatedMacS2c(const Botan::secure_vector<Botan::byte> &macAlgo)
{
    return negotiatedMac(macAlgo, &_s2cMacMethod);
}

bool CppsshCrypto::negotiatedCmprs(Botan::secure_vector<Botan::byte> &cmprsAlgo, cmprsMethods* cmprsMethod)
{
    bool ret = false;
    if (equal(cmprsAlgo.begin(), cmprsAlgo.end(), "none") == true)
    {
        *cmprsMethod = NONE;
        ret = true;
    }
    else if (equal(cmprsAlgo.begin(), cmprsAlgo.end(), "zlib") == true)
    {
        *cmprsMethod = ZLIB;
        ret = true;
    }
    if (ret == false)
    {
        //ne7ssh::errors()->push(_session->getSshChannel(), "Compression algorithm: '%B' not defined.", &cmprsAlgo);
    }
    return ret;
}

bool CppsshCrypto::negotiatedCmprsC2s(Botan::secure_vector<Botan::byte> &cmprsAlgo)
{
    return negotiatedCmprs(cmprsAlgo, &_c2sCmprsMethod);
}

bool CppsshCrypto::negotiatedCmprsS2c(Botan::secure_vector<Botan::byte> &cmprsAlgo)
{
    return negotiatedCmprs(cmprsAlgo, &_s2cCmprsMethod);
}

bool CppsshCrypto::getKexPublic(Botan::BigInt &publicKey)
{
    bool ret = true;
    std::string dlGroup;
    switch (_kexMethod)
    {
    case DH_GROUP1_SHA1:
        dlGroup = "modp/ietf/1024";
        break;

    case DH_GROUP14_SHA1:
        dlGroup = "modp/ietf/2048";
        break;

    default:
        //ne7ssh::errors()->push(_session->getSshChannel(), "Undefined DH Group: '%s'.", _kexMethod);
        ret = false;
        break;
    }
    if (ret == true)
    {
        _privKexKey.reset(new Botan::DH_PrivateKey(*CppsshImpl::RNG, Botan::DL_Group(dlGroup)));
        Botan::DH_PublicKey pubKexKey = *_privKexKey;

        publicKey = pubKexKey.get_y();
        if (publicKey.is_zero())
        {
            ret = false;
        }
    }
    return ret;
}