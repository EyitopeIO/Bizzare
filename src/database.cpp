#include "database.h"
#include "helpers.h"
#include <sqlite3.h>
#include <string>

sqlite3* g_db;

void db_setup(void)
{
    show_info("Setting up database");
    int rc = sqlite3_open("bizzare.db", &g_db);
    if (rc) {
        show_error("Can't open database");
    }

    const char* const create_table =
        "CREATE TABLE network_usage ( "
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp BIGINT NOT NULL, "
        "duration_ms INTEGER NOT NULL, "
        "rx_mb FLOAT NOT NULL, "
        "tx_mb FLOAT NOT NULL "
        " );";

    char* err_msg = nullptr;

    sqlite3_exec(g_db, create_table, nullptr, nullptr, &err_msg);
    if (err_msg) {
        show_error(err_msg);
    }
}

void db_save(uint64_t timestamp, uint64_t duration_ms, float rx_mb, float tx_mb)
{
    std::string insert_query = "INSERT INTO network_usage (timestamp, duration_ms, rx_mb, tx_mb) VALUES (";
    insert_query += std::to_string(timestamp) + ", ";
    insert_query += std::to_string(duration_ms) + ", ";
    insert_query += std::to_string(rx_mb) + ", ";
    insert_query += std::to_string(tx_mb) + ");";

    char* err_msg = nullptr;
    sqlite3_exec(g_db, insert_query.c_str(), nullptr, nullptr, &err_msg);
    if (err_msg) {
        show_warning_cpp(err_msg);
        sqlite3_free(err_msg);
    }
}