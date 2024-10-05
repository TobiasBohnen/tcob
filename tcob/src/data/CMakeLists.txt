# data
list(APPEND SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/ConfigFile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CSV.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ConfigSchema.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ConfigTypes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/config_parsers/ConfigParser_bsbd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/config_parsers/ConfigParser_ini.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/config_parsers/ConfigParser_json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/config_parsers/ConfigParser_xml.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/config_parsers/ConfigParser_yaml.cpp
)

list(APPEND HDR
    ${TCOB_INC_DIR}/tcob/data/ConfigFile.hpp
    ${TCOB_INC_DIR}/tcob/data/CSV.hpp
    ${TCOB_INC_DIR}/tcob/data/Config.hpp
    ${TCOB_INC_DIR}/tcob/data/ConfigConversions.hpp
    ${TCOB_INC_DIR}/tcob/data/ConfigSchema.hpp
    ${TCOB_INC_DIR}/tcob/data/ConfigTypes.hpp
    ${TCOB_INC_DIR}/tcob/data/ConfigTypes.inl
)

# sqlite
if(TCOB_ENABLE_ADDON_DATA_SQLITE)
    list(APPEND SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/Sqlite.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SqliteColumn.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SqliteDatabase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SqliteSavepoint.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SqliteStatement.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SqliteTable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SqliteVFS.cpp
    )
    list(APPEND HDR
        ${TCOB_INC_DIR}/tcob/data/Sqlite.hpp
        ${TCOB_INC_DIR}/tcob/data/SqliteConversions.hpp
        ${TCOB_INC_DIR}/tcob/data/SqliteColumn.hpp
        ${TCOB_INC_DIR}/tcob/data/SqliteColumn.inl
        ${TCOB_INC_DIR}/tcob/data/SqliteDatabase.hpp
        ${TCOB_INC_DIR}/tcob/data/SqliteDatabase.inl
        ${TCOB_INC_DIR}/tcob/data/SqliteSavepoint.hpp
        ${TCOB_INC_DIR}/tcob/data/SqliteStatement.hpp
        ${TCOB_INC_DIR}/tcob/data/SqliteStatement.inl
        ${TCOB_INC_DIR}/tcob/data/SqliteTable.hpp
        ${TCOB_INC_DIR}/tcob/data/SqliteTable.inl
    )
endif()

tcob_add_obj_library(tcob_data "${SRC}" "${HDR}")