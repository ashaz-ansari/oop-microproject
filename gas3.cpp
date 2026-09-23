/* =========================================================================
   GAS STATION MANAGEMENT SYSTEM  -  MICROPROJECT ( OOP , CM31203 )

   Three of us worked on this file :
        <MEMBER 1>   Part 1   checking the input, helper routines, String class
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

// the codes that all the checking routines give back
enum ErrCode
{
    OK = 0,                  // all right
    ERR_NOT_NUMBER = 1,      // a letter or a symbol was typed, not a number
    ERR_NOT_POSITIVE = 2,    // zero or a negative value
    ERR_TOO_BIG = 3,         // bigger than the limit we allow
    ERR_EMPTY = 4,           // nothing was typed at all
    ERR_BAD_VEHICLE = 5,     // not a proper vehicle number
    ERR_BAD_NAME = 6,        // the name has a digit or a symbol in it
    ERR_FILE = 7,            // a file could not be opened
    ERR_NOT_FOUND = 8        // that transaction id is not there
};

// one record of the binary backup file transactions.dat
struct TxnRecord
{
    int   id;
    char  name[22];
    char  vehicle[13];
    char  fuel[11];
    float quantity;
    float rate;
    float amount;
    float discount;
    float finalAmount;
};

// prints the character ch, n times.  both the arguments have a default value
inline void line(char ch = '=', int n = RULE_WIDTH) {
    int i;

    for (i = 0; i < n; i++)
        cout << ch;
    cout << '\n';
}

// a line of = signs, the title in the middle, and another line of = signs
void heading(const char *title, int width = RULE_WIDTH) {
    line('=', width);
    cout << "      " << title << '\n';
    line('=', width);
}

// waits for one key press so that the screen does not go away
inline void pauseScreen() {
    cout << "\nPress any key to continue...";
    getch();
}

// 0 = not a number, 1 = a positive number, 2 = a negative number
int checkNumber(const char *s) {
    int i = 0, dot = 0, digits = 0, neg = 0;

    if (s[0] == '-') {
        neg = 1;
        i = 1;
    }
    if (s[i] == '\0')
        return 0;

    for (; s[i] != '\0'; i++) {
        if (s[i] == '.') {
            if (dot != 0)
                return 0;                 // a second dot, so not a number
            dot = 1;
        }
        else if (s[i] >= '0' && s[i] <= '9')
            digits++;
        else
            return 0;                     // something that is not a digit
    }

    if (digits == 0)
        return 0;
    if (neg != 0)
        return 2;
    return 1;
}

// prints the message that goes with an error code
void showError(int code, const char *field) {
    cout << "\nError: " << field;

    switch (code) {
    case ERR_NOT_NUMBER:
        cout << " must be a number.\n";
        break;
    case ERR_NOT_POSITIVE:
        cout << " must be greater than zero.\n";
        break;
    case ERR_TOO_BIG:
        cout << " is too large. Please enter less.\n";
        break;
    case ERR_EMPTY:
        cout << " cannot be empty.\n";
        break;
    case ERR_BAD_VEHICLE:
        cout << " is not valid. Example: MH12AB1234\n";
        break;
    case ERR_BAD_NAME:
        cout << " must contain letters only.\n";
        break;
    case ERR_FILE:
        cout << " file could not be opened.\n";
        break;
    case ERR_NOT_FOUND:
        cout << " was not found.\n";
        break;
    default:
        cout << " is not correct.\n";
        break;
    }
}

// asks for one number, checks it, and writes it back through the pointer
int readNumber(const char *prompt, const char *field, float *value, float maxValue) {
    char buf[30];
    int  kind;

    cout << prompt;
    cin.getline(buf, 30);
    if (cin.fail()) {                    // the line was longer than 30 characters
        cin.clear();
        cin.ignore(100, '\n');
    }

    kind = checkNumber(buf);
    if (kind == 0) {
        showError(ERR_NOT_NUMBER, field);
        return ERR_NOT_NUMBER;
    }

    *value = (float) atof(buf);          // text to float

    if (kind == 2 || *value <= 0.0) {
        showError(ERR_NOT_POSITIVE, field);
        return ERR_NOT_POSITIVE;
    }
    if (maxValue > 0.0 && *value > maxValue) {
        showError(ERR_TOO_BIG, field);
        return ERR_TOO_BIG;
    }

    return OK;
}

// reads one line of text.  1 if something was typed, 0 if it was empty
int readText(const char *prompt, char *buf, int size) {
    cout << prompt;
    cin.getline(buf, size);
    if (cin.fail()) {
        cin.clear();
        cin.ignore(200, '\n');
    }

    if (strlen(buf) == 0)
        return 0;
    return 1;
}

// a name may have letters, spaces and dots only
int checkName(const char *s) {
    int i, letters = 0;

    for (i = 0; s[i] != '\0'; i++) {
        char c = s[i];

        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              c == ' ' || c == '.'))
            return 0;
        if (c != ' ')
            letters++;
    }

    if (letters == 0)
        return 0;
    return 1;
}

// a vehicle number like MH12AB1234 : letters and digits, at least six of them
int checkVehicle(const char *s) {
    int i, count = 0;

    for (i = 0; s[i] != '\0'; i++) {
        char c = s[i];

        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9')))
            return 0;
        count++;
    }

    if (count < 6)
        return 0;
    return 1;
}

// reads one menu choice.  -1 means the choice was wrong
int readChoice(int low, int high) {
    char buf[10];
    int  n;

    cout << "\nEnter your choice : ";
    cin.getline(buf, 10);
    if (cin.fail()) {
        cin.clear();
        cin.ignore(100, '\n');
    }

    if (checkNumber(buf) != 1)
        return -1;

    n = atoi(buf);
    if (n < low || n > high)
        return -1;
    return n;
}

// the "try again" or "go back" question that every error asks
int askRetry() {
    int c;

    cout << "\n1. Try Again\n";
    cout << "2. Back\n";
    c = readChoice(1, 2);

    if (c == -1) {
        showError(ERR_NOT_NUMBER, "Choice");
        return 0;                        // a wrong choice means go back
    }
    if (c == 1)
        return 1;
    return 0;
}

// cuts a line like  Petrol|105.50  into its fields at every | sign
int splitLine(char *text, char fields[][30], int maxFields) {
    int n = 0, i = 0, j;

    while (text[i] != '\0' && n < maxFields) {
        j = 0;
        while (text[i] != '\0' && text[i] != '|' && j < 29) {
            fields[n][j] = text[i];
            j++;
            i++;
        }
        fields[n][j] = '\0';
        n++;
        if (text[i] == '|')
            i++;
    }
    return n;
}

// money is always shown with two decimals
void money(float v) {
    cout << setiosflags(ios::fixed) << setprecision(2) << v;
}

// rounds a value to the given number of decimals
float roundTo(float v, int decimals) {
    float factor = 1.0;
    int   i;

    for (i = 0; i < decimals; i++)
        factor = factor * 10.0;

    return (float) (floor(v * factor + 0.5) / factor);
}

// rounds a value up to the given number of decimals
float roundUpTo(float v, int decimals) {
    float factor = 1.0;
    int   i;

    for (i = 0; i < decimals; i++)
        factor = factor * 10.0;

    return (float) (ceil(v * factor) / factor);
}

// our own String class.  Turbo C++ 3.0 has no string class, so the characters
// are kept in one fixed array and the operators are overloaded for it
class String {
    char data[40];

public:
    String();                        // an empty string
    String(const char *s);           // built from "Petrol" and such
    String(const String &s);         // copy constructor
    ~String();                       // destructor

    String& operator=(const String &s);
    String& operator=(const char *s);
    String  operator+(const String &s) const;
    int     operator==(const String &s) const;
    int     operator!=(const String &s) const;
    int     operator<(const String &s) const;
    int     operator!() const;       // 1 if the string is empty
    char&   operator[](int i);
    char    operator[](int i) const;

    int         length() const;
    void        upper();
    void        swap(String &s);
    const char* c_str() const;

    friend ostream& operator<<(ostream &os, const String &s);
};

String::String() {
    data[0] = '\0';
}

String::String(const char *s) {
    strncpy(data, s, 39);
    data[39] = '\0';
}

String::String(const String &s) {
    strcpy(data, s.data);
}

String::~String() {
    // the characters live inside the object itself, so nothing to free
}

String& String::operator=(const String &s) {
    if (this == &s)                  // copying the object onto itself
        return *this;
    strcpy(data, s.data);
    return *this;
}

String& String::operator=(const char *s) {
    strncpy(data, s, 39);
    data[39] = '\0';
    return *this;
}

String String::operator+(const String &s) const {
    String temp(data);
    int room = 39 - (int) strlen(temp.data);

    if (room > 0)
        strncat(temp.data, s.data, room);
    return temp;
}

int String::operator==(const String &s) const {
    return strcmp(data, s.data) == 0;
}

int String::operator!=(const String &s) const {
    return strcmp(data, s.data) != 0;
}

int String::operator<(const String &s) const {
    return strcmp(data, s.data) < 0;
}

int String::operator!() const {
    return data[0] == '\0';
}

char& String::operator[](int i) {
    return data[i];
}

char String::operator[](int i) const {
    return data[i];
}

int String::length() const {
    return (int) strlen(data);
}

void String::upper() {
    int i;

    for (i = 0; data[i] != '\0'; i++)
        if (data[i] >= 'a' && data[i] <= 'z')
            data[i] = data[i] - 32;
}

void String::swap(String &s) {
    char temp[40];

    strcpy(temp, data);
    strcpy(data, s.data);
    strcpy(s.data, temp);
}

const char* String::c_str() const {
    return data;
}

ostream& operator<<(ostream &os, const String &s) {
    os << s.data;
    return os;
}



/* =========================================================================
   PART 2   -   <MEMBER 2>
   The class hierarchy ( Person, Auth, Customer, Admin and the three fuels )
   and the Bill class, which is the class the whole program is built around.
   ========================================================================= */

/* Person is the base class of the customer side.  The name is protected so
   that the classes below can use it directly, but main() cannot. */
class Person
{
protected:
    String name;

public:
    Person();
    Person(const String &n);

    void setName(const String &n);
    const String& getName() const;
    void show() const;
};

Person::Person() : name("")
{
}

Person::Person(const String &n) : name(n)
{
}

void Person::setName(const String &n)
{
    name = n;
}

const String& Person::getName() const
{
    return name;
}

void Person::show() const
{
    cout << "Name : " << name << '\n';
}

/* Auth keeps the login of the administrator.  It is the second base class of
   Admin, and that is where the multiple inheritance of this project comes in. */
class Auth
{
protected:
    String user;
    String pass;

public:
    Auth();
    Auth(const String &u, const String &p);

    int verify(const String &u, const String &p) const;
};

Auth::Auth() : user("admin"), pass("admin123")
{
}

Auth::Auth(const String &u, const String &p) : user(u), pass(p)
{
}

int Auth::verify(const String &u, const String &p) const
{
    if (user == u && pass == p)
        return 1;

    return 0;
}

/* A Customer is a Person who also has a vehicle number.  This is the single
   inheritance, and Bill below makes the chain a multilevel one. */
class Customer : public Person
{
protected:
    String vehicle;

public:
    Customer();
    Customer(const String &n, const String &v);

    void setVehicle(const String &v);
    const String& getVehicle() const;
    void show() const;
};

Customer::Customer() : Person(), vehicle("")
{
}

/* The base constructor Person(n) runs before the body of this constructor,
   and then the vehicle is set. */
Customer::Customer(const String &n, const String &v) : Person(n), vehicle(v)
{
}

void Customer::setVehicle(const String &v)
{
    vehicle = v;
}

const String& Customer::getVehicle() const
{
    return vehicle;
}

void Customer::show() const
{
    Person::show();                    /* the base version is called first */
    cout << "Vehicle : " << vehicle << '\n';
}

/* Admin takes the name from Person and the login from Auth, so it has two
   base classes. */
class Admin : public Person, public Auth
{
public:
    Admin();

    int  login(const String &u, const String &p) const;
    void show() const;
};

Admin::Admin() : Person("Administrator"), Auth("admin", "admin123")
{
}

int Admin::login(const String &u, const String &p) const
{
    return verify(u, p);               /* verify() comes from the Auth base */
}

void Admin::show() const
{
    Person::show();
    cout << "Login : " << user << '\n';
}

/* Fuel is an abstract class.  getUnit() is a pure virtual function, so an
   object of Fuel itself can never be made - only of Petrol, Diesel or CNG. */
class Fuel
{
protected:
    String name;
    float  rate;

public:
    Fuel(const String &n, float r);
    virtual ~Fuel();                   /* virtual, so delete is safe */

    virtual float calculateAmount(float quantity) const;
    virtual float calculateQuantity(float amount) const;

    virtual const char* getUnit() const = 0;

    const String& getName() const;
    float getRate() const;
    virtual int setRate(float r);      /* 0 when the rate is accepted */

    virtual void show() const;
};

Fuel::Fuel(const String &n, float r) : name(n)
{
    rate = r;
}

Fuel::~Fuel()
{
    /* the virtual destructor runs when a derived object is deleted through
       a Fuel pointer, and the right one is chosen at run time */
}

float Fuel::calculateAmount(float quantity) const
{
    return quantity * rate;
}

/* the other way round : used when the customer fills for a round amount */
float Fuel::calculateQuantity(float amount) const
{
    return amount / rate;
}

const String& Fuel::getName() const
{
    return name;
}

float Fuel::getRate() const
{
    return rate;
}

int Fuel::setRate(float r)
{
    if (r <= 0.0)
        return ERR_NOT_POSITIVE;

    if (r > 500.0)                     /* a litre cannot cost as much as that */
        return ERR_TOO_BIG;

    rate = r;
    return OK;
}

void Fuel::show() const
{
    cout << "Fuel   : " << name << " @ Rs.";
    money(rate);
    cout << '\n';
}

/* Both of these inherit Fuel with the keyword virtual, so a DualFuel object
   gets only one copy of Fuel and not two. */
class LiquidFuel : virtual public Fuel
{
public:
    LiquidFuel(const String &n, float r) : Fuel(n, r)
    {
    }

    const char* getUnit() const
    {
        return "L";                    /* litres */
    }
};

class GasFuel : virtual public Fuel
{
public:
    GasFuel(const String &n, float r) : Fuel(n, r)
    {
    }

    const char* getUnit() const
    {
        return "Kg";                   /* kilograms */
    }
};

/* Petrol is sold by the litre, and the bill is rounded to the nearest paisa. */
class Petrol : public LiquidFuel
{
public:
    Petrol(float r);

    float calculateAmount(float quantity) const;
    void  show() const;
};

Petrol::Petrol(float r) : Fuel("Petrol", r), LiquidFuel("Petrol", r)
{
}

float Petrol::calculateAmount(float quantity) const
{
    return roundTo(quantity * rate, 2);
}

void Petrol::show() const
{
    cout << "Petrol : Rs.";
    money(rate);
    cout << " per Litre\n";
}

/* Diesel is a LiquidFuel like Petrol, and it can also be built out of a
   Petrol object - that is the class to class conversion. */
class Diesel : public LiquidFuel
{
public:
    Diesel(float r);
    Diesel(const Petrol &p, float dieselRate);

    float calculateAmount(float quantity) const;
    void  show() const;
};

Diesel::Diesel(float r) : Fuel("Diesel", r), LiquidFuel("Diesel", r)
{
}

/* The same fuel is being sold, only the rate changes from the petrol rate to
   the diesel rate. */
Diesel::Diesel(const Petrol &p, float dieselRate)
    : Fuel("Diesel", dieselRate), LiquidFuel("Diesel", dieselRate)
{
    (void) p;                          /* the petrol object shows the quantity */
}

float Diesel::calculateAmount(float quantity) const
{
    return roundTo(quantity * rate, 2);
}

void Diesel::show() const
{
    cout << "Diesel : Rs.";
    money(rate);
    cout << " per Litre\n";
}

/* CNG is sold by weight, so the amount is rounded UP to the paisa. */
class CNG : public GasFuel
{
public:
    CNG(float r);

    float calculateAmount(float quantity) const;
    void  show() const;
};

CNG::CNG(float r) : Fuel("CNG", r), GasFuel("CNG", r)
{
}

float CNG::calculateAmount(float quantity) const
{
    return roundUpTo(quantity * rate, 2);
}

void CNG::show() const
{
    cout << "CNG    : Rs.";
    money(rate);
    cout << " per Kg\n";
}

/* DualFuel takes after both LiquidFuel and GasFuel.  As Fuel is a virtual
   base class, the object holds only one Fuel inside it. */
class DualFuel : public LiquidFuel, public GasFuel
{
public:
    DualFuel();

    float calculateAmount(float quantity) const;
    const char* getUnit() const;       /* settles the ambiguity between the two */
};

DualFuel::DualFuel() : Fuel("Dual", 0.0), LiquidFuel("Dual", 0.0), GasFuel("Dual", 0.0)
{
}

float DualFuel::calculateAmount(float quantity) const
{
    return roundTo(quantity * rate, 2);
}

const char* DualFuel::getUnit() const
{
    return "unit";
}

/* Bill is the class the project is really about.  It inherits Customer, so a
   bill is also a customer and also a person.  It holds a Fuel pointer, and
   the call fuelPtr->calculateAmount() is decided at run time. */
class Bill : public Customer
{
private:
    static int count;                  /* the next transaction id */
    static int made;                   /* how many Bill objects were made */
    static int gone;                   /* how many were destroyed */

    int    id;
    int    fuelIndex;                  /* 0 Petrol, 1 Diesel, 2 CNG */
    String fuelName;
    String unit;                       /* L for the first two, Kg for CNG */
    Fuel  *fuelPtr;
    float  quantity;
    float  rate;
    float  amount;
    float  discount;
    float  finalAmount;
    int    mode;                       /* 1 by quantity, 2 by amount */

    /* private member functions : only this class can call them */
    float slabRate() const;
    void  calculateAmount();
    void  calculateDiscount();

public:
    Bill();
    Bill(const String &n, const String &v = "MH00AA0000");
    Bill(float amt);                   /* a plain amount becomes a bill */
    Bill(const Bill &b);               /* copy constructor */
    ~Bill();

    static int  nextId();
    static void syncId(int lastId);
    static int  madeCount();
    static int  goneCount();
    static void resetCounters();

    void   setFuel(int index, Fuel *f);
    Bill&  setQuantity(float q);
    Bill&  setAmount(float a);
    void   setId(int i);

    int           getId() const;
    int           getFuelIndex() const;
    const String& getFuelName() const;
    float         getQuantity() const;
    float         getRate() const;
    float         getAmount() const;
    float         getDiscount() const;
    float         getFinalAmount() const;
    int           getMode() const;
    unsigned long address() const;

    void showPreview() const;
    void showBill() const;
    void saveTo(ostream &out) const;
    int  loadFrom(const char *text);
    void toRecord(TxnRecord &r) const;
    void fromRecord(const TxnRecord &r);

    Bill& operator++();
    Bill  operator+(const Bill &b) const;
    int   operator>(const Bill &b) const;
    operator float() const;

    float toFloat() const;

    friend ostream& operator<<(ostream &os, const Bill &b);
};

/* the static data members have to be defined once, outside the class */
int Bill::count = 1001;
int Bill::made  = 0;
int Bill::gone  = 0;

Bill::Bill() : Customer(), id(0), fuelIndex(0), fuelName(""), unit("L"), fuelPtr(0),
               quantity(0.0), rate(0.0), amount(0.0), discount(0.0),
               finalAmount(0.0), mode(1)
{
    made++;
}

/* the vehicle number has a default value, so Bill("Rahul") alone is allowed */
Bill::Bill(const String &n, const String &v) : Customer(n, v), id(0), fuelIndex(0),
               fuelName(""), unit("L"), fuelPtr(0), quantity(0.0), rate(0.0),
               amount(0.0), discount(0.0), finalAmount(0.0), mode(1)
{
    made++;
}

/* a plain number such as Rs.500 is turned into a Bill object.  The quantity
   is worked out from the amount at the petrol rate. */
Bill::Bill(float amt) : Customer("Walk In Customer", "MH00AA0000"), id(0), fuelIndex(0),
               fuelName("Petrol"), unit("L"), fuelPtr(0), quantity(0.0), rate(105.50),
               amount(amt), discount(0.0), finalAmount(amt), mode(2)
{
    made++;
    quantity = amount / rate;
    calculateDiscount();
    finalAmount = amount - discount;
}

/* This one runs every time a Bill is passed to a function by value. */
Bill::Bill(const Bill &b) : Customer(b.getName(), b.getVehicle())
{
    made++;

    id          = b.id;
    fuelIndex   = b.fuelIndex;
    fuelName    = b.fuelName;
    unit        = b.unit;
    fuelPtr     = b.fuelPtr;
    quantity    = b.quantity;
    rate        = b.rate;
    amount      = b.amount;
    discount    = b.discount;
    finalAmount = b.finalAmount;
    mode        = b.mode;
}

Bill::~Bill()
{
    gone++;
}

int Bill::nextId()
{
    return count++;
}

/* after the file has been read, the ids have to continue from the last one */
void Bill::syncId(int lastId)
{
    count = lastId;
}

int Bill::madeCount()
{
    return made;
}

int Bill::goneCount()
{
    return gone;
}

/* The array of Bill objects inside GasStation is built before main() starts,
   so those 100 objects must not be counted as bills made by the user. */
void Bill::resetCounters()
{
    made = 0;
    gone = 0;
}

/* The bill keeps a pointer to the Fuel object, so it can ask the fuel itself
   what the amount is.  The rate and the unit are taken from the object too. */
void Bill::setFuel(int index, Fuel *f)
{
    fuelIndex = index;
    fuelPtr   = f;
    fuelName  = f->getName();
    unit      = f->getUnit();
    rate      = f->getRate();

    calculateAmount();
    calculateDiscount();
}

/* setQuantity() gives back *this, so calls can be joined together as
   bill.setQuantity(5).setAmount(500);  It also calls the two private member
   functions of the class, which is the nesting of member functions. */
Bill& Bill::setQuantity(float q)
{
    this->quantity = q;
    mode = 1;

    calculateAmount();
    calculateDiscount();

    return *this;
}

Bill& Bill::setAmount(float a)
{
    this->amount = a;
    mode = 2;

    if (rate > 0.0)
        this->quantity = a / rate;

    calculateDiscount();

    return *this;
}

void Bill::setId(int i)
{
    id = i;
}

int Bill::getId() const
{
    return id;
}

int Bill::getFuelIndex() const
{
    return fuelIndex;
}

const String& Bill::getFuelName() const
{
    return fuelName;
}

float Bill::getQuantity() const
{
    return quantity;
}

float Bill::getRate() const
{
    return rate;
}

float Bill::getAmount() const
{
    return amount;
}

float Bill::getDiscount() const
{
    return discount;
}

float Bill::getFinalAmount() const
{
    return finalAmount;
}

int Bill::getMode() const
{
    return mode;
}

/* the address of the object that called this member function */
unsigned long Bill::address() const
{
    return (unsigned long) this;
}

/* the discount slab : nothing below Rs.1000, then 2 per cent, then 5 */
float Bill::slabRate() const
{
    if (amount < 1000.0)
        return 0.0;

    if (amount < 3000.0)
        return 0.02;

    return 0.05;
}

/* amount = quantity x rate.  A bill that was made from an amount keeps that
   amount, and the quantity was already found back from it. */
void Bill::calculateAmount()
{
    if (mode == 1)
    {
        if (fuelPtr != 0)
            amount = fuelPtr->calculateAmount(quantity);
        else
            amount = roundTo(quantity * rate, 2);   /* a bill read from the file */
    }
    else
        amount = roundTo(amount, 2);
}

void Bill::calculateDiscount()
{
    discount    = roundTo(amount * slabRate(), 2);
    finalAmount = roundTo(amount - discount, 2);
}

/* This function uses the other member functions of the same class, such as
   getName() and getVehicle().  That is the nesting of member functions. */
void Bill::showPreview() const
{
    cout << "\nTransaction ID : " << id << '\n';
    cout << "Customer Name  : " << getName() << '\n';
    cout << "Vehicle No.    : " << getVehicle() << '\n';
    cout << "Fuel Type      : " << fuelName << '\n';

    cout << "Rate           : Rs.";
    money(rate);
    cout << "/" << unit << '\n';

    cout << "Quantity       : ";
    money(quantity);
    cout << " " << unit << '\n';

    cout << "Fuel Amount    : Rs.";
    money(amount);
    cout << '\n';

    cout << "Discount       : Rs.";
    money(discount);
    cout << '\n';

    cout << "Final Amount   : Rs.";
    money(finalAmount);
    cout << '\n';
}

void Bill::showBill() const
{
    heading("GAS STATION");

    cout << "\nTransaction ID : " << id << '\n';
    cout << "Customer Name  : " << getName() << '\n';
    cout << "Vehicle No.    : " << getVehicle() << "\n\n";

    cout << "Fuel           : " << fuelName << '\n';

    cout << "Rate           : Rs.";
    money(rate);
    cout << "/" << unit << '\n';

    cout << "Quantity       : ";
    money(quantity);
    cout << " " << unit << "\n\n";

    cout << "Fuel Amount    : Rs.";
    money(amount);
    cout << '\n';

    cout << "Discount       : Rs.";
    money(discount);
    cout << '\n';

    line('-');

    cout << "Final Amount   : Rs.";
    money(finalAmount);
    cout << '\n';

    line('=');
    cout << "          Thank You! Visit Again\n";
    line('=');
}

/* one transaction is written as nine fields separated by the | character */
void Bill::saveTo(ostream &out) const
{
    out << setiosflags(ios::fixed) << setprecision(2);

    out << id << '|' << getName().c_str() << '|'
        << getVehicle().c_str() << '|' << fuelName.c_str() << '|'
        << quantity << '|' << rate << '|'
        << amount << '|' << discount << '|' << finalAmount << '\n';
}

/* and read back from the same sort of line */
int Bill::loadFrom(const char *text)
{
    char f[9][30];

    if (splitLine((char *) text, f, 9) != 9)
        return 0;

    setId(atoi(f[0]));
    setName(f[1]);
    setVehicle(f[2]);

    fuelName    = f[3];
    quantity    = (float) atof(f[4]);
    rate        = (float) atof(f[5]);
    amount      = (float) atof(f[6]);
    discount    = (float) atof(f[7]);
    finalAmount = (float) atof(f[8]);

    mode    = 1;
    fuelPtr = 0;                       /* a bill from the file has no Fuel object */
    unit    = "L";

    if (fuelName == String("CNG"))
        unit = "Kg";

    return 1;
}

/* the same transaction as one fixed size record for the binary file */
void Bill::toRecord(TxnRecord &r) const
{
    r.id = id;

    strncpy(r.name, getName().c_str(), 21);
    r.name[21] = '\0';

    strncpy(r.vehicle, getVehicle().c_str(), 12);
    r.vehicle[12] = '\0';

    strncpy(r.fuel, fuelName.c_str(), 10);
    r.fuel[10] = '\0';

    r.quantity    = quantity;
    r.rate        = rate;
    r.amount      = amount;
    r.discount    = discount;
    r.finalAmount = finalAmount;
}

void Bill::fromRecord(const TxnRecord &r)
{
    setId(r.id);
    setName(r.name);
    setVehicle(r.vehicle);

    fuelName    = r.fuel;
    quantity    = r.quantity;
    rate        = r.rate;
    amount      = r.amount;
    discount    = r.discount;
    finalAmount = r.finalAmount;

    mode    = 1;
    fuelPtr = 0;
    unit    = "L";

    if (fuelName == String("CNG"))
        unit = "Kg";
}

/* ++bill moves the bill on to the next transaction id */
Bill& Bill::operator++()
{
    id = nextId();
    return *this;
}

/* bill1 + bill2 gives a bill whose amounts are the totals of both, and this
   is how the total sale of the day is worked out */
Bill Bill::operator+(const Bill &b) const
{
    Bill temp(*this);                  /* the copy constructor runs here */

    temp.amount      = amount      + b.amount;
    temp.discount    = discount    + b.discount;
    temp.finalAmount = finalAmount + b.finalAmount;
    temp.quantity    = quantity    + b.quantity;

    return temp;
}

/* two bills are compared by what the customer finally paid */
int Bill::operator>(const Bill &b) const
{
    if (finalAmount > b.finalAmount)
        return 1;

    return 0;
}

/* a Bill object can be used where a float is expected */
Bill::operator float() const
{
    return finalAmount;
}

float Bill::toFloat() const
{
    return finalAmount;
}

/* so that one whole row of the history can be printed with  cout << history[i] */
ostream& operator<<(ostream &os, const Bill &b)
{
    os << setiosflags(ios::left) << setw(7) << b.getId()
       << setw(16) << b.getName().c_str()
       << setw(14) << b.getVehicle().c_str()
       << setw(10) << b.getFuelName().c_str()
       << setiosflags(ios::right) << setw(10);

    os << setiosflags(ios::fixed) << setprecision(2) << b.getFinalAmount();

    return os;
}



/* =========================================================================
   PART 3   -   <MEMBER 3>
   The GasStation class that runs everything : the menus, the billing, the
   searching, the admin side and all the file work.  main() is at the end.
   ========================================================================= */

/* GasStation holds the three Fuel objects, one Admin object and the array of
   Bill objects, and it is the class that shows every screen of the program. */
class GasStation
{
private:
    Fuel *fuels[MAX_FUELS];            // Petrol, Diesel and CNG
    Admin admin;                       // the administrator of the pump
    Bill  history[MAX_BILLS];          // the transactions kept in memory
    int   billCount;                   // how many are there right now

    int   readValidQuantity(float *q);
    int   readValidAmount(float *a);
    int   readValidPrice(float *p);
    void  printMainMenu();
    void  printFuelMenu();
    void  printFillingMenu();
    void  printPreviewMenu();
    void  printSearchMenu();
    void  printAdminMenu();
    void  showOneBill(const Bill &b) const;
    void  showRateLine(int index) const;

public:
    GasStation();
    ~GasStation();

    void init();
    void mainMenu();
    void customerBilling();
    int  selectFuel();
    int  fillByQuantity(Bill &b);
    int  fillByAmount(Bill &b);
    void showPreviewLoop(Bill b);      // by value, so the copy constructor runs
    void generateBill(Bill &b);
    void saveTransaction(Bill b);
    void viewFuelPrices();
    void searchTransaction();
    void searchByTransactionId();
    void searchByVehicleNumber();
    void viewTransactions();
    void adminLogin();
    void adminMenu();
    void updateFuelPrice();
    void backupTransactions();
    void deleteTransactions();
    void oopDemo();
    void loadFuelPrices();
    void saveFuelPrices();
    void loadTransactions();
    float totalSale() const;
};

/* The three fuel objects are made on the heap while the program is running.
   This is the dynamic initialisation of objects, not at the start of main(). */
GasStation::GasStation() : admin()
{
    fuels[0] = new Petrol(105.50);
    fuels[1] = new Diesel(92.30);
    fuels[2] = new CNG(88.00);

    billCount = 0;
}

// the virtual destructor of Fuel makes these deletes all right
GasStation::~GasStation()
{
    int i;

    for (i = 0; i < MAX_FUELS; i++)
        delete fuels[i];
}

void GasStation::init()
{
    Bill::resetCounters();
    loadFuelPrices();
    loadTransactions();
}

// reads fuel_prices.txt.  each line is like   Petrol|105.50
void GasStation::loadFuelPrices()
{
    char  text[60];
    char  f[2][30];
    float r;
    int   i;

    ifstream fin("fuel_prices.txt");

    if (fin.fail())
    {
        cout << "\nError: fuel_prices.txt could not be opened.\n";
        cout << "The default prices have been loaded and the file will be created.\n";
        pauseScreen();

        saveFuelPrices();
        return;
    }

    for (i = 0; i < MAX_FUELS; i++)
    {
        fin.getline(text, 60);
        if (fin.fail())
            break;

        if (splitLine(text, f, 2) == 2)
        {
            r = (float) atof(f[1]);
            fuels[i]->setRate(r);
        }
    }

    fin.close();
}

void GasStation::saveFuelPrices()
{
    int i;

    ofstream fout("fuel_prices.txt", ios::out | ios::trunc);

    if (fout.fail())
    {
        showError(ERR_FILE, "fuel_prices.txt");
        return;
    }

    fout << setiosflags(ios::fixed) << setprecision(2);

    for (i = 0; i < MAX_FUELS; i++)
        fout << fuels[i]->getName() << '|' << fuels[i]->getRate() << '\n';

    fout.close();
}

// reads every transaction of transactions.txt into the array of objects, and
// leaves the next id ready after the last one that was in the file
void GasStation::loadTransactions()
{
    char text[120];
    int  lastId = 1000;

    ifstream fin("transactions.txt");
    billCount = 0;

    if (fin.fail())                    // first run, the file is not there yet
        return;

    while (billCount < MAX_BILLS)
    {
        fin.getline(text, 120);
        if (fin.fail())
            break;

        if (history[billCount].loadFrom(text) == 1)
        {
            if (history[billCount].getId() > lastId)
                lastId = history[billCount].getId();
            billCount++;
        }
    }

    fin.close();
    Bill::syncId(lastId + 1);
}

void GasStation::printMainMenu()
{
    clrscr();
    heading("GAS STATION MANAGEMENT SYSTEM");

    cout << "\n1. Customer Billing\n";
    cout << "2. View Fuel Prices\n";
    cout << "3. Search Transaction\n";
    cout << "4. View Transaction History\n";
    cout << "5. Admin Login\n";
    cout << "6. Exit\n";
}

void GasStation::mainMenu()
{
    int choice;

    do
    {
        printMainMenu();
        choice = readChoice(1, 6);

        while (choice == -1)
        {
            showError(ERR_NOT_NUMBER, "Choice");
            cout << "Please enter a number between 1 and 6.\n";
            choice = readChoice(1, 6);
        }

        switch (choice)
        {
            case 1 : customerBilling();     break;
            case 2 : viewFuelPrices();      break;
            case 3 : searchTransaction();   break;
            case 4 : viewTransactions();    break;
            case 5 : adminLogin();          break;
            case 6 : clrscr();
                     heading("GAS STATION MANAGEMENT SYSTEM");
                     cout << "\nThank you for using the system.\n";
                     cout << "Total sale of this session : Rs.";
                     money(totalSale());
                     cout << "\n\n";
                     break;
        }
    } while (choice != 6);
}

/* =========================================================================
   CUSTOMER BILLING
   ========================================================================= */

void GasStation::printFuelMenu()
{
    cout << "\n1. Petrol\n";
    cout << "2. Diesel\n";
    cout << "3. CNG\n";
    cout << "4. Back\n";
}

void GasStation::printFillingMenu()
{
    cout << "\n1. Fill by Quantity\n";
    cout << "2. Fill by Amount\n";
    cout << "3. Change Fuel\n";
    cout << "4. Cancel Billing\n";
}

void GasStation::printPreviewMenu()
{
    cout << "\n1. Confirm and Generate Bill\n";
    cout << "2. Change Quantity / Amount\n";
    cout << "3. Change Fuel\n";
    cout << "4. Cancel Transaction\n";
}

void GasStation::showRateLine(int index) const
{
    cout << "Selected Fuel : " << fuels[index]->getName() << '\n';

    cout << "Current Rate  : Rs.";
    money(fuels[index]->getRate());
    cout << "/" << fuels[index]->getUnit() << '\n';
}

/* the whole billing screen : the customer details first, then the fuel, then
   the filling method, and at the end the preview */
void GasStation::customerBilling()
{
    char name[22], vehicle[14];
    Bill bill;
    int  choice, fuel;

    clrscr();
    heading("CUSTOMER BILLING");

    cout << "\n1. Start New Billing\n";
    cout << "2. Back to Main Menu\n";
    choice = readChoice(1, 2);

    if (choice != 1)
        return;

    clrscr();
    heading("CUSTOMER DETAILS");

    while (1)
    {
        if (readText("\nEnter Customer Name : ", name, 22) == 1 &&
            checkName(name) == 1)
            break;

        if (strlen(name) == 0)
            showError(ERR_EMPTY, "Customer Name");
        else
            showError(ERR_BAD_NAME, "Customer Name");

        if (askRetry() == 0)
            return;
    }

    while (1)
    {
        if (readText("Enter Vehicle Number : ", vehicle, 14) == 1 &&
            checkVehicle(vehicle) == 1)
            break;

        if (strlen(vehicle) == 0)
            showError(ERR_EMPTY, "Vehicle Number");
        else
            showError(ERR_BAD_VEHICLE, "Vehicle Number");

        if (askRetry() == 0)
            return;
    }

    bill.setName(name);
    bill.setVehicle(vehicle);

    fuel = selectFuel();
    if (fuel == -1)
        return;

    // the base class pointer is kept inside the bill
    bill.setFuel(fuel, fuels[fuel]);

    clrscr();
    heading("SELECT FILLING METHOD");
    showRateLine(fuel);

    while (1)
    {
        printFillingMenu();
        choice = readChoice(1, 4);

        if (choice == -1)
        {
            showError(ERR_NOT_NUMBER, "Choice");
            continue;
        }

        if (choice == 4)
            return;                    // cancel the billing

        if (choice == 3)
        {
            fuel = selectFuel();
            if (fuel == -1)
                return;

            bill.setFuel(fuel, fuels[fuel]);

            clrscr();
            heading("SELECT FILLING METHOD");
            showRateLine(fuel);
            continue;
        }

        if (choice == 1)
        {
            if (fillByQuantity(bill) == 1)
                break;
            else
                return;
        }

        if (choice == 2)
        {
            if (fillByAmount(bill) == 1)
                break;
            else
                return;
        }
    }

    showPreviewLoop(bill);
}

// the bill is passed by value here, so the copy constructor runs
void GasStation::showPreviewLoop(Bill b)
{
    int choice;

    while (1)
    {
        clrscr();
        heading("BILL PREVIEW");
        b.showPreview();
        printPreviewMenu();

        choice = readChoice(1, 4);

        if (choice == -1)
        {
            showError(ERR_NOT_NUMBER, "Choice");
            continue;
        }

        if (choice == 1)
        {
            ++b;                       // the bill gets its transaction id
            generateBill(b);
            return;
        }

        if (choice == 2)               // fill the same bill again
        {
            if (b.getMode() == 1)
            {
                if (fillByQuantity(b) == 0)
                    return;
            }
            else
            {
                if (fillByAmount(b) == 0)
                    return;
            }
            continue;
        }

        if (choice == 3)
        {
            int fuel = selectFuel();
            if (fuel == -1)
                return;

            b.setFuel(fuel, fuels[fuel]);
            continue;
        }

        if (choice == 4)
        {
            cout << "\nTransaction cancelled.\n";
            pauseScreen();
            return;
        }
    }
}

// the choice becomes 0 for Petrol, 1 for Diesel and 2 for CNG.  -1 is Back
int GasStation::selectFuel()
{
    int choice;

    clrscr();
    heading("FUEL SELECTION");
    printFuelMenu();

    choice = readChoice(1, 4);

    while (choice == -1)
    {
        showError(ERR_NOT_NUMBER, "Choice");
        choice = readChoice(1, 4);
    }

    if (choice == 4)
        return -1;

    return choice - 1;
}

// the quantity is written back through the pointer.  1 = done, 0 = go back
int GasStation::readValidQuantity(float *q)
{
    int e;

    while (1)
    {
        e = readNumber("Enter Quantity : ", "Quantity", q, 200.0);

        if (e == OK)
            return 1;

        if (askRetry() == 0)
            return 0;
    }
}

int GasStation::readValidAmount(float *a)
{
    int e;

    while (1)
    {
        e = readNumber("Enter Amount : Rs.", "Amount", a, 100000.0);

        if (e == OK)
            return 1;

        if (askRetry() == 0)
            return 0;
    }
}

int GasStation::readValidPrice(float *p)
{
    int e;

    while (1)
    {
        e = readNumber("Enter New Price : Rs.", "Price", p, 500.0);

        if (e == OK)
            return 1;

        if (askRetry() == 0)
            return 0;
    }
}

int GasStation::fillByQuantity(Bill &b)
{
    float q;

    cout << "\n";

    if (readValidQuantity(&q) == 0)
        return 0;

    b.setQuantity(q);
    return 1;
}

// the customer says the amount instead, so quantity = amount / rate
int GasStation::fillByAmount(Bill &b)
{
    float a;

    cout << "\n";

    if (readValidAmount(&a) == 0)
        return 0;

    b.setAmount(a);
    return 1;
}

void GasStation::generateBill(Bill &b)
{
    clrscr();
    b.showBill();
    cout << "\n";
    pauseScreen();

    saveTransaction(b);
}

// also passed by value, so the copy constructor runs once more
void GasStation::saveTransaction(Bill b)
{
    ofstream fout("transactions.txt", ios::out | ios::app);

    if (fout.fail())
    {
        showError(ERR_FILE, "transactions.txt");
        return;
    }

    b.saveTo(fout);
    fout.close();

    if (billCount < MAX_BILLS)
        history[billCount++] = b;

    cout << "\nTransaction saved successfully.\n";
}

/* =========================================================================
   VIEW FUEL PRICES
   ========================================================================= */

void GasStation::viewFuelPrices()
{
    int i, choice;

    clrscr();
    heading("CURRENT FUEL PRICES");
    cout << "\n";

    for (i = 0; i < MAX_FUELS; i++)
        fuels[i]->show();              // the virtual function runs here

    cout << "\n1. Back to Main Menu\n";
    choice = readChoice(1, 1);

    while (choice == -1)
    {
        showError(ERR_NOT_NUMBER, "Choice");
        choice = readChoice(1, 1);
    }
}

/* =========================================================================
   SEARCH TRANSACTION
   ========================================================================= */

void GasStation::printSearchMenu()
{
    cout << "\n1. Search by Transaction ID\n";
    cout << "2. Search by Vehicle Number\n";
    cout << "3. Back to Main Menu\n";
}

void GasStation::showOneBill(const Bill &b) const
{
    cout << "\nTransaction ID : " << b.getId() << '\n';
    cout << "Customer Name  : " << b.getName() << '\n';
    cout << "Vehicle No.    : " << b.getVehicle() << '\n';
    cout << "Fuel Type      : " << b.getFuelName() << '\n';

    cout << "Quantity       : ";
    money(b.getQuantity());
    cout << '\n';

    cout << "Rate           : Rs.";
    money(b.getRate());
    cout << '\n';

    cout << "Final Amount   : Rs.";
    money(b.getFinalAmount());
    cout << '\n';
}

void GasStation::searchByTransactionId()
{
    float value;
    int   i, id, found, choice;

    while (1)
    {
        if (readNumber("\nEnter Transaction ID : ", "Transaction ID", &value, 999999.0) != OK)
        {
            if (askRetry() == 0)
                return;
            continue;
        }

        id    = (int) value;
        found = -1;

        for (i = 0; i < billCount; i++)
            if (history[i].getId() == id)
            {
                found = i;
                break;
            }

        if (found == -1)
        {
            clrscr();
            heading("SEARCH TRANSACTION");
            showError(ERR_NOT_FOUND, "Transaction");

            if (askRetry() == 0)
                return;
        }
        else
        {
            clrscr();
            heading("TRANSACTION FOUND");
            showOneBill(history[found]);
            pauseScreen();
            return;
        }
    }
}

// the vehicle number is changed to capitals and then compared, so that
// mh12ab1234 and MH12AB1234 both find the same transactions
void GasStation::searchByVehicleNumber()
{
    char   target[14];
    String key;
    int    i, found;

    if (readText("\nEnter Vehicle Number : ", target, 14) == 0)
        return;

    key = target;
    key.upper();

    found = 0;

    clrscr();
    heading("TRANSACTION FOUND");

    String msg = String("Search result for vehicle ") + key;
    cout << "\n" << msg << "\n";
    line('-');

    for (i = 0; i < billCount; i++)
    {
        String v = history[i].getVehicle();
        v.upper();

        if (v == key)
        {
            showOneBill(history[i]);
            line('-');
            found++;
        }
    }

    if (found == 0)
    {
        showError(ERR_NOT_FOUND, "Vehicle Number");

        if (askRetry() == 1)
            searchByVehicleNumber();
        return;
    }

    cout << "\n" << found << " transaction(s) found.\n";
    pauseScreen();
}

void GasStation::searchTransaction()
{
    int choice;

    while (1)
    {
        clrscr();
        heading("SEARCH TRANSACTION");
        printSearchMenu();

        choice = readChoice(1, 3);

        if (choice == -1)
        {
            showError(ERR_NOT_NUMBER, "Choice");
            continue;
        }

        if (choice == 1)
            searchByTransactionId();
        else if (choice == 2)
            searchByVehicleNumber();
        else
            return;
    }
}

/* every row is printed with  cout << history[i]  because the operator << was
   overloaded for the Bill class */
void GasStation::viewTransactions()
{
    int  i, j, choice;
    Bill total;

    clrscr();
    heading("TRANSACTION HISTORY");

    if (billCount == 0)
        cout << "\nNo transactions have been recorded yet.\n";
    else
    {
        cout << "\nID     Customer        Vehicle       Fuel         Amount\n";
        line('-', 57);

        for (i = 0; i < billCount; i++)
            cout << history[i] << '\n';

        line('-', 57);

        total = total + history[0];    // the operator + adds the bills up
        for (i = 1; i < billCount; i++)
            total = total + history[i];

        cout << "Total sale : Rs.";
        money(total.getFinalAmount());
        cout << "\n";
    }

    cout << "\n1. Search Transaction\n";
    cout << "2. Sort by Amount (highest first)\n";
    cout << "3. Back to Main Menu\n";
    choice = readChoice(1, 3);

    while (choice == -1)
    {
        showError(ERR_NOT_NUMBER, "Choice");
        choice = readChoice(1, 3);
    }

    if (choice == 1)
        searchTransaction();
    else if (choice == 2)
    {
        // a plain bubble sort, and the bills are compared with operator >
        for (i = 0; i < billCount - 1; i++)
            for (j = 0; j < billCount - 1 - i; j++)
                if (history[j + 1] > history[j])
                {
                    Bill temp = history[j];
                    history[j] = history[j + 1];
                    history[j + 1] = temp;
                }

        cout << "\nSorted by amount (highest first).\n";
        pauseScreen();
        viewTransactions();
    }
}

/* =========================================================================
   ADMIN LOGIN AND THE ADMIN MENU
   ========================================================================= */

void GasStation::adminLogin()
{
    char user[26], pass[26];
    int  attempts = 0, ok = 0;

    clrscr();
    heading("ADMIN LOGIN");

    while (attempts < 3 && ok == 0)
    {
        while (readText("\nEnter Username : ", user, 26) == 0)
            showError(ERR_EMPTY, "Username");

        while (readText("Enter Password : ", pass, 26) == 0)
            showError(ERR_EMPTY, "Password");

        if (admin.login(user, pass) == 1)
            ok = 1;
        else
        {
            attempts++;
            cout << "\nInvalid username or password!  ( attempt "
                 << attempts << " of 3 )\n";
        }
    }

    if (ok == 1)
    {
        cout << "\nLogin successful. Welcome, Administrator.\n";
        pauseScreen();
        adminMenu();
    }
    else
    {
        cout << "\nBack to the main menu.\n";
        pauseScreen();
    }
}

void GasStation::printAdminMenu()
{
    cout << "\n1. View Fuel Prices\n";
    cout << "2. Update Fuel Price\n";
    cout << "3. View All Transactions\n";
    cout << "4. Search Transaction\n";
    cout << "5. Backup Transactions (binary file)\n";
    cout << "6. Delete All Transaction Records\n";
    cout << "7. OOP Concepts Demo\n";
    cout << "8. Logout\n";
}

void GasStation::adminMenu()
{
    int choice;

    while (1)
    {
        clrscr();
        heading("ADMIN MENU");
        printAdminMenu();

        choice = readChoice(1, 8);

        if (choice == -1)
        {
            showError(ERR_NOT_NUMBER, "Choice");
            continue;
        }

        switch (choice)
        {
            case 1 : viewFuelPrices();      break;
            case 2 : updateFuelPrice();     break;
            case 3 : viewTransactions();    break;
            case 4 : searchTransaction();   break;
            case 5 : backupTransactions();  break;
            case 6 : deleteTransactions();  break;
            case 7 : oopDemo();             break;
            case 8 : cout << "\nLogged out.\n";
                     pauseScreen();
                     return;
        }
    }
}

void GasStation::updateFuelPrice()
{
    float price;
    int   choice, index;

    clrscr();
    heading("UPDATE FUEL PRICE");
    printFuelMenu();

    choice = readChoice(1, 4);

    while (choice == -1)
    {
        showError(ERR_NOT_NUMBER, "Choice");
        choice = readChoice(1, 4);
    }

    if (choice == 4)
        return;

    index = choice - 1;

    while (1)
    {
        cout << "\nCurrent " << fuels[index]->getName() << " Price : Rs.";
        money(fuels[index]->getRate());
        cout << '\n';

        if (readValidPrice(&price) == 0)
            return;

        cout << "\nNew Price : Rs.";
        money(price);
        cout << "\n\n1. Confirm Update\n";
        cout << "2. Enter Again\n";
        cout << "3. Cancel\n";
        choice = readChoice(1, 3);

        if (choice == -1)
        {
            showError(ERR_NOT_NUMBER, "Choice");
            continue;
        }

        if (choice == 1)
        {
            if (fuels[index]->setRate(price) == OK)
            {
                saveFuelPrices();
                cout << "\nPrice updated and saved in fuel_prices.txt\n";
            }
            else
                showError(ERR_TOO_BIG, "Price");

            pauseScreen();
            return;
        }

        if (choice == 2)
            continue;

        return;
    }
}

/* =========================================================================
   BINARY FILE AND FILE DELETING
   ========================================================================= */

/* Every transaction goes into transactions.dat as one fixed size record.
   After writing, the last record is read back by jumping straight to it,
   which shows the random access of a file. */
void GasStation::backupTransactions()
{
    TxnRecord rec;
    int  i;
    long offset, size;

    clrscr();
    heading("BACKUP TRANSACTIONS");

    if (billCount == 0)
    {
        cout << "\nThere is nothing to back up.\n";
        pauseScreen();
        return;
    }

    ofstream fout("transactions.dat", ios::out | ios::binary | ios::trunc);

    if (fout.fail())
    {
        showError(ERR_FILE, "transactions.dat");
        pauseScreen();
        return;
    }

    for (i = 0; i < billCount; i++)
    {
        history[i].toRecord(rec);
        fout.write((char *) &rec, (int) sizeof(TxnRecord));
    }

    size = (long) fout.tellp();
    fout.close();

    cout << "\n" << billCount << " record(s) written to transactions.dat\n";
    cout << "Size of one record  : " << (int) sizeof(TxnRecord) << " bytes\n";
    cout << "Size of the file    : " << size << " bytes\n";

    ifstream fin("transactions.dat", ios::in | ios::binary);

    if (fin.fail())
    {
        showError(ERR_FILE, "transactions.dat");
        pauseScreen();
        return;
    }

    // jump over all the records before the last one
    offset = (long) (billCount - 1) * (long) sizeof(TxnRecord);
    fin.seekg(offset);
    fin.read((char *) &rec, (int) sizeof(TxnRecord));

    if (fin.fail())
        cout << "\nError: the record could not be read back.\n";
    else
    {
        Bill check;
        check.fromRecord(rec);

        cout << "\nRandom access check - record number " << billCount
             << " read by seekg :\n";
        showOneBill(check);
    }

    fin.close();
    pauseScreen();
}

/* A plain copy of transactions.txt is made first, character by character with
   get() and put(), and then remove() deletes the file itself. */
void GasStation::deleteTransactions()
{
    char ch;
    int  choice;

    clrscr();
    heading("DELETE ALL TRANSACTION RECORDS");
    cout << "\nThis will delete every transaction of transactions.txt\n";

    cout << "\n1. Delete All Records\n";
    cout << "2. Cancel\n";
    choice = readChoice(1, 2);

    while (choice == -1)
    {
        showError(ERR_NOT_NUMBER, "Choice");
        choice = readChoice(1, 2);
    }

    if (choice == 2)
        return;

    ifstream fin("transactions.txt", ios::in);

    if (fin.fail())
    {
        cout << "\nThere is no transactions.txt to delete.\n";
        pauseScreen();
        return;
    }

    ofstream fout("transactions.bak", ios::out | ios::trunc);

    if (fout.fail())
    {
        showError(ERR_FILE, "transactions.bak");
        fin.close();
        pauseScreen();
        return;
    }

    while (fin.get(ch))
        fout.put(ch);

    fin.close();
    fout.close();

    cout << "\nA copy was kept in transactions.bak\n";

    if (remove("transactions.txt") == 0)
    {
        billCount = 0;
        cout << "transactions.txt has been deleted.\n";
        cout << "The file will be created again when the next bill is saved.\n";
    }
    else
        cout << "\nError: transactions.txt could not be deleted.\n";

    pauseScreen();
}

/* =========================================================================
   OOP CONCEPTS DEMONSTRATION
   One screen that shows every object oriented feature of the project, which
   is the screen to open while explaining the project.
   ========================================================================= */
void GasStation::oopDemo()
{
    int i, before;

    clrscr();
    heading("OOP CONCEPTS DEMONSTRATION");

    cout << "\nINHERITANCE\n";
    cout << "  Person -> Customer -> Bill          ( single + multilevel )\n";
    cout << "  Fuel   -> Petrol, Diesel, CNG       ( hierarchical )\n";
    cout << "  Admin  -> Person + Auth             ( multiple )\n";
    cout << "  DualFuel -> LiquidFuel + GasFuel    ( hybrid, virtual base )\n";

    cout << "\nSTATIC BINDING vs DYNAMIC BINDING\n";

    Petrol petrol(fuels[0]->getRate());
    Fuel  *fp = &petrol;
    Fuel  &fr = petrol;

    cout << "  petrol.calculateAmount(10)        = Rs.";
    money(petrol.calculateAmount(10));
    cout << "   ( static binding )\n";

    cout << "  fp->calculateAmount(10)           = Rs.";
    money(fp->calculateAmount(10));
    cout << "   ( dynamic binding - virtual )\n";

    cout << "  fr.Fuel::calculateAmount(10)      = Rs.";
    money(fr.Fuel::calculateAmount(10));
    cout << "   ( base version, forced )\n";

    cout << "\n  The same address is used, but the virtual function\n";
    cout << "  chooses Petrol::calculateAmount at run time.\n";

    cout << "\nVIRTUAL FUNCTION show()\n";

    for (i = 0; i < MAX_FUELS; i++)
    {
        cout << "  ";
        fuels[i]->show();              // one call, three different results
    }

    cout << "\nVIRTUAL BASE CLASS\n";

    DualFuel dual;
    cout << "  A DualFuel object has only ONE Fuel sub object because\n";
    cout << "  LiquidFuel and GasFuel inherit Fuel as a virtual base.\n";

    cout << "  dual.calculateAmount(10) = Rs.";
    money(dual.calculateAmount(10));
    cout << "\n  dual.getUnit() returns : " << dual.getUnit() << '\n';

    cout << "\nTYPE CONVERSION\n";

    Bill quick = (float) 500.00;       // basic to class
    cout << "  basic to class   : Bill quick = (float) 500.00;\n";
    cout << "                     quantity of that bill = ";
    money(quick.getQuantity());
    cout << " litre\n";

    float paid = quick;                // class to basic
    cout << "  class to basic   : float paid = quick;   paid = Rs.";
    money(paid);
    cout << '\n';

    Diesel diesel(petrol, fuels[1]->getRate());   // class to class
    cout << "  class to class   : Diesel diesel(petrol);\n";
    cout << "                     the same fuel now costs Rs.";
    money(diesel.calculateAmount(10));
    cout << " for 10 litre instead of Rs.";
    money(petrol.calculateAmount(10));
    cout << '\n';

    cout << "\nPOINTERS, 'this' POINTER AND OBJECT SIZE\n";
    cout << "  sizeof(Bill)                 = " << (int) sizeof(Bill) << " bytes\n";
    cout << "  sizeof(TxnRecord)            = " << (int) sizeof(TxnRecord) << " bytes\n";

    Bill *bp = new Bill(String("Demo Customer"), String("MH12AB0000"));
    cout << "  address of the heap object   = " << (unsigned long) bp << '\n';
    cout << "  bp->address() returns 'this' = " << bp->address() << '\n';
    cout << "  Both numbers are the same, so 'this' is the address of\n";
    cout << "  the object that called the member function.\n";

    delete bp;                         // the destructor runs here

    cout << "\nDESTRUCTOR\n";

    before = Bill::goneCount();
    {
        Bill temporary("Temporary Bill");
        cout << "  a Bill object was created inside a block\n";
    }
    cout << "  objects destroyed so far : " << Bill::goneCount()
         << "  ( before this screen : " << before << " )\n";
    cout << "  Bill objects created so far : " << Bill::madeCount() << '\n';

    cout << "\nOUR OWN String CLASS\n";

    String a("Petrol"), b("Diesel");

    cout << "  a = " << a << "     b = " << b << '\n';
    cout << "  a + b         = " << (a + b) << '\n';
    cout << "  a == b        = " << (a == b) << '\n';
    cout << "  a < b         = " << (a < b) << '\n';
    cout << "  a[0]          = " << a[0] << '\n';
    cout << "  a.length()    = " << a.length() << '\n';

    a.swap(b);
    cout << "  after a.swap(b) : a = " << a << "   b = " << b << '\n';

    String empty;
    cout << "  !empty        = " << (!empty) << "   ( 1 means the string is empty )\n";

    cout << "\n";
    pauseScreen();
}

// the operator + adds the whole day up, bill by bill
float GasStation::totalSale() const
{
    Bill total;
    int  i;

    for (i = 0; i < billCount; i++)
        total = total + history[i];

    return total.getFinalAmount();
}

/* =========================================================================
   main()
   The GasStation object is kept outside main() on purpose.  It holds an array
   of 100 Bill objects, and Turbo C++ 3.0 keeps only a small stack for the
   local variables of a function, so a local object here could overflow it.
   ========================================================================= */

GasStation station;

int main()
{
    cout << setiosflags(ios::fixed) << setprecision(2);

    station.init();                    // read the two data files
    station.mainMenu();

    return 0;
}


