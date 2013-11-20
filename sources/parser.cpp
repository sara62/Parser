/*************************************************************************************
* parser.cpp
* Defines the Parser class, its methods, and its helper functions
* 
* Copyright (C) 2013 by Eric Osburn.
* The redistribution terms are provided in the LICENSE file that must
* be distributed with this source code.
*************************************************************************************/

/*************************************************************************************
* Includes
*************************************************************************************/
#include "parser.h"

/*************************************************************************************
* Turn on debug mode for now
*************************************************************************************/
#define DEBUG

/*************************************************************************************
* Parser class constructor
*************************************************************************************/
Parser::Parser( ) {
    // Make temp variable for variable list enumeration
    char cChar;

    // Enumerate the variable list with chars from 65-90 ( A-Z )
    for( cChar = 65; cChar < 91; ++cChar ){
        this->vList[ cChar ] = 0.0;
    }

    // Add all operators to the oList. So many calls :/
    this->oList[ "WAIT" ]   = Parser::Operator( (unsigned) 0, wait_ );
    this->oList[ "POP" ]    = Parser::Operator( (unsigned) 0, pop_ );
    this->oList[ "SER" ]    = Parser::Operator( (unsigned) 0, ser_ );
    this->oList[ "PEEK" ]   = Parser::Operator( (unsigned) 0, peek_ );
    this->oList[ "=" ]      = Parser::Operator( (unsigned) 2, assign_ );
    this->oList[ "+" ]      = Parser::Operator( (unsigned) 2, add_ );
    this->oList[ "-" ]      = Parser::Operator( (unsigned) 2, sub_ );
    this->oList[ "*" ]      = Parser::Operator( (unsigned) 2, mul_ );
    this->oList[ "/" ]      = Parser::Operator( (unsigned) 2, div_ );
    this->oList[ "**" ]     = Parser::Operator( (unsigned) 2, pow_ );
    this->oList[ "SQRT" ]   = Parser::Operator( (unsigned) 1, sqrt_ );
    this->oList[ "%" ]      = Parser::Operator( (unsigned) 2, mod_ );
    this->oList[ "ABS" ]    = Parser::Operator( (unsigned) 1, abs_ );
    this->oList[ "SIN" ]    = Parser::Operator( (unsigned) 1, sin_ );
    this->oList[ "COS" ]    = Parser::Operator( (unsigned) 1, cos_ );
    this->oList[ "TAN" ]    = Parser::Operator( (unsigned) 1, tan_ );
    this->oList[ "ASIN" ]   = Parser::Operator( (unsigned) 1, asin_ );
    this->oList[ "ACOS" ]   = Parser::Operator( (unsigned) 1, acos_ );
    this->oList[ "ATAN" ]   = Parser::Operator( (unsigned) 1, atan_ );
    this->oList[ "ATNT" ]   = Parser::Operator( (unsigned) 2, atnt_ );
    this->oList[ "EXP" ]    = Parser::Operator( (unsigned) 1, exp_ );
    this->oList[ "LOG" ]    = Parser::Operator( (unsigned) 1, log_ );
    this->oList[ "LOGT" ]   = Parser::Operator( (unsigned) 1, logt_ );
    this->oList[ ">" ]      = Parser::Operator( (unsigned) 2, grt_ );
    this->oList[ "<" ]      = Parser::Operator( (unsigned) 2, lst_ );
    this->oList[ ">=" ]     = Parser::Operator( (unsigned) 2, geq_ );
    this->oList[ "<=" ]     = Parser::Operator( (unsigned) 2, leq_ );
    this->oList[ "==" ]     = Parser::Operator( (unsigned) 2, eql_ );
    this->oList[ "!=" ]     = Parser::Operator( (unsigned) 2, neq_ );
};

/*************************************************************************************
* Parser class destructor
*************************************************************************************/
Parser::~Parser( ) { };

/*************************************************************************************
* Parser runParse
* Runs the command line for the parser
*************************************************************************************/
int Parser::runParse( ) {
    // This is used for more than just a file...
    std::string file;

    // Start the parse engine again, in case it was stopped previously
    this->running = true;

    // DO DIS FO-EVAH
    while( 1 ){
        // I'm terrible at keeping consistant, I use printf and cout interchangeably..
        printf( "Clever prompt -> " );

        // Get a word from the user
        std::cin >> file;

        // Make the command all lowercase ( Makes matching a breeze )
        std::transform(file.begin(), file.end(), file.begin(), ::tolower);

        // Find the command they want! No default case needed.
        if( file.compare( std::string( "exit" ) ) == 0 ){
            // They want to quit? TERMINATE ALL THE THINGS
            this->reset( );
            break /* free, and believe */;

        } else if( file.compare( std::string( "load" ) ) == 0 ){
            // Give them something to watch
            printf( "Loading" );

            // Next command had better be the file
            std::cin >> file;

            // Try to open the file
            if( this->openFile( file ) == -1 ){
                // If we couldn't, let them know
                printf( "\nCould not open file %s", file.c_str() );
            }
            printf( "\n" );

            // openFile takes care of setting everything up, so nothing else need be done

        } else if( file.compare( std::string( "run" ) ) == 0 ){
            // Pretty obvious, parse the file
            this->runFile( );

        } else if( file.compare( std::string( "clear" ) ) == 0 ){
            // Reset the parser to default state
            this->reset( );

        } else if( file.compare( std::string( "list" ) ) == 0 ){
            // List out the program, line by line
            this->listProg( );

        } else if( file.compare( std::string( "help" ) ) == 0 ){
            // Give them some help, they clearly need it... :)
            printf( "Commands: EXIT, LOAD, RUN, CLEAR, LIST, HELP\n" );

        }
    }

    // We should never get here, but MSVS complains that a branch has no return
    return 0;
};

/*************************************************************************************
* Parser findTop
* Finds the top control object of a given type
*************************************************************************************/
Parser::Control Parser::findTop( Parser::ControlType type, bool popoff = false ){
    // Two temporary iterators
    std::deque<Parser::Control>::reverse_iterator retfinder;
    // This one is so we dont have to recalculate .rend( ) every iteration
    std::deque<Parser::Control>::reverse_iterator endoflist;

    // If we have nothing, return invalid
    if( this->cStack.size( ) == 0 ) return Parser::Control( );

    // Parse the control stack backwards
    for( retfinder = this->cStack.rbegin( ), endoflist = this->cStack.rend( ); retfinder != endoflist; ++retfinder ){
        // If we find a good object
        if( retfinder->type == type ){
            // If the user wants to delete everything on top of it
            if( popoff ){
                // Provided it isn't the only thing in the stack
                if( retfinder != this->cStack.rbegin( ) ){
                    // Erase all objects above it on the stack
                    this->cStack.erase( retfinder.base( ), this->cStack.end( ) );

                }
            
                // Return the new top, because the iterator is borked
                return this->cStack[ this->cStack.size( ) - 1 ];

            }

            // If they don't want to delete everything else, just return what we found
            return *retfinder;
        }
    }
    
    // We didn't find one. Return invalid.
    return Parser::Control( );
};

/*************************************************************************************
* Parser reset
* Sets all parser states back to default
*************************************************************************************/
void Parser::reset( ) {
    // Make temp variable for variable list clearing
    char cChar;

    // Close the current file
    this->file.close( );

    // Clear both the stacks
    this->vStack.erase( this->vStack.begin( ), this->vStack.end( ) );
    this->cStack.erase( this->cStack.begin( ), this->cStack.end( ) );

    // Set all variables to 0.0
    for( cChar = 65; cChar < 91; ++cChar ){
        this->vList[ cChar ] = 0.0;
    }
};

/*************************************************************************************
* Parser openFile
* Opens a file, gets all the linestarts for it
*************************************************************************************/
int Parser::openFile( std::string& filename ){
    // Searching variables
    char fchar;
    bool mark = true;

    // Close file if already open
    if( this->file.is_open( ) ) this->reset( );

    // Save filename
    this->filename = filename;

    // Open the file, debug print
    // MSVS 9 DOESNT SUPPORT C++11 OH MY GAWD
    // Why cant I open with a std::string???
    this->file.open( this->filename.c_str( ), std::fstream::in );
    if( ! this->file.is_open( ) ) return -1;

    // Clear the last linestarts ( if any )
    this->linestarts.clear( );

    // Add on line 0 ( Invalid line anyways )
    this->linestarts.push_back( 0 );
    
    // Get the first char, for reasons
    this->file.get(fchar);

    // While we havent hit the end of the file
    while( !this->file.eof() ){
        // If we had a newline, and this char is not a newline
        if( mark && fchar != 13 ){
            // No more mark
            mark = false;

            // Add the current file position to the linestarts
            this->linestarts.push_back( (unsigned short) this->file.tellg( ) - 1 );

            // And print a dot to make the output dynamic
            printf( "." );
        }

        // If we found a newline, start looking for non-newline
        if( fchar == 10 ){
            mark = true;
        }

        // Get the next char
        this->file.get( fchar );
    }

    // Reset the file after eof
    // NOTE: TEST THIS, ITS POSSIBLY A WINDOWS ONLY THING :(
    this->file.clear( );

    // Return line count
    return this->linestarts.size( );
};

/*************************************************************************************
* Parser listProg
* Prints all the lines in a file, with line numbers
*************************************************************************************/
void Parser::listProg( ){
    // Counter variable and current line
    unsigned i;
    std::string line;

    // Return if the file is not open
    // QUESTION: Print an error message here?
    if( ! this->file.is_open( ) ) return;

    // Reset the EOF if necessary
    if( this->file.eof( ) ) this->file.clear( );

    // Loop through the linestarts
    for( i = 1; i < this->linestarts.size( ); ++i ){
        // Seek into the file
        this->file.seekg( this->linestarts[ i ], this->file.beg );

        // Get a line
        if( getline( this->file, line ) ){
            // Told you I switch between cout and printf!
            // Output looks like this:
            // #: asdfasdfasdfasdfasdfasdf
            std::cout << i << ": " << line << std::endl;
        } else break;
        // If we couldn't get a line, just break

    }
};

/*************************************************************************************
* Parser runFile
* Runs the current file, parsing line by line
*************************************************************************************/
int Parser::runFile( ) {
    // Set first line to execute to 1
    this->line = 1;

    // Get a counter for the total lines
    unsigned lncnt = this->linestarts.size( ) - 1;

    // We break out of this other ways if needed also
    // Loop while the line is in bounds, and we are actively parsing
    while( !( this->line > lncnt || this->line < 1 ) && this->running ){
        // I used exceptions for once!
        try{
            // Parse dat line
            this->parseLine( this->line );

        } catch( SyntaxError e ){
            // If there was a syntax error, print out what it was, and where it occurred
            std::cout << e.what( ) << " At line:" << this->line << std::endl;

            // Break out of the file parse
            return -1;

        }

        // Increment the line to parse
        // NOTE: This causes problems with jumps, which I handle with magic number offsets.. :)
        ++( this->line );
    }

    // Always print a success message, because everyone likes positivity, even when it didn't work!
    printf( "Program terminated successfully.\n" );
    return 0;
};

/*************************************************************************************
* Parser parseLine
* The big one! Parse an entire line, one operator at a time.
*************************************************************************************/
int Parser::parseLine( unsigned linenum ) {
    // Line from file -> stringstream lets me >> operators out of the line
    std::stringstream line;
    // Current operator
    std::string oper;
    // Used for If branch checking
    Parser::Control criter;
    // We're printing the rest of the line
    bool printing = false;
    // We're looking for a LOOP after a BREAK statement
    static bool findLoop = false;

    // If the file is not good
    if( ! this->file.is_open( ) ){
        // Set running false, this stops the runFile method
        this->running = false;
        return -1;
    }

    // If we hit eof, reset the file
    if( this->file.eof( ) ) this->file.clear( );

    // Seek to the proper position for this line
    this->file.seekg( this->linestarts[ linenum ] );

    // Get a line into a string
    std::getline( this->file, oper );
    // Put the line into the stringstream
    line << oper;

    // Check for empty line
    if( oper.find_first_not_of( std::string( " " ) ) == oper.npos ) return 0;

    // For all operators in the line
    while( ! line.eof( ) ){
        // By default, the last branch was true
        bool lastifbranch = true;

        // Get an operand!
        line >> oper;

        // If the operand is a comment marker, ignore the rest of the line
        if( oper.compare( std::string( "REM" ) ) == 0 ) return 0;

        // If we're searching for a LOOP
        if( findLoop ){
            // And we find one
            if( oper.compare( std::string( "LOOP" ) ) == 0 ){
                // We're not looking anymore
                findLoop = false;
                // Start on the next line
                return 0;

            } else continue;
            // We don't want to evaluate the operand, so continue
        }

        // If we need to print the rest of the line
        if( printing ){
            // Print the operand and a space, continue to the next one
            std::cout << oper << " ";
            continue;
        }

        // If we even have anything on the control stack
        if( this->cStack.size( ) > 0 ){
            // Find the topmost if statement, get its branch
            criter = findTop( Parser::CONTROL_IF );

            // Small error check here, in case its not actually an IF
            if( criter.type == Parser::CONTROL_IF ) lastifbranch = criter.branch;
        }

        // Check for and parse ELSE/ENDIF unconditionally
        if( oper.compare( std::string( "ELSE" ) ) == 0 ) {
            // There needs to be an IF on the top of the stack, or this fails
            if( this->cStack.size( ) == 0 || this->cStack.back( ).type != Parser::CONTROL_IF ) throw SyntaxError( "ELSE with no immediate prior IF!" );
            
            // Invert the last IF's branch
            this->cStack.back( ).branch = ! this->cStack.back( ).branch;
            
            // Continue to next operand
            continue;
        }
        else if( oper.compare( std::string( "ENDIF" ) ) == 0 ) {
            // There needs to be an IF on the top of the stack, or this fails
            if( this->cStack.size( ) == 0 || this->cStack.back( ).type != Parser::CONTROL_IF ) throw SyntaxError( "ELSE with no immediate prior IF!" );
            
            // Wipe that silly IF off your stack!
            this->cStack.pop_back( );

            // Continue to next operand
            continue;
        }
        
        // The meaty parsing portion now
        // But only if we actually want to parse it
        if( lastifbranch == true ){

            // If we get a PRINT, print the rest of the line
            if( oper.compare( std::string( "PRINT" ) ) == 0 )
                { std::cout << std::endl; printing = true; continue; }

            // If we get a QUIT, let runFile know, and return immediately
            else if( oper.compare( std::string( "QUIT" ) ) == 0 )
                { this->running = false; return 0; }

            // If we get a LINE, print the current line number
            else if( oper.compare( std::string( "LINE" ) ) == 0 )
                { std::cout << "Line: " << this->line << std::endl; }

            // If we get a PRNT, do a complete stack dump
            // QUESTION: Should the variable and control stacks be broken into two
            //   separate commands?
            else if( oper.compare( std::string( "PRNT" ) ) == 0 ) {
                // Iterators for value and control stacks
                std::deque<Parser::Value>::reverse_iterator riter, end;
                std::deque<Parser::Control>::reverse_iterator criter, cend;
                // Again, the ends are there so we dont have to recalculate .rend( ) every loop

                // Mark the sections
                std::cout << std::endl << "Stack Dump:" << std::endl;

                // For each value on the stack
                for( riter = this->vStack.rbegin( ), end = this->vStack.rend( ); riter != end; ++riter ){
                    // If they're a variable, print their tag, else a space
                    std::cout << (char) ( (riter->isVar) ? riter->tag : 32 );

                    // Then print their value
                    std::cout << " " << riter->getVal( vList ) << std::endl;
                }

                // For each control object on the stack
                std::cout << std::endl << "Control Stack Dump:" << std::endl;
                for( criter = this->cStack.rbegin( ), cend = this->cStack.rend( ); criter != cend; ++criter ){
                    // Print their type, with an output of DURRR if someone manages to break the object
                    std::cout << ( ( criter->type == Parser::CONTROL_INVALID ) ? "INVALID" : ( ( criter->type == Parser::CONTROL_GOSUB ) ? "GOSUB" : ( ( criter->type == Parser::CONTROL_IF ) ? "IF" : ( ( criter->type == Parser::CONTROL_WHILE ) ? "WHILE" : ( "DURRR" ) ) ) ) );
                    // Then print ALL the information from them, this could mean getting bogus values
                    // I'm just too lazy to switch by type, and only print the useful stuff |-P
                    std::cout << " " << ( ( criter->branch ) ? "true" : "false" ) << " " << criter->line << " " << (char) criter->var << std::endl;
                }
            }
            
            // Now come the control statements, that need access to the Parser class variables
            // If we get a GOTO
            else if( oper.compare( std::string( "GOTO" ) ) == 0 ) {
                // Check we have a line to go to
                if( this->vStack.size( ) < 1) throw SyntaxError( "GOTO with no variables on stack!" );

                // This cast makes fun things happen with negative numbers :)
                // I'll leave it like this though, the users must take caution...
                unsigned tline = ( unsigned ) this->vStack.back( ).getVal( this->vList );

                // Sanity check the line is in range
                if( tline > this->linestarts.size( ) || tline == 0 ) throw SyntaxError( "GOTO to invalid line!" );

                // See the magic numbers!
                // Use the magic numbers!
                // Go magic numbers, go!

                // This offsets correctly so we jump to the intended line
                this->line = tline - 1;

                // Then remove the line value from the stack
                this->vStack.pop_back( );
            }
            // If we get a GOSUB, jump with ret
            else if( oper.compare( std::string( "GOSUB" ) ) == 0 ) {
                // Check we actually have a line number to use
                if( this->vStack.size( ) < 1) throw SyntaxError( "GOSUB with no variables on stack!" );

                // Get the line number, not worrying about negatives, or out of range values
                unsigned tline = ( unsigned ) this->vStack.back( ).getVal( this->vList );

                // Sanity check the line is in range
                if( tline > this->linestarts.size( ) || tline == 0 ) throw SyntaxError( "GOSUB to invalid line!" );

                // Set next line to the desired line
                this->line = tline - 1;

                // Pop the line value from the stack
                this->vStack.pop_back( );

                // Push to the control stack an object so the user can RET later
                this->cStack.push_back( Control::cGosub( this->line ) );
            }
            // If we get a RET
            else if( oper.compare( std::string( "RET" ) ) == 0 ) {
                // Check that we even have a GoSub object
                if( this->cStack.size( ) == 0 ) throw SyntaxError( "RET with no prior GOSUB!" );
                // Get the topmost GoSub object, wiping all other controls
                //   You can return inside an IF for example
                criter = findTop( Parser::CONTROL_GOSUB, true );

                // Set line to the return address
                this->line = criter.line;

                // Pop the GoSub off the control stack
                this->cStack.pop_back( );
            }
            // If we get an IF
            else if( oper.compare( std::string( "IF" ) ) == 0 ) {
                // Sanity check stack size
                if( this->vStack.size( ) == 0 ) throw SyntaxError( "IF with nothing on the stack!" );

                // Push an If to the control stack, with the current branch
                this->cStack.push_back( Parser::Control::cIf( ( this->vStack.back( ).getVal( this->vList ) > 0 ) ? true : false ) );
                
                // Pop off the value used
                this->vStack.pop_back( );
            }
            // If we get a WHILE
            else if( oper.compare( std::string( "WHILE" ) ) == 0 ) {
                // Sanity check stack size
                if( this->vStack.size( ) == 0 ) throw SyntaxError( "WHILE with nothing on the stack!" );

                // Push a While object to the control stack
                this->cStack.push_back( Parser::Control::cWhile( this->line, this->vStack.back( ) ) );

                // Pop off the Variable assigned to the loop
                this->vStack.pop_back( );
            }
            // If we get a LOOP
            else if( oper.compare( std::string( "LOOP" ) ) == 0 ) {
                // Get the topmost While object, wiping everything above it
                criter = findTop( Parser::CONTROL_WHILE, true );

                // Sanity check object type
                if( criter.type != Parser::CONTROL_WHILE ) throw SyntaxError( "LOOP with no prior WHILE!" );

                // Check the loop condition variable, jump if true
                if( this->vList[ criter.var ] > 0 ) this->line = criter.line;

                // Pop off the while if the condition is false
                // Pssstt... Notice the else at the beginning of the line :)
                else this->cStack.pop_back( );
            }
            // If we get a BREAK
            else if( oper.compare( std::string( "BREAK" ) ) == 0 ) {
                // Get the topmost While object
                criter = findTop( Parser::CONTROL_WHILE, true );

                // Sanity check the object's type
                if( criter.type != Parser::CONTROL_WHILE ) throw SyntaxError( "BREAK with no prior WHILE!" );

                // Pop off the While object
                this->cStack.pop_back( );

                // And mark that we're looking for a LOOP operator
                findLoop = true;
            }
            
            // Okay, so it wasn't a control operator
            // Check the list of operators now
            else if( this->oList.find( oper ) != this->oList.end( ) )
                // If we find it, run it's do method
                this->oList[ oper ].doDatThang( this->vStack, this->vList );
            
            // At this point, it must be a Variable, right?
            else if( oper.length( ) == 1 && this->vList.find( oper[ 0 ] ) != this->vList.end( ) )
                // Push it to the stack if it is
                this->vStack.push_back( Parser::Value::Variable( oper[ 0 ] ) );

            // Okay, it's either a literal, or its invalid.
            else {
                // MSVS has no support for Infinity / NAN macros :-S
                // That really bugs me...

                // Get a float with a value of infinity ( Least likely input value from user )
                float f = std::numeric_limits<float>::infinity( );
                // Get an unnecessary stringstream
                // C++11 defines -> float std::stof( std::string )
                std::istringstream iss( oper );
                
                // Try to convert the operator to a float
                iss >> f;

                // If we succeeded
                if( f != std::numeric_limits<float>::infinity( ) )
                    // Push it to the stack as a Literal
                    this->vStack.push_back( Parser::Value::Literal( f ) );

                // Oper is bad, and should feel bad
                else throw( SyntaxError( "Invalid operator!" ) );
            }
        } else continue;
    }

    return 0;
};

/*************************************************************************************
* Ready for the loooong list of operators supported?? :D
*
* Basic format is this: void operatorname_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ) {...};
*
* See Language.txt in resources for a description of each of these
*   I'm not going to restate all of them here, sorry
*     Yet... I may in future, once I stop tweaking the wording...
*   Heck, even go check out the header for this class, it has the actual command listings
*************************************************************************************/

/*************************************************************************************
* The useful debugging ones!
*************************************************************************************/
/*************************************************************************************
* wait_
*************************************************************************************/
void wait_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    // TODO: This is a windows only call :/
    Sleep( (DWORD) ( 1000.0 * stack.back( ).getVal( vList ) ) );
    stack.pop_back( );
}
/*************************************************************************************
* pop_
*************************************************************************************/
void pop_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    peek_( stack, vList );
    stack.pop_back( );
}
/*************************************************************************************
* ser_
*************************************************************************************/
void ser_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    // TODO: If serial comms are enabled, this should send the top value there instead
    peek_( stack, vList );
    stack.pop_back( );
}
/*************************************************************************************
* peek_
*************************************************************************************/
void peek_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    Parser::Value val = stack.back( );
    std::cout << (char) ( (val.isVar) ? val.tag : 32 );
    std::cout << " " << val.getVal( vList ) << std::endl;
}

/*************************************************************************************
* Some random ones...
*************************************************************************************/
//INPUT

// Some other ones!
//RAND
//DUPE

/*************************************************************************************
* The basic one!
*************************************************************************************/
/*************************************************************************************
* assign_
*************************************************************************************/
void assign_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // Get rhs and lhs values
    stack[len-1].setVal( vList, stack[len-2].getVal( vList ) );
    stack.pop_back( ); stack.pop_back( );
};

/*************************************************************************************
* Arithmetic time
*************************************************************************************/
/*************************************************************************************
* add_
*************************************************************************************/
void add_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( stack[len-2].getVal( vList ) + stack[len-1].getVal( vList ) );
    stack.pop_back( );
};
/*************************************************************************************
* sub_
*************************************************************************************/
void sub_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( stack[len-2].getVal( vList ) - stack[len-1].getVal( vList ) );
    stack.pop_back( );
};
/*************************************************************************************
* mul_
*************************************************************************************/
void mul_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( stack[len-2].getVal( vList ) * stack[len-1].getVal( vList ) );
    stack.pop_back( );
};
/*************************************************************************************
* div_
*************************************************************************************/
void div_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( stack[len-2].getVal( vList ) / stack[len-1].getVal( vList ) );
    stack.pop_back( );
};

/*************************************************************************************
* Gettin fancy wit dem maths
*************************************************************************************/
/*************************************************************************************
* pow_
*************************************************************************************/
void pow_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( pow( stack[len-2].getVal( vList ), stack[len-1].getVal( vList ) ) );
    stack.pop_back( );
};
/*************************************************************************************
* sqrt_
*************************************************************************************/
void sqrt_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( sqrt( stack[len-1].getVal( vList ) ) );
};
/*************************************************************************************
* mod_
*************************************************************************************/
void mod_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( fmod( stack[len-2].getVal( vList ), stack[len-1].getVal( vList ) ) );
    stack.pop_back( );
};
/*************************************************************************************
* abs_
*************************************************************************************/
void abs_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( abs( stack[len-1].getVal( vList ) ) );
};

/*************************************************************************************
* Next up come the trg functions!
*************************************************************************************/
/*************************************************************************************
* sin_
*************************************************************************************/
void sin_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( sin( stack[len-1].getVal( vList ) ) );
};
/*************************************************************************************
* cos_
*************************************************************************************/
void cos_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( cos( stack[len-1].getVal( vList ) ) );
};
/*************************************************************************************
* tan_
*************************************************************************************/
void tan_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( tan( stack[len-1].getVal( vList ) ) );
};
/*************************************************************************************
* asin_
*************************************************************************************/
void asin_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( asin( stack[len-1].getVal( vList ) ) );
};
/*************************************************************************************
* acos_
*************************************************************************************/
void acos_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( acos( stack[len-1].getVal( vList ) ) );
};
/*************************************************************************************
* atan_
*************************************************************************************/
void atan_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( atan( stack[len-1].getVal( vList ) ) );
};
/*************************************************************************************
* atnt_
*************************************************************************************/
void atnt_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( atan2( stack[len-2].getVal( vList ), stack[len-1].getVal( vList ) ) );
    stack.pop_back( );
};

/*************************************************************************************
* Log functions!
*************************************************************************************/
/*************************************************************************************
* exp_
*************************************************************************************/
void exp_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( exp( stack[len-1].getVal( vList ) ) );
};
/*************************************************************************************
* log_
*************************************************************************************/
void log_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( log( stack[len-1].getVal( vList ) ) );
};
/*************************************************************************************
* logt_
*************************************************************************************/
void logt_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-1] = Parser::Value::Literal( log10( stack[len-1].getVal( vList ) ) );
};

/*************************************************************************************
* And bringing up the rear, the comparators!
*************************************************************************************/
/*************************************************************************************
* grt_
*************************************************************************************/
void grt_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( ( stack[len-2].getVal( vList ) > stack[len-1].getVal( vList ) ) ? 1.0f : 0.0f );
    stack.pop_back( );
};
/*************************************************************************************
* lst_
*************************************************************************************/
void lst_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( ( stack[len-2].getVal( vList ) < stack[len-1].getVal( vList ) ) ? 1.0f : 0.0f );
    stack.pop_back( );
};
/*************************************************************************************
* geq_
*************************************************************************************/
void geq_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( ( stack[len-2].getVal( vList ) >= stack[len-1].getVal( vList ) ) ? 1.0f : 0.0f );
    stack.pop_back( );
};
/*************************************************************************************
* leq_
*************************************************************************************/
void leq_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( ( stack[len-2].getVal( vList ) <= stack[len-1].getVal( vList ) ) ? 1.0f : 0.0f );
    stack.pop_back( );
};
/*************************************************************************************
* eql_
*************************************************************************************/
void eql_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( ( stack[len-2].getVal( vList ) == stack[len-1].getVal( vList ) ) ? 1.0f : 0.0f );
    stack.pop_back( );
};
/*************************************************************************************
* neq_
*************************************************************************************/
void neq_( std::deque<Parser::Value>& stack, std::map<char, float>& vList ){
    unsigned len = stack.size( );
    // In-place replace the result
    stack[len-2] = Parser::Value::Literal( ( stack[len-2].getVal( vList ) != stack[len-1].getVal( vList ) ) ? 1.0f : 0.0f );
    stack.pop_back( );
};
