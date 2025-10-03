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
void pickProduct(sqlite3 *db, std::string &prodCode, int &qoh, double &price);
int createLine(sqlite3 *, int, std::string, int, double, int);
int updateProduct(sqlite3 *, std::string, int);

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
    //todo You must write a new customer invoice.
        //todo pick a customer or
        //todo add new customer
        //todo create a new invoice using the cust_number
        //todo add products to the invoice
            //todo pick a product
           // todo add line record for that product
   // todo You must reduce the quantity on hand in the product’s inventory.
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

    int invNumber = createInvoice(db, custNumber);
    if (invNumber == -1)
    {
        std::cout << "Error inserting a new invoice" << std::endl;
        rollback(db);
        return;
    }

    char more = 'y';
    int lineNum = 1;
    double total = 0;
    do
    {
        std::string productCode;
        int qoh;
        double price;
        pickProduct(db, productCode, qoh, price);
        if (productCode == "error")
        {
            std::cout << "Error picking product." << std::endl;
            rollback(db);
            return;
        }
        else if (productCode == "cancel")
        {
            std::cout << "Canceling Sale..." << std::endl;
            rollback(db);
            return;
        }
        int qty = createLine(db, invNumber, productCode, qoh, price, lineNum);
        if (qty == -2)
        {
            lineNum--;
        }
        else if (qty == -1)
        {
            rollback(db);
            std::cout << "Error creating line." << std::endl;
            return;
        }
        else
        {
            total += qty * price;
            int rc = updateProduct(db, productCode, qty);
            if (rc == -1)
            {
                rollback(db);
                std::cout << "Error updating product." << std::endl;
                return;
            }
        }
        std::cout << "Would you like another product? ";
        std::cin >> more;
        std::cout << std::endl;
        lineNum++;

    } while (more == 'y');

    commit(db);
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

int createInvoice(sqlite3 *db, int customer)
{
    std::string query = "insert into invoice (cust_number, inv_date) values (@customer, @date)";
    char formatDate[80];
    time_t currentDate = time(NULL);
    strftime(formatDate, 80, "%F", localtime(&currentDate));
    // std::string invDate(formatDate);
    sqlite3_stmt *res;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL);
    if (checkForError(rc, db, res, "unable to insert invoice."))
    {
        std::cout << query << std::endl;
        return -1;
    }
    rc = sqlite3_bind_int(res, sqlite3_bind_parameter_index(res, "@customer"), customer);
    if (checkForError(rc, db, res, "unable to insert invoice."))
    {
        std::cout << query << std::endl;
        return -1;
    }
    rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@date"), formatDate, 80, SQLITE_STATIC);
    if (checkForError(rc, db, res, "unable to insert invoice."))
    {
        std::cout << query << std::endl;
        return -1;
    }
    sqlite3_step(res);
    int invNum = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(res);
    return invNum;
}

int addCustomer(sqlite3 *db)
{
    std::string query = "insert into customer(cust_lname, cust_fname, cust_phone, cust_balance)\n"
                        "values(@lname,@fname, @phone,0)";

    std::string lname;
    std::string fname;
    std::string phone;
    std::cout << "Enter the customer last name: ";
    std::getline(std::cin >> std::ws, lname);
    std::cout << "Enter the customer first name: ";
    std::getline(std::cin >> std::ws, fname);
    std::cout << "Enter the customer phone number: ";
    std::getline(std::cin >> std::ws, phone);
    sqlite3_stmt *res;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL);

    if (checkForError(rc, db, res, "Unable to insert customer."))
    {
        std::cout << query << std::endl;
        return -1;
    }

    rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@lname"), lname.c_str(), -1, SQLITE_STATIC);
    if (checkForError(rc, db, res, "unable to insert customer lname."))
    {
        std::cout << query << std::endl;
        return -1;
    }
    rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@fname"), fname.c_str(), -1, SQLITE_STATIC);
    if (checkForError(rc, db, res, "unable to insert customer fname."))
    {
        std::cout << query << std::endl;
        return -1;
    }
    rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@phone"), phone.c_str(), -1, SQLITE_STATIC);
    if (checkForError(rc, db, res, "unable to insert customer phone."))
    {
        std::cout << query << std::endl;
        return -1;
    }
    sqlite3_step(res);
    int custNumber = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(res);
    return custNumber;
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

void pickProduct(sqlite3 *db, std::string &prodCode, int &qoh, double &price)
{
    std::string query = "select prod_code, prod_descript, prod_qoh, prod_price from product";
    sqlite3_stmt *result;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &result, NULL);
    if (checkForError(rc, db, result, "There was an error with the product query:"))
    {
        std::cout << query << std::endl;
        prodCode = "error";
        return;
    }
    int count = 1;
    int choice = 0;
    std::cout << "Please choose a product: " << std::endl;
    for (rc = sqlite3_step(result); rc == SQLITE_ROW; rc = sqlite3_step(result))
    {
        std::cout << count << ". " << sqlite3_column_text(result, 0);
        std::cout << " - " << sqlite3_column_text(result, 1);
        std::cout << " (Quantity on Hand: " << sqlite3_column_text(result, 2) << ")";
        std::cout << std::endl;
        count++;
    }
    std::cout << count << ". Cancel Invoice" << std::endl;
    choice = getInteger(1, count);
    if (choice == count)
    {
        sqlite3_finalize(result);
        prodCode = "cancel";
        return;
    }
    sqlite3_reset(result);
    for (int i = 0; i < choice; i++)
    {
        sqlite3_step(result);
    }
    qoh = sqlite3_column_int(result, 2);
    price = sqlite3_column_double(result, 3);
    prodCode = reinterpret_cast<const char *>(sqlite3_column_text(result, 0));
    sqlite3_finalize(result);
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
int createLine(sqlite3 *db, int invNum, std::string pc, int qoh, double price, int line)
{
    int quantity;
    std::cout << "How many would you like? (Quantity on hand: " << qoh << ") Enter 0 to cancel: ";
    std::cin >> quantity;
    std::cout << std::endl;

    while (!std::cin || quantity < 1 || quantity > qoh)
    {

        if (!std::cin)
            resetStream();
        else if (quantity == 0)
        {
            return -2;
        }
        std::cout << "The amount entered is invalid. Please try again." << std::endl;
        std::cout << "How many would you like? (Quantity on hand: " << qoh << ") Enter 0 to cancel: ";
        std::cin >> quantity;
        std::cout << std::endl;
    }
    std::string query = "insert into line (inv_number, line_number, prod_code, line_units, line_price) values (?1,?2,?3,?4,?5)";

    sqlite3_stmt *res;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(res);
        std::cout << "unable to insert line." << sqlite3_errmsg(db) << std::endl;
        std::cout << query << std::endl;
        return -1;
    }
    sqlite3_bind_int(res, 1, invNum);
    sqlite3_bind_int(res, 2, line);
    sqlite3_bind_text(res, 3, pc.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(res, 4, quantity);
    sqlite3_bind_double(res, 5, price);
    sqlite3_step(res);
    sqlite3_finalize(res);
    return quantity;
}

int updateProduct(sqlite3 *db, std::string prodCode, int q)
{
    std::string query = "update product set prod_qoh = prod_qoh - ? where prod_code = ?";

    sqlite3_stmt *res;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(res);
        std::cout << "unable to update product." << sqlite3_errmsg(db) << std::endl;
        std::cout << query << std::endl;
        return -1;
    }
    sqlite3_bind_int(res, 1, q);
    sqlite3_bind_text(res, 2, prodCode.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(res);
    sqlite3_finalize(res);

    return SQLITE_OK;
}