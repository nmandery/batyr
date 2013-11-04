Development
===========

Creating a debug build
----------------------

    cmake -DCMAKE_BUILD_TYPE=Debug .

For a DEBUG build also the *.dbg packages of the POCO libraries are required :
 
    sudo apt-get install libpocofoundation9-dbg libpoconet9-dbg libpocoutil9-dbg


Verbose messages when running make
----------------------------------

    make VERBOSE=1


Creating a debian package
-------------------------

    make -f Makefile.devel deb


Create changelog with changes since the last commit
---------------------------------------------------

    make -f Makefile.devel changelog


ToDo
====

Also see "TODO" comments in the code.

High priority
-------------

* Interval-based pulling. Example: Pull layer xyz every n seconds
  This also requires a more intelligent queuing to estimate how long it would take
  until all queued jobs are finished to ensure the interval is satisfied.

Medium priority
---------------

* Implement Access Control Headers for HTTP api
  see https://developer.mozilla.org/en-US/docs/HTTP/Access_control_CORS?

Low Priority
------------

* Once POCO 1.5 is in debian stable rapidjson might be dropped in favor of
  the JSON support in POCO.


Ideas
-----

* inotify-listener to read files from the local filesystem one they have changed

