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
        float setVal( std::map<char, float>& vList, float value ) { if( this->isVar ) vList[ this->tag ] = value; else throw( SyntaxError( "Assignment target not a variable!" ) ); };

        bool isVar;
        float value;
        char tag;
    };

    class Control {
    public:
        Control( ) { this->type = Parser::CONTROL_INVALID; };
        static Control cIf( bool branch ) { Control temp; temp.type = Parser::CONTROL_IF; temp.branch = branch; return temp; };
        static Control cGosub( unsigned line ) { Control temp; temp.type = Parser::CONTROL_GOSUB; temp.line = line; return temp; };
        static Control cWhile( unsigned line, Value& var ) { if( var.isVar == true ){ Control temp; temp.line = line; temp.var = var.tag; return temp; } else { throw SyntaxError( "While not initialized with a variable! " ); } };
        ~Control( ) { };

        Parser::ControlType type;
        unsigned line;
        char var;
        bool branch;
    };

    class Operator{
    public:
        Operator( unsigned numArgs, void (* func)(std::stack<Parser::Value>&) ) { this->numArgs = numArgs; this->doFunc = func; };
        ~Operator( ) { };

        unsigned numArgs;
        void (* doFunc)(std::stack<Parser::Value>&);
    };

    std::vector<unsigned short> linestarts;
    std::string filename;
    std::fstream file;

    bool running;

    std::deque<Parser::Value> vStack;
    std::deque<Parser::Control> cStack;
    // use the back as a stack.
    // i.e. use push_back, and pop_back. Also, .back() is the top of the stack
    std::map<char, float> vList;
    std::map<std::string, Operator> oList;

    int runParse( );

#ifndef DEBUG
private:
#endif
    void reset( );
    int openFile( std::string& );
    void listProg( );
    int runFile( );
    int parseLine( unsigned );

    // Ready for the loooong list of operators supported?? :D
    // Basic format is this: void operatorname_( std::stack<Parser::Value>& stack ) {...};

    // TODO:
    /*
    "RAND": operator(0, lambda: stack.append(literal(uniform(0,1)))),
    "DUPE": operator(1, lambda: stack.append(stack[-1])),
    "POP":  operator(1, lambda: print("Line:",cline+1,stack.pop())),
    "PEEK": operator(1, lambda: print("Line:",cline+1,stack[-1])),
    "PRNT": operator(0, lambda: print(stack,"\n",cstack)),
    "LINE": operator(0, lambda: print("Line:",cline+1)),
    "WAIT": operator(1, lambda: sleep(stack.pop().value)),

    def input_():
    */
    
    // The on basic one!
    void assign_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // Get rhs and lhs values
        stack[len-2].setVal( this->vList, stack[len-1].getVal( this->vList ) );
        stack.pop_back( ); stack.pop_back( );
    };

    // Arithmetic time!
    void add_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-2] = Value::Literal( stack[len-2].getVal( this->vList ) + stack[len-1].getVal( this->vList ) );
        stack.pop_back( );
    };
    void sub_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-2] = Value::Literal( stack[len-2].getVal( this->vList ) - stack[len-1].getVal( this->vList ) );
        stack.pop_back( );
    };
    void mul_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-2] = Value::Literal( stack[len-2].getVal( this->vList ) * stack[len-1].getVal( this->vList ) );
        stack.pop_back( );
    };
    void div_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-2] = Value::Literal( stack[len-2].getVal( this->vList ) / stack[len-1].getVal( this->vList ) );
        stack.pop_back( );
    };

    // More advanced maths
    void pow_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-2] = Value::Literal( pow( stack[len-2].getVal( this->vList ), stack[len-1].getVal( this->vList ) ) );
        stack.pop_back( );
    };
    void sqrt_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-1] = Value::Literal( sqrt( stack[len-1].getVal( this->vList ) ) );
        stack.pop_back( );
    };
    void mod_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-2] = Value::Literal( fmod( stack[len-2].getVal( this->vList ), stack[len-1].getVal( this->vList ) ) );
        stack.pop_back( );
    };
    void abs_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-1] = Value::Literal( abs( stack[len-1].getVal( this->vList ) ) );
        stack.pop_back( );
    };

    // Next up, trig!
    void sin_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-1] = Value::Literal( sin( stack[len-1].getVal( this->vList ) ) );
        stack.pop_back( );
    };
    void cos_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-1] = Value::Literal( cos( stack[len-1].getVal( this->vList ) ) );
        stack.pop_back( );
    };
    void tan_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-1] = Value::Literal( tan( stack[len-1].getVal( this->vList ) ) );
        stack.pop_back( );
    };
    void asin_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-1] = Value::Literal( asin( stack[len-1].getVal( this->vList ) ) );
        stack.pop_back( );
    };
    void acos_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-1] = Value::Literal( acos( stack[len-1].getVal( this->vList ) ) );
        stack.pop_back( );
    };
    void atan_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-1] = Value::Literal( atan( stack[len-1].getVal( this->vList ) ) );
        stack.pop_back( );
    };
    void atnt_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-2] = Value::Literal( atan2( stack[len-2].getVal( this->vList ), stack[len-1].getVal( this->vList ) ) );
        stack.pop_back( );
    };

    // Now come the log functions!
    void exp_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-1] = Value::Literal( exp( stack[len-1].getVal( this->vList ) ) );
        stack.pop_back( );
    };
    void log_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-1] = Value::Literal( log( stack[len-1].getVal( this->vList ) ) );
        stack.pop_back( );
    };
    void logt_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-1] = Value::Literal( log10( stack[len-1].getVal( this->vList ) ) );
        stack.pop_back( );
    };

    // And last but not least, the comparators!
    void grt_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-2] = Value::Literal( ( stack[len-2].getVal( this->vList ) > stack[len-1].getVal( this->vList ) ) ? 1.0f : 0.0f );
        stack.pop_back( );
    };
    void lst_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-2] = Value::Literal( ( stack[len-2].getVal( this->vList ) < stack[len-1].getVal( this->vList ) ) ? 1.0f : 0.0f );
        stack.pop_back( );
    };
    void geq_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-2] = Value::Literal( ( stack[len-2].getVal( this->vList ) >= stack[len-1].getVal( this->vList ) ) ? 1.0f : 0.0f );
        stack.pop_back( );
    };
    void leq_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-2] = Value::Literal( ( stack[len-2].getVal( this->vList ) <= stack[len-1].getVal( this->vList ) ) ? 1.0f : 0.0f );
        stack.pop_back( );
    };
    void eql_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-2] = Value::Literal( ( stack[len-2].getVal( this->vList ) == stack[len-1].getVal( this->vList ) ) ? 1.0f : 0.0f );
        stack.pop_back( );
    };
    void neq_( std::deque<Parser::Value>& stack ){
        unsigned len = stack.size( );
        // In-place replace the result
        stack[len-2] = Value::Literal( ( stack[len-2].getVal( this->vList ) != stack[len-1].getVal( this->vList ) ) ? 1.0f : 0.0f );
        stack.pop_back( );
    };
};

#endif
