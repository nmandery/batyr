#include "server/server.h"


using namespace Batyr;


int
main(int argc, char** argv)
{
    Server server;
    return server.run(argc, argv);
}
