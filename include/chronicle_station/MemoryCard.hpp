//
// Created by Peter Savory on 26/10/2023.
//

#ifndef CHRONICLE_STATION_MEMORY_CARD_H
#define CHRONICLE_STATION_MEMORY_CARD_H


namespace ChronicleStation {
    class MemoryCard {
    public:

        MemoryCard() {
            for (auto & block : blocks) {
                block = nullptr;
            }
        }

        MemoryCard(std::array<MemoryCardBlock*, 15> blocks) {
            for (int i = 0; i < 15; i++) {
                this->blocks[i] = blocks[i];
            }
        }

        ~MemoryCard() {
            for (auto &block : blocks) {

                if (block == nullptr) {
                    continue;
                }

                delete(block);
            }
        }

        void debugPrintInfo() {
            // For testing purposes - Prints information about this memory card
            std::cout<<"Memory Card Block Information: "<<std::endl<<std::endl;

            for (auto &block : blocks) {

                if (block == nullptr) {
                    continue;
                }

                std::string country;

                switch (block->getHeader()->getCountryCode()) {
                    case MemoryCardBlockHeader::CountryCode::BI:
                        country = "Asia";
                        break;
                    case MemoryCardBlockHeader::CountryCode::BA:
                        country = "North America";
                        break;
                    case MemoryCardBlockHeader::CountryCode::BE:
                        country = "Europe";
                        break;
                    case MemoryCardBlockHeader::CountryCode::Undefined:
                        country = "Unknown";
                        break;
                }
                std::cout<<"Block "<<block->getHeader()->getBlockNumber()<<": "<<block->getHeader()->getProductCode()<<" - "<<block->getHeader()->getIdentifier()<< " ("<<country << ")"<<std::endl;
            }
        }

    private:

        MemoryCardBlock* blocks[15]{};
    };
}

#endif // CHRONICLE_STATION_MEMORY_CARD_H
