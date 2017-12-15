#include "smt.h"
#include <cstddef>

/**
 * ------ SMT::Marker ------
 */

SMT::Marker::Marker( unsigned id)
{
    this->id = id;
    this->counter = 0;
}

void SMT::Marker::AddPoint( Point* point)
{
    Marker* old_marker = point->GetMarker();

    this->counter += old_marker->counter;
    old_marker->counter = 0;

    for ( std::list<Point*>::iterator it = old_marker->marked_points.begin();
          it != old_marker->marked_points.end();
          ++it)
    {
        this->marked_points.push_back( *it);
        ( *it)->SetMarker( this);
    }

    old_marker->marked_points.clear();
}

void SMT::Marker::InitByPoint( Point* point)
{
    this->counter = 1;
    this->marked_points.clear();
    this->marked_points.push_back( point);
    point->SetMarker( this);
}

unsigned SMT::Marker::GetId()
{
    return this->id;
}

unsigned SMT::Marker::GetCounter()
{
    return this->counter;
}

/**
 * ------ SMT::Point ------
 */


unsigned SMT::Point::GetPosX()
{
    return this->posX;
}

unsigned SMT::Point::GetPosY()
{
    return this->posY;
}

SMT::Marker* SMT::Point::GetMarker()
{
    return this->marker;
}

unsigned SMT::Point::GetSCCId()
{
    return this->marker->GetId();
}

unsigned SMT::Point::GetSCCCounter()
{
    return this->marker->GetCounter();
}

void SMT::Point::SetMarker( Marker* m)
{
    this->marker = m;
}

bool SMT::Point::IsPin()
{
    return this->type == Pin;
}

SMT::Point::Point( unsigned x,
                   unsigned y,
                   PointType t)
{
    this->posX = x;
    this->posY = y;
    this->type = t;
    this->marker = NULL;
}


/**
 * ------ SMT::Edge ------
 */


unsigned SMT::Edge::GetLength()
{
    return this->length;
}

unsigned SMT::Edge::GetPosX1()
{
    return this->point1->GetPosX();
}

unsigned SMT::Edge::GetPosY1()
{
    return this->point1->GetPosY();
}

unsigned SMT::Edge::GetPosX2()
{
    return this->point2->GetPosX();
}

unsigned SMT::Edge::GetPosY2()
{
    return this->point2->GetPosY();
}

bool SMT::Edge::IsTemporary()
{
    return this->status == Temporary;
}

bool SMT::Edge::IsInOneSCC()
{
    return this->point1->GetSCCId() == this->point2->GetSCCId();
}

void SMT::Edge::Link()
{
    if ( this->point1->GetSCCCounter() < this->point2->GetSCCCounter() )
        std::swap( this->point1, this->point2);

    this->point1->GetMarker()->AddPoint( this->point2);
}

unsigned SMT::Edge::GetSCCCounter()
{
    if ( this->IsInOneSCC() )
        return this->point1->GetSCCCounter();

    return 0;
}

SMT::Edge::Edge( Point* p1,
                 Point* p2,
                 Status s)
{
    this->point1 = p1;
    this->point2 = p2;
    this->status = s;

    unsigned x1 = p1->GetPosX(),
             y1 = p1->GetPosY(),
             x2 = p2->GetPosX(),
             y2 = p2->GetPosY();

    if ( x1 > x2 )
        std::swap( x1, x2);

    if ( y1 > y2 )
        std::swap( y1, y2);

    this->length = ( x2 - x1 ) + ( y2 - y1 );
}


/**
 * ------ SMT::Cell ------
 */


bool SMT::Cell::IsHanan()
{
    return this->is_hanan_x && this->is_hanan_y && !this->is_pin;
}

void SMT::Cell::SetHananX( bool is_hanan_x)
{
    this->is_hanan_x = is_hanan_x;
}

void SMT::Cell::SetHananY( bool is_hanan_y)
{
    this->is_hanan_y = is_hanan_y;
}

void SMT::Cell::SetPin( bool is_pin)
{
    this->is_pin = is_pin;
}

SMT::Cell::Cell()
{
    this->is_hanan_x = false;
    this->is_hanan_y = false;
    this->is_pin = false;
}


/**
 * ------ SMT ------
 */


void SMT::AddHananPoint( unsigned x, unsigned y)
{
    Point* point = new Point( x, y, Point::Hanan);
    this->hanan_points.push_back( point);
}

void SMT::AddExistingPoint( unsigned x, unsigned y, Point::PointType t, Edge::Status s)
{
    Point* point = new Point( x, y, t);

    for ( std::list<Point*>::iterator it = this->existing_points.begin();
          it != this->existing_points.end();
          ++it)
    {
        this->AddEdge( point, *it, s);
    }

    Marker* marker = new Marker( this->num_of_points);
    marker->InitByPoint( point);
    this->markers.push_back( marker);

    this->existing_points.push_back( point);
    this->num_of_points++;
}

void SMT::AddPseudoPoint( unsigned x, unsigned y)
{
    this->AddExistingPoint( x, y, Point::Pseudo, Edge::Valid);
}

void SMT::AddTemporaryPoint( unsigned x, unsigned y)
{
    this->AddExistingPoint( x, y, Point::Pseudo, Edge::Temporary);
}

void SMT::AddPin( unsigned x, unsigned y)
{
    this->AddExistingPoint( x, y, Point::Pin, Edge::Valid);
}

void SMT::AddEdge( Point* p1, Point* p2, Edge::Status s)
{
    Edge* edge = new Edge( p1, p2, s);
    unsigned length = edge->GetLength();
    std::list<Edge*>::iterator it;

    for ( it = this->edges.begin();
          it != this->edges.end() && length > ( *it)->GetLength();
          ++it);

    this->edges.insert( it, edge);
}

void SMT::AddEdge( Point* p1, Point* p2)
{
    this->AddEdge( p1, p2, Edge::Valid);
}

void SMT::DeleteTemporaryEdges()
{
    std::list<Edge*>::iterator it, next;

    for ( std::list<Edge*>::iterator it = this->edges.begin();
          it != this->edges.end();
          it = next)
    {
        next = it;
        next++;

        if ( ( *it)->IsTemporary() )
        {
            delete *it;
            this->edges.erase( it);
        }
    }
}

void SMT::DeleteTemporaryPoint()
{
    /** Temporary point is pushed back, so we can just pop it */
    delete this->existing_points.back();
    this->existing_points.pop_back();

    /** We also have to delete the last added marker */
    delete this->markers.back();
    this->markers.pop_back();

    this->num_of_points--;
}

void SMT::ResetMarkers()
{
    std::list<Point*>::iterator it_point;
    std::list<Marker*>::iterator it_marker;

    for ( it_point = this->existing_points.begin(),
          it_marker = this->markers.begin();
          it_point != this->existing_points.end();
          ++it_point, ++it_marker)
    {
        ( *it_marker)->InitByPoint( *it_point);
    }
}

unsigned SMT::GetGridSize()
{
    return this->grid_size;
}

unsigned SMT::GetPinCount()
{
    return this->pin_count;
}

void SMT::CollectHananPoints()
{
    Cell cell[this->grid_size][this->grid_size];

    this->hanan_points.clear();

    for ( std::list<Point*>::iterator it = this->existing_points.begin();
          it != this->existing_points.end();
          ++it)
    {
        if ( !( *it)->IsPin() )
            continue;

        unsigned pinX = ( *it)->GetPosX();
        unsigned pinY = ( *it)->GetPosY();

        cell[ pinX][ pinY].SetPin( true);

        /** grid is a square, so we can pass the row and a column at the same time */
        for ( unsigned pos = 0;
              pos < this->grid_size;
              ++pos)
        {
            cell[ pos][ pinY].SetHananY( true);
            cell[ pinX][ pos].SetHananX( true);
        }
    }

    for ( unsigned x = 0;
          x < this->grid_size;
          ++x)
    {
        for ( unsigned y = 0;
              y < this->grid_size;
              ++y)
        {
            if ( cell[ x][ y].IsHanan() )
                this->AddHananPoint( x, y);
        }
    }
}

unsigned SMT::CalculateMST()
{
    unsigned length = 0;
    unsigned scc_counter = 1;

    for ( std::list<Edge*>::iterator it = this->edges.begin();
          it != this->edges.end() && scc_counter != this->num_of_points;
          ++it)
    {
        if ( ( *it)->IsInOneSCC() )
            continue;

        length += ( *it)->GetLength();
        ( *it)->Link();

        unsigned curr_scc_counter = ( *it)->GetSCCCounter();
        if ( curr_scc_counter > scc_counter )
            scc_counter = curr_scc_counter;
    }

    if ( scc_counter != this->num_of_points )
        length = -1;

    this->ResetMarkers();

    return length;
}

bool SMT::SMTIteration()
{
    unsigned new_length = -1;
    Point* winner = NULL;
    std::list<Point*>::iterator it, winner_it;

    for ( it = this->hanan_points.begin();
          it != this->hanan_points.end();
          ++it)
    {
        unsigned length = -1;
        Point* hanan = *it;

        this->AddTemporaryPoint( hanan->GetPosX(), hanan->GetPosY());
        length = this->CalculateMST();

        if ( length < this->current_MST_length
             && length < new_length )
        {
            winner = hanan;
            new_length = length;
            winner_it = it;
        }

        this->DeleteTemporaryEdges();
        this->DeleteTemporaryPoint();
        this->ResetMarkers();
    }

    if ( !winner )
        return false;

    this->AddPseudoPoint( winner->GetPosX(), winner->GetPosY());
    delete *winner_it;
    this->hanan_points.erase( winner_it);
    this->current_MST_length = new_length;

    return true;
}

unsigned SMT::BuildSMT()
{
    this->CollectHananPoints();
    this->current_MST_length = this->CalculateMST();

    while( this->SMTIteration());

    /** TODO: vias and pin corrections */
    /** TODO: fill real edges */

    return this->current_MST_length;
}

SMT::SMT( unsigned N,
          unsigned M)
{
    this->grid_size = N;
    this->pin_count = M;
    this->num_of_points = 0;
    this->current_MST_length = -1;
}

SMT::~SMT()
{
    for ( std::list<Point*>::iterator it = this->existing_points.begin();
          it != this->existing_points.end();
          ++it)
    {
        delete *it;
    }

    for ( std::list<Point*>::iterator it = this->hanan_points.begin();
          it != this->hanan_points.end();
          ++it)
    {
        delete *it;
    }

    for ( std::list<Edge*>::iterator it = this->edges.begin();
          it != this->edges.end();
          ++it)
    {
        delete *it;
    }

    for ( std::list<Marker*>::iterator it = this->markers.begin();
          it != this->markers.end();
          ++it)
    {
        delete *it;
    }
}
