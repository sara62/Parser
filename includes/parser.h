/*************************************************************************************
* parser.h
* Declares the Parser class, and all its helper functions.
* Copyright (C) 2013 by Eric Osburn.
* The redistribution terms are provided in the LICENSE file that must
* be distributed with this source code.
*************************************************************************************/
#ifndef _PARSER_H
#define _PARSER_H

/*************************************************************************************
* Includes
*************************************************************************************/
#include <algorithm>
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include "stdio.h"

// TODO: This is windows only!
#include "Windows.h"

/*************************************************************************************
* Turn on debug mode for now
*************************************************************************************/
#define DEBUG

/*************************************************************************************
* SyntaxError Class
* Provides a throwable error used in the parser.
*************************************************************************************/
class SyntaxError : public std::exception {
public:
    SyntaxError( const char* strptr) { this->str = std::string( strptr ); };
                                            /* Constructor                          */
    virtual const char* what( ) const throw( ){ return this->str.c_str( ); };
                                            /* Printable method returns error string f*/
    std::string str;                        /* Error message holder                 */
};

/*************************************************************************************
* Parser Class
* Standalone class that implements an RPN BASIC parser.
*************************************************************************************/
class Parser{
public:
    Parser( );                              /* Constructor                          */
    ~Parser( );                             /* Destructor                           */

    int runParse( );                        /* Starts the parser                    */

    typedef enum { CONTROL_INVALID, CONTROL_IF, CONTROL_GOSUB, CONTROL_WHILE } ControlType;
                                            /* Control type code enumeration        */

    /*********************************************************************************
    * Value Class
    * Dual Variable/Literal class
    *
    * QUESTION: Would this be better implemented by a pure virtual base class,
    *   and two separate child classes?
    *
    * QUESTION: Should all these classes be in a namespace instead?
    *********************************************************************************/
    class Value {
    public:
        Value( ) { this->isVar = false; this->value = 0.0; this->tag = 0; };
                                            /* Default constructor                  */
        static Value Variable( char tag ) { Value temp; temp.isVar = true; temp.value = 0.0; temp.tag = tag; return temp; };
                                            /* Variable constructor                 */
        static Value Literal( float value ) { Value temp; temp.isVar = false; temp.value = value; temp.tag = 0; return temp; };
                                            /* Literal constructor                  */
        ~Value( ) { };                      /* Destructor                           */

        float getVal( std::map<char, float>& vList ) { if( this->isVar ) return vList[ this->tag ]; else return this->value; };
                                            /* Value getter, handles Variables and Literals */
        void setVal( std::map<char, float>& vList, float value ) { if( this->isVar ) vList[ this->tag ] = value; else throw( SyntaxError( "Assignment target not a variable!" ) ); };
                                            /* Value setter, throws error if not a Variable */

        bool isVar;                         /* Am I a variable?                     */
        float value;                        /* Literal value storage                */
        char tag;                           /* Variable tag storage                 */
    };

    /*********************************************************************************
    * Control Class
    * Triple If/GoSub/While Class
    *
    * QUESTION: Would this be better implemented by a pure virtual base class,
    *   and three separate child classes?
    *********************************************************************************/
    class Control {
    public:
        Control( ) { this->type = Parser::CONTROL_INVALID; };
                                            /* Default constructor, INVALID type    */
        static Control cIf( bool branch ) { Control temp; temp.type = Parser::CONTROL_IF; temp.branch = branch; return temp; };
                                            /* If constructor                       */
        static Control cGosub( unsigned line ) { Control temp; temp.type = Parser::CONTROL_GOSUB; temp.line = line; return temp; };
                                            /* GoSub constructor                    */
        static Control cWhile( unsigned line, Value& var ) { if( var.isVar == true ){ Control temp; temp.type = Parser::CONTROL_WHILE; temp.line = line; temp.var = var.tag; return temp; } else { throw SyntaxError( "While not initialized with a variable! " ); } };
                                            /* While constructor                    */
        ~Control( ) { };                    /* Destructor                           */

        Parser::ControlType type;           /* Type code                            */
        unsigned line;                      /* Return line number                   */
        char var;                           /* While watch variable                 */
        bool branch;                        /* If current branch                    */
    };

    /*********************************************************************************
    * Operator Class
    * This is the basis for every operator/command added to the list
    *********************************************************************************/
    class Operator{
    public:
        Operator( ) { };                    /* Default constructor, needed for some STL reason */
        Operator( unsigned numArgs, void (* func)(std::deque<Parser::Value>&, std::map<char, float>&) ) { this->numArgs = numArgs; this->doFunc = func; };
                                            /* Constructor with options!            */
        ~Operator( ) { };                   /* Destructor                           */

        void doDatThang( std::deque<Parser::Value>& vStack, std::map<char, float>& vList ) { if( vStack.size( ) < this->numArgs ) throw SyntaxError( "Operator with too few values on stack!" ); else this->doFunc( vStack, vList ); };
                                            /* Runs the associated operator function */
        unsigned numArgs;                   /* Number of required Values on the stack */
        void (* doFunc)(std::deque<Parser::Value>&, std::map<char, float>&);
                                            /* Function pointer to operator method  */
    };

private:
    std::vector<unsigned short> linestarts; /* Line -> File seek position           */
    std::string filename;                   /* D'oh! Current filename               */
    std::fstream file;                      /* ''    Current file stream            */

    bool running;                           /* Are we actively parsing?             */
    unsigned line;                          /* Current line to parse, jump magic happens here */

    std::deque<Parser::Value> vStack;       /* Value stack                          */
    std::deque<Parser::Control> cStack;     /* Control stack                        */
    std::map<char, float> vList;            /* Variable list ( tag -> value )       */
    std::map<std::string, Parser::Operator> oList;
                                            /* Operator list ( see the init ;) )    */

    Parser::Control findTop( Parser::ControlType, bool );
                                            /* Finds the topmost type Control on cStack */

    void reset( );                          /* Gets ready for the next run          */
    int openFile( std::string& );           /* Sets the file to parse               */
    void listProg( );                       /* Lists out the program with line nums */
    int runFile( );                         /* Parses the open file                 */
    int parseLine( unsigned );              /* Parses an given line from the open file */
};

/*************************************************************************************
* Function declarations for all the operators :-S
*
* See the Language.txt file in resources for operator descriptions.
*************************************************************************************/
void wait_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* WAIT                                 */
void pop_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* POP                                  */
void ser_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* SER                                  */
void peek_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* PEEK                                 */
void assign_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* =                                    */
void add_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* +                                    */
void sub_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* -                                    */
void mul_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* *                                    */
void div_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* /                                    */
void pow_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* **                                   */
void sqrt_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* SQRT                                 */
void mod_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* %                                    */
void abs_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* ABS                                  */
void sin_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* SIN                                  */
void cos_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* COS                                  */
void tan_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* TAN                                  */
void asin_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* ASIN                                 */
void acos_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* ACOS                                 */
void atan_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* ATAN                                 */
void atnt_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* ATNT                                 */
void exp_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* EXP                                  */
void log_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* LOG                                  */
void logt_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* LOGT                                 */
void grt_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* >                                    */
void lst_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* <                                    */
void geq_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* >=                                   */
void leq_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* <=                                   */
void eql_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* ==                                   */
void neq_( std::deque<Parser::Value>&, std::map<char, float>& );
                                            /* !=                                   */

#endif
