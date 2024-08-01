Ethelo Engine
==========

Requirements
------------
 - Docker

Getting started
----------------

Get the dependencies, which are all pulled in as git submodules right now:

 - `git submodule init && git submodule update`

Build the docker image:

 - `docker-compose build`

Build the engine:

 - `docker-compose run engine ./ethelo.sh`

Tests
-----

Run the full test suite:

 - `docker-compose run engine make test`

Run an individual test file:
 - `docker-compose run engine ./bin/complex_decision_tests`

Decision runner
---------------

The build includes a *basic* stand-alone executable that can run on an input directory of JSON files:

 - `docker-compose run engine bin/runner /app/api/tests/fixtures/budget_decision_full_vote`

Debugging
---------

To build with debug symbols:
 - `docker-compose run engine ./ethelo.sh Debug`

The images include gdb, so. eg to run the integration tests through the debugger:

 - `docker-compose run engine gdb /app/build/bin/integration_tests`

Logging
-------

The simple logger is `plog` (https://github.com/SergiusTheBest/plog).  It's controlled with two environment variables -- already set to the following defaults in docker-compose:
 - `ENGINE_LOG_PATH: /app/log/engine.log`
 - `ENGINE_LOG_LEVEL: 5` (5 is `debug`, the most verbose level)

Note that given the volume mapping in docker-compose, you'll see this logfile in `./log/engine.log`.  It`ll get big if you don't clear it out regularly or tweak the above verbose settings!

Rebuilding the docker image
-----------------------

If you add, remove, or upgrade 3rd party dependencies, you'll need to rebuild the docker image - grab a coffee, this'll take a while!

 - `docker-compose build`

Then push the image to the docker repo so that no else has to go through this!

 - `docker-compose push`

Note: ethelo.sh files
---------------------

They're a minimalist shell-scripted build system. I couldn't use `make` as I normally would because half the stuff here already uses Makefiles and I didn't want to muck around with alternate Makefiles for such a simple scenario. Basically:
 - `ethelo.sh` in a particular directory builds that directory
 - E.g., `ethelo.sh` in the build directory builds the app
 - `3rdparty/ethelo.sh` builds all the third-party deps (in parallel, woo)
 - `3rdparty/<dep>/ethelo.sh` builds that dependency

TODO: remove these sh files!! ...we've forked all the dependency repos for the sole purpose of adding them, and it makes them hard to upgrade :(
