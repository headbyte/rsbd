//
// Created by firstbyte on 02/01/2020.
//

#ifndef RSBD_BINARY_H
#define RSBD_BINARY_H

#include <iostream>


namespace rsbd {

    struct serializable {
        virtual void serialize(std::ostream &output) = 0;

        virtual void deserialize(std::istream &input) = 0;
    };

    struct binary_output_stream {
        std::ostream &stream;

        binary_output_stream(std::ostream &stream) : stream(stream) {}

        template<class T>
        void write(T &data) {
            stream.write((char *) &data, sizeof(data));
        }

        template<class T>
        void write(const T *data, size_t length) {
            stream.write((char *) data, sizeof(T) * length);
        }
    };

    struct binary_input_stream {
        std::istream &stream;

        binary_input_stream(std::istream &stream) : stream(stream) {}


        template<class T>
        void read(T &data) {
            T temp;
            stream.read((char *) &temp, sizeof(data));
            data = temp;
        }

        template<class T>
        void read(T *data, int length) {
            stream.read((char *) data, sizeof(T) * length);
        }

        template<class T>
        void reverse_read(T &data) {
            size_t byte_count = sizeof(T);
            stream.seekg(-byte_count, std::ios_base::cur);
            T temp;
            stream.read((char *) &temp, byte_count);
            data = temp;
            stream.seekg(-byte_count, std::ios_base::cur);
        }

        template<class T>
        void reverse_read(T *data, int length) {
            size_t byte_count = sizeof(T) * length;
            stream.seekg(-byte_count, std::ios_base::cur);
            stream.read((char *) data, byte_count);
            stream.seekg(-byte_count, std::ios_base::cur);
        }
    };

    struct binary_tools {
        // Taken from http://www.i42.co.uk/stuff/hexdump.htm
        template<class E, class T>
        static inline void hex_dump(const void *data, std::size_t length,
                                    std::basic_ostream<E, T> &stream, std::size_t columns = 16) {
            const char *const start = static_cast<const char *>(data);
            const char *const end = start + length;
            const char *line = start;
            while (line != end) {
                stream.width(4);
                stream.fill('0');
                stream << std::hex << line - start << " : ";
                std::size_t lineLength = std::min(columns, static_cast<std::size_t>(end - line));
                for (std::size_t pass = 1; pass <= 2; ++pass) {
                    for (const char *next = line; next != end && next != line + columns; ++next) {
                        char ch = *next;
                        switch (pass) {
                            case 1:
                                stream << ((ch < 32 || ch > 128) ? '.' : ch);
                                break;
                            case 2:
                                if (next != line)
                                    stream << " ";
                                stream.width(2);
                                stream.fill('0');
                                stream << std::hex << std::uppercase
                                       << static_cast<int>(static_cast<unsigned char>(ch));
                                break;
                        }
                    }
                    if (pass == 1 && lineLength != columns)
                        stream << std::string(columns - lineLength, ' ');
                    stream << " ";
                }
                stream << std::endl;
                line = line + lineLength;
            }
        }

    };

}


#endif //RSBD_BINARY_H
