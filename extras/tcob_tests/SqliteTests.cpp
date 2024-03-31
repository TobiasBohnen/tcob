#include "tests.hpp"

using namespace tcob::data::sqlite;

struct foo {
    i32         ID;
    std::string Name;
    i32         Age;
    float       Height;
    bool        Alive;

    auto operator==(foo const& lhs) const -> bool
    {
        return ID == lhs.ID && Name == lhs.Name && Age == lhs.Age && Height == lhs.Height && Alive == lhs.Alive;
    }
};

TEST_CASE("Data.Sqlite.Select")
{
    std::string tableName {"testTable"};

    database db {database::OpenMemory()};
    auto     dbTable {db.create_table(tableName,
                                      column {"ID", type::Integer, true, primary_key {}},
                                      column {"Name", type::Text, true},
                                      column {"Age"},
                                      column {"Height"},
                                      column {"Alive"})};
    REQUIRE(dbTable);
    std::vector<std::tuple<i32, std::string, i32, f32, bool>> vec;
    for (i32 i {1}; i <= 100; ++i) {
        vec.emplace_back(i, std::to_string(i), i * 100, static_cast<f32>(i) * 1.5f, i % 2 == 0);
    }

    REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height", "Alive")(vec));

    SUBCASE("select")
    {
        {
            auto rows = dbTable->select_from<i32, std::string, i32, f32, bool>()();
            REQUIRE(rows.size() == 100);
            REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
            REQUIRE(rows[1] == std::tuple {2, "2", 200, 3.0f, true});
        }
        {
            auto rows = dbTable->select_from<std::string, i32, f32>("Name", "Age", "Height")();
            REQUIRE(rows.size() == 100);
            REQUIRE(rows[0] == std::tuple {"1", 100, 1.5f});
            REQUIRE(rows[1] == std::tuple {"2", 200, 3.0f});
        }
    }
    SUBCASE("where")
    {
        {
            auto rows = dbTable->select_from<i32, std::string, i32, f32, bool>().where("Age = 100")();
            REQUIRE(rows.size() == 1);
            REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
        }
        {
            auto rows = dbTable->select_from<i32, std::string, i32, f32, bool>().where("Age = ?")(500);
            REQUIRE(rows.size() == 1);
            REQUIRE(rows[0] == std::tuple {5, "5", 500, 7.5f, false});
        }
    }
    SUBCASE("order_by")
    {
        auto rows = dbTable->select_from<i32, std::string, i32, f32, bool>().order_by("Name DESC")();
        REQUIRE(rows.size() == 100);
        REQUIRE(rows[0] == std::tuple {99, "99", 9900, 148.50f, false});
        REQUIRE(rows[1] == std::tuple {98, "98", 9800, 147.00f, true});
    }
    SUBCASE("limit")
    {
        auto rows = dbTable->select_from<i32, std::string, i32, f32, bool>().limit(1)();
        REQUIRE(rows.size() == 1);
        REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
    }
    SUBCASE("limit offset")
    {
        auto rows = dbTable->select_from<i32, std::string, i32, f32, bool>("ID", "Name", "Age", "Height", "Alive").limit(1, 1)();
        REQUIRE(rows.size() == 1);
        REQUIRE(rows[0] == std::tuple {2, "2", 200, 3.0f, true});
    }
    SUBCASE("distinct")
    {
        auto rows = dbTable->select_from<bool>(distinct, "Alive")();
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == false);
        REQUIRE(rows[1] == true);
    }
    SUBCASE("tuple to type")
    {
        std::vector<foo> rows = dbTable->select_from<i32, std::string, i32, f32, bool>();
        REQUIRE(rows.size() == 100);
        REQUIRE(rows[0] == foo {1, "1", 100, 1.5f, false});
        REQUIRE(rows[1] == foo {2, "2", 200, 3.0f, true});
    }
}

TEST_CASE("Data.Sqlite.Aggregate")
{
    std::string tableName {"testTable"};

    database db {database::OpenMemory()};
    auto     dbTable {db.create_table(tableName,
                                      column {"ID", type::Integer, true, primary_key {}},
                                      column {"Name", type::Text, true},
                                      column {"Age"})};
    REQUIRE(dbTable);

    std::vector<std::tuple<std::string, int>> vec {};
    vec.emplace_back("A", 100);
    vec.emplace_back("A", 110);
    vec.emplace_back("A", 120);
    vec.emplace_back("A", 130);
    vec.emplace_back("B", 200);
    vec.emplace_back("B", 210);
    vec.emplace_back("B", 220);
    vec.emplace_back("B", 230);
    vec.emplace_back("B", 240);

    REQUIRE(dbTable->insert_into("Name", "Age")(vec));

    SUBCASE("group_by")
    {
        auto rows = dbTable->select_from<i32>(sum("Age")).group_by("Name")();
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == (100 + 110 + 120 + 130));
        REQUIRE(rows[1] == (200 + 210 + 220 + 230 + 240));
    }
}

TEST_CASE("Data.Sqlite.Insert")
{
    std::string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName,
                                  column {"ID", type::Integer},
                                  column {"Name", type::Text},
                                  column {"Age", type::Integer},
                                  column {"Height", type::Integer})};
    REQUIRE(dbTable);

    SUBCASE("tuples")
    {
        auto tup0 = std::tuple {1, "A", 100, 1.5f};
        auto tup1 = std::tuple {2, "B", 200, 4.5f};

        REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup0, tup1));
        auto rows = dbTable->select_from<i32, std::string, i32, f32>("ID", "Name", "Age", "Height")();
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == tup0);
        REQUIRE(rows[1] == tup1);
    }
    SUBCASE("vector of tuples")
    {
        auto vec = std::vector {std::tuple {1, "A", 100, 1.5f}, std::tuple {2, "B", 200, 4.5f}};
        REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(vec));

        auto rows = dbTable->select_from<i32, std::string, i32, f32>("ID", "Name", "Age", "Height")();
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == vec[0]);
        REQUIRE(rows[1] == vec[1]);
    }
    SUBCASE("values")
    {
        REQUIRE(dbTable->insert_into("Age")(100, 200));
        auto rows = dbTable->select_from<i32>("Age")();
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == 100);
        REQUIRE(rows[1] == 200);
    }
}

TEST_CASE("Data.Sqlite.Update")
{
    std::string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName,
                                  column {"ID", type::Integer, true, primary_key {}},
                                  column {"Name", type::Text, true},
                                  column {"Age"},
                                  column {"Height"})};
    REQUIRE(dbTable);

    auto tup0 = std::tuple {1, "A", 100, 1.5f};
    auto tup1 = std::tuple {2, "B", 200, 4.5f};

    REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup0, tup1));
    REQUIRE(dbTable->update("Age", "Height").where("ID = 2")(100, 4.2));
    {
        auto rows = dbTable->select_from<i32, std::string, i32, f32>("ID", "Name", "Age", "Height")();
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == tup0);
        REQUIRE(rows[1] == std::tuple {2, "B", 100, 4.2f});
    }
}

TEST_CASE("Data.Sqlite.Delete")
{
    std::string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName,
                                  column {"ID", type::Integer, true, primary_key {}},
                                  column {"Name", type::Text, true},
                                  column {"Age"},
                                  column {"Height"})};
    REQUIRE(dbTable);

    auto tup0 = std::tuple {1, "A", 100, 1.5f};
    auto tup1 = std::tuple {2, "B", 200, 4.5f};

    REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup0, tup1));
    REQUIRE(dbTable->get_row_count() == 2);
    REQUIRE(dbTable->delete_from().where("ID = 1")());
    REQUIRE(dbTable->get_row_count() == 1);
}

TEST_CASE("Data.Sqlite.CreateTable")
{
    std::string tableName0 = "testTable0";
    std::string tableName1 = "testTable1";

    database db {database::OpenMemory()};

    REQUIRE(db.create_table(tableName0, column {"ID", type::Integer, true, primary_key {}}));

    REQUIRE(db.get_table_names().size() == 1);
    REQUIRE(db.get_table_names().contains(tableName0));

    REQUIRE(db.create_table(tableName1,
                            column {"ID", type::Integer, false, primary_key {}},
                            column {"Name", type::Text, true},
                            column {"Age"},
                            column {"Height"}));
    auto nameSet {db.get_table(tableName1)->get_column_names()};
    for (auto const& col : std::array {"ID", "Name", "Age", "Height"}) {
        REQUIRE(nameSet.contains(col));
    }

    REQUIRE(db.get_table_names().size() == 2);
    REQUIRE(db.get_table_names().contains(tableName0));
    REQUIRE(db.get_table_names().contains(tableName1));

    REQUIRE(db.table_exists(tableName0));
    REQUIRE(db.table_exists(tableName1));
    REQUIRE_FALSE(db.table_exists("foobar"));
}

TEST_CASE("Data.Sqlite.DropTable")
{
    std::string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName, column {"ID", type::Integer, true, primary_key {}})};
    REQUIRE(db.table_exists(tableName));

    REQUIRE(db.get_table_names().size() == 1);
    REQUIRE(db.get_table_names().contains(tableName));

    REQUIRE(db.drop_table(tableName));

    REQUIRE_FALSE(db.table_exists(tableName));
    REQUIRE(db.get_table_names().size() == 0);
}

TEST_CASE("Data.Sqlite.VacuumInto")
{
    std::string tableName {"testTable"};
    std::string fileName {"test0.db"};
    {
        database db {database::OpenMemory()};

        REQUIRE(db.create_table(tableName, column {"ID", type::Integer, true, primary_key {}}));

        io::delete_file(fileName);
        REQUIRE_FALSE(io::exists(fileName));
        REQUIRE(db.vacuum_into(fileName));
    }
    {
        REQUIRE(io::exists(fileName));
        auto db {database::Open(fileName)};
        REQUIRE(db);
        REQUIRE(db->table_exists(tableName));
    }
}

TEST_CASE("Data.Sqlite.Blobs")
{
    std::string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName,
                                  column {"ID", type::Integer, true, primary_key {}},
                                  column {"Point", type::Blob},
                                  column {"Size", type::Blob},
                                  column {"Rect", type::Blob},
                                  column {"Color", type::Blob})};
    REQUIRE(db.table_exists(tableName));
    auto tup0 = std::tuple {1, point_f {4.2f, 2.4f}, size_u {420, 69}, rect_i {-5, 5, 20, 9}, colors::Aqua};
    auto tup1 = std::tuple {2, point_f {6.9f, 9.6f}, size_u {123, 456}, rect_i {5, -5, -20, 9}, colors::RebeccaPurple};

    REQUIRE(dbTable->insert_into("ID", "Point", "Size", "Rect", "Color")(tup0, tup1));
    {
        auto rows = dbTable->select_from<i32, point_f, size_u, rect_i, color>()();
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == tup0);
        REQUIRE(rows[1] == tup1);
    }
}

TEST_CASE("Data.Sqlite.NullAndOptional")
{
    std::string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName,
                                  column {"ID", type::Integer, true, primary_key {}},
                                  column {"Age", type::Integer, false})};
    REQUIRE(db.table_exists(tableName));

    auto tup0 = std::tuple {100};
    auto tup1 = std::tuple {nullptr};
    auto tup2 = std::tuple {std::nullopt};

    REQUIRE(dbTable->insert_into("Age")(tup0, tup1, tup2));
    {
        auto rows = dbTable->select_from<std::optional<i32>>("Age")();
        REQUIRE(rows.size() == 3);
        REQUIRE(rows[0] == 100);
        REQUIRE(rows[1] == std::nullopt);
        REQUIRE(rows[2] == std::nullopt);
    }
}

TEST_CASE("Data.Sqlite.Views")
{
    std::string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName,
                                  column {"ID", type::Integer, true, primary_key {}},
                                  column {"A", type::Integer},
                                  column {"B", type::Integer},
                                  column {"C", type::Integer},
                                  column {"D", type::Integer})};
    REQUIRE(db.table_exists(tableName));

    std::vector<std::tuple<i32, i32, i32, i32>> values;
    rng                                         r {54321};
    for (i32 i {0}; i < 650; i++) {
        values.emplace_back(r(0, 100), r(0, 100), r(0, 100), r(0, 100));
    }

    REQUIRE(dbTable->insert_into("A", "B", "C", "D")(values));
    {
        auto select {std::move(dbTable->select_from<i32, i32, i32, i32>("A", "B", "C", "D").where("A > 50"))};
        auto rows {select()};

        auto dbView {db.create_view("testView", select)};
        REQUIRE(dbView.has_value());
        REQUIRE(db.view_exists("testView"));
        if (dbView) {
            auto viewrows {dbView->select_from<i32, i32, i32, i32>()()};
            REQUIRE(viewrows.size() == rows.size());
        }
    }
}

TEST_CASE("Data.Sqlite.Join")
{
    std::string tableName0 = "People";
    std::string tableName1 = "Countries";

    database db {database::OpenMemory()};

    SUBCASE("LeftJoin")
    {
        {
            auto dbTable {db.create_table(tableName0,
                                          column {"ID", type::Integer, true, primary_key {}},
                                          column {"Name", type::Text},
                                          column {"CountryID", type::Integer})};
            REQUIRE(db.table_exists(tableName0));

            auto tup0 = std::tuple {"Peter", 1};
            auto tup1 = std::tuple {"Paul", 1};
            auto tup2 = std::tuple {"Marie", 2};

            REQUIRE(dbTable->insert_into("Name", "CountryID")(tup0, tup1, tup2));
        }
        {
            auto dbTable {db.create_table(tableName1,
                                          column {"ID", type::Integer, true, primary_key {}},
                                          column {"Code", type::Text})};
            REQUIRE(db.table_exists(tableName1));

            auto tup0 = std::tuple {1, "UK"};
            auto tup1 = std::tuple {2, "USA"};

            REQUIRE(dbTable->insert_into("ID", "Code")(tup0, tup1));
        }

        auto rows = db.get_table(tableName0)->select_from<std::string, std::string>("Name", "Code").left_join(tableName1, "People.CountryID = Countries.ID")();
        REQUIRE(rows.size() == 3);
        REQUIRE(rows[0] == std::tuple {"Peter", "UK"});
        REQUIRE(rows[1] == std::tuple {"Paul", "UK"});
        REQUIRE(rows[2] == std::tuple {"Marie", "USA"});
    }
    SUBCASE("InnerJoin")
    {
        {
            auto dbTable {db.create_table(tableName0,
                                          column {"ID", type::Integer, true, primary_key {}},
                                          column {"Name", type::Text},
                                          column {"CountryID", type::Integer})};
            REQUIRE(db.table_exists(tableName0));

            auto tup0 = std::tuple {"Peter", 1};
            auto tup1 = std::tuple {"Paul", 1};
            auto tup2 = std::tuple {"Marie", 2};

            REQUIRE(dbTable->insert_into("Name", "CountryID")(tup0, tup1, tup2));
        }
        {
            auto dbTable {db.create_table(tableName1,
                                          column {"ID", type::Integer, true, primary_key {}},
                                          column {"Code", type::Text})};
            REQUIRE(db.table_exists(tableName1));

            auto tup0 = std::tuple {1, "UK"};

            REQUIRE(dbTable->insert_into("ID", "Code")(tup0));
        }

        auto rows = db.get_table(tableName0)
                        ->select_from<std::string, std::string>("Name", "Code")
                        .inner_join(tableName1, "People.CountryID = Countries.ID")();
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == std::tuple {"Peter", "UK"});
        REQUIRE(rows[1] == std::tuple {"Paul", "UK"});
    }
    SUBCASE("CrossJoin")
    {
        {
            REQUIRE(db.create_table(tableName0, column {"Color", type::Text}));
            REQUIRE(db.get_table(tableName0)->insert_into("Color")("Green", "Blue", "Yellow"));
        }
        {
            REQUIRE(db.create_table(tableName1, column {"Number", type::Integer}));
            REQUIRE(db.get_table(tableName1)->insert_into("Number")(1, 2));
        }

        auto rows = db.get_table(tableName0)->select_from<std::string, i32>("Color", "Number").cross_join(tableName1)();
        REQUIRE(rows.size() == 6);
        REQUIRE(rows[0] == std::tuple {"Green", 1});
        REQUIRE(rows[1] == std::tuple {"Green", 2});
        REQUIRE(rows[2] == std::tuple {"Blue", 1});
        REQUIRE(rows[3] == std::tuple {"Blue", 2});
        REQUIRE(rows[4] == std::tuple {"Yellow", 1});
        REQUIRE(rows[5] == std::tuple {"Yellow", 2});
    }
}

TEST_CASE("Data.Sqlite.SavePoint")
{
    std::string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName,
                                  column {"ID", type::Integer},
                                  column {"Name", type::Text},
                                  column {"Age", type::Integer},
                                  column {"Height", type::Integer})};
    REQUIRE(dbTable);

    auto tup0 = std::tuple {1, "A", 100, 1.5f};
    REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup0));
    auto rows = dbTable->select_from<i32, std::string, i32, f32>("ID", "Name", "Age", "Height")();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0] == tup0);

    {
        auto sp = db.create_savepoint("t1");

        auto tup1 = std::tuple {2, "B", 200, 4.5f};
        REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup1));
        rows = dbTable->select_from<i32, std::string, i32, f32>("ID", "Name", "Age", "Height")();
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == tup0);
        REQUIRE(rows[1] == tup1);
    }

    rows = dbTable->select_from<i32, std::string, i32, f32>("ID", "Name", "Age", "Height")();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0] == tup0);
}
