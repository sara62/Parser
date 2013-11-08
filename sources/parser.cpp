#include "parser.h"

#include "stdio.h"

#undef DEBUG

Parser::Parser( ) {
    // TODO: Set up all the functions/variables... :-S
};
Parser::~Parser( ) { };

int Parser::runParse( ) {
    std::string file;

    this->running = true;

    while( 1 ){
        printf( "Program? -> " );
        std::cin >> file;

        std::transform(file.begin(), file.end(), file.begin(), ::tolower);

        if( file.compare( std::string( "exit" ) ) == 0 ){
            exit( 0 );
        } else if( file.compare( std::string( "load" ) ) == 0 ){
            std::cin >> file;
            // TODO: RESET
            if( this->openFile( file ) == -1 ){
                printf( "Could not open file %s\n", file.c_str() );
            }
        } else if( file.compare( std::string( "run" ) ) == 0 ){
            // TODO: runProg()
        } else if( file.compare( std::string( "clear" ) ) == 0 ){
            // TODO: reset()
        } else if( file.compare( std::string( "list" ) ) == 0 ){
            // TODO: for each line, print the line
        } else if( file.compare( std::string( "help" ) ) == 0 ){
            printf( "Commands: EXIT, LOAD, RUN, CLEAR, LIST, HELP\n" );
        }
    }

    return 0;
};

void Parser::reset( ) {
    this->file.close( );
    while( ! this->vStack.empty( ) ) this->vStack.pop( );
    this->cStack.erase( this->cStack.begin( ), this->cStack.end( ) );
    // TODO: CLEAR ALL VARIABLES
};

int Parser::openFile( std::string& filename ){
#ifdef DEBUG
    char buf[ 12 ];
#endif
    char fchar;
    bool mark = true;

    // Save filename
    this->filename = filename;

    // Open the file, debug print
    // MSVC 2008 DOESNT SUPPORT C++11 OH MY GAWD
    this->file.open( this->filename.c_str( ), std::fstream::in );
#ifdef DEBUG
    printf( "%d\n", this->file.is_open( ) == true ? 1 : 0 );
#endif
    if( ! this->file.is_open( ) ) return -1;

    // Get line starts
    this->linestarts.clear( );
    this->linestarts.push_back( 0 );
    this->file.get(fchar);
    while( !this->file.eof() ){
        if( mark && fchar != 13 ){
            mark = false;
            this->linestarts.push_back( (unsigned short) this->file.tellg( ) - 1 );
        }
        if( fchar == 10 ){
            mark = true;
        }
        this->file.get( fchar );
    }

    // Reset the file after eof
    // NOTE: TEST THIS, ITS POSSIBLY A WINDOWS ONLY THING :(
    this->file.clear( );

#ifdef DEBUG
    for( unsigned i = 1; i < this->linestarts.size(); ++i ){
        this->file.seekg( this->linestarts[ i ], this->file.beg );
        this->file.get( buf, 12 );
        if( this->file.rdstate( ) == this->file.goodbit ){
            printf( "Line: %d\nPosition: %d\n-> %s\n", i, this->linestarts[ i ], buf );
        } else {
            printf( "FAILURE reading line: %d ->%d%d%d%d\n", i, (this->file.good())?1:0, (this->file.eof())?1:0, (this->file.fail())?1:0, (this->file.bad())?1:0 );
        }
    }
#endif

    // Return line count
    return this->linestarts.size( );
};

int Parser::runFile( ) {
    // TODO: This is the python function runProg(...)
    // while true:
        // if line number is out of bounds, exit good
        // try to parseLine a line
        // catch and print syntax errors
        // if running false
            // break
        // else
            // increment line counter
        
    return 0;
};

int Parser::parseLine( unsigned linenum ) {
    // TODO: This is the python function parseRPN(...)

    // getline the line
    // while not endofline
        // line >> oper
        // seach cStack for if
            // -1 if not one, index else
        // if index -1, branch is true, or is ELSE or is ENDIF
            // check operator list for oper
                // run if in
            // check variable list for oper
                // add to stack if in
            // check for float conversion
                // add to stack if in
            // if is REM
                // return
            // if is PRINT
                // print line number
            // else
                // SyntaxError
        // else
            // continue
        
    return 0;
};
