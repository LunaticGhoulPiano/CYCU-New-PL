# include <cstdio>
# include <iostream>
# include <cstring>
using namespace std ;

// For a line that is 2000 char long, we need 2001 char-spaces.
# define SIZE 2001
# define NOT !

typedef char InputStrType[SIZE] ;

void GetUserInput( InputStrType userInput ) ;
void ProcessUserInput ( bool & done, InputStrType inputStr, string & response ) ;
void PrintSystemOutput( string response ) ;

int main( ) {

  InputStrType inputStr = {} ;  // initialized to be all NULLs
  bool done = false ;

  printf( "Enter any string, followed by an ENTER. Enter 'done' when you are done.\n" ) ;

  while ( NOT done ) {

    printf( "> " ) ;

    /*
    if ( cin.eof() ) { // no more user input
      done = true ;
      printf( "\n" ) ;  // Ctrl-D should be echoed as line-enter
    } // if no more user input

    else // there is more user input
      GetUserInput( inputStr ) ;
    */

    GetUserInput( inputStr ) ;

    // as it seems, if the user enters Ctrl-D at the start of a line, cin.eof() returns false ;
    // getline() will directly return if the first char is Ctrl-D ; then, cin.eof() returns true
    // (getline() will not return if the first char is not Ctrl-D and there is no line-enter (yet))

    if ( cin.eof() ) {
      done = true ;
      printf( "\n" ) ;
    } // if no more input

    if ( NOT done ) {

      string response = "" ;
      ProcessUserInput( done, inputStr, response ) ;

      if ( NOT done )
        PrintSystemOutput( response ) ;

    } // if NOT done

    // else done
    //   nothing needs to be done ; just exit this interaction loop

  } // while NOT done

  printf( "Bye! You have a good day!\n" ) ;

} // main()

void GetUserInput( InputStrType userInput ) {
// There is user input, and it is guaranteed not to start with a Ctrl-D ;
// Get one line of user input and put it in 'userInput' (at most SIZE char are allowed, including NULL)

  // scanf( "%s", inputStr ) ; // this does not get the whole line
  cin.getline( userInput, SIZE ) ;
  // cin >> ch ;     // No need to read in the ENTER char when we use getline()

} // GetUserInput()

void ProcessUserInput ( bool & done, InputStrType inputStr, string & response ) {

  response = "" ; // if done

  if ( strcmp( "done", inputStr ) == 0 )
    done = true ;
  else if ( strcmp( "quit", inputStr ) == 0 )
    done = true ;
  else if ( strcmp( "exit", inputStr ) == 0 )
    done = true ;
  else // NOT done
    response = "echoing : " + string( inputStr ) + "\n" ;

} // ProcessUserInput()

void PrintSystemOutput( string response ) {

  printf( "%s", response.c_str() ) ;

} // PrintSystemOutput()
