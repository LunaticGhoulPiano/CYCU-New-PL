# include <cstdio>
# include <iostream>
# include <cstring>
using namespace std ;

// For a line that is 2000 char long, we need 2001 char-spaces.
# define SIZE 2001
# define NOT !

int main( ) {

  char ch ;
  char * str = new char[SIZE] ;
  bool done = false ;

  printf( "Enter any string, followed by an ENTER. Enter 'done' when you are done.\n" ) ;

  while ( NOT done ) {

    printf( "> " ) ;

    //   scanf( "%s", str ) ; // this does not get the whole line
    cin.getline( str, SIZE ) ;
    //   cin >> ch ;     // No need to read in the ENTER char when we use getline()

    if ( NOT done )
      if ( strcmp( "done", str ) == 0 )
        done = true ;
      else if ( strcmp( "quit", str ) == 0 )
        done = true ;
      else if ( strcmp( "exit", str ) == 0 )
        done = true ;
      else // not done or quit or exit
        printf( "echoing : %s\n", str ) ;

    else // done
      ;  // nothing needs to be done ; just exit the system-user interaction loop

  } // while NOT done

  printf( "Bye! You have a good day!\n" ) ;

} // main()
