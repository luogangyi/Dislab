#include <iostream>
#include<stdlib.h>
#include<sstream>
#include "String.h"

String operator+(const char* c, const String& rhs) 
{
	String tmp=c;
	return tmp + rhs;
}

ostream& operator<<(ostream& out, const String& str) {
    out << str.m_data;
    return out;
}
istream& operator>>(istream& in, String& str) {
    in >> str.m_data;
    if (!in) {
        str = String();
    }
    return in;
}

String::String() {
	
    setString("");
	
}

String::String(const float f)
{
	ostringstream ofs;
	ofs<<f;
	string s=ofs.str();
	setString(s.c_str());

}

String::String(const unsigned int num)
{
	ostringstream ofs;
	ofs<<num;
	string s=ofs.str();
	setString(s.c_str());
}

String::String(const long num)
{
	ostringstream ofs;
	ofs<<num;
	string s=ofs.str();
	setString(s.c_str());
}

//String::String(const char* str = "") {
String::String(const char* str) {
    setString(str);
}

String::String(const int num)
{
	ostringstream ofs;
	ofs<<num;
	string s=ofs.str();
	setString(s.c_str());
}

String::String(const String &other) {
    setString(other.m_data);
}

String&
String::operator =(const char *str) {
    if (strcmp(m_data, str) == 0) { // 
        return *this;
    }
    delete[] m_data; // ���ͷ�
    setString(str);
    return *this;
}

String&
String::operator =(const String&rhs) {
    if (this == &rhs)
        return *this;
    else {		
		cout<<"m_data:"<<rhs<<endl;		
		cout<<"before"<<endl;
        delete[] m_data; // ���ͷ�
        cout<<"after"<<endl;
        setString(rhs.m_data);
    }
    return *this;
}

String::~String() {
	if( m_data == NULL)	
    	delete[] m_data;
	
}


 String&
String::operator +=(const String& rhs) {
    //strcat(m_data, rhs.m_data); // �������strcat
    int n = size() + rhs.size(); // rhs�Ҳ���const�汾��size()�����const������Ե���const�汾�ĺ���
    char *tmp = new char[n+1];
    strcpy(tmp, m_data);
    strcat(tmp, rhs.m_data);
    delete[] m_data;
    m_data = tmp;

    return *this;
}

 bool
String::operator<(const String& rhs) const {
    int ret = strcmp(m_data, rhs.m_data);
    if (ret < 0) 
		return true;
    else 
		return false;
}


bool String::operator ==(const String& rhs) const {
    int ret = strcmp(m_data, rhs.m_data);
    if (ret == 0) 
		return true;
    else 
		return false;
}

 char&
String::operator [](size_t ix) {
    size_t n = strlen(m_data);
    if (ix < 0 || ix > n) { // ix can be n, [0n]
        cerr << "Error: "<< ix << "out of range" << endl;
        exit(1); 
    }
    return m_data[ix];
}

 const char&
String::operator [](size_t ix) const {
    size_t n = strlen(m_data);
    if (ix < 0 || ix > n) { // ix can be n, [0n]
        cerr << "Error: " << ix << " out of range" << endl;
        exit(1); 
    }
    return m_data[ix];
}

inline size_t String::length() const {
    return strlen(m_data);
}

inline size_t String::size() const {
    return strlen(m_data);
}

void String::setString(const char* str) 
{
	size_t n = 0;
	if( str != NULL )		
	    n = strlen(str);
	
    m_data = new char[n+1];

    if (n > 0) {
        strcpy(m_data, str);
    } else {
        m_data[0] = '\0';
    }
}
