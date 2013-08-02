#ifndef STRING_H
#define STRING_H
#include <iostream>
#include <cstddef> // size_t
#include <cstring>
using namespace std;

class String {
    friend ostream& operator<<(ostream&, const String&); //1.��Ԫ   ��������������
    friend istream& operator>>(istream&, String&);
public:
    //String();  //2.����cpp��ʵ��
    String(const char* str );//3Ĭ��ʵ��
    String();
    // copy control
    String(const String&); //���ƹ���
    String& operator=(const String&);
    String& operator=(const char*); 
    ~String();
    String(int i);
    //��������
    friend String operator+(const char*, const String&) ;
    friend String operator+(const String&,const char*) ;
    String operator+(const String&) const; // һ�������ֵ���������� +�������ܸı䱾���ֵ  
    String& operator+=(const String&);
    //String operator+(const char*); 
    //String& operator+=(const char*&);

    // ��ϵ���� < == (����<������>��������>=��<=)
    bool operator<(const String&) const;
    bool operator==(const String&) const ;

    //bool operator>(const String&);
    bool operator>(const String& rhs) const { return rhs < *this; } // rhsΪconst�����������const�汾��<�����
    bool operator>=(const String& rhs) const { return !(*this < rhs); }
    bool operator<=(const String& rhs) const { return !(rhs < *this); }
    bool operator!=(const String& rhs) const    { return !(*this == rhs); }
    // �±����
    char& operator[](size_t); // ע���������
    const char& operator[](size_t) const; // �ο�primer p443
    // �õ��Ӵ������س���

	size_t size() const; 
	size_t length() const; 
	const char* c_str()
	{
		return m_data;
	}

private:
    char *m_data;
    void setString(const char*);
};

inline String String::operator +(const String& rhs) const
{
   String ret(*this); // ִ��֮��ret��m_data�ĳ�����lhs����ȣ�������cat
   ////strcat(ret.m_data, rhs.m_data);
   ret += rhs; // ʹ��+=��ʵ��+����Ч
   return ret;
}





#endif
