## Simple Cross

SimpleCross - a process that matches internal orders

Overview:
	Accept/remove orders as they are entered and keep a book of resting orders
	Determine if an accepted order would be satisfied by previously accepted orders (i.e. a buy would cross a resting sell)
	Output (print) crossing events and remove completed (fully filled) orders from the book

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
	The implementation should be a standalone Linux console application (include source files, testing tools and Makefile in submission)
	The use of third party libraries is not permitted. 
	The app should respond to malformed input and other errors with a RESULT of type 'E' and a descriptive error message
	Development should be production level quality. Design and implementation choices should be documented
	Performance is always a concern in software, but understand that this is an unrealistic test. 
	Only be concerned about performance where it makes sense to the important sections of this application (i.e. reading actions.txt is not important).
	All orders are standard limit orders (a limit order means the order remains in the book until it is either canceled, or fully filled by order(s) for its same symbol on the opposite side with an equal or better price).
	Orders should be selected for crossing using price-time (FIFO) priority
	Orders for different symbols should not cross (i.e. the book must support multiple symbols)
	
Design Choices:
	The design choices were all geared towards increasing effeciency of crossing events. For this reason the following data structures and algorithms were chosen:
		The data was divided into two structures to increase speed for accessing price ordered list and also for deleting orders quickly.
		There is an overarching unordered map structure that uses the symbol as the key. Under this are two more oredred maps (chosen for its self sorting/balancing behavior) that represent a buy and a sell book for each symbol (ticker). Each of these is ordered internally based on price to quickly access lowest prices for crossing.
		Another unordered map is maintained to quickly associate order ID and the location of the order in the main nest map. This helps in quickly detecting duplicates and deleting entries.
