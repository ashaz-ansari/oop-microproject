/* =========================================================================
   GAS STATION MANAGEMENT SYSTEM  -  MICROPROJECT ( OOP , CM31203 )

   Three of us worked on this file :
        <MEMBER 1>   Part 1   input checking, helper routines, String class
        <MEMBER 2>   Part 2   the class hierarchy and the Bill class
        <MEMBER 3>   Part 3   the GasStation class, the menus and the files
   ========================================================================= */

#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_BILLS  100      // how many transactions we keep in memory
#define MAX_FUELS  3        // Petrol, Diesel and CNG
#define RULE_WIDTH 40       // length of the line of = signs

/* =========================================================================
   PART 1   -   <MEMBER 1>
   Everything that reads what the user types and decides whether it is
   correct, the small printing helpers, and our own String class.
   ========================================================================= */

// the codes that all the checking routines give back.  OK means all right
enum ErrCode
{
    OK = 0, ERR_NOT_NUMBER = 1, ERR_NOT_POSITIVE = 2, ERR_TOO_BIG = 3,
    ERR_EMPTY = 4, ERR_BAD_VEHICLE = 5, ERR_BAD_NAME = 6, ERR_FILE = 7,
    ERR_NOT_FOUND = 8
};

// prints ch, n times.  both the arguments have a default value
inline void line(char ch = '=', int n = RULE_WIDTH) {
    int i;
    for (i = 0; i < n; i++) cout << ch;
    cout << '\n';
}

// a line of = signs, the title in the middle, and another line of = signs
void heading(const char *title, int width = RULE_WIDTH) {
    line('=', width);
    cout << "      " << title << '\n';
    line('=', width);
}

// waits for one key press so that the screen does not go away
inline void pauseScreen() { cout << "\nPress any key to continue..."; getch(); }

// 0 = not a number at all, 1 = a positive number, 2 = a negative number
int checkNumber(const char *s) {
    int i = 0, dot = 0, digits = 0, neg = 0;

    if (s[0] == '-') { neg = 1; i = 1; }
    if (s[i] == '\0') return 0;

    for (; s[i] != '\0'; i++) {
        if (s[i] == '.') {
            if (dot != 0) return 0;              // a second dot, not a number
            dot = 1;
        }
        else if (s[i] >= '0' && s[i] <= '9') digits++;
        else return 0;                           // something that is not a digit
    }
    if (digits == 0) return 0;
    return (neg != 0) ? 2 : 1;
}

// prints the message that goes with an error code
void showError(int code, const char *field) {
    cout << "\nError: " << field;
    switch (code)
    {
    case ERR_NOT_NUMBER:   cout << " must be a number.\n"; break;
    case ERR_NOT_POSITIVE: cout << " must be greater than zero.\n"; break;
    case ERR_TOO_BIG:      cout << " is too large. Please enter less.\n"; break;
    case ERR_EMPTY:        cout << " cannot be empty.\n"; break;
    case ERR_BAD_VEHICLE:  cout << " is not valid. Example: MH12AB1234\n"; break;
    case ERR_BAD_NAME:     cout << " must contain letters only.\n"; break;
    case ERR_FILE:         cout << " file could not be opened.\n"; break;
    case ERR_NOT_FOUND:    cout << " was not found.\n"; break;
    default:               cout << " is not correct.\n"; break;
    }
}

// asks for one number, checks it, and writes it back through the pointer
int readNumber(const char *prompt, const char *field, float *value, float maxValue) {
    char buf[30];
    int kind;

    cout << prompt;
    cin.getline(buf, 30);
    if (cin.fail()) { cin.clear(); cin.ignore(100, '\n'); }

    kind = checkNumber(buf);
    if (kind == 0) { showError(ERR_NOT_NUMBER, field); return ERR_NOT_NUMBER; }

    *value = (float) atof(buf);                  // text to float
    if (kind == 2 || *value <= 0.0) { showError(ERR_NOT_POSITIVE, field); return ERR_NOT_POSITIVE; }
    if (maxValue > 0.0 && *value > maxValue) { showError(ERR_TOO_BIG, field); return ERR_TOO_BIG; }
    return OK;
}

// reads one line of text.  1 if something was typed, 0 if it was empty
int readText(const char *prompt, char *buf, int size) {
    cout << prompt;
    cin.getline(buf, size);
    if (cin.fail()) { cin.clear(); cin.ignore(200, '\n'); }
    return (strlen(buf) == 0) ? 0 : 1;
}

// a name may have letters, spaces and dots only
int checkName(const char *s) {
    int i, letters = 0;

    for (i = 0; s[i] != '\0'; i++) {
        char c = s[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == ' ' || c == '.'))
            return 0;
        if (c != ' ') letters++;
    }
    return (letters == 0) ? 0 : 1;
}

// a vehicle number like MH12AB1234 : letters and digits, at least six of them
int checkVehicle(const char *s) {
    int i, count = 0;

    for (i = 0; s[i] != '\0'; i++) {
        char c = s[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')))
            return 0;
        count++;
    }
    return (count < 6) ? 0 : 1;
}

// reads one menu choice.  -1 means the choice was wrong
int readChoice(int low, int high) {
    char buf[10];
    int n;

    cout << "\nEnter your choice : ";
    cin.getline(buf, 10);
    if (cin.fail()) { cin.clear(); cin.ignore(100, '\n'); }
    if (checkNumber(buf) != 1) return -1;

    n = atoi(buf);
    return (n < low || n > high) ? -1 : n;
}

// the "try again" or "go back" question that every error asks
int askRetry() {
    int c;
    cout << "\n1. Try Again\n2. Back\n";
    c = readChoice(1, 2);
    if (c == -1) { showError(ERR_NOT_NUMBER, "Choice"); return 0; }
    return (c == 1) ? 1 : 0;
}

// cuts a line like  Petrol|105.50  into its fields at every | sign
int splitLine(char *text, char fields[][30], int maxFields) {
    int n = 0, i = 0, j;

    while (text[i] != '\0' && n < maxFields) {
        j = 0;
        while (text[i] != '\0' && text[i] != '|' && j < 29) { fields[n][j] = text[i]; j++; i++; }
        fields[n][j] = '\0';
        n++;
        if (text[i] == '|') i++;
    }
    return n;
}

// money with two decimals.  sprintf, as TC++ 3.0 drops trailing zeros ( 92.3 )
void money(float v) { char b[24]; sprintf(b, "%.2f", v); cout << b; }

// rounds a value to the given number of decimals
float roundTo(float v, int decimals) {
    float factor = (float) pow(10.0, decimals);
    return (float) (floor(v * factor + 0.5) / factor);
}

// rounds a value up to the given number of decimals
float roundUpTo(float v, int decimals) {
    float factor = (float) pow(10.0, decimals);
    return (float) (ceil(v * factor) / factor);
}

// our own String class : the characters sit in one fixed array inside the
// object and the operators are overloaded so it works like a normal variable
class String {
    char data[40];
public:
    String() { data[0] = '\0'; }
    String(const char *s) { strncpy(data, s, 39); data[39] = '\0'; }
    String(const String &s) { strcpy(data, s.data); }
    ~String() { }                            // nothing to free, the array is inside

    String& operator=(const String &s) {
        if (this != &s) strcpy(data, s.data);    // not copying onto itself
        return *this;
    }
    String& operator=(const char *s) { strncpy(data, s, 39); data[39] = '\0'; return *this; }

    int operator==(const String &s) const { return strcmp(data, s.data) == 0; }
    int operator!=(const String &s) const { return strcmp(data, s.data) != 0; }
    int operator<(const String &s) const  { return strcmp(data, s.data) < 0; }
    int operator!() const { return data[0] == '\0'; }   // 1 if the string is empty
    char& operator[](int i) { return data[i]; }
    char  operator[](int i) const { return data[i]; }

    int length() const { return (int) strlen(data); }
    const char* c_str() const { return data; }

    friend ostream& operator<<(ostream &os, const String &s);
};

ostream& operator<<(ostream &os, const String &s) { os << s.data; return os; }


/* -------------------------------------------------------------------------
   PART 2   -   <MEMBER 2>    the class hierarchy and the Bill class
   ------------------------------------------------------------------------- */

/* Person is the base class of the customer side.  The name is protected, so
   the classes below can use it directly and main() cannot. */
class Person {
protected:
    String name;
public:
    Person() : name("") { }
    Person(const String &n) : name(n) { }
    void setName(const String &n) { name = n; }
    const String& getName() const { return name; }
    void show() const { cout << "Name : " << name << '\n'; }
};

/* Auth keeps the login of the administrator.  It is the second base class of
   Admin, and that is where the multiple inheritance comes in. */
class Auth {
protected:
    String user, pass;
public:
    Auth() : user("admin"), pass("admin123") { }
    Auth(const String &u, const String &p) : user(u), pass(p) { }
    int verify(const String &u, const String &p) const { return (user == u && pass == p); }
};

/* A Customer is a Person who also has a vehicle number.  Bill below it makes
   this chain a multilevel inheritance. */
class Customer : public Person {
protected:
    String vehicle;
public:
    Customer() : Person(), vehicle("") { }
    Customer(const String &n, const String &v) : Person(n), vehicle(v) { }
    void setVehicle(const String &v) { vehicle = v; }
    const String& getVehicle() const { return vehicle; }
    void show() const { Person::show(); cout << "Vehicle : " << vehicle << '\n'; }
};

/* Admin takes the name from Person and the login from Auth, so it has two
   base classes. */
class Admin : public Person, public Auth {
public:
    Admin() : Person("Administrator"), Auth("admin", "admin123") { }
    int login(const String &u, const String &p) const { return verify(u, p); }
    void show() const { Person::show(); cout << "Login : " << user << '\n'; }
};

/* Fuel is an abstract class.  getUnit() is a pure virtual function, so an
   object of Fuel itself can never be made - only of Petrol, Diesel or CNG. */
class Fuel {
protected:
    String name;
    float rate;
public:
    Fuel(const String &n, float r) : name(n) { rate = r; }
    virtual ~Fuel() { }                  // virtual, so delete is safe

    virtual float calculateAmount(float quantity) const { return quantity * rate; }
    virtual float calculateQuantity(float amount) const { return amount / rate; }
    virtual const char* getUnit() const = 0;
    virtual int setRate(float r) {
        if (r <= 0.0) return ERR_NOT_POSITIVE;
        if (r > 500.0) return ERR_TOO_BIG;       // a litre cannot cost as much as that
        rate = r;
        return OK;
    }
    virtual void show() const { cout << "Fuel   : " << name << " @ Rs."; money(rate); cout << '\n'; }

    const String& getName() const { return name; }
    float getRate() const { return rate; }
};

/* LiquidFuel and GasFuel both inherit Fuel as a virtual base, so a DualFuel
   object gets only one copy of Fuel instead of two. */
class LiquidFuel : virtual public Fuel {
public:
    LiquidFuel(const String &n, float r) : Fuel(n, r) { }
    const char* getUnit() const { return "L"; }
};

class GasFuel : virtual public Fuel {
public:
    GasFuel(const String &n, float r) : Fuel(n, r) { }
    const char* getUnit() const { return "Kg"; }
};

/* Petrol is sold by the litre and the bill is rounded to the nearest paisa. */
class Petrol : public LiquidFuel {
public:
    Petrol(float r) : Fuel("Petrol", r), LiquidFuel("Petrol", r) { }
    float calculateAmount(float quantity) const { return roundTo(quantity * rate, 2); }
    void show() const { cout << "Petrol : Rs."; money(rate); cout << " per Litre\n"; }
};

/* Diesel is a LiquidFuel like Petrol, and it can also be built out of a
   Petrol object - that is the class to class conversion. */
class Diesel : public LiquidFuel {
public:
    Diesel(float r) : Fuel("Diesel", r), LiquidFuel("Diesel", r) { }
    Diesel(const Petrol &p, float dieselRate)
        : Fuel("Diesel", dieselRate), LiquidFuel("Diesel", dieselRate) { (void) p; }
    float calculateAmount(float quantity) const { return roundTo(quantity * rate, 2); }
    void show() const { cout << "Diesel : Rs."; money(rate); cout << " per Litre\n"; }
};

/* CNG is sold by weight, so the amount is rounded UP to the paisa. */
class CNG : public GasFuel {
public:
    CNG(float r) : Fuel("CNG", r), GasFuel("CNG", r) { }
    float calculateAmount(float quantity) const { return roundUpTo(quantity * rate, 2); }
    void show() const { cout << "CNG    : Rs."; money(rate); cout << " per Kg\n"; }
};

/* DualFuel takes after both LiquidFuel and GasFuel.  As Fuel is a virtual base
   class, the object holds only one Fuel inside it. */
class DualFuel : public LiquidFuel, public GasFuel {
public:
    DualFuel() : Fuel("Dual", 0.0), LiquidFuel("Dual", 0.0), GasFuel("Dual", 0.0) { }
    float calculateAmount(float quantity) const { return roundTo(quantity * rate, 2); }
    const char* getUnit() const { return "unit"; }   // settles the ambiguity
};

/* Bill is what the project is really about.  It inherits Customer, so a bill
   is also a customer and also a person.  It keeps a pointer to the Fuel
   object, so fuelPtr->calculateAmount() is decided at run time. */
class Bill : public Customer {
private:
    static int count, made, gone;      /* next id, how many made, how many gone */
    int id, fuelIndex, mode;           /* mode 1 = by quantity, 2 = by amount */
    String fuelName, unit;             /* L for petrol and diesel, Kg for CNG */
    Fuel *fuelPtr;
    float quantity, rate, amount, discount, finalAmount;

    /* private member functions : only this class can call them */
    float slabRate() const {
        if (amount < 1000.0) return 0.0;
        if (amount < 3000.0) return 0.02;
        return 0.05;
    }
    void calculateAmount() {
        if (mode == 1) {
            if (fuelPtr != 0) amount = fuelPtr->calculateAmount(quantity);
            else amount = roundTo(quantity * rate, 2);    /* a bill from the file */
        }
        else amount = roundTo(amount, 2);
    }
    void calculateDiscount() {
        discount = roundTo(amount * slabRate(), 2);
        finalAmount = roundTo(amount - discount, 2);
    }

public:
    Bill();
    Bill(const String &n, const String &v = "MH00AA0000");
    Bill(float amt);                   // a plain amount becomes a bill
    Bill(const Bill &b);               // copy constructor
    ~Bill() { gone++; }

    static int nextId() { return count++; }          static void syncId(int lastId) { count = lastId; }
    static int madeCount() { return made; }          static int goneCount() { return gone; }
    static void resetCounters() { made = 0; gone = 0; }

    void setFuel(int index, Fuel *f);                Bill& setQuantity(float q);
    Bill& setAmount(float a);                        void setId(int i) { id = i; }

    int getId() const { return id; }                 int getFuelIndex() const { return fuelIndex; }
    const String& getFuelName() const { return fuelName; }
    float getQuantity() const { return quantity; }   float getRate() const { return rate; }
    float getAmount() const { return amount; }       float getDiscount() const { return discount; }
    float getFinalAmount() const { return finalAmount; }
    int getMode() const { return mode; }             unsigned long address() const { return (unsigned long) this; }

    void showPreview() const;                        void showBill() const;
    void saveTo(ostream &out) const;                 int loadFrom(const char *text);

    Bill& operator++() { id = nextId(); return *this; }
    Bill operator+(const Bill &b) const;
    int operator>(const Bill &b) const { return (finalAmount > b.finalAmount); }
    operator float() const { return finalAmount; }   float toFloat() const { return finalAmount; }

    friend ostream& operator<<(ostream &os, const Bill &b);
};

/* the static data members have to be defined once, outside the class */
int Bill::count = 1001;
int Bill::made = 0;
int Bill::gone = 0;

Bill::Bill() : Customer(), id(0), fuelIndex(0), fuelName(""), unit("L"), fuelPtr(0),
    quantity(0.0), rate(0.0), amount(0.0), discount(0.0), finalAmount(0.0), mode(1) { made++; }

/* the vehicle number has a default value, so Bill("Rahul") alone is allowed */
Bill::Bill(const String &n, const String &v) : Customer(n, v), id(0), fuelIndex(0),
    fuelName(""), unit("L"), fuelPtr(0), quantity(0.0), rate(0.0), amount(0.0),
    discount(0.0), finalAmount(0.0), mode(1) { made++; }

/* a plain number such as Rs.500 becomes a Bill object */
Bill::Bill(float amt) : Customer("Walk In Customer", "MH00AA0000"), id(0), fuelIndex(0),
    fuelName("Petrol"), unit("L"), fuelPtr(0), quantity(0.0), rate(105.50), amount(amt),
    discount(0.0), finalAmount(amt), mode(2) {
    made++;
    quantity = amount / rate;    calculateDiscount();
    finalAmount = amount - discount;
}

/* this one runs every time a Bill is passed to a function by value */
Bill::Bill(const Bill &b) : Customer(b.getName(), b.getVehicle()) {
    made++;
    id = b.id;           fuelIndex = b.fuelIndex;   fuelName = b.fuelName;
    unit = b.unit;       fuelPtr = b.fuelPtr;       quantity = b.quantity;
    rate = b.rate;       amount = b.amount;         discount = b.discount;
    finalAmount = b.finalAmount;                    mode = b.mode;
}

/* the bill keeps a pointer to the Fuel object, so it can ask the fuel itself
   what the amount is.  The rate and the unit come from the object too. */
void Bill::setFuel(int index, Fuel *f) {
    fuelIndex = index;   fuelPtr = f;   fuelName = f->getName();
    unit = f->getUnit(); rate = f->getRate();
    calculateAmount();   calculateDiscount();
}

/* setQuantity() gives back *this, so calls can be joined together as
   bill.setQuantity(5).setAmount(500); */
Bill& Bill::setQuantity(float q) {
    this->quantity = q;
    mode = 1;
    calculateAmount();   calculateDiscount();
    return *this;
}

Bill& Bill::setAmount(float a) {
    this->amount = a;
    mode = 2;
    calculateDiscount();
    if (rate > 0.0) this->quantity = a / rate;
    return *this;
}

void Bill::showPreview() const {
    cout << "\nTransaction ID : " << id << '\n';
    cout << "Customer Name  : " << getName() << '\n';
    cout << "Vehicle No.    : " << getVehicle() << '\n';
    cout << "Fuel Type      : " << fuelName << '\n';
    cout << "Rate           : Rs.";  money(rate);        cout << "/" << unit << '\n';
    cout << "Quantity       : ";     money(quantity);    cout << " " << unit << '\n';
    cout << "Fuel Amount    : Rs.";  money(amount);      cout << '\n';
    cout << "Discount       : Rs.";  money(discount);    cout << '\n';
    cout << "Final Amount   : Rs.";  money(finalAmount); cout << '\n';
}

void Bill::showBill() const {
    heading("GAS STATION");
    cout << "\nTransaction ID : " << id << '\n';
    cout << "Customer Name  : " << getName() << '\n';
    cout << "Vehicle No.    : " << getVehicle() << "\n\n";
    cout << "Fuel           : " << fuelName << '\n';
    cout << "Rate           : Rs.";  money(rate);        cout << "/" << unit << '\n';
    cout << "Quantity       : ";     money(quantity);    cout << " " << unit << "\n\n";
    cout << "Fuel Amount    : Rs.";  money(amount);      cout << '\n';
    cout << "Discount       : Rs.";  money(discount);    cout << '\n';
    line('-');
    cout << "Final Amount   : Rs.";  money(finalAmount); cout << '\n';
    line('=');
    cout << "          Thank You! Visit Again\n";
    line('=');
}

/* one transaction is written as nine fields separated by the | character */
void Bill::saveTo(ostream &out) const {
    char q[24], r[24], a[24], d[24], f[24];
    sprintf(q, "%.2f", quantity);   sprintf(d, "%.2f", discount);
    sprintf(r, "%.2f", rate);       sprintf(a, "%.2f", amount);
    sprintf(f, "%.2f", finalAmount);
    out << id << '|' << getName().c_str() << '|' << getVehicle().c_str() << '|'
        << fuelName.c_str() << '|' << q << '|' << r << '|' << a << '|' << d << '|' << f << '\n';
}

/* and read back from the same sort of line */
int Bill::loadFrom(const char *text) {
    char f[9][30];

    if (splitLine((char *) text, f, 9) != 9) return 0;

    setId(atoi(f[0]));                setName(f[1]);
    setVehicle(f[2]);                 fuelName = f[3];
    quantity = (float) atof(f[4]);    rate = (float) atof(f[5]);
    amount = (float) atof(f[6]);      discount = (float) atof(f[7]);
    finalAmount = (float) atof(f[8]);

    mode = 1;
    fuelPtr = 0;                       /* a bill from the file has no Fuel object */
    unit = "L";
    if (fuelName == String("CNG")) unit = "Kg";
    return 1;
}

/* bill1 + bill2 gives a bill whose amounts are the totals of both, and this
   is how the total sale of the day is worked out */
Bill Bill::operator+(const Bill &b) const {
    Bill temp(*this);                  /* the copy constructor runs here */
    temp.amount = amount + b.amount;
    temp.discount = discount + b.discount;
    temp.finalAmount = finalAmount + b.finalAmount;
    temp.quantity = quantity + b.quantity;
    return temp;
}

/* so that one whole row of the history can be printed with  cout << history[i] */
ostream& operator<<(ostream &os, const Bill &b) {
    char amt[24];   sprintf(amt, "%.2f", b.getFinalAmount());
    os << setiosflags(ios::left) << setw(7) << b.getId()
       << setw(16) << b.getName().c_str() << setw(14) << b.getVehicle().c_str()
       << setw(10) << b.getFuelName().c_str() << setiosflags(ios::right) << setw(10)
       << amt;
    return os;
}


/* -------------------------------------------------------------------------
   PART 3   -   <MEMBER 3>    GasStation, the menus, the files and main()
   ------------------------------------------------------------------------- */

/* GasStation holds the three Fuel objects, one Admin object and the array of
   Bill objects, and it shows every screen of the program. */
class GasStation {
private:
    Fuel *fuels[MAX_FUELS];            // Petrol, Diesel and CNG
    Admin admin;                       // the administrator of the pump
    Bill history[MAX_BILLS];           // the transactions kept in memory
    int billCount;                     // how many are there right now

    int  readValid(const char *prompt, const char *field, float *v, float max);
    void printMainMenu();          void printFuelMenu();       void printFillingMenu();
    void printPreviewMenu();       void printAdminMenu();      void showRateLine(int index) const;
    void loadFuelPrices();         void saveFuelPrices();      void loadTransactions();

public:
    GasStation();                  ~GasStation();
    void init();                   void mainMenu();            void customerBilling();
    int  selectFuel();             void viewFuelPrices();      void viewTransactions();
    void adminLogin();             void adminMenu();           void updateFuelPrice();
    int  fillBy(Bill &b, int byAmount);
    void showPreviewLoop(Bill b);  // by value, so the copy constructor runs
    void generateBill(Bill &b);    void saveTransaction(Bill b);
    float totalSale() const;
};

/* the three fuel objects are made on the heap while the program is running.
   This is the dynamic initialisation of objects, not before main(). */
GasStation::GasStation() : admin() {
    fuels[0] = new Petrol(105.50);
    fuels[1] = new Diesel(92.30);
    fuels[2] = new CNG(88.00);
    billCount = 0;
}

// the virtual destructor of Fuel makes these deletes all right
GasStation::~GasStation() {
    int i;
    for (i = 0; i < MAX_FUELS; i++) delete fuels[i];
}

void GasStation::init() {
    Bill::resetCounters();   loadFuelPrices();   loadTransactions();
}

// reads fuel_prices.txt.  each line is like   Petrol|105.50
void GasStation::loadFuelPrices() {
    char text[60], f[2][30];
    float r;    int i;

    ifstream fin("fuel_prices.txt");
    if (fin.fail()) {                  // first run, the file is not there yet
        cout << "\nError: fuel_prices.txt could not be opened.\n";
        cout << "The default prices have been loaded and the file will be created.\n";
        pauseScreen();
        saveFuelPrices();
        return;
    }
    for (i = 0; i < MAX_FUELS; i++) {
        fin.getline(text, 60);
        if (fin.fail()) break;
        if (splitLine(text, f, 2) == 2) {
            r = (float) atof(f[1]);
            fuels[i]->setRate(r);
        }
    }
    fin.close();
}

void GasStation::saveFuelPrices() {
    char r[24];    int i;
    ofstream fout("fuel_prices.txt", ios::out | ios::trunc);
    if (fout.fail()) { showError(ERR_FILE, "fuel_prices.txt"); return; }

    for (i = 0; i < MAX_FUELS; i++) {
        sprintf(r, "%.2f", fuels[i]->getRate());
        fout << fuels[i]->getName() << '|' << r << '\n';
    }
    fout.close();
}

/* reads every transaction of transactions.txt into the array of objects, and
   leaves the next id ready after the last one that was in the file */
void GasStation::loadTransactions() {
    char text[120];
    int lastId = 1000;

    ifstream fin("transactions.txt");
    billCount = 0;
    if (fin.fail()) return;            // first run, the file is not there yet

    while (billCount < MAX_BILLS) {
        fin.getline(text, 120);
        if (fin.fail()) break;
        if (history[billCount].loadFrom(text) == 1) {
            if (history[billCount].getId() > lastId) lastId = history[billCount].getId();
            billCount++;
        }
    }
    fin.close();
    Bill::syncId(lastId + 1);
}

void GasStation::printMainMenu() {
    clrscr();
    heading("GAS STATION MANAGEMENT SYSTEM");
    cout << "\n1. Customer Billing\n2. View Fuel Prices\n3. View Transaction History\n"
            "4. Admin Login\n5. Exit\n";
}

void GasStation::mainMenu() {
    int choice;
    do {
        printMainMenu();
        choice = readChoice(1, 5);
        while (choice == -1) {
            showError(ERR_NOT_NUMBER, "Choice");
            cout << "Please enter a number between 1 and 5.\n";
            choice = readChoice(1, 5);
        }
        switch (choice)
        {
        case 1: customerBilling(); break;
        case 2: viewFuelPrices(); break;
        case 3: viewTransactions(); break;
        case 4: adminLogin(); break;
        case 5: clrscr();
                heading("GAS STATION MANAGEMENT SYSTEM");
                cout << "\nThank you for using the system.\n";
                cout << "Total sale of this session : Rs.";
                money(totalSale());   cout << "\n\n";
                break;
        }
    } while (choice != 5);
}

void GasStation::printFuelMenu() {
    cout << "\n1. Petrol\n2. Diesel\n3. CNG\n4. Back\n";
}

void GasStation::printFillingMenu() {
    cout << "\n1. Fill by Quantity\n2. Fill by Amount\n3. Change Fuel\n4. Cancel Billing\n";
}

void GasStation::printPreviewMenu() {
    cout << "\n1. Confirm and Generate Bill\n2. Change Quantity / Amount\n"
            "3. Change Fuel\n4. Cancel Transaction\n";
}

void GasStation::showRateLine(int index) const {
    cout << "Selected Fuel : " << fuels[index]->getName() << '\n';
    cout << "Current Rate  : Rs.";  money(fuels[index]->getRate());
    cout << "/" << fuels[index]->getUnit() << '\n';
}

/* the whole billing screen : the customer details first, then the fuel, then
   the filling method, and at the end the preview of the bill */
void GasStation::customerBilling() {
    char name[22], vehicle[14];
    Bill bill;
    int choice, fuel;

    clrscr();
    heading("CUSTOMER BILLING");
    cout << "\n1. Start New Billing\n2. Back to Main Menu\n";
    if (readChoice(1, 2) != 1) return;

    clrscr();
    heading("CUSTOMER DETAILS");
    while (1) {
        if (readText("\nEnter Customer Name : ", name, 22) == 1 && checkName(name) == 1) break;
        if (strlen(name) == 0) showError(ERR_EMPTY, "Customer Name");
        else showError(ERR_BAD_NAME, "Customer Name");
        if (askRetry() == 0) return;
    }
    while (1) {
        if (readText("Enter Vehicle Number : ", vehicle, 14) == 1 && checkVehicle(vehicle) == 1) break;
        if (strlen(vehicle) == 0) showError(ERR_EMPTY, "Vehicle Number");
        else showError(ERR_BAD_VEHICLE, "Vehicle Number");
        if (askRetry() == 0) return;
    }
    bill.setName(name);
    bill.setVehicle(vehicle);

    fuel = selectFuel();
    if (fuel == -1) return;
    bill.setFuel(fuel, fuels[fuel]);   // the base class pointer goes inside

    clrscr();
    heading("SELECT FILLING METHOD");
    showRateLine(fuel);

    while (1) {
        printFillingMenu();
        choice = readChoice(1, 4);
        if (choice == -1) { showError(ERR_NOT_NUMBER, "Choice"); continue; }
        if (choice == 4) return;                   // cancel the billing
        if (choice == 3) {                         // come back and change the fuel
            fuel = selectFuel();
            if (fuel == -1) return;
            bill.setFuel(fuel, fuels[fuel]);
            clrscr();
            heading("SELECT FILLING METHOD");
            showRateLine(fuel);
            continue;
        }
        if (fillBy(bill, choice - 1) == 1) break;  // choice 1 = quantity, 2 = amount
        else return;
    }
    showPreviewLoop(bill);
}

/* the bill is passed by value here, so the copy constructor runs on the way in */
void GasStation::showPreviewLoop(Bill b) {
    int choice;

    while (1) {
        clrscr();
        heading("BILL PREVIEW");
        b.showPreview();
        printPreviewMenu();
        choice = readChoice(1, 4);
        if (choice == -1) { showError(ERR_NOT_NUMBER, "Choice"); continue; }
        if (choice == 1) {                         // confirm the transaction
            ++b;                                   // the bill gets its id here
            generateBill(b);
            return;
        }
        if (choice == 2) {                         // fill the same bill again
            if (fillBy(b, (b.getMode() == 1) ? 0 : 1) == 0) return;
            continue;
        }
        if (choice == 3) {                         // change the fuel
            int fuel = selectFuel();
            if (fuel == -1) return;
            b.setFuel(fuel, fuels[fuel]);
            continue;
        }
        cout << "\nTransaction cancelled.\n";
        pauseScreen();
        return;
    }
}

/* the choice becomes 0 for Petrol, 1 for Diesel and 2 for CNG.  -1 means Back */
int GasStation::selectFuel() {
    int choice;
    clrscr();
    heading("FUEL SELECTION");
    printFuelMenu();
    choice = readChoice(1, 4);
    while (choice == -1) { showError(ERR_NOT_NUMBER, "Choice"); choice = readChoice(1, 4); }
    return (choice == 4) ? -1 : choice - 1;
}

/* asks until the number is right.  1 = done, 0 = the user went back */
int GasStation::readValid(const char *prompt, const char *field, float *v, float max) {
    while (1) {
        if (readNumber(prompt, field, v, max) == OK) return 1;
        if (askRetry() == 0) return 0;
    }
}

/* byAmount 0 = the customer gives the quantity, 1 = the customer gives the amount */
int GasStation::fillBy(Bill &b, int byAmount) {
    float v;
    cout << "\n";
    if (byAmount == 0) {
        if (readValid("Enter Quantity : ", "Quantity", &v, 200.0) == 0) return 0;
        b.setQuantity(v);
    }
    else {
        if (readValid("Enter Amount : Rs.", "Amount", &v, 100000.0) == 0) return 0;
        b.setAmount(v);
    }
    return 1;
}

void GasStation::generateBill(Bill &b) {
    clrscr();
    b.showBill();
    cout << "\n";
    pauseScreen();
    saveTransaction(b);
}

// the bill is passed by value again, so the copy constructor runs once more
void GasStation::saveTransaction(Bill b) {
    ofstream fout("transactions.txt", ios::out | ios::app);
    if (fout.fail()) { showError(ERR_FILE, "transactions.txt"); return; }

    b.saveTo(fout);
    fout.close();

    if (billCount < MAX_BILLS) history[billCount++] = b;
    cout << "\nTransaction saved successfully.\n";
}

void GasStation::viewFuelPrices() {
    int i, choice;
    clrscr();
    heading("CURRENT FUEL PRICES");
    cout << "\n";
    for (i = 0; i < MAX_FUELS; i++) fuels[i]->show();   // the virtual function
    cout << "\n1. Back to Main Menu\n";   choice = readChoice(1, 1);
    while (choice == -1) { showError(ERR_NOT_NUMBER, "Choice"); choice = readChoice(1, 1); }
}

/* every row is printed with  cout << history[i]  because operator << was
   overloaded for the Bill class */
void GasStation::viewTransactions() {
    int i, j, choice;    Bill total;

    clrscr();
    heading("TRANSACTION HISTORY");
    if (billCount == 0) cout << "\nNo transactions have been recorded yet.\n";
    else {
        cout << "\nID     Customer        Vehicle       Fuel         Amount\n";
        line('-', 57);
        for (i = 0; i < billCount; i++) cout << history[i] << '\n';
        line('-', 57);

        for (i = 0; i < billCount; i++) total = total + history[i];
        cout << "Total sale : Rs.";
        money(total.getFinalAmount());   cout << '\n';
    }
    cout << "\n1. Sort by Amount (highest first)\n2. Back to Main Menu\n";
    choice = readChoice(1, 2);
    while (choice == -1) { showError(ERR_NOT_NUMBER, "Choice"); choice = readChoice(1, 2); }
    if (choice == 2) return;

    // a plain bubble sort, and the bills are compared with operator >
    for (i = 0; i < billCount - 1; i++)
        for (j = 0; j < billCount - 1 - i; j++)
            if (history[j + 1] > history[j]) {
                Bill temp = history[j];
                history[j] = history[j + 1];
                history[j + 1] = temp;
            }
    cout << "\nSorted by amount (highest first).\n";
    pauseScreen();
    viewTransactions();
}

void GasStation::printAdminMenu() {
    cout << "\n1. View Fuel Prices\n2. Update Fuel Price\n3. View All Transactions\n"
            "4. Logout\n";
}

void GasStation::adminLogin() {
    char user[26], pass[26];
    int attempts = 0, ok = 0;

    clrscr();
    heading("ADMIN LOGIN");
    while (attempts < 3 && ok == 0) {
        while (readText("\nEnter Username : ", user, 26) == 0) showError(ERR_EMPTY, "Username");
        while (readText("Enter Password : ", pass, 26) == 0) showError(ERR_EMPTY, "Password");

        if (admin.login(user, pass) == 1) ok = 1;
        else {
            attempts++;
            cout << "\nInvalid username or password!  ( attempt " << attempts << " of 3 )\n";
        }
    }
    if (ok == 1) {
        cout << "\nLogin successful. Welcome, Administrator.\n";
        pauseScreen();
        adminMenu();
    }
    else { cout << "\nBack to the main menu.\n"; pauseScreen(); }
}

void GasStation::adminMenu() {
    int choice;
    while (1) {
        clrscr();
        heading("ADMIN MENU");
        printAdminMenu();
        choice = readChoice(1, 4);
        if (choice == -1) { showError(ERR_NOT_NUMBER, "Choice"); continue; }
        switch (choice)
        {
        case 1: viewFuelPrices(); break;
        case 2: updateFuelPrice(); break;
        case 3: viewTransactions(); break;
        case 4: cout << "\nLogged out.\n";
                pauseScreen();
                return;
        }
    }
}

void GasStation::updateFuelPrice() {
    float price;    int choice, index;

    clrscr();
    heading("UPDATE FUEL PRICE");
    printFuelMenu();
    choice = readChoice(1, 4);
    while (choice == -1) { showError(ERR_NOT_NUMBER, "Choice"); choice = readChoice(1, 4); }
    if (choice == 4) return;
    index = choice - 1;

    while (1) {
        cout << "\nCurrent " << fuels[index]->getName() << " Price : Rs.";
        money(fuels[index]->getRate());
        cout << '\n';

        if (readValid("Enter New Price : Rs.", "Price", &price, 500.0) == 0) return;
        cout << "\nNew Price : Rs.";
        money(price);
        cout << "\n\n1. Confirm Update\n2. Enter Again\n3. Cancel\n";
        choice = readChoice(1, 3);
        if (choice == -1) { showError(ERR_NOT_NUMBER, "Choice"); continue; }
        if (choice == 1) {
            if (fuels[index]->setRate(price) == OK) {
                saveFuelPrices();
                cout << "\nPrice updated and saved in fuel_prices.txt\n";
            }
            else showError(ERR_TOO_BIG, "Price");
            pauseScreen();
            return;
        }
        if (choice == 2) continue;
        return;
    }
}

// operator + adds the whole day up, one bill at a time
float GasStation::totalSale() const {
    Bill total;
    int i;
    for (i = 0; i < billCount; i++) total = total + history[i];
    return total.getFinalAmount();
}

/* main()  -  the GasStation object is kept outside main() on purpose.  It holds
   an array of 100 Bill objects, and Turbo C++ 3.0 keeps only a small stack for
   the local variables of a function, so a local object could overflow it. */

GasStation station;

int main() {
    station.init();                    // read the two data files
    station.mainMenu();
    return 0;
}

