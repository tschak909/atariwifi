#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <memory>
#include <vector>
#include "../../include/debug.h"
#include "qrmanager.h"
#include "qrcode.h"

QRManager qrManager;

std::vector<uint8_t> QRManager::encode(const void* src, size_t len, size_t version, size_t ecc, size_t *out_len) {
    QRCode qr_code;
    uint8_t qr_bytes[qrcode_getBufferSize(version)];

    uint8_t err = qrcode_initText(&qr_code, qr_bytes, version, ecc, (const char*)src);

    size_t size = qr_code.size;
    *out_len = size*size;

    qrManager.out_buf.clear();
    qrManager.out_buf.shrink_to_fit();

    if (err == 0) {
        for (uint8_t x = 0; x < size; x++) {
            for (uint8_t y = 0; y < size; y++) {
                uint8_t on = qrcode_getModule(&qr_code, x, y);
                qrManager.out_buf.push_back(on);
            }
        }
    }

    qrManager.version = version;
    qrManager.ecc_mode = ecc;

    return qrManager.out_buf;
}

void QRManager::to_binary(void) {
    auto bytes = qrManager.out_buf;
    size_t len = bytes.size();
    std::vector<uint8_t> out;

    uint8_t val = 0;
    for (auto i = 0; i < len; i++) {
        auto bit = i % 8;
        if (bit == 0 && i > 0) {
            out.push_back(val);
            val = 0;
        }
        val |= bytes[i+bit] << bit;
    }
    out.push_back(val);

    qrManager.out_buf = out;
}

void QRManager::to_bitmap(void) {
    auto bytes = qrManager.out_buf;
    size_t size = 17 + 4 * qrManager.version;
    size_t len = bytes.size();
    size_t bytes_per_row = ceil(size / 8.0);
    std::vector<uint8_t> out;

    uint8_t val = 0;
    uint8_t x = 0;
    uint8_t y = 0;
    for (auto i = 0; i < len; i++) {
        val |= bytes[i];
        x++;
        if (x == size) {
            val = val << (bytes_per_row * 8 - x);
            out.push_back(val);
            val = 0;
            x = 0;
            y++;
        }
        else if (x % 8 == 0 && i > 0) {
            out.push_back(val);
            val = 0;
        }
        else {
            val = val << 1;
        }
        if (y == size) {
            break;
        }
    }

    qrManager.out_buf = out;
}

/*
This collapses each 2x2 groups of modules (pixels) into a single ATASCII character.
Values for ATASCII look up are as calculated as follows. Note that 6 and 9 have no
suitable ATASCII character, so diagonal lines are used instead. These seem to work
in most cases, but for best results you may want to use custom characters for those.

0    1    2    3    4    5    6*   7    8    9*   10   11   12   13   14   15
- -  x -  - x  x x  - -  x -  - x  x x  - -  x -  - x  x x  - -  x -  - x  x x
- -  - -  - -  - -  x -  x -  x -  x -  - x  - x  - x  - x  x x  x x  x x  x x
32   12   11   149  15   25  (11)  137  9   (12)  153  143  21   139  140  160
               [21]           ?    [9]       ?    [25] [15]      [11] [12] [32]
*/
//                     0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15
uint8_t atascii[16] = {32,  12,  11,  149, 15,  25,  6,   137, 9,   7,   153, 143, 21,  139, 140, 160};

void QRManager::to_atascii(void) {
    auto bytes = qrManager.out_buf;
    size_t size = qrManager.size();
    std::vector<uint8_t> out;

    for (auto y = 0; y < size; y += 2) {
        for (auto x = 0; x < size; x += 2) {
            uint8_t val = bytes[y*size+x];
            // QR Codes have odd number of rows/columns, so last ATASCII char is only half full
            if (x+1 < size) val |= bytes[y*size+x+1] << 1;
            if (y+1 < size) val |= bytes[(y+1)*size+x] << 2;
            if (y+1 < size && x+1 < size) val |= bytes[(y+1)*size+x+1] << 3;
            out.push_back(atascii[val]);
        }
        out.push_back(155); // Atari newline
    }

    qrManager.out_buf = out;
}
