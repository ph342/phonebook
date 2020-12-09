#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

/*  this is needed to make the operators >, !=, etc. work for class "person",
    even though we only implemented < and == */
using namespace std::rel_ops;

class person {
    private:
        string n, surn;

    public:
        person(const string& name, const string& surname) : n(name), surn(surname) {}

        // The default copy constructor and assignment operator= don't need to be overridden.

        // declaration of a virtual destructor in case it needs to be overriden by child classes
        virtual ~person() {};

        string name() const {
            return n;
        }

        string surname() const {
            return surn;
        }

        virtual string print_details() const {
            // write this person to a string
            return "<person S " + surname() + " N " + name();
        }

        virtual bool has_telephone() const {
            return false;
        }

        bool operator==(const person& p) const {
            // persons are equal if the names match
            return this->n == p.name() && this->surn == p.surname();
        }

        bool operator<(const person& p) const {
            if (this->surn < p.surname())
                // the surname is smaller
                return true;

            else if (this->surn == p.surname())
                // if the surnames are equal, equality depends on the name
                return this->n < p.name();

            else
                // both names are equal or p is greater than *this
                return false;
        }
};

class person_with_telephone : public virtual person {
    private:
        string tel;

    public:
        person_with_telephone(const string& name, const string& surname, const string& telephone) : person(name, surname), tel(telephone) {}

        virtual string telephone() const {
            return tel;
        }

        virtual bool has_telephone() const override {
            return true;
        }

        virtual void set_telephone(const string& telephone) {
            tel = telephone;
        }

        virtual string print_details() const override {
            // print the details from the base class and add the telephone number
            return person::print_details() + " T " + telephone();
        }
};

class person_with_email : public virtual person {
    private:
        string em;

    public:
        person_with_email(const string& name, const string& surname, const string& email) : person(name, surname), em(email) {}

        virtual string email() const {
            return em;
        }

        virtual bool has_telephone() const override {
            return false;
        }

        virtual void set_email(const string& email) {
            em = email;
        }

        virtual string print_details() const override {
            return person::print_details() + " E " + email();
        }
};

class person_with_telephone_and_email : public person_with_telephone, public person_with_email {

    public:
        // call the constructors in the correct order
        person_with_telephone_and_email(const string& name, const string& surname, const string& email, const string& telephone): person(name, surname), person_with_telephone(name, surname, telephone), person_with_email(name, surname, email) {}

        virtual bool has_telephone() const override {
            return true;
        }

        virtual string print_details() const override {
            /*  We could have used auxiliary member functions to print the data one by one:
                person::print_details();
                person_with_telephone::print_telephone();
                person_with_email::print_email(); */
            return person_with_telephone::print_details() + " E " + email();
        }
};

ostream& operator<<(ostream& o, const person& p) {
    // dynamic binding through overriding of "print_details"
    return o << p.print_details() << " >";
}

class contacts {

    private:
        vector<person*> p;

        void sort_contacts() {
            /* The phone book uses a vector which does not keep the pointers in a persistent order. */
            sort(p.begin(), p.end(), [](const person * pp1, const person * pp2) {
                return *pp1 < *pp2;
            });
        }

    public:
        void add(person* newp) { // pass pointer to person by value
            // see if we already have this person
            for(auto i = begin(p); i != end(p); ++i) {
                if (**i == *newp) {
                    // replace the existing person
                    *i = newp;
                    return;
                }
            }
            // if we get here, the person needs to be added
            p.push_back(newp);

            // when the contacts are changed, sort them
            sort_contacts();
        }

        const vector<const person*> find(const string& term) const {
            /*  Return a read-only vector of read-only persons.
                Returning by value is okay because the vector only contains small pointers. */
            vector<const person*> ret;

            for_each(begin(p), end(p), [&term, &ret](const person * pp) {
                if(pp->name() == term || pp->surname() == term)
                    ret.push_back(pp);
            });
            return ret;
        }

        void in_order(ostream& out) const {
            // print all persons to an output stream
            for_each(begin(p), end(p), [&out](const person * pp) {
                out << *pp << '\n';
            });
        }

        void with_telephone(ostream& out) const {
            // print all persons to an output stream who have a telephone number
            for_each(begin(p), end(p), [&out](const person * pp) {
                if (pp->has_telephone())
                    out << *pp << '\n';
            });
        }

};

istream& read_person(istream& in, person*& p) {
    char indicator;
    string dummy, name, surname, telephone, email;
    bool line_read(false);

    // read in the first part of the line ("<person") and
    // also check if the stream is valid
    if(!(in >> dummy)) {
        in.setstate(ios::badbit);
        return in;
    }

    // read in the indicating character (T for telephone...)
    // and the subsequent value
    while(in >> indicator && line_read == false) {
        switch(indicator) {
            case 'S' :
                in >> surname;
                break;
            case 'N' :
                in >> name;
                break;
            case 'T' :
                in >> telephone;
                break;
            case 'E' :
                in >> email;
                break;
            case '>':
            case '\n':
                line_read = true;
                break;
            default :
                in.setstate(ios::badbit);
                return in;
        }
    }

    // instantiate accordingly
    if (telephone != "" && email != "")
        p = new person_with_telephone_and_email(name, surname, email, telephone);
    else if (telephone != "")
        p = new person_with_telephone(name, surname, telephone);
    else if (email != "")
        p = new person_with_email(name, surname, email);
    else
        p = new person(name, surname);

    return in;
}

int main() {
    person* pp = 0;
    contacts book;

    while (read_person(cin, pp) && pp)
        book.add(pp);

    //add last person in the input
    book.add(pp);

    book.in_order(cout);
    cout << '\n';

    // upgrade a person to a person_with_email
    person_with_email tom("Tom", "Smith", "smith@mail.net");
    book.add(&tom); // overwrite the person

    book.in_order(cout);
    cout << '\n';

    // find all Johns and print them
    auto searchresults = book.find("John");
    for_each(begin(searchresults), end(searchresults), [](const person * pp) {
        cout << *pp << endl;
    });

    cout << '\n';

    // print all persons with a telephone number
    book.with_telephone(cout);

    return 0;
}