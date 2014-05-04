#ifndef EXCEPTION_H_
#define EXCEPTION_H_

class Exception: public std::exception
{
public:
    Exception(const char *text)
    {
        _msg.assign(text);
    }
    
    Exception(std::string text)
    {
        _msg.assign(text);
    }
    
    virtual ~Exception() throw () {}
    
    virtual const char* what() const throw () {
       return _msg.c_str();
    }

protected:
    std::string _msg;
};

#endif
