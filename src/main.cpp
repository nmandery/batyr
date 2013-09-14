#include "server.h"


using namespace Geopoll;


int
main(int argc, char** argv)
{
    Server server;
    return server.run(argc, argv);
}
