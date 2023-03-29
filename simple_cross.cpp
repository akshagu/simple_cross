/*
SimpleCross - a process that matches internal orders

Overview:
    * Accept/remove orders as they are entered and keep a book of
      resting orders
    * Determine if an accepted order would be satisfied by previously
      accepted orders (i.e. a buy would cross a resting sell)
    * Output (print) crossing events and remove completed (fully filled)
      orders from the book

Inputs:
    A string of space separated values representing an action.  The number of
    values is determined by the action to be performed and have the following
    format:

    ACTION [OID [SYMBOL SIDE QTY PX]]

    ACTION: single character value with the following definitions
    O - place order, requires OID, SYMBOL, SIDE, QTY, PX
    X - cancel order, requires OID
    P - print sorted book (see example below)

    OID: positive 32-bit integer value which must be unique for all orders

    SYMBOL: alpha-numeric string value. Maximum length of 8.

    SIDE: single character value with the following definitions
    B - buy
    S - sell

    QTY: positive 16-bit integer value

    PX: positive double precision value (7.5 format)

Outputs:
    A list of strings of space separated values that show the result of the
    action (if any).  The number of values is determined by the result type and
    have the following format:

    RESULT OID [SYMBOL [SIDE] (FILL_QTY | OPEN_QTY) (FILL_PX | ORD_PX)]

    RESULT: single character value with the following definitions
    F - fill (or partial fill), requires OID, SYMBOL, FILL_QTY, FILL_PX
    X - cancel confirmation, requires OID
    P - book entry, requires OID, SYMBOL, SIDE, OPEN_QTY, ORD_PX (see example below)
    E - error, requires OID. Remainder of line represents string value description of the error

    FILL_QTY: positive 16-bit integer value representing qty of the order filled by
              this crossing event

    OPEN_QTY: positive 16-bit integer value representing qty of the order not yet filled

    FILL_PX:  positive double precision value representing price of the fill of this
              order by this crossing event (7.5 format)

    ORD_PX:   positive double precision value representing original price of the order (7.5 format)
              (7.5 format means up to 7 digits before the decimal and exactly 5 digits after the decimal)

Conditions/Assumptions:
    * The implementation should be a standalone Linux console application (include
      source files, testing tools and Makefile in submission)
    * The use of third party libraries is not permitted. 
    * The app should respond to malformed input and other errors with a RESULT
      of type 'E' and a descriptive error message
    * Development should be production level quality. Design and
      implementation choices should be documented
	* Performance is always a concern in software, but understand that this is an unrealistic test. 
	  Only be concerned about performance where it makes sense to the important sections of this application (i.e. reading actions.txt is not important).
    * All orders are standard limit orders (a limit order means the order remains in the book until it
      is either canceled, or fully filled by order(s) for its same symbol on the opposite side with an
      equal or better price).
    * Orders should be selected for crossing using price-time (FIFO) priority
    * Orders for different symbols should not cross (i.e. the book must support multiple symbols)

Example session:
    INPUT                                   | OUTPUT
    ============================================================================
    "O 10000 IBM B 10 100.00000"            | results.size() == 0
    "O 10001 IBM B 10 99.00000"             | results.size() == 0
    "O 10002 IBM S 5 101.00000"             | results.size() == 0
    "O 10003 IBM S 5 100.00000"             | results.size() == 2
                                            | results[0] == "F 10003 IBM 5 100.00000"
                                            | results[1] == "F 10000 IBM 5 100.00000"
    "O 10004 IBM S 5 100.00000"             | results.size() == 2
                                            | results[0] == "F 10004 IBM 5 100.00000"
                                            | results[1] == "F 10000 IBM 5 100.00000"
    "X 10002"                               | results.size() == 1
                                            | results[0] == "X 10002"
    "O 10005 IBM B 10 99.00000"             | results.size() == 0
    "O 10006 IBM B 10 100.00000"            | results.size() == 0
    "O 10007 IBM S 10 101.00000"            | results.size() == 0
    "O 10008 IBM S 10 102.00000"            | results.size() == 0
    "O 10008 IBM S 10 102.00000"            | results.size() == 1
                                            | results[0] == "E 10008 Duplicate order id"
    "O 10009 IBM S 10 102.00000"            | results.size() == 0
    "P"                                     | results.size() == 6
                                            | results[0] == "P 10009 IBM S 10 102.00000"
                                            | results[1] == "P 10008 IBM S 10 102.00000"
                                            | results[2] == "P 10007 IBM S 10 101.00000"
                                            | results[3] == "P 10006 IBM B 10 100.00000"
                                            | results[4] == "P 10001 IBM B 10 99.00000"
                                            | results[5] == "P 10005 IBM B 10 99.00000"
    "O 10010 IBM B 13 102.00000"            | results.size() == 4
                                            | results[0] == "F 10010 IBM 10 101.00000"
                                            | results[1] == "F 10007 IBM 10 101.00000"
                                            | results[2] == "F 10010 IBM 3 102.00000"
                                            | results[3] == "F 10008 IBM 3 102.00000"

So, for the example actions.txt, the desired output from the application with the below main is:
F 10003 IBM 5 100.00000
F 10000 IBM 5 100.00000
F 10004 IBM 5 100.00000
F 10000 IBM 5 100.00000
X 10002
E 10008 Duplicate order id
P 10009 IBM S 10 102.00000
P 10008 IBM S 10 102.00000
P 10007 IBM S 10 101.00000
P 10006 IBM B 10 100.00000
P 10001 IBM B 10 99.00000
P 10005 IBM B 10 99.00000
F 10010 IBM 10 101.00000
F 10007 IBM 10 101.00000
F 10010 IBM 3 102.00000
F 10008 IBM 3 102.00000

*/

// Stub implementation and example driver for SimpleCross.
// Your crossing logic should be accesible from the SimpleCross class.
// Other than the signature of SimpleCross::action() you are free to modify as needed.
#include <string>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <map>
#include <vector>
#include <unordered_map>

typedef std::list<std::string> results_t;
typedef std::vector<std::string> vlist_t;
typedef std::map<double, std::map<int, std::string> > book_t;

enum Inputs {
  ACTION = 0, 
  OID = 1,
  SYMBOL = 2, 
  SIDE = 3,
  QTY = 4,
  PX = 5
};

class SimpleCross
{
public:
    results_t action(const std::string& line) {
      results_t output, print_book;
      vlist_t split_line = this->split(line, ' ');
      int order_id;
      switch (split_line[ACTION][0]){
        case 'O':
          order_id = std::stoi(split_line[OID]);
          if (OIDs.find(order_id) == OIDs.end()){
            OIDs[order_id] = line;
            output = this->cross_order(line);
          } else {
            error_symbol = 'E';
            output.push_back(error_symbol+" "+split_line[OID]+" "+"Duplicate order id");
          }
          break;
        case 'P':
          print_book = this->print_all_sorted();
          for (std::string& order : print_book) {
            order[0] = 'P';
            output.push_back(order);
          }
          break;
        case 'X':
          this->delete_from_book(line, buy_book);
          this->delete_from_book(line, sell_book);
          output.push_back(line);
          break;
        default:
          error_symbol = 'E';
          output.push_back(error_symbol+" "+"Incorrect action character");
      }
      return output;
    }

    results_t cross_order (const std::string& line){
      vlist_t split_line = this->split(line, ' ');
      results_t buy_sell_result;
      switch (split_line[SIDE][0]){
        case 'B':
          buy_sell_result = this->buy_cross(line);
          break;
        case 'S':
          buy_sell_result = this->sell_cross(line);
          break;
        default:
          error_symbol = 'E';
          buy_sell_result.push_back(error_symbol+" "+"Incorrect side character");
      }
      return buy_sell_result;
    }

    results_t buy_cross(const std::string& line){
      results_t fulfilled;
      vlist_t split_line = this->split(line, ' ');
      std::string fulfilled_symbol;
      fulfilled_symbol = 'F';
      book_t::const_iterator sell_iterator = sell_book.begin();
      std::map<int, std::string> orders;
      double price = std::stod(split_line[PX]);
      int buy_quantity = std::stoi(split_line[QTY]);
      while (sell_iterator->first<=price && buy_quantity>0 && sell_iterator != sell_book.end()){
        results_t orders_strings;
        orders = sell_iterator->second;
        append_orders_for_key(orders, orders_strings);
        for (std::string& order : orders_strings){
          vlist_t split_order = this->split(order, ' ');
          int sell_quantity = std::stoi(split_order[QTY]);
          if (sell_quantity>buy_quantity){
            sell_quantity = sell_quantity - buy_quantity;
            buy_quantity = 0;
            split_order[QTY] = std::to_string(sell_quantity);
            order = this->merge(split_order, ' ');
            update_in_book(order, sell_book);
            fulfilled.push_back(fulfilled_symbol+" "+split_line[OID]+" "+split_line[SYMBOL]+" "+split_line[QTY]+" "+split_order[PX]);
            fulfilled.push_back(fulfilled_symbol+" "+split_order[OID]+" "+split_line[SYMBOL]+" "+split_line[QTY]+" "+split_order[PX]);
            break;
          } else if (sell_quantity==buy_quantity) {
            buy_quantity = 0;
            this->delete_from_book(order,sell_book);
            fulfilled.push_back(fulfilled_symbol+" "+split_line[OID]+" "+split_line[SYMBOL]+" "+split_line[QTY]+" "+split_order[PX]);
            fulfilled.push_back(fulfilled_symbol+" "+split_order[OID]+" "+split_line[SYMBOL]+" "+split_line[QTY]+" "+split_order[PX]);
            break;
          } else {
            buy_quantity = buy_quantity - sell_quantity;
            this->delete_from_book(order,sell_book);
            fulfilled.push_back(fulfilled_symbol+" "+split_line[OID]+" "+split_line[SYMBOL]+" "+split_order[QTY]+" "+split_order[PX]);
            fulfilled.push_back(fulfilled_symbol+" "+split_order[OID]+" "+split_line[SYMBOL]+" "+split_order[QTY]+" "+split_order[PX]);
          }
        }
        sell_iterator++;
      }
      if (buy_quantity) {
        split_line[QTY] = std::to_string(buy_quantity);
        std::string new_line = this->merge(split_line, ' ');
        this->add_to_book(new_line, buy_book);
        }
      return fulfilled;
    }

    results_t sell_cross(const std::string& line){
      results_t fulfilled;
      vlist_t split_line = this->split(line, ' ');
      std::string fulfilled_symbol;
      fulfilled_symbol = 'F';
      book_t::const_reverse_iterator buy_iterator = buy_book.rbegin();
      std::map<int, std::string> orders;
      double price = std::stod(split_line[PX]);
      int sell_quantity = std::stoi(split_line[QTY]);
      while (buy_iterator->first>=price && sell_quantity>0 && buy_iterator != buy_book.rend()){
        results_t orders_strings;
        orders = buy_iterator->second;
        append_orders_for_key(orders, orders_strings);
        for (std::string& order : orders_strings){
          vlist_t split_order = this->split(order, ' ');
          int buy_quantity = std::stoi(split_order[QTY]);
          if (buy_quantity>sell_quantity){
            buy_quantity = buy_quantity - sell_quantity;
            sell_quantity = 0;
            split_order[QTY] = std::to_string(buy_quantity);
            order = this->merge(split_order, ' ');
            update_in_book(order, buy_book);
            fulfilled.push_back(fulfilled_symbol+" "+split_line[OID]+" "+split_line[SYMBOL]+" "+split_line[QTY]+" "+split_line[PX]);
            fulfilled.push_back(fulfilled_symbol+" "+split_order[OID]+" "+split_line[SYMBOL]+" "+split_line[QTY]+" "+split_line[PX]);
            break;
          } else if (sell_quantity==buy_quantity) {
            sell_quantity = 0;
            this->delete_from_book(order,buy_book);
            fulfilled.push_back(fulfilled_symbol+" "+split_line[OID]+" "+split_line[SYMBOL]+" "+split_line[QTY]+" "+split_line[PX]);
            fulfilled.push_back(fulfilled_symbol+" "+split_order[OID]+" "+split_line[SYMBOL]+" "+split_line[QTY]+" "+split_line[PX]);
            break;
          } else {
            sell_quantity = sell_quantity - buy_quantity;
            this->delete_from_book(order,buy_book);
            fulfilled.push_back(fulfilled_symbol+" "+split_line[OID]+" "+split_line[SYMBOL]+" "+split_order[QTY]+" "+split_line[PX]);
            fulfilled.push_back(fulfilled_symbol+" "+split_order[OID]+" "+split_line[SYMBOL]+" "+split_order[QTY]+" "+split_line[PX]);
          }
        }
        buy_iterator++;
      }
      if (sell_quantity) {
        split_line[QTY] = std::to_string(sell_quantity);
        std::string new_line = this->merge(split_line, ' ');
        this->add_to_book(new_line, sell_book);
        }
      return fulfilled;
    }
    
    void add_to_book (const std::string& line, book_t& book){
      vlist_t split_line = this->split(line, ' ');
      double price = std::stod(split_line[PX]);
      int order_id = std::stoi(split_line[OID]);
      if (book.find(price) == book.end()){
        std::map<int, std::string> orders;
        orders[order_id] = line;
        book[price] = orders;
      } else {
        std::map<int, std::string> orders = book[price];
        orders[order_id] = line;
        book[price] = orders;
      }
    }

    void delete_from_book (const std::string& line, book_t& book){
      vlist_t split_line = this->split(line, ' ');
      int order_id = std::stoi(split_line[OID]);
      std::map<int, std::string> orders;
      // add error if oid not found
      for (book_t::const_iterator i = book.begin(); i != book.end(); i++){
        orders = i->second;
        orders.erase(order_id);
        book[i->first] = orders;
      }
    }

    void update_in_book (const std::string& line, book_t& book){
      vlist_t split_line = this->split(line, ' ');
      int order_id = std::stoi(split_line[OID]);
      std::map<int, std::string> orders;
      // add error if oid not found
      for (book_t::const_iterator i = book.begin(); i != book.end(); i++){
        orders = i->second;
        orders[order_id] = line;
        book[i->first] = orders;
      }
    }

    results_t print_all_sorted (){
      book_t::const_iterator buy_iterator = buy_book.begin();
      book_t::const_iterator sell_iterator = sell_book.begin();
      results_t all_sorted;
      std::map<int, std::string> orders;
      while (buy_iterator != buy_book.end() && sell_iterator != sell_book.end()){
        if(buy_iterator->first<=sell_iterator->first){
          orders = buy_iterator->second;
          append_orders_for_key(orders, all_sorted);
          buy_iterator++;
        } else {
          orders = sell_iterator->second;
          append_orders_for_key(orders, all_sorted);
          sell_iterator++;
        }
      }
      while (buy_iterator != buy_book.end()){
        orders = buy_iterator->second;
        append_orders_for_key(orders, all_sorted);
        buy_iterator++;
      }
      while (sell_iterator != sell_book.end()){
        orders = sell_iterator->second;
        append_orders_for_key(orders, all_sorted);
        sell_iterator++;
      }
      all_sorted.reverse();
      return all_sorted;
    }

    void append_orders_for_key (std::map<int, std::string>& orders, results_t& order_list){
      for (std::map<int, std::string>::const_iterator order_iterator = orders.begin(); order_iterator != orders.end(); order_iterator++){
        order_list.push_back(order_iterator->second);
      }
    }

    vlist_t split(std::string line, char delimiter){
      std::string temp_holder;
      std::stringstream ss(line);
      vlist_t string_array;
      while (getline(ss, temp_holder, delimiter)){
        string_array.push_back(temp_holder);
      }
      return string_array;
    }

    std::string merge(const vlist_t& split_line, char delimiter){
      std::string line;
      for (std::string chunk : split_line){
        line = line + chunk + delimiter;
      }
      return line;
    }
private:
    book_t buy_book;
    book_t sell_book;
    std::string error_symbol;
    std::unordered_map<int, std::string> OIDs;
};

int main(int argc, char **argv)
{
    SimpleCross scross;
    std::string line;
    std::ifstream actions("actions.txt", std::ios::in);
    while (std::getline(actions, line))
    {
        results_t results = scross.action(line);
        for (results_t::const_iterator it=results.begin(); it!=results.end(); ++it)
        {
            std::cout << *it << std::endl;
        }
    }
    return 0;
}

