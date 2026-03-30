#include "header.hpp"

std::string hashPassword(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256((unsigned char*)input.c_str(), input.size(), hash);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << std::setw(2) << (int)hash[i];

    return ss.str();
}

bool verifyPassword(const std::string& password, const std::string& hash) {
    return hashPassword(password) == hash;
}