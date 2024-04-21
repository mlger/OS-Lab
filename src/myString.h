#ifndef MYSTRING_H
#define MYSTRING_H


#include <regex>
#include <stdexcept>
#include <string>
#include <vector>
class myString {
   private:
    std::vector<char> mystr;
    myString getSubString(int l, int r) {
        myString res;
        while (l < r) {
            res.append(this->charAt(l));
            l++;
        }
        return res;
    }

   public:
    // Construction
    myString() {
        mystr.clear();
        return;
    }

    myString(int x) {
		mystr.clear();
		if (x==0) {
			mystr.push_back('0');
			return;
		}
		bool flag = false;
		if (x<0){
			flag = true;
			x = -x;
		}
		while (x) {
			mystr.push_back(x%10+'0');
			x/=10;
		}
		if (flag) {
			mystr.push_back('-');
		}
		std::reverse(mystr.begin(), mystr.end());
    }

    myString(unsigned int x) {
		mystr.clear();
		if (x==0) {
			mystr.push_back('0');
			return;
		}
		while (x) {
			mystr.push_back(x%10+'0');
			x/=10;
		}
		std::reverse(mystr.begin(), mystr.end());		
    }

    myString(std::string str) {
        mystr.clear();
        for (auto c : str) {
            mystr.push_back(c);
        }
    }

    myString(char str[], int len = -1) {
        mystr.clear();
        if (len == -1) {
            for (int i = 0;; i++) {
                if (str[i] == '\0')
                    return;
                mystr.push_back(str[i]);
            }
        } else {
            for (int i = 0; i < len; i++) {
                mystr.push_back(str[i]);
            }
        }
    }

    myString(std::vector<char> str) {
        mystr.clear();
        for (auto c : str) {
            mystr.push_back(c);
        }
    }

    // append:
    void append(std::string str) {
        for (auto c : str) {
            mystr.push_back(c);
        }
    }

    void append(char* str) {
        for (int i = 0;; i++) {
            if (str[i] == '\0')
                return;
            mystr.push_back(str[i]);
        }
    }

    void append(myString str) { this->append(str.toCharArray()); }

    void append(char ch) { mystr.push_back(ch); }

    // transform:
    char* toCharArray() {
        size_t size = mystr.size();
        char* charArray = new char[size + 1];
        for (int i = 0; i < size; i++) {
            charArray[i] = mystr[i];
        }
        charArray[size] = '\0';
        return charArray;
    }

    std::string toString() {
        std::string str("");
        for (int i = 0; i < mystr.size(); i++) {
            str.push_back(mystr[i]);
        }
        return str;
    }

    // get:
    myString subString(int begin, int end = -1) {
        if (end == -1) {
            return this->getSubString(begin, this->mystr.size());
        }
        return this->getSubString(begin, end);
    }

    int getLength() { return this->mystr.size(); }

    char charAt(int index) {
        if (index < 0 || index >= mystr.size())
            throw std::invalid_argument("error index!");
        return this->mystr[index];
    }

    int length() { return this->mystr.size(); }

    int indexOf(char ch) {  // TODO: replace with the regex
        int len = this->getLength();
        for (int i = 0; i < len; i++) {
            if (this->charAt(i) == ch)
                return i;
        }
        return -1;
    }

    bool isEmpty() { return this->getLength() == 0; }

    void clear() {
        mystr.clear();
        return;
    }

    std::vector<myString> split(char ch) {  // TODO: replace with the regex
        std::vector<myString> res;
        myString temp;
        int len = this->getLength();
        for (auto c : mystr) {
            if (c == ch) {
                if (!temp.isEmpty()) {
                    res.push_back(temp);
                }
                temp.clear();
            } else {
                temp.append(c);
            }
        }
        if (!temp.isEmpty()) {
            res.push_back(temp);
        }
        return res;
    }

    myString aggregate(myString str) {
        myString res;
        res.append(this->toCharArray());
        res.append(str);
        return res;
    }
    void removeTail(char ch) {
        int len = this->getLength();
        while (len > 0 && this->mystr[len - 1] == ch) {
            mystr.pop_back();
            --len;
        }
    }

    bool equals(myString str) {
        if (this->getLength() != str.getLength()) {
            return false;
        }
        for (int i = 0; i < this->getLength(); i++) {
            if (this->charAt(i) != str.charAt(i)) {
                return false;
            }
        }
        return true;
    }

    void replace(std::regex pattern, myString str) {
        std::string s = this->toString();
        s = std::regex_replace(s, pattern, str.toString());
        mystr.clear();
        for (auto c : s) {
            mystr.push_back(c);
        }
    }

    void remove(std::regex pattern) {
        std::string s = this->toString();
        s = std::regex_replace(s, pattern, "");
        mystr.clear();
        for (auto c : s) {
            mystr.push_back(c);
        }
    }

    bool startWith(myString str) {
        return (this->getLength() >= str.length()) &&
               this->subString(0, str.length()).equals(str);
    }

    bool match(std::regex pattern) {
        std::string s = this->toString();
        return std::regex_match(s, pattern);
    }

	myString readLine() {
		myString res;
		char ch;
		while (true) {
			ch = getchar();
			if (ch == '\n') {
				break;
			}
			res.append(ch);
		}
		return res;
	}

    // operator:
    char operator[](int index) { return this->charAt(index); }

    myString operator+(myString str) {
        myString res;
        res.append(this->toCharArray());
        res.append(str);
        return res;
    }

    myString operator+(std::string str) {
        myString res;
        res.append(this->toCharArray());
        res.append(str);
        return res;
    }

    myString operator+(char ch) {
        myString res;
        res.append(this->toCharArray());
        res.append(ch);
        return res;
    }

    myString operator+(int x) {
        myString res;
        res.append(this->toCharArray());
        res.append(myString(x));
        return res;
    }

    myString operator+(unsigned int x) {
        myString res;
        res.append(this->toCharArray());
        res.append(myString(x));
        return res;
    }

    myString operator+(char* str) {
        myString res;
        res.append(this->toCharArray());
        res.append(str);
        return res;
    }

    myString operator+=(myString str) {
        this->append(str);
        return *this;
    }

    myString operator+=(std::string str) {
        this->append(str);
        return *this;
    }

    myString operator+=(char ch) {
        this->append(ch);
        return *this;
    }

    myString operator+=(int x) {
        this->append(myString(x));
        return *this;
    }

    myString operator+=(unsigned int x) {
        this->append(myString(x));
        return *this;
    }

    myString operator+=(char* str) {
        this->append(str);
        return *this;
    }
    bool operator==(myString str) { return this->equals(str); }
};

#endif  // MYSTRING_H