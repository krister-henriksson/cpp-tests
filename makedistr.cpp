
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <limits>

#include <boost/algorithm/string.hpp>

#define DEBUG 0
#define EASY_SUBSTR 0


using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::istringstream;
using std::ifstream;
using std::setprecision;
using std::scientific;


/* Function declarations */
int get_substrings( const string & line,
		    const string & delims,
		    vector<string> & substrings		    
		    );

/* ###############################################################################
   MAIN
   ############################################################################### */
int main(int argc, char **argv){

  vector<string> filenames;
  int Nfiles, colRead;
  size_t Nbins, col;
  double x1, x2, binw;
  bool normYES, x1_used, x2_used, use_binw;

  x1_used = x2_used = false;
  x1 = x2 = 0.0;
  normYES = true;
  colRead = 0;
  Nbins = 100;
  use_binw = false;
  Nfiles = 0;


  /* Help on usage. */
  if (argc < 2){
    cout << "Purpose: Make a histogram from input data in text files." << endl;
    cout << "Usage:" << endl;
    cout << " " << string(argv[0]) << " [options]" << endl;
    cout << "Options:" << endl;
    cout << "     -x1        Consider numerical values >=x1, e.g. -1.0. Default: not used, all values considered" << endl;
    cout << "     -x2        Consider numerical values <=x2, e.g. 12.1. Default: not used, all values considered" << endl;
    cout << "     -nn        Do not normalize." << endl;
    cout << "     -c col     Column containing data. First column is denoted 0, etc. Default: " << colRead << endl;
    cout << "     -N Nbins   Number of bins. Default: " << Nbins << endl;
    cout << "     -d binw    Bin width. Not needed if number of bins has been specified." << endl;
    cout << "     -f files   Data files. Use rest of line." << endl;
    cout << ""  << endl;
    cout << ""  << endl;
    return 0;
  }

  /* Parse options. */
  for (int i=1; i<argc; ++i){
    string flag = string(argv[i]);
    istringstream isstream;

    if (flag == "-x1"){
      isstream.str(string(argv[i+1]));
      isstream >> x1;
      isstream.clear();
      i++; x1_used=true;
    } else if (flag == "-x2"){
      isstream.str(string(argv[i+1]));
      isstream >> x2;
      isstream.clear();
      i++; x2_used=true;
    } else if (flag == "-nn")
      normYES = 0;
    else if (flag == "-c"){
      isstream.str(argv[i+1]);
      isstream >> colRead;
      isstream.clear();
      i++;
    } else if (flag == "-N"){
      isstream.str(argv[i+1]);
      isstream >> Nbins;
      isstream.clear();
      i++;
    } else if (flag == "-d"){
      isstream.str(argv[i+1]);
      isstream >> binw;
      isstream.clear();
      i++; use_binw = true;
    } else if (flag == "-f"){
      Nfiles = argc-i-1;
      filenames.resize(0);
      string inputf;
      for (int j=0; j<Nfiles; j++){
	isstream.str(argv[i+1 + j]);
	isstream >> inputf;
	filenames.push_back(inputf);
	isstream.clear();
      }
    }
  }


  /* ###############################################################################
     Debugging of options.
     ############################################################################### */
  if (x1_used && x2_used && x1 >= x2){
    cerr << "Error: Specified x1 = " << x1 << " is >= x2 = " << x2 << ". Exiting." << endl;
    exit(1);
  }
  if (use_binw && binw < 0){
    cerr << "Error: Bin width " << binw << " is negative. Exiting." << endl;
    exit(1);
  }
  if (colRead<0){
    cerr << "Error: Column to use have negative index. Exiting." << endl;
    exit(1);
  }
  col = colRead;

  if (Nfiles==0){
    cerr << "Error: No files to process. Exiting." << endl;
    exit(1);
  }



  size_t Nx = 10;
  vector<double> x(Nx);

  /* Read data. */
  size_t linecount = 0;
  for (int j=0; j<Nfiles; j++) {

    ifstream fp;
    fp.open(filenames[j].c_str());
    if (! fp){
      cerr << "Error opening file " << filenames[j] << ". Exiting." << endl;
    }

    string line;
    istringstream isstream;
    vector<string> substrings;

    /* Read a line */
    while (getline(fp, line)){

      /* Ignore comment lines. */
      if ((line[0]=='#') || (line[0]=='!')) continue;

      /* Ignore empty lines. */
      if ( get_substrings(line, " \t", substrings) == 0) continue;

      linecount++;

      Nx = x.size();
      if (linecount > Nx){
	Nx *= 1.5;
	x.resize(Nx);
      }

      isstream.str(substrings[col]);
      isstream >> x[linecount-1];
      isstream.clear();

#if DEBUG
      cout << "substrings[" << col << "] : " << substrings[col] << endl;
      cout << "Read x[" << linecount-1 << "]: " << x[linecount-1] << endl;
#endif
    }
    fp.close();
  }
  if (linecount==0){
    cout << "There are no data lines to process. Exiting." << endl;
    return 1;
  }
  Nx = linecount;
  x.resize(Nx);

  /* Get min and max data points. */
  double xmin = x[0], xmax = x[0];
  for (size_t i=0; i<Nx; ++i) {
    if (x[i] < xmin) xmin = x[i];
    if (x[i] > xmax) xmax = x[i];
  }
  if (x1_used) xmin = x1;
  if (x2_used) xmax = x2;

#if DEBUG
  cout << "Minimum and maximum x values are " << xmin << " " << xmax << endl;
#endif

  if (use_binw){
    double tmpd = floor((xmax - xmin)/binw);
    while (tmpd * binw > xmax) tmpd--;

    if (tmpd < 0){
      cerr << "Number of bins is negative "
	   << "for xmax = " << xmax << " and xmin = " << xmin << " and bin width " << binw
	   << ". Debug program. Exiting." << endl;
      exit(1);
    }
    if (tmpd >= std::numeric_limits<size_t>::max()){
      cerr << "Number of bins exceed the upper numerical limit "
	   << "for xmax = " << xmax << " and xmin = " << xmin << " and bin width " << binw
	   << ". Debug program. Exiting." << endl;
      exit(1);
    }

    Nbins = tmpd;
  }
  else {
    if (Nbins==0) Nbins=1;
    binw = (xmax-xmin) / Nbins;
  }

  vector<double> ft(Nbins);
  vector<double> err_ft(Nbins);
  for (size_t i=0; i<Nbins; i++) ft[i] = 0.0;

  /* Put into bins. */
  for (size_t i=0; i<Nx; i++){
    if (x[i]<xmin || x[i]>xmax) continue;

    double tmpd = floor((x[i] - xmin)/binw);
    
    if (tmpd < 0 && ++tmpd < 0){
      cerr << "Bin index " << tmpd << " is negative for x[" << i << "] = " << x[i]
	   << "for xmax = " << xmax << " and xmin = " << xmin << " and bin width " << binw
	   << ". Debug program. Exiting." << endl;
      exit(1);
    }
    if (tmpd >= std::numeric_limits<size_t>::max()){
      cerr << "Number of bins exceed the upper numerical limit "
	   << "for xmax = " << xmax << " and xmin = " << xmin << " and bin width " << binw
	   << ". Debug program. Exiting." << endl;
      exit(1);
    }
    if (tmpd >= Nbins && --tmpd >= Nbins){
      cerr << "Bin index " << tmpd << " is too large for x[" << i << "] = " << x[i]
	   << "for xmax = " << xmax << " and xmin = " << xmin << " and bin width " << binw
	   << ". Debug program. Exiting." << endl;
      exit(1);
    }

    size_t idx = tmpd;
    ft[idx] += 1.0;
  }

  /* Estimate the uncertainty. Use counting statistics error. */
  for (size_t i=0; i<Nbins; i++){
    err_ft[i] = sqrt(ft[i]);
  }

  /* Normalization. */
  if (normYES && Nbins>1){
    double norm = 0.0;
    for (size_t i=0; i<Nbins; i++) norm += ft[i];
    if (norm < 0) norm *= -1.0;
    if (norm < std::numeric_limits<double>::epsilon()){
      norm=1.0;
      cerr << "Warning: Normalization factor smaller than machine accuracy. Not using normalization." << endl;
    }
    double inorm=1.0/norm;
    for (size_t i=0; i<Nbins; i++){
      ft[i]     *= inorm;
      err_ft[i] *= inorm;
    }
  }

  /* Write out distribution */
  for (size_t i=0; i<Nbins; i++){
    double t = xmin + (i + 0.5) * binw;

    cout << setprecision(10) << scientific
	 << t << "  " << ft[i] << "  " << err_ft[i] << endl;
  }

  return 0;
}




/* #############################################################################

   Function: Split a string into substrings based on given delimiters.

   Arguments:
   line           Input string, assumed initialized.
   delims         String containing delimiters.
   substrings     Vector containing read substrings.

   Return value: Number of substrings found.

   ########################################################################## */


int get_substrings( const string & line,
		    const string & delims,
		    vector<string> & substrings		    
		    ){

  substrings.resize(0);
  string buf(line);

#if EASY_SUBSTR
  // #############################################
  // The easy version, using Boost:
  // #############################################


  // Remove leading and trailing whitespace, i.e. use boost::trim:
  boost::trim_if(buf, boost::is_any_of("\t "));

  // Don't bother with boost::split if trimmed input is empty:
  if (buf.size()==0) return 0;
  
  substrings.clear();
  substrings.resize(0);
  boost::split(substrings, buf, boost::is_any_of(delims), boost::token_compress_on);

#else

  char ch;
  bool isDelim, readingWord = false;
  string word = "";

  // Go through all characters in input string. Also read the terminating '\0' character.
  for (size_t idx=0; idx<=buf.length(); ++idx){

    ch = buf[idx];

    isDelim=false;
    for (size_t i=0; i<delims.length(); ++i){
      if (ch==delims[i]){
	isDelim=true;
	break;
      }
    }
    if (ch=='\0') isDelim=true;

    if (readingWord && isDelim){
      substrings.push_back(word);
      word = "";
      readingWord=false;
    } else if ( readingWord && !isDelim){
      word += ch;
    } else if (!readingWord && !isDelim){
      word = "";
      word += ch;
      readingWord = true;
    }

  }
#endif

#if DEBUG
  cout << "Read substrings: ";
  for (size_t i=0; i<substrings.size(); ++i)
    cout << " " << substrings[i];
  cout << endl;
#endif

  return substrings.size();
}

