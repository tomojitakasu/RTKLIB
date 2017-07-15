# RTKLIB 2.4.3 extended version 

This extended version of RTKLIB is a merge of 
https://github.com/HoughtonAssoc/rtklib-mops and RTKLIB 2.4.3 to have 
protection level (PL) in the newest version of RTKLIB.
Further enhacements to Houghton version:

* Linux make files instead of cygwin
* Full integration of PL into rnx2rtkp

## Usage

Add -ws switch to the rnx2rtkp command line to turn on PL calculation.
Horizontal (hpl) and vertical (vpl) protection levels can be seen in the
last two columns of the output list 
