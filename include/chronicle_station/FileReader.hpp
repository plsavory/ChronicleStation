#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <array>
#include <cstdlib>
#include <algorithm>
#include <iterator>

namespace ChronicleStation {
    class FileReader {
    public:
        MemoryCard* read(const std::string &fileName) {
            // Determine the size of the file
            struct stat fileStat{};
            int fileSize;

            int fileStatus = stat(fileName.c_str(), &fileStat);

            if (fileStatus != 0) {
                // File does not exist
                throw ChronicleStationException("Memory card file does not exist");
            }

            fileSize = (int) fileStat.st_size;

            if (fileSize != 0x20000) {
                throw ChronicleStationException("Unexpected file size (131072k expected)");
            }

            typedef std::istream_iterator<unsigned char> istream_iterator;

            std::ifstream fs(fileName, std::ios::binary);

            fs >> std::noskipws; // Do not skip white space, we want every single character of the file.
            std::copy(istream_iterator(fs), istream_iterator(),
                      std::back_inserter(
                              rawData)); // Copy the contents of the file into the temporary storage vector

            // Ensure that the correct header "MC" is present
            if (rawData[0] != 0x4D || rawData[1] != 0x43) {
                throw ChronicleStationException("Invalid memory card file format");
            }

            // Read the directory frame
            std::array<MemoryCardBlock*, 15> blocks{};

            for (int i = 0; i < 15; i++) {
                blocks[i] = readBlock(i);
            }

            return new MemoryCard(blocks);
        }

    private:

        inline static int getDataOffset(int block, int frame) {
            return (block * 0x2000) + (frame * 0x80);
        }

        static int getDirectoryOffset(int directoryNumber) {
            return getDataOffset(0, 1 + directoryNumber);
        }

        MemoryCardBlock *readBlock(int blockNumber) {
            MemoryCardBlockHeader *blockHeader = getBlockHeader(blockNumber);

            if (blockHeader == nullptr) {
                return nullptr; // Empty/unused block
            }

            return getBlockData(blockHeader);
        }

        MemoryCardBlockHeader *getBlockHeader(int blockNumber) {
            int directoryIndex = getDirectoryOffset(blockNumber);

            MemoryCardBlockHeader::Availability availability;

            switch ((rawData[directoryIndex] & 0xF0) >> 4) {
                case 0xA:
                    availability = MemoryCardBlockHeader::Availability::Available;
                    break;
                case 0x5:
                    availability = MemoryCardBlockHeader::Availability::PartiallyUsed;
                    break;
                case 0xF:
                    availability = MemoryCardBlockHeader::Availability::Unusable;
                    break;
                default:
                    throw ChronicleStationException("Unrecognised directory availability");
            }

            MemoryCardBlockHeader::Type type;

            switch (rawData[directoryIndex] & 0xF) {
                case 0x0:
                    type = MemoryCardBlockHeader::Type::Unused;
                    break;
                case 0x1:
                    type = MemoryCardBlockHeader::Type::NoLink;
                    break;
                case 0x2:
                    type = MemoryCardBlockHeader::Type::MidLink;
                    break;
                case 0x3:
                    type = MemoryCardBlockHeader::Type::TerminatingLink;
                    break;
                case 0xF:
                    type = MemoryCardBlockHeader::Type::Unusable;
                    break;
                default:
                    throw ChronicleStationException("Unrecognised directory type");
            }

            if (type == MemoryCardBlockHeader::Type::Unused) {
                return nullptr;
            }

            bool isReserved;

            unsigned int reservedValue = readBytes(directoryIndex + 0x01, 3);

            switch (reservedValue) {
                case 0x0:
                    isReserved = false;
                    break;
                case 0xFFFFFF:
                    isReserved = true;
                    break;
                default:
                    throw ChronicleStationException("Unknown reserved value");
            }


            // +0x04 - 0x07	Use byte 00 00 00 - Open block middle link block, or end link block Block * 0x2000 - No link, but will be a link (00 20 00 - one blocks will be used) (00 40 00 - two blocks will be used) 00 E0 01 - 15 blocks will be used)
            // TODO is 0x07 unused or a typo in the docs? Need to figure this out by testing a game which uses multiple blocks for a save (Digimon World 2003 maybe?)
            unsigned int useByteData = readBytes(directoryIndex + 0x04, 3);

            unsigned char saveBlockSize;
            bool isALinkBlock = false;

            if ((useByteData & 0xFFFF00) == 0xE000) {
                // Multi block game save with a specified number
                saveBlockSize = useByteData & 0xFF;
            } else {
                switch (useByteData) {
                    case 0x2000:
                        isALinkBlock = true;
                    case 0x0:
                        saveBlockSize = 1;
                        break;
                    case 0x4000:
                        saveBlockSize = 2;
                        break;
                    default:
                        throw ChronicleStationException("Malformed use byte information");
                }
            }

            unsigned char linkOrder = 0;

            unsigned int linkOrderData = readBytes(directoryIndex + 0x08, 2);

            if (linkOrderData == 0xFFFF) {
                linkOrder = 0;
            } else {

                if (linkOrder & 0xFF00) {
                    throw ChronicleStationException("Malformed link order");
                }

                linkOrder = (linkOrderData & 0xFF); // TODO test this with a multi block game
            }

            unsigned int countryCodeData = readBytes(directoryIndex + 0x0A, 2);

            MemoryCardBlockHeader::CountryCode countryCode;

            switch (countryCodeData) { // Probably could've done this by reading as a string, but no big deal.
                case 0x4249:
                    countryCode = MemoryCardBlockHeader::CountryCode::BI;
                    break;
                case 0x4241:
                    countryCode = MemoryCardBlockHeader::CountryCode::BA;
                    break;
                case 0x4245:
                    countryCode = MemoryCardBlockHeader::CountryCode::BE;
                    break;
                default:
                    throw ChronicleStationException("Malformed country code");
            }

            std::string productCode = readString(directoryIndex + 0x0C, 10);

            std::string identifier = readString(directoryIndex + 0x16, 8);

            return new MemoryCardBlockHeader(blockNumber, availability, type, isReserved, saveBlockSize, isALinkBlock,
                                             linkOrder, countryCode, identifier, productCode);
        }

        MemoryCardBlock *getBlockData(MemoryCardBlockHeader *header) {
            int id = header->getBlockNumber();
            int blockNumber = id + 1;

            int dataIndex = getDataOffset(blockNumber, 0);

            // Validate that the block contains the "SC" string at the start of it.
            if (readBytes(dataIndex, 2) != 0x5343) {
                throw ChronicleStationException("Invalid title frame");
            }

            // Determine how many frames the icon has
            unsigned char iconFrames;

            switch (rawData[dataIndex + 0x2]) {
                case 0x0:
                    iconFrames = 0;
                    break;
                case 0x11:
                    iconFrames = 1;
                    break;
                case 0x12:
                    iconFrames = 2;
                    break;
                case 0x13:
                    iconFrames = 3;
                    break;
                default:
                    throw ChronicleStationException("Invalid icon display flag");
            }

            // Determine block number (Probably block number within a save game of multiple blocks? TODO be sure of this)
            unsigned char blockNumberData = rawData[dataIndex + 0x3] & 0xF;

            // Get file name/title (TODO Shift-JIS encoded, not sure how to decode this yet)
            std::array<unsigned char, 0x40> title{};

            for (int i = 0; i < 0x3F; i++) {
                title[i] = rawData[dataIndex + 0x4 + i];
            }

            std::array<unsigned char, 0x20> iconPaletteData{};

            for (int i = 0; i < 0x1F; i++) {
                iconPaletteData[i] = rawData[dataIndex + 0x60 + i];
            }

            // Get icon bitmap data
            std::vector<std::array<unsigned char, 0x80>> iconData;

            iconData.reserve(iconFrames);

            for (int i = 0; i < iconFrames; i++) {
                iconData.push_back(readFrame(blockNumber, i + 1));
            }

            // Get save data
            int saveDataOffset = getDataOffset(blockNumber, 4);

            std::array<unsigned char, 0x1E00> saveData{};

            for (int i = 0; i < 0x1E00; i++) {
                saveData[i] = rawData[saveDataOffset + i];
            }

            return new MemoryCardBlock(header,
                                       blockNumberData,
                                       iconFrames,
                                       title,
                                       iconPaletteData,
                                       iconData,
                                       saveData);
        }

        unsigned int readBytes(int offset, int count) {
            unsigned int output = 0;

            for (int i = 0; i < count; i++) {
                output += (rawData[offset + i] << ((count - 1 - i) * 8));
            }

            return output;
        }

        std::string readString(int offset, int count) {
            std::string output;

            for (int i = 0; i < count; i++) {
                output += (char) rawData[offset + i];
            }

            return output;
        }

        std::array<unsigned char, 0x80> readFrame(int block, int frame) {
            std::array<unsigned char, 0x80> output{};
            int offset = getDataOffset(block, frame);

            for (int i = 0; i < 0x7F; i++) {
                output[i] = rawData[offset + i];
            }

            return output;
        }

        std::vector<unsigned char> rawData;
    };


}