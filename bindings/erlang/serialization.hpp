#pragma once

namespace db
{
    class archive {
    public:
        enum byteorder { order_little, order_big };

        static byteorder system_endian() {
            union {
                uint32_t q; uint8_t s[4];
            } test = { 0x01020304 };
            return test.s[0] == 1 ? order_big : order_little;
        }

        archive() : _order(order_big) {};
        archive(byteorder order) : _order(order) {};
        virtual ~archive(){};

        void set_byteorder(byteorder order) {
            _order = order;
        }

        virtual void serialize(void* data, size_t length) = 0;
        virtual void seek(size_t position) {};
        virtual size_t tell() const { return 0; }
        virtual size_t size() const { return 0; }
        virtual bool good() const { return true; }

        void serialize_ordered(void* data, size_t length, byteorder order) {
            if (_order != order)
                for (ptrdiff_t i = length - 1; i >= 0; i--)
                    serialize((char*)data + i, 1);
            else
                serialize(data, length);
        }

        void serialize_ordered(void* data, size_t length) {
            serialize_ordered(data, length, system_endian());
        }

        template<typename T>
        archive& operator<<(const T& value) {
            serialize_ordered((char*)&value, sizeof(value));
            return *this;
        }
        archive& operator<<(const char* value) {
            (*this) << std::string(value);
            return *this;
        }
        archive& operator<<(const std::vector<char>& value) {
            (*this) << (uint32_t) value.size();
            serialize((void*)value.data(), value.size());
            return *this;
        }
        archive& operator<<(const std::string& value) {
            (*this) << (uint32_t) value.size();
            serialize((void*)value.c_str(), value.size());
            return *this;
        }

        template<typename T>
        archive& operator>>(T& value) {
            serialize_ordered((char*)&value, sizeof(value));
            return *this;
        }
        archive& operator>>(std::vector<char>& value) {
            uint32_t size; (*this) >> size;
            if (good()) {
                value.resize(size);
                serialize((void*)value.data(), value.size());
            } return *this;
        }
        archive& operator>>(std::string& value) {
            std::vector<char> storage;
            (*this) >> storage;
            value.assign(storage.begin(), storage.end());
            return *this;
        }

    private:
        byteorder _order;
    };

    class parameters : public archive {
        std::vector<char> _buffer;
        size_t _position;
        bool _failure;

    public:
        void serialize(void* data, size_t length) {
            size_t remaining = _buffer.size() - _position;
            _failure = length > remaining;
            if (!_failure) {
                memcpy(data, _buffer.data() + _position, length);
                _position += length;
            }
        }

        parameters(void* data, size_t length) :
            _position(0),
            _failure(false)
        {
            _buffer.resize(length);
            memcpy(_buffer.data(), data, length);
        }

        parameters(std::vector<char> data) :
            _buffer(std::move(data)),
            _position(0),
            _failure(false)
        {};

        void seek(size_t position) {
            size_t size = _buffer.size();
            _position = position < size ? position : size;
            _failure = false;
        }

        size_t tell() const {
            return _position;
        }

        size_t size() const {
            return _buffer.size();
        }

        bool good() const {
            return !_failure;
        }
    };

    class writer : public archive {
        std::vector<char> _buffer;

    public:
        void serialize(void* data, size_t length) {
            size_t size = _buffer.size();
            _buffer.resize(size + length);
            memcpy(_buffer.data() + size, data, length);
        }

        writer() {};
        writer(byteorder order) : archive(order) {};
        const char* data() const { return _buffer.data(); }
        char* data() { return _buffer.data(); }
        size_t size() const { return _buffer.size(); }
    };
}
