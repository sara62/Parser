#include "parser.h"

#include "stdio.h"

#define DEBUG

Parser::Parser( ) {
    // TODO: Set up all the functions... :-S
    //   Thats gonna be a lot of oList calls...
    char cChar;

    for( cChar = 65; cChar < 91; ++cChar ){
        this->vList[ cChar ] = 0.0;
    }
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
    unsigned line = 1;
    unsigned lncnt = this->linestarts.size( ) - 1;

    while( 1 ){
        if( line > lncnt || line < 1 ) break;
        try{
            this->parseLine( line );
        } catch( SyntaxError e ){
            std::cout << e.what( ) << " At line:" << line << std::endl;
            return -1;
        }
        if( this->running == false ) break;
        else ++line;
    }

    printf( "Program terminated successfully.\n" );
    return 0;
};

int Parser::parseLine( unsigned linenum ) {
    std::stringstream line;
    std::string oper;
    std::deque<Parser::Control>::reverse_iterator criter;
    bool printing = false;

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
        // Oper is a comment marker
        if( oper.compare( std::string( "REM" ) ) == 0 ) return 0;

        if( printing ){
            std::cout << oper << " ";
            continue;
        }
        
        // Find the topmost if statement
        for( criter = this->cStack.rbegin( ); criter != this->cStack.rend( ); ++criter ){
            if( criter->type == Parser::CONTROL_IF ){
                // If there is one, get the branch we are on
                lastifbranch = criter->branch;
                break;
            }
        }
        
        // Check for ELSE/ENDIF unconditionally
        if( oper.compare( std::string( "ELSE" ) ) == 0 ) {}
        else if( oper.compare( std::string( "ENDIF" ) ) == 0 ) {}

        if( lastifbranch == true ){
            printf( "\n:%d:%s:", linenum, oper.c_str( ) );
            
            // Oper is a print statement
            if( oper.compare( std::string( "PRINT" ) ) == 0 ) { std::cout << std::endl; printing = true; continue; }
            // Oper is quitting execution
            else if( oper.compare( std::string( "QUIT" ) ) == 0 ) { this->running = false; return 0; }
            // check oper for control statements
            /*
                def ret_():
                def if_():
                def else_():
                def endif_():
                def while_():
                def loop_():
                def break_():
            */
            // check operator list for oper
                // run if in
            // Oper is a variable
            else if( oper.length( ) == 1 && this->vList.find( oper.c_str( )[ 0 ] ) != this->vList.end( ) )
                this->vStack.push_back( Value::Variable( oper.c_str( )[ 0 ] ) );
            // check for float conversion
                // add to stack if good
            // Oper is bad, and should feel bad
            else throw( SyntaxError( "Invalid operator!" ) );
        } else continue;
    }

    printf( "\n" );
        
    return 0;
};
