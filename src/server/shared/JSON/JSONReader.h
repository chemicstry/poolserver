#ifndef JSON_READER_H_
#define JSON_READER_H_

#include "Common.h"
#include "JSON.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_iso8859_1.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <string>

namespace iso8859_1 = boost::spirit::iso8859_1;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

namespace JSONReader
{
    class SemanticFunctions
    {
    public:
        SemanticFunctions(JSON& node)
        {
            _stack.push_back(&node);
        }
        
        ~SemanticFunctions()
        {
            for (uint32 i = 0; i+1 < _stack.size(); ++i) {
                delete _stack.back();
                _stack.pop_back();
            } 
        }
        
        void BeginObject(char ch)
        {
            if (_stack.back()->GetType() == JSON_NULL)
                _stack.back()->SetType(JSON_OBJECT);
            else {
                JSON* node = new JSON(JSON_OBJECT);
                _stack.push_back(node);
            }
        }
        void EndObject(char ch)
        {
            JSON* node = _stack.back();
            _stack.pop_back();
            if (_stack.size()) {
                SetData(*node);
                delete node;
            }
        }
        
        void BeginArray(char ch)
        {
            if (_stack.back()->GetType() == JSON_NULL)
                _stack.back()->SetType(JSON_ARRAY);
            else {
                JSON* node = new JSON(JSON_ARRAY);
                _stack.push_back(node);
            }
        }
        void EndArray(char ch)
        {
            JSON* node = _stack.back();
            _stack.pop_back();
            if (_stack.size()) {
                SetData(*node);
                delete node;
            }
        }
        
        void ObjectKey(std::string str)
        {
            _nameStack.push_back(str);
        }
        
        void String(std::string str)
        {
            SetData(str);
        }
        
        void BoolTrue(std::string str)
        {
            SetData(true);
        }
        
        void BoolFalse(std::string str)
        {
            SetData(false);
        }
        
        void Null(std::string str)
        {
            SetData(JSON(JSON_NULL));
        }
        
        void Integer(int64 val)
        {
            SetData(val);
        }
        
        void Double(double val)
        {
            SetData(val);
        }
        
        template<typename T>
        void SetData(T data)
        {
            if (_stack.back()->GetType() == JSON_OBJECT) {
                _stack.back()->Set<T>(_nameStack.back(), data);
                _nameStack.pop_back();
            } else if (_stack.back()->GetType() == JSON_ARRAY)
                _stack.back()->Add<T>(data);
            else
                throw "fail";
        }
    private:
        // Push all nested nodes here
        std::vector<JSON*> _stack;
        // Node is added to the main tree when is closed so we need a name stack
        std::vector<std::string> _nameStack;
    };
    
    template<typename T>
    struct strict_real_policies : qi::real_policies<T>
    {
        static bool const expect_dot = true;
    };
    
    qi::real_parser<double, strict_real_policies<double> > strict_double;
    
    template<typename Iterator>
    struct Grammar : qi::grammar<Iterator, iso8859_1::space_type>
    {
        Grammar(SemanticFunctions& smfunc) : _smfunc(smfunc), Grammar::base_type(json)
        {
            typedef boost::function<void(char, qi::unused_type, qi::unused_type)> CharFunc;
            typedef boost::function<void(std::string, qi::unused_type, qi::unused_type)> StrFunc;
            typedef boost::function<void(double, qi::unused_type, qi::unused_type)> DoubleFunc;
            typedef boost::function<void(int64, qi::unused_type, qi::unused_type)> IntFunc;
            
            CharFunc BeginObj(boost::bind(&SemanticFunctions::BeginObject, &_smfunc, _1));
            CharFunc EndObj(boost::bind(&SemanticFunctions::EndObject, &_smfunc, _1));
            CharFunc BeginArray(boost::bind(&SemanticFunctions::BeginArray, &_smfunc, _1));
            CharFunc EndArray(boost::bind(&SemanticFunctions::EndArray, &_smfunc, _1));
            StrFunc NewKey(boost::bind(&SemanticFunctions::ObjectKey, &_smfunc, _1));
            StrFunc NewStr(boost::bind(&SemanticFunctions::String, &_smfunc, _1));
            StrFunc NewTrue(boost::bind(&SemanticFunctions::BoolTrue, &_smfunc, _1));
            StrFunc NewFalse(boost::bind(&SemanticFunctions::BoolFalse, &_smfunc, _1));
            StrFunc NewNull(boost::bind(&SemanticFunctions::Null, &_smfunc, _1));
            DoubleFunc NewDouble(boost::bind(&SemanticFunctions::Double, &_smfunc, _1));
            IntFunc NewInt(boost::bind(&SemanticFunctions::Integer, &_smfunc, _1));
            
            json = object | array;
            object = qi::char_('{')[BeginObj] >> -members >> qi::char_('}')[EndObj];
            members = pair % ',';
            pair = string[NewKey] >> ':' >> value;
            array = qi::char_('[')[BeginArray] >> -elements >> qi::char_(']')[EndArray];
            elements = value % ',';
            value = object | array | string[NewStr] | number | qi::string("true")[NewTrue] | qi::string("false")[NewFalse] | qi::string("null")[NewNull];
            string = qi::lexeme['"' >> *(qi::char_ - '"') >> '"'];
            number = strict_double[NewDouble] | qi::long_long[NewInt];
        }
        
        qi::rule<Iterator, iso8859_1::space_type> json;
        qi::rule<Iterator, iso8859_1::space_type> object;
        qi::rule<Iterator, iso8859_1::space_type> members;
        qi::rule<Iterator, iso8859_1::space_type> pair;
        qi::rule<Iterator, iso8859_1::space_type> array;
        qi::rule<Iterator, iso8859_1::space_type> elements;
        qi::rule<Iterator, iso8859_1::space_type> value;
        qi::rule<Iterator, std::string(), iso8859_1::space_type> string;
        qi::rule<Iterator, iso8859_1::space_type> number;
        
        // Functions
        SemanticFunctions& _smfunc;
    };
    
    inline void Parse(std::string str, JSON& node)
    {
        SemanticFunctions smfunc(node);
        Grammar<std::string::const_iterator> g(smfunc);
        
        std::string::const_iterator begin = str.begin();
        std::string::const_iterator end = str.end();
        
        if (!qi::phrase_parse(begin, end, g, iso8859_1::space))
            throw JSONException("Failed to parse JSON");
    }
}

#endif