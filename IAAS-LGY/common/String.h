#ifndef STRING_H
#define STRING_H
#include <iostream>
#include <cstddef> // size_t
#include <cstring>
#include <cstdlib>
#include <sstream>

#define BUFF_SIZE 4000
using namespace std;

class String {
    friend ostream& operator<<(ostream&, const String&); //1.友元   必须是两个参数
    friend istream& operator>>(istream&, String&);
public:
    //String();  //2.放在cpp中实现
    String(const char* str );//3默认实参
    String();
	String(string& str);
    // copy control
    String(const String&); //复制构函
    String& operator=(const String&);
    String& operator=(const char*); 
	String(int i);
    ~String();
	
    //算术运算
    friend String operator+(const char*, const String&) ;
    String operator+(const String&) const; // 一个参数！返回值不能是引用 +操作不能改变本身的值  
    String& operator+=(const String&);
    //String operator+(const char*); 
    //String& operator+=(const char*&);

    // 关系操作 < == (有了<就有了>，就有了>=和<=)
    bool operator<(const String&) const;
    bool operator==(const String&) const ;

    //bool operator>(const String&);
    bool operator>(const String& rhs) const { return rhs < *this; } // rhs为const左操作数，若无const版本的<则出错
    bool operator>=(const String& rhs) const { return !(*this < rhs); }
    bool operator<=(const String& rhs) const { return !(rhs < *this); }
    bool operator!=(const String& rhs) const    { return !(*this == rhs); }
    // 下标操作
    char& operator[](size_t); // 注意参数类型
    const char& operator[](size_t) const; // 参考primer p443
    // 得到子串，返回长度

	size_t size() const; 
	size_t length() const; 
	const char* c_str()
	{
		return m_data;
	}

	bool hasStr(String str);
	void toLower();
private:
    char *m_data;
    void setString(const char*);
	
};

inline String String::operator +(const String& rhs) const
{
   String ret(*this); // 执行之后ret的m_data的长度与lhs的相等，不能再cat
   ////strcat(ret.m_data, rhs.m_data);
   ret += rhs; // 使用+=来实现+更有效
   return ret;
}





#endif
