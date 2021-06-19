// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

#include <vector>

#include <tcob/core/io/FileSystem.hpp>

struct PHYSFS_File;

namespace tcob {
namespace detail::io {
    class FileStream {
    public:
        explicit FileStream(PHYSFS_File* handle)
            : _handle { handle }
        {
            if (!_handle) {
                throw std::runtime_error("failed to aquire file handle");
            }
        }

        virtual ~FileStream()
        {
            if (!_closed)
                close();
        }

        FileStream(const FileStream&) = delete;
        auto operator=(const FileStream& other) -> FileStream& = delete;

        auto close() -> bool;

        auto flush() const -> bool;

        auto eof() const -> bool;

        auto tell() const -> std::streamsize;

        auto length() const -> std::streamsize;

        auto seek(std::streamoff off, std::ios_base::seekdir way) const -> bool;

    protected:
        static auto OpenRead(const std::string& path) -> PHYSFS_File*;
        static auto OpenWrite(const std::string& path) -> PHYSFS_File*;
        static auto OpenAppend(const std::string& path) -> PHYSFS_File*;

        void buffer(u64 size);

        auto read(void* s, std::streamsize n, isize size) const -> std::streamsize;
        auto write(const void* s, std::streamsize n, isize size) const -> std::streamsize;

    private:
        bool _closed { false };
        PHYSFS_File* _handle { nullptr };
    };

    template <typename T>
    class InputFileStream final : public FileStream {
        using FileStream::read;

    public:
        explicit InputFileStream(const std::string& path, u64 bufferSize = 4096)
            : FileStream { OpenRead(path) }
        {
            buffer(bufferSize);
        }

        auto read(T* s, std::streamsize n) const -> std::streamsize
        {
            return read(s, n, sizeof(T));
        }

        auto read() const -> T
        {
            T s;
            read(&s, 1, sizeof(T));
            return s;
        }

        auto read_all() const -> std::vector<T>
        {
            std::vector<T> retValue;
            retValue.reserve(length());

            do {
                std::array<T, 1024> buffer;
                std::streamsize readbytes { read(buffer.data(), sizeof(buffer)) };
                retValue.insert(retValue.end(), buffer.begin(), buffer.begin() + readbytes);
            } while (!eof());

            return retValue;
        }
    };

    template <typename T>
    class OutputFileStream final : public FileStream {
        using FileStream::write;

    public:
        explicit OutputFileStream(const std::string& path)
            : FileStream { OpenWrite(path) }
        {
        }

        auto write(const T s) const -> std::streamsize
        {
            return write(&s, 1, sizeof(T));
        }

        auto write(const std::string& s) const -> std::streamsize
        {
            return write(s.c_str(), s.size(), 1);
        }

        auto write(const T* s, std::streamsize n) const -> std::streamsize
        {
            return write(s, n, sizeof(T));
        }
    };

    template <typename T>
    class AppendFileStream final : public FileStream {
        using FileStream::write;

    public:
        explicit AppendFileStream(const std::string& path)
            : FileStream { OpenAppend(path) }
        {
        }

        auto write(const T s) const -> std::streamsize
        {
            return write(&s, 1, sizeof(T));
        }

        auto write(const std::string& s) const -> std::streamsize
        {
            return write(s.c_str(), s.size(), 1);
        }

        auto write(const T* s, std::streamsize n) const -> std::streamsize
        {
            return write(s, n, sizeof(T));
        }
    };
}

using InputFileStream = detail::io::InputFileStream<byte>;
using OutputFileStream = detail::io::OutputFileStream<byte>;
using AppendFileStream = detail::io::AppendFileStream<byte>;
using InputFileStreamU = detail::io::InputFileStream<ubyte>;
using OutputFileStreamU = detail::io::OutputFileStream<ubyte>;
using AppendFileStreamU = detail::io::AppendFileStream<ubyte>;
}