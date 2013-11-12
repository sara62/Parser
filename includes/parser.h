#ifndef _PARSER_H
#define _PARSER_H

#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stack>
#include <exception>
#include <algorithm>
#include <cmath>
#include <limits>

#include "stdio.h"

// TODO: This is windows only!
#include "Windows.h"

#define DEBUG

class SyntaxError : public std::exception {
public:
    SyntaxError( const char* strptr) { this->str = std::string( strptr ); };
    virtual const char* what( ) const throw( ){
        return this->str.c_str( );
    };

    std::string str;
};

class Parser{
public:
    Parser( );
    ~Parser( );

    typedef enum { CONTROL_INVALID, CONTROL_IF, CONTROL_GOSUB, CONTROL_WHILE } ControlType;

    class Value {
    public:
        Value( ) { this->isVar = false; this->value = 0.0; this->tag = 0; };
        static Value Variable( char tag ) { Value temp; temp.isVar = true; temp.value = 0.0; temp.tag = tag; return temp; };
        static Value Literal( float value ) { Value temp; temp.isVar = false; temp.value = value; temp.tag = 0; return temp; };
        ~Value( ) { };

        float getVal( std::map<char, float>& vList ) { if( this->isVar ) return vList[ this->tag ]; else return this->value; };
        void setVal( std::map<char, float>& vList, float value ) { if( this->isVar ) vList[ this->tag ] = value; else throw( SyntaxError( "Assignment target not a variable!" ) ); };

        bool isVar;
        float value;
        char tag;
    };

    class Control {
    public:
        Control( ) { this->type = Parser::CONTROL_INVALID; };
        static Control cIf( bool branch ) { Control temp; temp.type = Parser::CONTROL_IF; temp.branch = branch; return temp; };
        static Control cGosub( unsigned line ) { Control temp; temp.type = Parser::CONTROL_GOSUB; temp.line = line; return temp; };
        static Control cWhile( unsigned line, Value& var ) { if( var.isVar == true ){ Control temp; temp.type = Parser::CONTROL_WHILE; temp.line = line; temp.var = var.tag; return temp; } else { throw SyntaxError( "While not initialized with a variable! " ); } };
        ~Control( ) { };

        Parser::ControlType type;
        unsigned line;
        char var;
        bool branch;
    };

    class Operator{
    public:
        Operator( ) { };
        Operator( unsigned numArgs, void (* func)(std::deque<Parser::Value>&, std::map<char, float>&) ) { this->numArgs = numArgs; this->doFunc = func; };
        ~Operator( ) { };

        void doDatThang( std::deque<Parser::Value>& vStack, std::map<char, float>& vList ) { if( vStack.size( ) < this->numArgs ) throw SyntaxError( "Operator with too few values on stack!" ); else this->doFunc( vStack, vList ); };

        unsigned numArgs;
        void (* doFunc)(std::deque<Parser::Value>&, std::map<char, float>&);
    };

    std::vector<unsigned short> linestarts;
    std::string filename;
    std::fstream file;

    bool running;
    unsigned line;

    std::deque<Parser::Value> vStack;
    std::deque<Parser::Control> cStack;
    // use the back as a stack.
    // i.e. use push_back, and pop_back. Also, .back() is the top of the stack
    std::map<char, float> vList;
    std::map<std::string, Parser::Operator> oList;

    int runParse( );

private:
    Parser::Control findTop( Parser::ControlType, bool );

    void reset( );
    int openFile( std::string& );
    void listProg( );
    int runFile( );
    int parseLine( unsigned );
};

// Function declarations for all the operators :-S
void wait_( std::deque<Parser::Value>&, std::map<char, float>& );
void pop_( std::deque<Parser::Value>&, std::map<char, float>& );
void ser_( std::deque<Parser::Value>&, std::map<char, float>& );
void peek_( std::deque<Parser::Value>&, std::map<char, float>& );
void assign_( std::deque<Parser::Value>&, std::map<char, float>& );
void add_( std::deque<Parser::Value>&, std::map<char, float>& );
void sub_( std::deque<Parser::Value>&, std::map<char, float>& );
void mul_( std::deque<Parser::Value>&, std::map<char, float>& );
void div_( std::deque<Parser::Value>&, std::map<char, float>& );
void pow_( std::deque<Parser::Value>&, std::map<char, float>& );
void sqrt_( std::deque<Parser::Value>&, std::map<char, float>& );
void mod_( std::deque<Parser::Value>&, std::map<char, float>& );
void abs_( std::deque<Parser::Value>&, std::map<char, float>& );
void sin_( std::deque<Parser::Value>&, std::map<char, float>& );
void cos_( std::deque<Parser::Value>&, std::map<char, float>& );
void tan_( std::deque<Parser::Value>&, std::map<char, float>& );
void asin_( std::deque<Parser::Value>&, std::map<char, float>& );
void acos_( std::deque<Parser::Value>&, std::map<char, float>& );
void atan_( std::deque<Parser::Value>&, std::map<char, float>& );
void atnt_( std::deque<Parser::Value>&, std::map<char, float>& );
void exp_( std::deque<Parser::Value>&, std::map<char, float>& );
void log_( std::deque<Parser::Value>&, std::map<char, float>& );
void logt_( std::deque<Parser::Value>&, std::map<char, float>& );
void grt_( std::deque<Parser::Value>&, std::map<char, float>& );
void lst_( std::deque<Parser::Value>&, std::map<char, float>& );
void geq_( std::deque<Parser::Value>&, std::map<char, float>& );
void leq_( std::deque<Parser::Value>&, std::map<char, float>& );
void eql_( std::deque<Parser::Value>&, std::map<char, float>& );
void neq_( std::deque<Parser::Value>&, std::map<char, float>& );

#endif
