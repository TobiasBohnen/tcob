#include "tests.hpp"

TEST_CASE("IO.FileSystem.BasicFileOperations")
{
    const string file { "test.file" };

    FileSystem::delete_file(file);
    REQUIRE_FALSE(FileSystem::exists(file));

    FileSystem::create_file(file);
    REQUIRE(FileSystem::exists(file));
    REQUIRE(FileSystem::is_file(file));
    REQUIRE_FALSE(FileSystem::is_folder(file));

    FileSystem::delete_file(file);
    REQUIRE_FALSE(FileSystem::exists(file));
}

TEST_CASE("IO.FileSystem.BasicFolderOperations")
{
    const string folder { "testfolder" };

    FileSystem::delete_folder(folder);
    REQUIRE_FALSE(FileSystem::exists(folder));

    FileSystem::create_folder(folder);
    REQUIRE(FileSystem::exists(folder));
    REQUIRE(FileSystem::is_folder(folder));
    REQUIRE_FALSE(FileSystem::is_file(folder));

    FileSystem::delete_folder(folder);
    REQUIRE_FALSE(FileSystem::exists(folder));
}

TEST_CASE("IO.FileStream.ReadWriteArray")
{
    const string file { "test.file2" };

    FileSystem::delete_file(file);
    REQUIRE_FALSE(FileSystem::exists(file));

    FileSystem::create_file(file);
    REQUIRE(FileSystem::exists(file));

    {
        OutputFileStream fs { file };
        array<char, 5> data { '1', '2', '3', '4', '5' };
        fs.write(data.data(), sizeof(data));
    }
    REQUIRE(FileSystem::filesize(file) == 5);
    {
        InputFileStream fs { file };
        array<char, 5> data;
        fs.read(data.data(), sizeof(data));
        REQUIRE(data == (array<char, 5> { '1', '2', '3', '4', '5' }));
    }

    FileSystem::delete_file(file);
    REQUIRE_FALSE(FileSystem::exists(file));
}

TEST_CASE("IO.FileStream.ReadWriteVector")
{
    const string file { "test.file3" };

    FileSystem::delete_file(file);
    REQUIRE_FALSE(FileSystem::exists(file));

    FileSystem::create_file(file);
    REQUIRE(FileSystem::exists(file));

    {
        OutputFileStream fs { file };
        vector<char> data { '1', '2', '3', '4', '5' };
        fs.write(data.data(), 5);
    }
    REQUIRE(FileSystem::filesize(file) == 5);
    {
        InputFileStream fs { file };
        vector<char> data = fs.read_all();
        REQUIRE(data == (vector<char> { '1', '2', '3', '4', '5' }));
    }

    FileSystem::delete_file(file);
    REQUIRE_FALSE(FileSystem::exists(file));
}

TEST_CASE("IO.FileStream.Seeking")
{
    const string file { "test.file4" };

    FileSystem::delete_file(file);
    REQUIRE_FALSE(FileSystem::exists(file));

    FileSystem::create_file(file);
    REQUIRE(FileSystem::exists(file));

    {
        OutputFileStream fs { file };
        vector<char> data { '1', '2', '3', '4', '5' };
        fs.write(data.data(), 5);
    }
    REQUIRE(FileSystem::filesize(file) == 5);

    {
        InputFileStream fs { file };
        char data { 0 };

        fs.read(&data, 1);
        REQUIRE(data == '1');

        fs.seek(1, std::ios_base::cur);
        fs.read(&data, 1);
        REQUIRE(data == '3');

        fs.seek(-2, std::ios_base::cur);
        fs.read(&data, 1);
        REQUIRE(data == '2');
    }

    {
        InputFileStream fs { file };
        char data { 0 };

        fs.seek(0, std::ios_base::beg);
        fs.read(&data, 1);
        REQUIRE(data == '1');

        fs.seek(2, std::ios_base::beg);
        fs.read(&data, 1);
        REQUIRE(data == '3');

        fs.seek(4, std::ios_base::beg);
        fs.read(&data, 1);
        REQUIRE(data == '5');
    }

    {
        InputFileStream fs { file };
        char data { 0 };

        fs.seek(-1, std::ios_base::end);
        fs.read(&data, 1);
        REQUIRE(data == '5');

        fs.seek(-3, std::ios_base::end);
        fs.read(&data, 1);
        REQUIRE(data == '3');

        fs.seek(-5, std::ios_base::end);
        fs.read(&data, 1);
        REQUIRE(data == '1');
    }

    FileSystem::delete_file(file);
    REQUIRE_FALSE(FileSystem::exists(file));
}