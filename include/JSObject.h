#ifndef JS_OBJECT_H
#define JS_OBJECT_H

#include <string>
#include <map>
#include <ostream>
#include "JSONLexer.h"

class JSObject
{
public:
    JSObject();
    virtual ~JSObject() {}
    virtual std::string toString();
public:
    TokenType objectType;
};

std::ostream & operator << (std::ostream &os,JSObject *obj);

std::ostream & operator << (std::ostream &os,JSObject &obj);

class JSNumber : public JSObject
{
public:
    JSNumber(std::string _str,TokenType _type);
    
    std::string toString() override;
    union 
    {
        long intVal;
        double floatVal;
    } numberVal;
};

class JSString : public JSObject
{
public:
    JSString(std::string _str);
    ~JSString() override;
    
    std::string toString() override;
    std::string *strRef;
};

class JSArray : public JSObject
{
public:
    JSArray();
    ~JSArray() override;
    
    void addObject(std::shared_ptr<JSObject> obj);
    std::string toString() override;
    bool empty();
    std::vector<std::shared_ptr<JSObject>> *arrayRef;
};

class JSMap : public JSObject
{
public:
    JSMap();
    ~JSMap() override;
    
    std::string toString() override;
    void setObjectAndKey(std::string key,std::shared_ptr<JSObject> value);
    std::map<std::string,std::shared_ptr<JSObject>> *mapRef;
};

#endif
