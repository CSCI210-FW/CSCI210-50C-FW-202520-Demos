#include <iostream>
#include <string>
#include <sqlite3.h>
#include <limits>

void resetStream();
void viewAssignmentsByProject(sqlite3 *db);

int main()
{
    sqlite3 *db;
    int rc = sqlite3_open_v2("ConstructCo.db", &db, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
    {
        std::cout << "Error opening Database: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_close(db);
    return 0;
}

void resetStream()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void viewAssignmentsbyProject(sqlite3 *db)
{
    std::string query = "select * from project";
    sqlite3_stmt *result;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &result, NULL);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(result);
        std::cout << "There was an error with the project query: " << sqlite3_errmsg(db) << std::endl;
        std::cout << query << std::endl;
        return;
    }

    sqlite3_finalize(result);
}