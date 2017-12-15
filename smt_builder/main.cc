#include "smt.h"
#include <iostream>

int main()
{
    SMT smt( 5, 6);

    smt.AddPin( 0, 0);
    smt.AddPin( 2, 0);
    smt.AddPin( 4, 0);
    smt.AddPin( 1, 2);
    smt.AddPin( 4, 4);
    smt.AddPin( 0, 4);

    std::cout << smt.BuildSMT() << std::endl << std::endl;

    for ( std::list<SMT::Point*>::iterator it = smt.existing_points.begin();
          it != smt.existing_points.end();
          ++it)
    {
        std::cout << ( *it)->GetPosX() << " " << ( *it)->GetPosY() << std::endl;
    }

    std::cout << std::endl;

    for ( std::list<SMT::Edge*>::iterator it = smt.edges.begin();
          it != smt.edges.end();
          ++it)
    {
        std::cout << ( *it)->GetPosX1() << " " << ( *it)->GetPosY1() << "   "
                  << ( *it)->GetPosX2() << " " << ( *it)->GetPosY2() << std::endl;
    }

    std::cout << std::endl;

    for ( std::list<SMT::Point*>::iterator it = smt.hanan_points.begin();
          it != smt.hanan_points.end();
          ++it)
    {
        std::cout << ( *it)->GetPosX() << " " << ( *it)->GetPosY() << std::endl;
    }

    return 0;
}
