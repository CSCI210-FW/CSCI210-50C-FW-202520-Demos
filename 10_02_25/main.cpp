#include <sqlite3.h>
#include <iostream>
#include <limits>
#include <string>

void makeSale(sqlite3 *db);
int startTransaction(sqlite3 *db);
void resetStream();
int commit(sqlite3 *db);
int rollback(sqlite3 *db);
bool checkForError(int rc, sqlite3 *db, sqlite3_stmt *r, std::string msg);
int pickCustomer(sqlite3 *db);
int createInvoice(sqlite3 *db, int customer);
int addCustomer(sqlite3 *db);
int getInteger(int low, int high);

int main()
{
    sqlite3 *saleCo;
    int returnCode;
    returnCode = sqlite3_open_v2("SaleCo.db", &saleCo, SQLITE_OPEN_READWRITE, NULL);
    if (returnCode != SQLITE_OK)
    {
        std::cout << "Error opening database: " << sqlite3_errmsg(saleCo) << std::endl;
        sqlite3_close(saleCo);
        return 0;
    }
    returnCode = sqlite3_exec(saleCo, "pragma foreign_keys;", NULL, NULL, NULL);
    makeSale(saleCo);
}
void resetStream()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

/*
    steps for making a sale
    todo You must write a new customer invoice.
        //todo pick a customer or
        todo add new customer
        todo create a new invoice using the cust_number
        todo add products to the invoice
            todo pick a product
            todo add line record for that product
    todo You must reduce the quantity on hand in the product’s inventory.
    todo You must update the account transactions.
        todo add the charge
        todo ask about payment
        optional todo add payment record
    todo You must update the customer balance.
*/

void makeSale(sqlite3 *db)
{
    int rc = startTransaction(db);
    if (rc != SQLITE_OK)
    {
        rollback(db);
        return;
    }
    int custNumber = pickCustomer(db);
    if (custNumber == -1)
    {
        std::cout << "No customer infomation available" << std::endl;
        rollback(db);
        return;
    }
    rollback(db);
    // int invNumber = createInvoice(db, custNumber);
    // if (invNumber == -1)
    //{
    //     std::cout << "Error inserting a new invoice" << std::endl;
    //     rollback(db);
    //     return;
    // }
}

int startTransaction(sqlite3 *db)
{
    int returnCode;
    returnCode = sqlite3_exec(db, "begin transaction", NULL, NULL, NULL);
    if (returnCode != SQLITE_OK)
    {
        std::cout << "There was an error in start transaction: "
                  << sqlite3_errmsg(db) << std::endl;
        return returnCode;
    }
    return SQLITE_OK;
}

int commit(sqlite3 *db)
{
    int rc = sqlite3_exec(db, "commit", NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        std::cout << "There was an error - committing the transaction: "
                  << sqlite3_errmsg(db) << std::endl;
        rollback(db);
        return rc;
    }
    return SQLITE_OK;
}

int rollback(sqlite3 *db)
{
    int rc = sqlite3_exec(db, "rollback", NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        std::cout << "There was an error - rolling back the transaction: "
                  << sqlite3_errmsg(db) << std::endl;
        return rc;
    }
    return SQLITE_OK;
}

int pickCustomer(sqlite3 *db)
{
    const int CUST_NUMBER = 0;
    const int CUST_NAME = 1;
    std::string query = "select cust_number, cust_lname || ', ' || cust_fname as name\n"
                        "from customer \n";
    sqlite3_stmt *result;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &result, NULL);
    if (checkForError(rc, db, result, "There was an error with the customer query: "))
    {
        std::cout << query << std::endl;
        return -1;
    }
    int count = 1;
    int choice = 0;
    std::cout << "Please choose a customer: " << std::endl;
    while (sqlite3_step(result) == SQLITE_ROW)
    {
        std::cout << count << ". " << sqlite3_column_text(result, CUST_NUMBER);
        std::cout << " - " << sqlite3_column_text(result, CUST_NAME);
        std::cout << std::endl;
        count++;
    }
    std::cout << count << ". Add New Customer" << std::endl;
    choice = getInteger(1, count);
    if (choice == count)
    {
        sqlite3_finalize(result);
        return addCustomer(db);
    }
    sqlite3_reset(result);
    for (int i = 0; i < choice; i++)
    {
        sqlite3_step(result);
    }
    int custNumber = sqlite3_column_int(result, CUST_NUMBER);
    sqlite3_finalize(result);
    return custNumber;
}

int addCustomer(sqlite3 *db)
{
    return 10019;
}

int getInteger(int low, int high)
{
    int retInt;
    std::cin >> retInt;
    while (!std::cin || retInt < low || retInt > high)
    {
        if (!std::cin)
            resetStream();
        std::cout << "That is not a valid choice! Try again!" << std::endl;
        std::cin >> retInt;
    }
    return retInt;
}

bool checkForError(int rc, sqlite3 *db, sqlite3_stmt *r, std::string msg)
{
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(r);
        std::cout << msg << " " << sqlite3_errmsg(db) << std::endl;
        return true;
    }
    return false;
}
