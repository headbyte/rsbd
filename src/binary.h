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

}


#endif //RSBD_BINARY_H
