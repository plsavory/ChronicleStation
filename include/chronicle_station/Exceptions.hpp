//
// Created by Peter Savory on 26/10/2023.
//

#include <iostream>
#include <exception>

#ifndef CHRONICLE_STATION_EXCEPTIONS_H
#define CHRONICLE_STATION_EXCEPTIONS_H

namespace ChronicleStation {
    class ChronicleStationException : public std::exception {
    public:
        explicit ChronicleStationException(const std::string &message) {
            fullMessage = message;
        };

        const char *what() {
            return fullMessage.c_str();
        }

    private:
        std::string fullMessage;

        using std::exception::what;
    };
}

#endif //CHRONICLE_STATION_EXCEPTIONS_H
