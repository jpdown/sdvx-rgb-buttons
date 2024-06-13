//
// Created by pneuma on 5/29/2024.
//

#ifndef CHUNI_SCALER_SIGNATURE_H
#define CHUNI_SCALER_SIGNATURE_H

#include <array>
#include <cstdint>
#include <span>

struct Byte {
    bool known = false;
    uint8_t byte = 0;
};

// TODO: make this compile time error https://blog.therocode.net/2018/09/compile-time-string-parsing
constexpr uint8_t char_to_nibble(char input) {
    if (input >= '0' && input <= '9') {
        return input - '0';
    }
    if (input >= 'A' && input <= 'F') {
        return input - 'A' + 10;
    }
    if (input >= 'a' && input <= 'f') {
        return input - 'f' + 10;
    }

    return 255;
}


template <std::size_t sig_size> class Signature {
public:
    constexpr explicit Signature(std::string_view sig) {
        // TODO this needs major cleanup
        int chars_processed = 0;
        int bytes_processed = 0;
        Byte curr_byte = Byte{.known = true, .byte = 0};

        for (auto c : sig) {
            if (c == ' ') {
                // We've reached the end of this byte
                chars_processed = 0;
                this->bytes[bytes_processed] = curr_byte;
                bytes_processed++;
                curr_byte = Byte{.known = true, .byte = 0};
            }
            else if (c == '?') {
                // This can support both ? and ??
                curr_byte.known = false;
            }
            else if (curr_byte.known) {
                // Convert this char from hex, and store the nibble in the correct spot in the byte
                uint8_t nibble = char_to_nibble(c);

                if (nibble > 16) {
                    return;
                }

                // the first char is the first half of the byte
                // TODO add compile time errors for incorrect sized bytes in string
                curr_byte.byte |= nibble << ((1 - chars_processed) * 4);
                chars_processed++;
            }

        }

        this->valid = true;
    }

    long scan(std::span<uint8_t> span) const {
        for (long offset = 0; offset < span.size() - sig_size; offset++) {
            bool found = true;
            int sig_byte = 0;
            while (sig_byte < sig_size) {
                if (!this->bytes[sig_byte].known) {
                    sig_byte++;
                }
                else if (span[offset + sig_byte] != this->bytes[sig_byte].byte) {
                    found = false;
                    sig_byte = 0;
                    break;
                }

                sig_byte++;
            }

            if (found) {
                return offset;
            }
        }

        return -1;
    }

    std::array<Byte, sig_size> bytes;
    bool valid = false;
};

constexpr int signature_length(std::string_view sig) {
    int num_bytes = 1;

    // The number of bytes is the number of spaces, plus one
    for (auto c : sig) {
        if (c == ' ') {
            num_bytes++;
        }
    }

    return num_bytes;
}

template<std::size_t size> struct StringLiteral {
    char str[size]{};
    std::size_t len{};

    constexpr StringLiteral(char const(&p)[size]) : len(size) {
        std::copy(p, p + size, str);
    }
};

template<StringLiteral str>
consteval auto operator"" _sig() {
    return Signature<str.len>(str.str);
}

#endif //CHUNI_SCALER_SIGNATURE_H