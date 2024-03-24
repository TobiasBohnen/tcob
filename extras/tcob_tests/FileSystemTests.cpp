#include "tests.hpp"

TEST_CASE("IO.FileSystem.BasicFileOperations")
{
    std::string const file {"test.file"};

    io::delete_file(file);
    REQUIRE_FALSE(io::exists(file));

    io::create_file(file);
    REQUIRE(io::exists(file));
    REQUIRE(io::is_file(file));
    REQUIRE_FALSE(io::is_folder(file));

    io::delete_file(file);
    REQUIRE_FALSE(io::exists(file));
}

TEST_CASE("IO.FileSystem.BasicFolderOperations")
{
    std::string const folder {"testfolder"};

    io::delete_folder(folder);
    REQUIRE_FALSE(io::exists(folder));

    io::create_folder(folder);
    REQUIRE(io::exists(folder));
    REQUIRE(io::is_folder(folder));
    REQUIRE_FALSE(io::is_file(folder));
    REQUIRE(io::is_folder_empty(folder));

    io::delete_folder(folder);
    REQUIRE_FALSE(io::exists(folder));
}

TEST_CASE("IO.FileSystem.Zip")
{
    SUBCASE("folder")
    {
        std::string const folder0 {"zipFolder"};
        std::string const folder1 {"newFolder"};
        std::string const file {"zipFile.zip"};

        io::delete_folder(folder0);
        REQUIRE_FALSE(io::exists(folder0));

        io::delete_folder(folder1);
        REQUIRE_FALSE(io::exists(folder1));

        io::delete_file(file);
        REQUIRE_FALSE(io::exists(file));

        io::create_folder(folder0);
        REQUIRE(io::is_folder(folder0));
        io::ofstream {folder0 + "/test1"}.write("1234");
        io::ofstream {folder0 + "/test2"}.write("4567");
        io::ofstream {folder0 + "/test3"}.write("7890");
        io::ofstream {folder0 + "/test4"}.write("0112");

        REQUIRE(io::zip(folder0, file));

        io::delete_folder(folder0);
        REQUIRE_FALSE(io::exists(folder0));

        REQUIRE(io::unzip(file, ""));
        REQUIRE(io::is_file(folder0 + "/test1"));
        REQUIRE(io::read_as_string(folder0 + "/test1") == "1234");
        REQUIRE(io::is_file(folder0 + "/test2"));
        REQUIRE(io::read_as_string(folder0 + "/test2") == "4567");
        REQUIRE(io::is_file(folder0 + "/test3"));
        REQUIRE(io::read_as_string(folder0 + "/test3") == "7890");
        REQUIRE(io::is_file(folder0 + "/test4"));
        REQUIRE(io::read_as_string(folder0 + "/test4") == "0112");

        REQUIRE(io::unzip(file, folder1));
        REQUIRE(io::is_file(folder1 + "/" + folder0 + "/test1"));
        REQUIRE(io::read_as_string(folder1 + "/" + folder0 + "/test1") == "1234");
        REQUIRE(io::is_file(folder1 + "/" + folder0 + "/test2"));
        REQUIRE(io::read_as_string(folder1 + "/" + folder0 + "/test2") == "4567");
        REQUIRE(io::is_file(folder1 + "/" + folder0 + "/test3"));
        REQUIRE(io::read_as_string(folder1 + "/" + folder0 + "/test3") == "7890");
        REQUIRE(io::is_file(folder1 + "/" + folder0 + "/test4"));
        REQUIRE(io::read_as_string(folder1 + "/" + folder0 + "/test4") == "0112");
    }
    SUBCASE("relative folder")
    {
        std::string const folder0 {"zipFolder2/sub0/sub1/sub2/sub3/"};
        std::string const folder1 {"newFolder2"};
        std::string const file {"zipFile2.zip"};

        io::delete_folder(folder0);
        REQUIRE_FALSE(io::exists(folder0));

        io::delete_folder(folder1);
        REQUIRE_FALSE(io::exists(folder1));

        io::delete_file(file);
        REQUIRE_FALSE(io::exists(file));

        io::create_folder(folder0);
        REQUIRE(io::is_folder(folder0));
        io::ofstream {folder0 + "test01"}.write("1234");
        io::ofstream {folder0 + "test02"}.write("4567");
        io::ofstream {folder0 + "test03"}.write("7890");
        io::ofstream {folder0 + "test04"}.write("0112");

        REQUIRE(io::zip(folder0, file, true));

        REQUIRE(io::unzip(file, folder1));
        REQUIRE(io::is_file(folder1 + "/test01"));
        REQUIRE(io::read_as_string(folder1 + "/test01") == "1234");
        REQUIRE(io::is_file(folder1 + "/test02"));
        REQUIRE(io::read_as_string(folder1 + "/test02") == "4567");
        REQUIRE(io::is_file(folder1 + "/test03"));
        REQUIRE(io::read_as_string(folder1 + "/test03") == "7890");
        REQUIRE(io::is_file(folder1 + "/test04"));
        REQUIRE(io::read_as_string(folder1 + "/test04") == "0112");
    }

    SUBCASE("file")
    {
        std::string const srcFile {"zipFile0"};
        std::string const dstFile {"zipFile.zip"};

        io::delete_file(srcFile);
        REQUIRE_FALSE(io::exists(srcFile));

        io::delete_file(dstFile);
        REQUIRE_FALSE(io::exists(dstFile));

        io::ofstream {srcFile}.write("1234");

        REQUIRE(io::zip(srcFile, dstFile));

        io::delete_file(srcFile);
        REQUIRE_FALSE(io::exists(srcFile));

        REQUIRE(io::unzip(dstFile, ""));
        REQUIRE(io::is_file(srcFile));
        REQUIRE(io::read_as_string(srcFile) == "1234");
    }
    SUBCASE("relative file")
    {
        std::string const srcFolder {"zipFile/sub0/sub1/sub2/"};
        std::string const srcFile {"zipFile1"};
        std::string const dstFile {"zipFile.zip"};

        io::delete_folder(srcFolder);
        REQUIRE_FALSE(io::exists(srcFolder));

        io::delete_file(dstFile);
        REQUIRE_FALSE(io::exists(dstFile));

        io::create_file(srcFolder + srcFile);
        io::ofstream {srcFolder + srcFile}.write("1234");

        REQUIRE(io::zip(srcFolder + srcFile, dstFile, true));

        io::delete_folder(srcFolder);
        REQUIRE_FALSE(io::exists(srcFolder));

        REQUIRE(io::unzip(dstFile, ""));
        REQUIRE(io::is_file(srcFile));
        REQUIRE(io::read_as_string(srcFile) == "1234");
    }
}

TEST_CASE("IO.FileSystem.Enumerate")
{
    SUBCASE("file name")
    {
        std::string const folder {"testfolder1"};

        io::delete_folder(folder);
        REQUIRE_FALSE(io::exists(folder));

        io::create_folder(folder);
        REQUIRE(io::exists(folder));

        std::string const file {folder + "/test.file1"};
        REQUIRE(io::is_folder_empty(folder));
        io::create_file(file);
        REQUIRE_FALSE(io::is_folder_empty(folder));

        auto files {io::enumerate(folder, {file})};
        REQUIRE(files.size() == 1);
        REQUIRE(files.contains(file));
    }

    SUBCASE("pattern 1")
    {
        std::string const folder {"testfolder2"};

        io::delete_folder(folder);
        REQUIRE_FALSE(io::exists(folder));

        io::create_folder(folder);
        REQUIRE(io::exists(folder));

        std::string const file1 {folder + "/test.file1"};
        std::string const file2 {folder + "/test.file2"};
        std::string const file3 {folder + "/test.file3"};

        REQUIRE(io::is_folder_empty(folder));
        io::create_file(file1);
        io::create_file(file2);
        io::create_file(file3);
        REQUIRE_FALSE(io::is_folder_empty(folder));
        REQUIRE(io::exists(file1));
        REQUIRE(io::exists(file2));
        REQUIRE(io::exists(file3));

        auto files {io::enumerate(folder, {"*.file*"})};

        REQUIRE(files.size() == 3);
        REQUIRE(files.contains(file1));
        REQUIRE(files.contains(file2));
        REQUIRE(files.contains(file3));
    }

    SUBCASE("pattern 2")
    {
        std::string const folder {"testfolder3"};

        io::delete_folder(folder);
        REQUIRE_FALSE(io::exists(folder));

        io::create_folder(folder);
        REQUIRE(io::exists(folder));

        std::string const file1 {folder + "/test.phile1"};
        std::string const file2 {folder + "/test.phile2"};
        std::string const file3 {folder + "/test.phile3"};

        REQUIRE(io::is_folder_empty(folder));
        io::create_file(file1);
        io::create_file(file2);
        io::create_file(file3);
        REQUIRE_FALSE(io::is_folder_empty(folder));
        REQUIRE(io::exists(file1));
        REQUIRE(io::exists(file2));
        REQUIRE(io::exists(file3));

        auto files {io::enumerate("/", {"*.phile*"})};

        REQUIRE(files.size() == 3);
        REQUIRE(files.contains(file1));
        REQUIRE(files.contains(file2));
        REQUIRE(files.contains(file3));
    }

    SUBCASE("pattern 3")
    {
        std::string const folder {"testfolder4/sub"};

        io::delete_folder(folder);
        REQUIRE_FALSE(io::exists(folder));

        io::create_folder(folder);
        REQUIRE(io::exists(folder));

        std::string const file1 {folder + "/test.xile1"};
        std::string const file2 {folder + "/test.xile2"};
        std::string const file3 {folder + "/test.xile3"};

        REQUIRE(io::is_folder_empty(folder));
        io::create_file(file1);
        io::create_file(file2);
        io::create_file(file3);
        REQUIRE_FALSE(io::is_folder_empty(folder));
        REQUIRE(io::exists(file1));
        REQUIRE(io::exists(file2));
        REQUIRE(io::exists(file3));

        auto files {io::enumerate("/", {"test.xile*", false})};

        REQUIRE(files.size() == 3);
        REQUIRE(files.contains(file1));
        REQUIRE(files.contains(file2));
        REQUIRE(files.contains(file3));
    }
}
