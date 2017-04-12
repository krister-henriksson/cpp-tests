

#include <iostream>
#include <cstring>

using namespace std;

/* Toy code for playing with char arrays. */

int main(int argc, char *argv[]) {

  if (argc <= 1){
    cout << "" << endl;
    cout << "  Print out an input string in reverse order." << endl;
    cout << "  Usage: " << argv[0] << " somestring" << endl;
    cout << "" << endl;
    return 0;
  }

  char *text = new char[strlen(argv[1])+1];
  strcpy(text, argv[1]);
  cout << "Input string is                 : " << text << endl;

  char *p = text;
  int len = 0;
  while (*p++ != '\0') len++;
  cout << "Length of input string is       : " << len << endl;

  p = text;
  char *q = text+len-1;
  char c;
  for (int i=0; i<len/2; i++){
    c  = *p;
    *p = *q;
    *q = c;
    p++;
    q--;
  }
  cout << "Input string in reverse order is: " << text << endl;

  delete[] text;
  return 0;	
}

