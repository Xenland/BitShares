bitshares - Polymorphic Digital Asset Library
=========

BitShares is a new blockchain that pays dividends and enables users
to short and trade any arbitrary asset type.


Project Status
------------
This code is only fit for developers and the api, design is evolving
rapidly.  If you would like to be involved please contact me via
github or bytemaster on bitcointalk.org   

Dependencies
-------------------
	g++ 4.6, VC2010 
	boost 1.54
	OpenSSL
	Qt 5.1 (for GUI only)
	cmake 2.8.11.2

Build
--------------------

	git clone https://github.com/InvictusInnovations/BitShares.git
	cd BitShares
	git clone https://github.com/InvictusInnovations/fc.git
	cmake .
	make 

Coding Standards
----------------
all lower case names with underscores with the exception of macros.
private implmentation classes in the detail namespace.
Make it look like the rest of the code.



