# include <cstdio>
# include <iostream>
# include <cstring>
using namespace std ;

// For a line that is 2000 char long, we need 2001 char-spaces.
# define SIZE 2001
# define NOT !

typedef char InputStrType[SIZE] ;
typedef char ShortString[50] ;

struct Variable {
  string name ;
  string value ;
} ;

typedef Variable VariableArray[ 50 ] ;

VariableArray uVars ; // at most 50 myShell-vars
int uVarCount = 0 ;   // currently none

void GetUserInput( InputStrType userInput ) ;
void ProcessUserInput ( bool & done, InputStrType inputStr, string & response ) ;
void PrintSystemOutput( string response ) ;

string FindValue( ShortString name ) ;
void SetValue( ShortString name, ShortString value ) ;
bool IsCmdAndProcessed( InputStrType inputStr, string & response ) ;

int main( ) {

  InputStrType inputStr = {} ;  // initialized to be all NULLs
  bool done = false ;

  printf( "Enter a var-name or '<varName> = <string>' or two or more words and then ENTER. Use 'done' to exit.\n" ) ;

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

string FindValue( ShortString varName ) {

  string toReturn = "" ;

  for ( int i = 0 ; i < uVarCount ; i++ )
    if ( uVars[i].name == string( varName ) )
      toReturn = uVars[i].value ;

  return toReturn + "\n" ;

} // FindValue()

void SetValue( ShortString name, ShortString value ) {

  bool exist = false ;  // found to exist (in uVars[])
  int inWhere = -1 ;    // of uVars[]

  for ( int i = 0 ; i < uVarCount && NOT exist ; i++ )
    if ( uVars[i].name == string( name ) ) {
      exist = true ;
      inWhere = i ;
    } // if found

  if ( exist )
    uVars[inWhere].value = string( value ) ;

  else { // NOT yet exist in uVars[]

    uVars[ uVarCount ].name = string( name ) ;
    uVars[ uVarCount ].value = string( value ) ;
    uVarCount++ ;

  } // else NOT yet exist (in uVars[])

} // SetValue()

bool IsCmdAndProcessed( InputStrType inputStr, string & response ) {
// We only accept two kinds of "commands" :
//   abc       // print the value of 'abc' in this case
//   abc = 10  // set 'abc' to '10' and print nothing
// It is  assumed that there is no error in the input.

  bool isCmd = false ;  // assume

  ShortString name, equalSign, value, more ;

  int numOfMatch = sscanf( inputStr, "%s%s%s%s", name, equalSign, value, more ) ;

  if ( numOfMatch == 1 || (( numOfMatch == 3 ) && string( equalSign ) == "=" ) ) {

    isCmd = true ;

    if ( numOfMatch == 1 )
      response = FindValue( name ) ;

    else { // ( numOfMatch == 3 && equalSign == "=" )

      SetValue( name, value ) ;
      response = "" ;

    } // else "abc = 20"

  } // if is command

  return isCmd ;

} // IsCmdAndProcessed()

void ProcessUserInput ( bool & done, InputStrType inputStr, string & response ) {

  response = "" ; // default for 'done'

  if ( strcmp( "done", inputStr ) == 0 )
    done = true ;
  else if ( strcmp( "quit", inputStr ) == 0 )
    done = true ;
  else if ( strcmp( "exit", inputStr ) == 0 )
    done = true ;
  else if ( IsCmdAndProcessed( inputStr, response ) )
    ; // NOT done ; response set by IsCmdAndProcessed()
  else // NOT 'done'/'quit'/'exit' and NOT a command
    response = "echoing : " + string( inputStr ) + "\n" ;

} // ProcessUserInput()

void PrintSystemOutput( string response ) {

  printf( "%s", response.c_str() ) ;

} // PrintSystemOutput()
