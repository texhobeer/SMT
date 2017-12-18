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

SMT::Marker::Marker( const Marker& other)
{
    this->id = other.id;
    this->counter = 0;
}

SMT::Marker& SMT::Marker::operator=( const Marker& other)
{
    this->id = other.id;
    this->counter = 0;
    this->marked_points.clear();

    return *this;
}

void SMT::Marker::AddPoint( Point* point)
{
    Marker* old_marker = point->GetMarker();

    this->counter += old_marker->counter;
    old_marker->counter = 0;

    for ( auto it = old_marker->marked_points.begin();
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

SMT::Point::PointType SMT::Point::GetType()
{
    return this->type;
}

void SMT::Point::SetMarker( Marker* m)
{
    this->marker = m;
}

void SMT::Point::SetType( PointType t)
{
    this->type = t;
}

bool SMT::Point::IsPin()
{
    return this->type == Pin;
}

bool SMT::Point::IsInvalid()
{
    return this->type == Invalid;
}

bool SMT::Point::IsInBothLayers()
{
    bool is_in_m2 = false;
    bool is_in_m3 = false;

    for ( auto it = this->edges.begin();
          it != this->edges.end();
          ++it)
    {
        if ( ( *it)->IsInM2Layer() )
            is_in_m2 = true;

        if ( ( *it)->IsInM3Layer() )
            is_in_m3 = true;
    }

    return is_in_m2 && is_in_m3;
}

bool SMT::Point::IsInM3Layer()
{
    for ( auto it = this->edges.begin();
          it != this->edges.end();
          ++it)
    {
        if ( ( *it)->IsInM3Layer() )
            return true;
    }

    return false;
}

bool SMT::Point::IsPinsM2()
{
    return this->type == Pins_M2;
}

void SMT::Point::FinalizeType()
{
    if ( this->IsPin()
         || this->IsPinsM2() )
        return;

    if ( this->IsInBothLayers() )
        this->type = M2_M3;
    else
        this->type = Invalid;
}

void SMT::Point::Link( SMT::Edge* edge)
{
    this->edges.push_back( edge);
}

void SMT::Point::Unlink()
{
    this->edges.clear();
}

void SMT::Point::Unlink( SMT::Edge* edge)
{
    this->edges.remove_if( [ edge]( Edge* e) { return e == edge; });
}

SMT::Point::Point( unsigned x,
                   unsigned y,
                   PointType t)
{
    this->posX = x;
    this->posY = y;
    this->type = t;
    this->marker = nullptr;
}

SMT::Point::Point( const SMT::Point& other)
{
    this->posX = other.posX;
    this->posY = other.posY;
    this->type = other.type;
    this->marker = nullptr;
}

SMT::Point& SMT::Point::operator=( const SMT::Point& other)
{
    this->posX = other.posX;
    this->posY = other.posY;
    this->type = other.type;
    this->marker = nullptr;
    this->edges.clear();

    return *this;
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

bool SMT::Edge::IsInM2Layer()
{
    return this->GetPosY1() == this->GetPosY2();
}

bool SMT::Edge::IsInM3Layer()
{
    return this->GetPosX1() == this->GetPosX2();
}

bool SMT::Edge::IsInBothLayers()
{
    return !this->IsInM2Layer() && !this->IsInM3Layer();
}

void SMT::Edge::PseudoLink()
{
    if ( this->IsInOneSCC() )
        return;

    if ( this->point1->GetSCCCounter() < this->point2->GetSCCCounter() )
        std::swap( this->point1, this->point2);

    this->point1->GetMarker()->AddPoint( this->point2);
}

void SMT::Edge::RealLink()
{
    this->PseudoLink();
    this->point1->Link( this);
    this->point2->Link( this);
}

void SMT::Edge::FinalLink()
{
    this->point1->Link( this);
    this->point2->Link( this);
}

unsigned SMT::Edge::GetSCCCounter()
{
    if ( this->IsInOneSCC() )
        return this->point1->GetSCCCounter();

    return 0;
}

SMT::Point* SMT::Edge::GetPoint1()
{
    return this->point1;
}

SMT::Point* SMT::Edge::GetPoint2()
{
    return this->point2;
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

    for ( auto it = this->existing_points.begin();
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

void SMT::DeleteExistingEdges()
{
    this->existing_edges.clear();

    for ( auto it = this->existing_points.begin();
          it != existing_points.end();
          ++it)
    {
        ( *it)->Unlink();
    }
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
    Cell** cell = new Cell* [ this->grid_size];
    for ( unsigned i = 0; i < this->grid_size; ++i)
    {
        cell[ i] = new Cell [ this->grid_size];
    }

    this->ClearListOfPointers( this->hanan_points);

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

    for ( unsigned i = 0; i < this->grid_size; ++i)
    {
        delete [] cell[ i];
    }

    delete cell;
}

unsigned SMT::CalculateMST( bool to_finalize)
{
    unsigned length = 0;
    unsigned scc_counter = 1;

    if ( to_finalize )
        this->DeleteExistingEdges();

    this->ResetMarkers();

    for ( auto it = this->edges.begin();
          it != this->edges.end() && scc_counter != this->num_of_points;
          ++it)
    {
        if ( ( *it)->IsInOneSCC() )
            continue;

        length += ( *it)->GetLength();

        if ( to_finalize )
        {
            ( *it)->RealLink();
            this->existing_edges.push_back( *it);
        }
        else
        {
            ( *it)->PseudoLink();
        }

        unsigned curr_scc_counter = ( *it)->GetSCCCounter();
        if ( curr_scc_counter > scc_counter )
            scc_counter = curr_scc_counter;
    }

    if ( scc_counter != this->num_of_points )
        length = -1;
    else if ( to_finalize )
        this->current_MST_length = length;

    return length;
}

unsigned SMT::CalculateMST()
{
    return this->CalculateMST( false);
}

bool SMT::SMTIteration()
{
    unsigned new_length = -1;
    Point* winner = nullptr;
    std::list<Point*>::iterator it, winner_it;

    for ( it = this->hanan_points.begin();
          it != this->hanan_points.end();
          ++it)
    {
        unsigned length = -1;
        Point* hanan = *it;

        this->ResetMarkers();

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
    }

    if ( !winner )
        return false;

    this->AddPseudoPoint( winner->GetPosX(), winner->GetPosY());
    delete *winner_it;
    this->hanan_points.erase( winner_it);
    this->CalculateMST( true);

    return true;
}

void SMT::FinalizeSMT()
{
    this->finalized = true;
    unsigned i = 0;

    for ( auto it = this->existing_points.begin();
          i < this->num_of_points;
          ++it, ++i)
    {
        ( *it)->FinalizeType();

        if ( !( *it)->IsPin() )
            continue;

        Point* pins_m2 = new Point( **it);
        pins_m2->SetType( Point::Pins_M2);
        this->existing_points.push_back( pins_m2);

        if ( !( *it)->IsInM3Layer() )
            continue;

        Point* m2_m3 = new Point( **it);
        m2_m3->SetType( Point::M2_M3);
        this->existing_points.push_back( m2_m3);
    }

    for ( auto it = this->existing_edges.begin();
          it != this->existing_edges.end(); )
    {
        if ( !( *it)->IsInBothLayers() )
        {
            ++it;
            continue;
        }

        unsigned x = ( *it)->GetPosX1();
        unsigned y = ( *it)->GetPosY2();

        Point* m2_m3 = new Point( x, y, Point::M2_M3);
        Point* p1 = ( *it)->GetPoint1();
        Point* p2 = ( *it)->GetPoint2();
        Edge* e1 = new Edge( p1, m2_m3, Edge::Valid);
        Edge* e2 = new Edge( p2, m2_m3, Edge::Valid);

        p1->Unlink( *it);
        p2->Unlink( *it);
        e1->FinalLink();
        e2->FinalLink();

        this->existing_points.push_back( m2_m3);

        this->extra_edges.push_back( e1);
        this->extra_edges.push_back( e2);

        it = this->existing_edges.erase( it);
    }
}

unsigned SMT::BuildSMT()
{
    if ( this->finalized )
        return this->current_MST_length;

    this->CollectHananPoints();
    this->CalculateMST( true);

    while( this->SMTIteration());

    this->FinalizeSMT();

    return this->current_MST_length;
}

std::list<SMT::Point> SMT::GetPointsList()
{
    return this->MakeSafeCopyForListOfPointers( this->existing_points);
}

std::list<SMT::Edge> SMT::GetEdgesList()
{
    auto res = this->MakeSafeCopyForListOfPointers( this->existing_edges);
    res.splice( res.begin(), this->MakeSafeCopyForListOfPointers( this->extra_edges));
    return res;
}

void SMT::Destroy()
{
    this->ClearListOfPointers( this->existing_points);
    this->ClearListOfPointers( this->hanan_points);
    this->ClearListOfPointers( this->edges);
    this->ClearListOfPointers( this->markers);
    this->ClearListOfPointers( this->extra_edges);
}

template<typename T> std::list<T*> SMT::DuplicateListOfPointers( const std::list<T*>& to_copy)
{
    std::list<T*> dest;

    for ( auto it = to_copy.begin();
          it != to_copy.end();
          ++it)
    {
        dest.push_back( new T( **it));
    }

    return dest;
}

template<typename T> std::list<T> SMT::MakeSafeCopyForListOfPointers( const std::list<T*>& to_copy)
{
    std::list<T> dest;

    for ( auto it = to_copy.begin();
          it != to_copy.end();
          ++it)
    {
        dest.push_back( T( **it));
    }

    return dest;
}

template<typename T> void SMT::ClearListOfPointers( std::list<T*>& to_clear)
{
    for ( auto it = to_clear.begin();
          it != to_clear.end();
          ++it)
    {
        delete *it;
    }

    to_clear.clear();
}

void SMT::PerformCopy( const SMT& other)
{
    this->grid_size = other.grid_size;
    this->pin_count = other.pin_count;
    this->num_of_points = 0;
    this->current_MST_length = -1;

    this->finalized = false;
    unsigned i = 0;

    for ( auto it = other.existing_points.begin();
          i < other.num_of_points;
          ++it, ++i)
    {
        this->AddExistingPoint( ( *it)->GetPosX(), ( *it)->GetPosY(), ( *it)->GetType(), Edge::Valid);
    }

    this->hanan_points = this->DuplicateListOfPointers( other.hanan_points);

    this->CalculateMST( true);
}

SMT::SMT( unsigned N,
          unsigned M)
{
    this->grid_size = N;
    this->pin_count = M;
    this->num_of_points = 0;
    this->current_MST_length = -1;

    this->finalized = false;
}

SMT::~SMT()
{
    this->Destroy();
}

SMT::SMT( const SMT& other)
{
    this->PerformCopy( other);
}

SMT& SMT::operator=( const SMT& other)
{
    this->Destroy();

    this->PerformCopy( other);

    return *this;
}

SMT& SMT::operator=( SMT&& other_tmp)
{
    this->Destroy();

    *this = other_tmp;

    return *this;
}
