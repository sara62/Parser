#include "parser.h"

#define DEBUG

Parser::Parser( ) {
    char cChar;

    for( cChar = 65; cChar < 91; ++cChar ){
        this->vList[ cChar ] = 0.0;
    }

    // Set up functions.. so many calls :/
    this->oList[ "WAIT" ]   = Parser::Operator( (unsigned)0, wait_ );
    this->oList[ "POP" ]    = Parser::Operator( (unsigned)0, pop_ );
    this->oList[ "SER" ]    = Parser::Operator( (unsigned)0, ser_ );
    this->oList[ "PEEK" ]   = Parser::Operator( (unsigned)0, peek_ );
    this->oList[ "=" ]      = Parser::Operator( (unsigned)2, assign_ );
    this->oList[ "+" ]      = Parser::Operator( (unsigned)2, add_ );
    this->oList[ "-" ]      = Parser::Operator( (unsigned)2, sub_ );
    this->oList[ "*" ]      = Parser::Operator( (unsigned)2, mul_ );
    this->oList[ "/" ]      = Parser::Operator( (unsigned)2, div_ );
    this->oList[ "**" ]     = Parser::Operator( (unsigned)2, pow_ );
    this->oList[ "SQRT" ]   = Parser::Operator( (unsigned)1, sqrt_ );
    this->oList[ "%" ]      = Parser::Operator( (unsigned)2, mod_ );
    this->oList[ "ABS" ]    = Parser::Operator( (unsigned)1, abs_ );
    this->oList[ "SIN" ]    = Parser::Operator( (unsigned)1, sin_ );
    this->oList[ "COS" ]    = Parser::Operator( (unsigned)1, cos_ );
    this->oList[ "TAN" ]    = Parser::Operator( (unsigned)1, tan_ );
    this->oList[ "ASIN" ]   = Parser::Operator( (unsigned)1, asin_ );
    this->oList[ "ACOS" ]   = Parser::Operator( (unsigned)1, acos_ );
    this->oList[ "ATAN" ]   = Parser::Operator( (unsigned)1, atan_ );
    this->oList[ "ATNT" ]   = Parser::Operator( (unsigned)2, atnt_ );
    this->oList[ "EXP" ]    = Parser::Operator( (unsigned)1, exp_ );
    this->oList[ "LOG" ]    = Parser::Operator( (unsigned)1, log_ );
    this->oList[ "LOGT" ]   = Parser::Operator( (unsigned)1, logt_ );
    this->oList[ ">" ]      = Parser::Operator( (unsigned)2, grt_ );
    this->oList[ "<" ]      = Parser::Operator( (unsigned)2, lst_ );
    this->oList[ ">=" ]     = Parser::Operator( (unsigned)2, geq_ );
    this->oList[ "<=" ]     = Parser::Operator( (unsigned)2, leq_ );
    this->oList[ "==" ]     = Parser::Operator( (unsigned)2, eql_ );
    this->oList[ "!=" ]     = Parser::Operator( (unsigned)2, neq_ );
};
Parser::~Parser( ) { };

int Parser::runParse( ) {
    std::string file;

    this->running = true;

    while( 1 ){
        printf( "Clever prompt -> " );
        std::cin >> file;

        std::transform(file.begin(), file.end(), file.begin(), ::tolower);

        if( file.compare( std::string( "exit" ) ) == 0 ){
            exit( 0 );
        } else if( file.compare( std::string( "load" ) ) == 0 ){
            printf( "Loading" );
            std::cin >> file;

            if( this->openFile( file ) == -1 ){
                printf( "\nCould not open file %s", file.c_str() );
            }
            printf( "\n" );
        } else if( file.compare( std::string( "run" ) ) == 0 ){
            this->runFile( );
        } else if( file.compare( std::string( "clear" ) ) == 0 ){
            this->reset( );
        } else if( file.compare( std::string( "list" ) ) == 0 ){
            this->listProg( );
        } else if( file.compare( std::string( "help" ) ) == 0 ){
            printf( "Commands: EXIT, LOAD, RUN, CLEAR, LIST, HELP\n" );
        }
    }

    return 0;
};

Parser::Control Parser::findTop( Parser::ControlType type, bool popoff = false ){
    std::deque<Parser::Control>::reverse_iterator retfinder;
    std::deque<Parser::Control>::reverse_iterator endoflist;

    if( this->cStack.size( ) == 0 ) return Parser::Control( );

    for( retfinder = this->cStack.rbegin( ), endoflist = this->cStack.rend( ); retfinder != endoflist; ++retfinder ){
        if( retfinder->type == type ){
            if( popoff ){
                if( retfinder != this->cStack.rbegin( ) ){
                    this->cStack.erase( retfinder.base( ), this->cStack.end( ) );
                }
            }

            return this->cStack[ this->cStack.size( ) - 1 ];
        }
    }
    return Parser::Control( );
};

void Parser::reset( ) {
    this->file.close( );
    this->vStack.erase( this->vStack.begin( ), this->vStack.end( ) );
    this->cStack.erase( this->cStack.begin( ), this->cStack.end( ) );
    // TODO: CLEAR ALL VARIABLES
};

int Parser::openFile( std::string& filename ){
    char fchar;
    bool mark = true;

    // Close file if already open
    if( this->file.is_open( ) ) this->reset( );

    // Save filename
    this->filename = filename;

    // Open the file, debug print
    // MSVC 2008 DOESNT SUPPORT C++11 OH MY GAWD
    this->file.open( this->filename.c_str( ), std::fstream::in );
    if( ! this->file.is_open( ) ) return -1;

    // Get line starts
    this->linestarts.clear( );
    this->linestarts.push_back( 0 );
    this->file.get(fchar);
    while( !this->file.eof() ){
        if( mark && fchar != 13 ){
            mark = false;
            this->linestarts.push_back( (unsigned short) this->file.tellg( ) - 1 );
            printf( "." );
        }
        if( fchar == 10 ){
            mark = true;
        }
        this->file.get( fchar );
    }

    // Reset the file after eof
    // NOTE: TEST THIS, ITS POSSIBLY A WINDOWS ONLY THING :(
    this->file.clear( );

    // Return line count
    return this->linestarts.size( );
};

void Parser::listProg( ){
    unsigned i;
    std::string line;

    // QUESTION: Print an error message here?
    if( ! this->file.is_open( ) ) return;

    if( this->file.eof( ) ) this->file.clear( );

    for( i = 1; i < this->linestarts.size( ); ++i ){
        this->file.seekg( this->linestarts[ i ], this->file.beg );
        if( getline( this->file, line ) ){
            std::cout << i << ": " << line << std::endl;
        } else break;
    }
};

int Parser::runFile( ) {
    this->line = 1;
    unsigned lncnt = this->linestarts.size( ) - 1;

    while( 1 ){
        if( this->line > lncnt || this->line < 1 ) break;
        try{
            this->parseLine( this->line );
        } catch( SyntaxError e ){
            std::cout << e.what( ) << " At line:" << this->line << std::endl;
            return -1;
        }
        if( this->running == false ) break;
        else ++( this->line );
    }

    printf( "Program terminated successfully.\n" );
    return 0;
};

int Parser::parseLine( unsigned linenum ) {
    std::stringstream line;
    std::string oper;
    Parser::Control criter;
    bool printing = false;
    static bool findLoop = false;

    // If the file is still good
    if( ! this->file.is_open( ) ){
        this->running = false;
        return -1;
    }

    // If we hit eof, reset the file
    if( this->file.eof( ) ) this->file.clear( );

    // Seek to the proper position
    this->file.seekg( this->linestarts[ linenum ] );

    // Get a line
    std::getline( this->file, oper );
    line << oper;

    // Check for empty line
    if( oper.find_first_not_of( std::string( " " ) ) == oper.npos ) return 0;

    while( ! line.eof( ) ){
        bool lastifbranch = true;

        line >> oper;
        // Oper is a comment marker, ignore rest of line
        if( oper.compare( std::string( "REM" ) ) == 0 ) return 0;

        // Searching for a LOOP
        if( findLoop ){
            if( oper.compare( std::string( "LOOP" ) ) == 0 ){
                this->line;
                findLoop = false;
                return 0;
            } else continue;
        }

        // Print rest of line
        if( printing ){
            std::cout << oper << " ";
            continue;
        }

        // Find the topmost if statement, get its branch
        if( this->cStack.size( ) > 0 ){
            criter = findTop( Parser::CONTROL_IF );
            if( criter.type == Parser::CONTROL_IF ) lastifbranch = criter.branch;
        }

        // Check for ELSE/ENDIF unconditionally
        if( oper.compare( std::string( "ELSE" ) ) == 0 ) {
            if( this->cStack.size( ) == 0 || this->cStack.back( ).type != Parser::CONTROL_IF ) throw SyntaxError( "ELSE with no immediate prior IF!" );
            this->cStack.back( ).branch = ! this->cStack.back( ).branch;
            continue;
        }
        else if( oper.compare( std::string( "ENDIF" ) ) == 0 ) {
            if( this->cStack.size( ) == 0 || this->cStack.back( ).type != Parser::CONTROL_IF ) throw SyntaxError( "ELSE with no immediate prior IF!" );
            this->cStack.pop_back( );
            continue;
        }



        if( lastifbranch == true ){
            // Oper is a print statement
            if( oper.compare( std::string( "PRINT" ) ) == 0 ) { std::cout << std::endl; printing = true; continue; }
            // Oper is quitting execution
            else if( oper.compare( std::string( "QUIT" ) ) == 0 ) { this->running = false; return 0; }
            // Oper is asking for a line print
            else if( oper.compare( std::string( "LINE" ) ) == 0 ) { std::cout << "Line: " << this->line << std::endl; }
            else if( oper.compare( std::string( "PRNT" ) ) == 0 ) {
                std::deque<Parser::Value>::reverse_iterator riter, end;
                std::deque<Parser::Control>::reverse_iterator criter, cend;
                std::cout << std::endl << "Stack Dump:" << std::endl;
                for( riter = this->vStack.rbegin( ), end = this->vStack.rend( ); riter != end; ++riter ){
                    std::cout << (char) ( (riter->isVar) ? riter->tag : 32 );
                    std::cout << " " << riter->getVal( vList ) << std::endl;
                }
                std::cout << std::endl << "Control Stack Dump:" << std::endl;
                for( criter = this->cStack.rbegin( ), cend = this->cStack.rend( ); criter != cend; ++criter ){
                    std::cout << ( ( criter->type == Parser::CONTROL_INVALID ) ? "INVALID" : ( ( criter->type == Parser::CONTROL_GOSUB ) ? "GOSUB" : ( ( criter->type == Parser::CONTROL_IF ) ? "IF" : ( ( criter->type == Parser::CONTROL_WHILE ) ? "WHILE" : ( "DURRR" ) ) ) ) );
                    std::cout << " " << ( ( criter->branch ) ? "true" : "false" ) << " " << criter->line << " " << (char) criter->var << std::endl;
                }
            }
            // check oper for control statements
            else if( oper.compare( std::string( "GOTO" ) ) == 0 ) {
                if( this->vStack.size( ) < 1) throw SyntaxError( "GOTO with no variables on stack!" );

                unsigned tline = ( unsigned ) this->vStack.back( ).getVal( this->vList );

                if( tline > this->linestarts.size( ) || tline == 0 ) throw SyntaxError( "GOTO to invalid line!" );

                this->line = tline - 2;
                this->vStack.pop_back( );
            }
            else if( oper.compare( std::string( "GOSUB" ) ) == 0 ) {
                if( this->vStack.size( ) < 1) throw SyntaxError( "GOSUB with no variables on stack!" );

                unsigned tline = ( unsigned ) this->vStack.back( ).getVal( this->vList );

                if( tline > this->linestarts.size( ) ) throw SyntaxError( "GOSUB to invalid line!" );

                this->line = tline - 2;
                this->vStack.pop_back( );
                this->cStack.push_back( Control::cGosub( this->line ) );
            }
            else if( oper.compare( std::string( "RET" ) ) == 0 ) {
                criter = findTop( Parser::CONTROL_GOSUB, true );

                if( this->cStack.size( ) == 0 ) throw SyntaxError( "RET with no prior GOSUB!" );
                else {
                    this->line = criter.line;
                    this->cStack.pop_back( );
                }
            }
            else if( oper.compare( std::string( "IF" ) ) == 0 ) {
                if( this->vStack.size( ) == 0 ) throw SyntaxError( "IF with nothing on the stack!" );

                this->cStack.push_back( Parser::Control::cIf( ( this->vStack.back( ).getVal( this->vList ) > 0 ) ? true : false ) );
                this->vStack.pop_back( );
            }
            else if( oper.compare( std::string( "WHILE" ) ) == 0 ) {
                if( this->vStack.size( ) == 0 ) throw SyntaxError( "WHILE with nothing on the stack!" );

                this->cStack.push_back( Parser::Control::cWhile( this->line, this->vStack.back( ) ) );
                this->vStack.pop_back( );
            }
            else if( oper.compare( std::string( "LOOP" ) ) == 0 ) {
                criter = findTop( Parser::CONTROL_WHILE, true );

                if( criter.type != Parser::CONTROL_WHILE ) throw SyntaxError( "LOOP with no prior WHILE!" );

                if( this->vList[ criter.var ] > 0 ) this->line = criter.line;
                else this->cStack.pop_back( );
            }
            else if( oper.compare( std::string( "BREAK" ) ) == 0 ) {
                criter = findTop( Parser::CONTROL_WHILE, true );

                if( criter.type != Parser::CONTROL_WHILE ) throw SyntaxError( "BREAK with no prior WHILE!" );
                this->cStack.pop_back( );
                findLoop = true;
            }
            else if( this->oList.find( oper ) != this->oList.end( ) ) this->oList[ oper ].doDatThang( this->vStack, this->vList );
            // Oper is a variable
            else if( oper.length( ) == 1 && this->vList.find( oper[ 0 ] ) != this->vList.end( ) )
                this->vStack.push_back( Parser::Value::Variable( oper[ 0 ] ) );
            else {
                // No C++11, no std::stof :(
                // Also msvc, so no support for Infinity / NAN macros :S
                float f = std::numeric_limits<float>::infinity( );
                std::istringstream iss( oper );
                iss >> f;

                if( f != std::numeric_limits<float>::infinity( ) ) this->vStack.push_back( Parser::Value::Literal( f ) );
                // Oper is bad, and should feel bad
                else throw( SyntaxError( "Invalid operator!" ) );
            }
        } else continue;
    }

    return 0;
};

// Ready for the loooong list of operators supported?? :D
// Basic format is this: void operatorname_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ) {...};

// The useful debugging ones!
void wait_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    // TODO: This is a windows only call :/
    Sleep( (DWORD) ( 1000.0 * stack.back( ).getVal( vList ) ) );
    stack.pop_back( );
}
//INPUT

// Some other ones!
//RAND
//DUPE
void pop_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    peek_( stack, vList );
    stack.pop_back( );
}
void ser_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    // TODO: If serial comms are enabled, this should send the top value there instead
    peek_( stack, vList );
    stack.pop_back( );
}
void peek_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    Parser::Value val = stack.back( );
    std::cout << (char) ( (val.isVar) ? val.tag : 32 );
    std::cout << " " << val.getVal( vList ) << std::endl;
}

// The one basic one!
void assign_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // Get rhs and lhs values
    stack[len-1].setVal( vList, stack[len-2].getVal( vList ) );
    stack.pop_back( ); stack.pop_back( );
};

// Arithmetic time!
void add_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( stack[len-2].getVal( vList ) + stack[len-1].getVal( vList ) );
    stack.pop_back( );
};
void sub_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( stack[len-2].getVal( vList ) - stack[len-1].getVal( vList ) );
    stack.pop_back( );
};
void mul_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( stack[len-2].getVal( vList ) * stack[len-1].getVal( vList ) );
    stack.pop_back( );
};
void div_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( stack[len-2].getVal( vList ) / stack[len-1].getVal( vList ) );
    stack.pop_back( );
};

// More advanced maths
void pow_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( pow( stack[len-2].getVal( vList ), stack[len-1].getVal( vList ) ) );
    stack.pop_back( );
};
void sqrt_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( sqrt( stack[len-1].getVal( vList ) ) );
};
void mod_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( fmod( stack[len-2].getVal( vList ), stack[len-1].getVal( vList ) ) );
    stack.pop_back( );
};
void abs_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( abs( stack[len-1].getVal( vList ) ) );
};

// Next up, trig!
void sin_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( sin( stack[len-1].getVal( vList ) ) );
};
void cos_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( cos( stack[len-1].getVal( vList ) ) );
};
void tan_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( tan( stack[len-1].getVal( vList ) ) );
};
void asin_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( asin( stack[len-1].getVal( vList ) ) );
};
void acos_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( acos( stack[len-1].getVal( vList ) ) );
};
void atan_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( atan( stack[len-1].getVal( vList ) ) );
};
void atnt_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( atan2( stack[len-2].getVal( vList ), stack[len-1].getVal( vList ) ) );
    stack.pop_back( );
};

// Now come the log functions!
void exp_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( exp( stack[len-1].getVal( vList ) ) );
};
void log_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( log( stack[len-1].getVal( vList ) ) );
};
void logt_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( log10( stack[len-1].getVal( vList ) ) );
};

// And last but not least, the comparators!
void grt_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( ( stack[len-2].getVal( vList ) > stack[len-1].getVal( vList ) ) ? 1.0f : 0.0f );
    stack.pop_back( );
};
void lst_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( ( stack[len-2].getVal( vList ) < stack[len-1].getVal( vList ) ) ? 1.0f : 0.0f );
    stack.pop_back( );
};
void geq_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( ( stack[len-2].getVal( vList ) >= stack[len-1].getVal( vList ) ) ? 1.0f : 0.0f );
    stack.pop_back( );
};
void leq_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( ( stack[len-2].getVal( vList ) <= stack[len-1].getVal( vList ) ) ? 1.0f : 0.0f );
    stack.pop_back( );
};
void eql_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( ( stack[len-2].getVal( vList ) == stack[len-1].getVal( vList ) ) ? 1.0f : 0.0f );
    stack.pop_back( );
};
void neq_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( ( stack[len-2].getVal( vList ) != stack[len-1].getVal( vList ) ) ? 1.0f : 0.0f );
    stack.pop_back( );
};
