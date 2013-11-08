#ifndef _PARSER_H
#define _PARSER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stack>
#include <exception>
#include <algorithm>

#define DEBUG

class SyntaxError : public std::exception {
public:
    SyntaxError( const char* strptr ) { this->str = std::string( strptr ); };
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
        Value( ) { this->isVar = false; this->value = 0.0; };

        bool isVar;
        float value;
        std::string tag;
    };
    class Variable : Value {
    public:
        Variable( float value, std::string& tag ) { this->isVar = true; this->value = value; this->tag = tag; };
        ~Variable( ) { };
    };
    class Literal : Value {
    public:
        Literal( float value ) { this->value = value; };
        ~Literal( ) { };
    };

    class Control {
    public:
        Control( ) { this->type = Parser::CONTROL_INVALID; };
        ~Control( ) { };

        Parser::ControlType type;
        unsigned line;
    };
    class cIf : Control {
        cIf( bool branch ) { this->type = Parser::CONTROL_IF; this->branch = branch; };
        ~cIf( ) { };

        bool branch;
    };
    class cGoSub : Control {
        cGoSub( unsigned line ) { this->type = Parser::CONTROL_GOSUB; this->line = line; };
        ~cGoSub( ) { };
    };
    class cWhile : Control {
        cWhile( unsigned line, Value& var ) { if( var.isVar == true ){ this->line = line; this->var = var.tag; } else { throw SyntaxError( "While not initialized with a variable" ); } };
        ~cWhile( ) { };

        std::string var;
    };

    std::vector<unsigned short> linestarts;
    std::string filename;
    std::fstream file;

    bool running;

    std::stack<Parser::Value> vStack;
    std::deque<Parser::Control> cStack;
    // TODO: std::map for de variables?

    int runParse( );

#ifndef DEBUG
private:
#endif
    void reset( );
    int openFile( std::string& );
    int runFile( );
    int parseLine( unsigned );
};

#endif
