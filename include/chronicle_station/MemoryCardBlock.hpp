//
// Created by Peter Savory on 26/10/2023.
//

#ifndef CHRONICLE_STATION_MEMORY_CARD_BLOCK_H
#define CHRONICLE_STATION_MEMORY_CARD_BLOCK_H

#include <vector>

namespace ChronicleStation {

    class MemoryCardBlock {
    public:

        explicit MemoryCardBlock(int id) {
            header = new MemoryCardBlockHeader(id);
            blockNumber = 0;
            iconFrames = 0;

            for (int i = 0; i < 0x40; i++) {
                title[i] = 0;
            }

            for (int i = 0; i < 0x20; i++) {
                iconPaletteData[i] = 0;
            }

            for (int i = 0; i < 0x1E00; i++) {
                saveData[i] = 0;
            }
        }

        MemoryCardBlock(MemoryCardBlockHeader *header,
                        unsigned char blockNumber,
                        unsigned char iconFrames,
                        std::array<unsigned char, 0x40> title,
                        std::array<unsigned char, 0x20> iconPaletteData,
                        std::vector<std::array<unsigned char, 0x80>> iconData,
                        std::array<unsigned char, 0x1E00> saveData) {
            this->header = header;
            this->blockNumber = blockNumber;
            this->iconFrames = iconFrames;
            this->title = title;
            this->iconPaletteData = iconPaletteData;
            this->iconData = iconData;
            this->saveData = saveData;
        }

        ~MemoryCardBlock() {
            delete (header);
        }

        MemoryCardBlockHeader* getHeader() {
            return header;
        }

    private:

        unsigned char blockNumber;
        unsigned char iconFrames;
        std::array<unsigned char, 0x40> title{};
        std::array<unsigned char, 0x20> iconPaletteData{};
        std::vector<std::array<unsigned char, 0x80>> iconData;
        std::array<unsigned char, 0x1E00> saveData{};
        MemoryCardBlockHeader *header;
    };

}

#endif //CHRONICLE_STATION_MEMORY_CARD_BLOCK_H
