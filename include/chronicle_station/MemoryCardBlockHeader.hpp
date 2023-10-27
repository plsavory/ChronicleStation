#include <utility>

#ifndef CHRONICLE_STATION_MEMORY_CARD_DIRECTORY_H
#define CHRONICLE_STATION_MEMORY_CARD_DIRECTORY_H

namespace ChronicleStation {

    class BlockHeader {
    public:


    };

    class MemoryCardBlockHeader {
    public:

        enum class Availability {
            Available,
            PartiallyUsed,
            Unusable
        };

        enum class Type {
            Unused,
            NoLink,
            MidLink,
            TerminatingLink,
            Unusable
        };

        enum class CountryCode {
            BI,
            BA,
            BE,
            Undefined
        };

        explicit MemoryCardBlockHeader(int id) {
            this->blockNumber = id;
            availability = Availability::Available;
            type = Type::Unused;
            reserved = false;
            saveBlockSize = 1;
            isALinkBlock = false;
            linkOrder = 0;
            countryCode = CountryCode::Undefined;
            identifier = "";
            productCode = "";
        }

        MemoryCardBlockHeader(int blockNumber,
                              Availability availability,
                              Type type,
                              bool reserved,
                              unsigned char saveBlockSize,
                              bool isALinkBlock,
                              unsigned char linkOrder,
                              CountryCode countryCode,
                              std::string identifier,
                              std::string productCode
        ) {
            this->blockNumber = blockNumber;
            this->availability = availability;
            this->type = type;
            this->reserved = reserved;
            this->saveBlockSize = saveBlockSize;
            this->isALinkBlock = isALinkBlock;
            this->linkOrder = linkOrder;
            this->countryCode = countryCode;
            this->identifier = identifier;
            this->productCode = std::move(productCode);
        }

        int getBlockNumber() {
            return blockNumber;
        }

        CountryCode getCountryCode() {
            return countryCode;
        }

        std::string getIdentifier() {
            return identifier;
        }

        std::string getProductCode() {
            return productCode;
        }

    private:
        unsigned char blockNumber;
        Availability availability;
        Type type;
        bool reserved;
        unsigned char saveBlockSize;
        bool isALinkBlock;
        unsigned char linkOrder;
        CountryCode countryCode;
        std::string identifier;
        std::string productCode;
    };

}

#endif