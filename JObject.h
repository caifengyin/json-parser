#ifndef _JOBJECT_H
#define _JOBJECT_H

#include <stdexcept>
#include <utility>
#include <variant>
#include <map>
#include <vector>
#include <string>
#include <string_view>
#include <sstream>
#include <type_traits>

namespace json
{
    using std::variant;
    using std::map;
    using std::string;
    using std::string_view;
    using std::stringstream;
    using std::vector;
    using std::get_if;

    enum TYPE
    {
        T_NULL,
        T_BOOL,
        T_INT,
        T_DOUBLE,
        T_STR,
        T_LIST,
        T_DICT
    };

    // 当前JOBJECT为不完全类型, 使用时需要前向声明
    class JObject;  

    // json中的类型定义
    using null_t = string;                // "null"对应string类型
    using bool_t = bool;                  // "true" "false"对应bool类型 
    using int_t = int32_t;                // "Number"类型对应int和double类型
    using double_t = double;
    using str_t = string;                 // "String"对应string类型
    using list_t = vector<JObject>;       // "List"对应vector类型
    using dict_t = map<string, JObject>;  // "Dict"对应map类型

// std::is_same定义在type_traits头文件中, 原型为: 
// template <class T, class U> struct is_same;
// 作用: 比较类型T和类型U是否相同
// 若T和U的类型相同: std::is_same<typea,typeb>::value 为true;
// 若T和U的类型不同: std::is_same<typea,typeb>::value 为false;
// type_traits中定义了一系列类用于获取有关编译时间的类型信息
#define IS_TYPE(typea, typeb) std::is_same<typea,typeb>::value

    template<class T>
    constexpr bool is_basic_type()
    {
        // 判断当前类型T是否为基本数据类型: string,bool(true/false),double,int
        if constexpr(IS_TYPE(T, str_t)    ||
                     IS_TYPE(T, bool_t)   ||
                     IS_TYPE(T, double_t) ||
                     IS_TYPE(T, int_t))
            return true;
        return false;
    }

    class JObject
    {
    public:
        // template <class... Types> class variant;
        // 类型安全的union, 要么保存其代替类型之一的值, 要么没有值
        // 如果variant拥有某个对象类型T的值，则T的对象表示直接分配在变体本身的对象表示中。
        // 不允许variant分配额外的（动态）内存。variant不允许持有数组、引用、void类型
        // 默认构造的variant对象保留其第一个替代的值
        using value_t = variant<bool_t, int_t, double_t, str_t, list_t, dict_t>;

        // 构造函数重载: 构造JObject对象
        JObject()
        {
            m_type = T_NULL;
            m_value = "null";
        }

        JObject(int_t value)
        {
            Int(value);
        }

        JObject(double_t value)
        {
            Double(value);
        }

        JObject(bool_t value)
        {
            Bool(value);
        }

        JObject(str_t const &value)
        {
            Str(value);
        }

        JObject(list_t value)
        {
            List(std::move(value));
        }

        JObject(dict_t value)
        {
            Dict(std::move(value));
        }
        // Null(),Int(),Bool(),Double(),Str(),List(),Dict用于(隐式转换)构造json中对应数据类型的数据对象
        // 对于List和Dict类型初始时: 使用List和Dict类型的拷贝构造, 效率较低
        // 优化: 运用move的移动语义加快符合类型List和Dict的构造
        // std::move是将对象的状态或者所有权从一个对象转移到另一个对象，只是转移，
        // 没有内存的搬迁或者内存拷贝所以可以提高利用效率,改善性能.。
        void Null()
        {
            m_type = T_NULL;
            m_value = "null";
        }

        void Int(int_t value)
        {
            m_value = value;
            m_type = T_INT;
        }

        void Bool(bool_t value)
        {
            m_value = value;
            m_type = T_BOOL;
        }

        void Double(double_t value)
        {
            m_type = T_DOUBLE;
            m_value = value;
        }

        void Str(string_view value)
        {
            m_value = string(value);
            m_type = T_STR;
        }

        void List(list_t value)
        {
            m_value = std::move(value);
            m_type = T_LIST;
        }

        void Dict(dict_t value)
        {
            m_value = std::move(value);
            m_type = T_DICT;
        }

//        operator string()
//        {
//            return Value<string>();
//        }
//
//        operator int()
//        {
//            return Value<int>();
//        }
//
//        operator bool()
//        {
//            return Value<bool>();
//        }
//
//        operator double()
//        {
//            return Value<double>();
//        }

#define THROW_GET_ERROR(erron) throw std::logic_error("type error in get "#erron" value!")

        template<class V>
        V &Value()  // Value根据值的类型构造值
        {
            //添加安全检查
            if constexpr(IS_TYPE(V, str_t))
            {   // 若V为string类型, 但m_type不是T_STR类型则产生异常
                if (m_type != T_STR)
                    THROW_GET_ERROR(string);
            } else if constexpr(IS_TYPE(V, bool_t))
            {
                if (m_type != T_BOOL)
                    THROW_GET_ERROR(BOOL);
            } else if constexpr(IS_TYPE(V, int_t))
            {
                if (m_type != T_INT)
                    THROW_GET_ERROR(INT);
            } else if constexpr(IS_TYPE(V, double_t))
            {
                if (m_type != T_DOUBLE)
                    THROW_GET_ERROR(DOUBLE);
            } else if constexpr(IS_TYPE(V, list_t))
            {
                if (m_type != T_LIST)
                    THROW_GET_ERROR(LIST);
            } else if constexpr(IS_TYPE(V, dict_t))
            {
                if (m_type != T_DICT)
                    THROW_GET_ERROR(DICT);
            }

            void* v = value();
            if (v == nullptr)
                throw std::logic_error("unknown type in JObject::Value()");
            return *((V *) v);
        }

        TYPE Type()
        {
            return m_type;
        }

        string to_string();

        void push_back(JObject item)
        {
            if (m_type == T_LIST)
            {
                auto &list = Value<list_t>();
                list.push_back(std::move(item));
                return;
            }
            throw std::logic_error("not a list type! JObjcct::push_back()");
        }

        void pop_back()
        {
            if (m_type == T_LIST)
            {
                auto &list = Value<list_t>();
                list.pop_back();
                return;
            }
            throw std::logic_error("not list type! JObjcct::pop_back()");
        }

        JObject &operator[](string const &key)
        {
            if (m_type == T_DICT)
            {
                auto &dict = Value<dict_t>();
                return dict[key];
            }
            throw std::logic_error("not dict type! JObject::opertor[]()");
        }

    private:
        //根据类型获取值的地址，直接硬转为void*类型，然后外界调用Value函数进行类型的强转
        void* value();

    private:
        TYPE m_type;        //  m_type表示数据类型
        value_t m_value;    //  m_value表示值
    };
} // namespace json

#endif // _JOBJECT_H