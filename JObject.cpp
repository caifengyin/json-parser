#include "JObject.h"
using namespace json;

// get_if定义在variant头文件中
// value的作用: 返回指向存在variant对象中值的指针
void* JObject::value()
{
    switch (m_type)
    {
        case T_NULL:
            return get_if<str_t>(&m_value);
        case T_BOOL:
            return get_if<bool_t>(&m_value);
        case T_INT:
            return get_if<int_t>(&m_value);
        case T_DOUBLE:
            return get_if<double_t>(&m_value);
        case T_LIST:
            return get_if<list_t>(&m_value);
        case T_DICT:
            return get_if<dict_t>(&m_value);
        case T_STR:
            return get_if<str_t>(&m_value);
        default:
            return nullptr;
    }
}
//用于简化指针强转过程的宏
#define GET_VALUE(type) *((type*) value)

string JObject::to_string()
{
    void* value = this->value();
    std::ostringstream os;
    // 通过m_type的类型, 输出value指向的值
    switch (m_type)
    {
        case T_NULL:
            os << "null";
            break;
        case T_BOOL: // bool_t
            if(GET_VALUE(bool))
                os << "true";
            else 
                os << "false";
            break;
        case T_INT: // int_t
            os << GET_VALUE(int); 
            break;
        case T_DOUBLE:  // double_t
            os << GET_VALUE(double);
            break;
        case T_STR:  // str_t
            os << '\"' << GET_VALUE(string) << '\"'; 
            break;
        case T_LIST:
        {
            list_t &list = GET_VALUE(list_t);
            os << '[';
            for (auto i = 0; i < list.size(); i++)
            {
                if (i != list.size() - 1)
                {
                    os << ((list[i]).to_string());
                    os << ',';
                } 
                else
                {
                    os << ((list[i]).to_string());
                }
                
            }
            os << ']';
            break;
        }
        case T_DICT:
        {
            dict_t &dict = GET_VALUE(dict_t);
            os << '{';
            for (auto it = dict.begin(); it != dict.end(); ++it)
            {
                if (it != dict.begin()) //为了保证最后的json格式正确
                    os << ',';
                os << '\"' << it->first << "\":" << it->second.to_string();
            }
            os << '}';
            break;
        }
        default:
            return "";
    }
    return os.str();
}