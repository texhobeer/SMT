#include "smt.h"
#include "rapidxml/rapidxml.hpp"
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <vector>

enum RetVal
{
    Success = 0,
    WrongArgNum,
    BadBench
};

int main( int argc, char** argv)
{
    if ( argc != 3 )
        return WrongArgNum;

    std::ifstream input( argv[ 1]);

    std::vector<char> buffer( ( std::istreambuf_iterator<char>( input)),
                                std::istreambuf_iterator<char>());
    buffer.push_back( '\0');
    input.close();

    rapidxml::xml_document<> bench;
    rapidxml::xml_node<>* net;

    bench.parse<0>( &buffer[ 0]);
    buffer.clear();

    net = bench.first_node();

    if ( strcmp( net->name(), "net") )
        return BadBench;

    unsigned grid_size = atoi( net->first_attribute( "grid_size")->value());
    unsigned pin_count = atoi( net->first_attribute( "pin_count")->value());

    SMT smt( grid_size, pin_count);

    for ( rapidxml::xml_node<>* point = net->first_node();
          point;
          point = point->next_sibling())
    {
        if ( strcmp( point->name(), "point") )
            return BadBench;

        const char* type = point->first_attribute( "type")->value();
        if ( strcmp( type, "pin") )
            return BadBench;

        const char* x = point->first_attribute( "x")->value();
        if ( !x )
            return BadBench;

        const char* y = point->first_attribute( "y")->value();
        if ( !y )
            return BadBench;

        smt.AddPin( atoi( x), atoi( y));
    }

    smt.BuildSMT();

    std::list<SMT::Point> sol_points = smt.GetPointsList();
    std::list<SMT::Edge> sol_edges = smt.GetEdgesList();

    std::ofstream output( argv[ 2]);

    output << "<net grid_size=\"" << grid_size << "\" pin_count=\"" << pin_count << "\">" << std::endl;

    for ( auto it = sol_points.begin();
          it != sol_points.end();
          ++it)
    {
        unsigned x = ( *it).GetPosX();
        unsigned y = ( *it).GetPosY();

        const char* layer;
        const char* type;

        switch ( ( *it).GetType() )
        {
            case SMT::Point::Pin:
                layer = "\"pins\"";
                type = "\"pin\"";
                break;
            case SMT::Point::Pins_M2:
                layer = "\"pins_m2\"";
                type = "\"via\"";
                break;
            case SMT::Point::M2_M3:
                layer = "\"m2_m3\"";
                type = "\"via\"";
                break;
            default:
                layer = "\"undef\"";
                type = "\"undef\"";
                break;
        }

        output << "    <point x=\"" << x << "\" y=\"" << y << "\" layer=" << layer << " type=" << type << " />\n";
    }

    for ( auto it = sol_edges.begin();
          it != sol_edges.end();
          ++it)
    {
        unsigned x1 = ( *it).GetPosX1();
        unsigned y1 = ( *it).GetPosY1();
        unsigned x2 = ( *it).GetPosX2();
        unsigned y2 = ( *it).GetPosY2();

        const char* layer = ( *it).IsInBothLayers() ? "\"undef\"" :
                            ( *it).IsInM2Layer() ? "\"m2\"" :
                            ( *it).IsInM3Layer() ? "\"m3\"" : "\"undef\"";

        output << "    <segment x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" layer=" << layer << " />\n";
    }

    output << "</net>";

    return Success;
}
