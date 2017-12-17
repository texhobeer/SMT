#ifndef SMT__SMT_H
#define SMT__SMT_H

#include <list>
#include <iostream>

/**
 *  Description for Steiner Minimal Tree
 */
class SMT
{

private:

    class Marker;

public:

    class Edge;

    /**
     *  Description for possible points
     *
     *  Possible points are pins and vias, there are also
     *  temporary pseudo points for SMT building
     */
    class Point
    {

    public:

        /**
         *  Possible points types
         */
        enum PointType
        {
            /** pins */
            Pin,

            /** Hanan points */
            Hanan,

            /** pseudo - new points, some of them are to become vias */
            Pseudo,

            /** Next types are used after SMT was built */
            /** types for vias */
            M2_M3,
            Pins_M2,

            /** Error */
            Invalid
        };

    private:

        unsigned posX;
        unsigned posY;
        PointType type;
        Marker* marker;
        std::list<Edge*> edges;

    public:

        Point( unsigned x, unsigned y, PointType t);
        ~Point() = default;
        Point( const Point& other);
        Point( Point&& other_tmp) = default;
        Point& operator=( const Point& other);
        Point& operator=( Point&& other_tmp) = default;

        unsigned GetPosX();
        unsigned GetPosY();
        Marker* GetMarker();
        unsigned GetSCCId();
        unsigned GetSCCCounter();
        PointType GetType();

        void SetMarker( Marker* m);
        void SetType( PointType t);

        bool IsPin();
        bool IsInvalid();
        bool IsInBothLayers();
        bool IsInM3Layer();
        bool IsPinsM2();

        void FinalizeType();
        void Link( Edge* edge);
        void Unlink( Edge* edge);
        void Unlink();
    };

    /**
     *  Description for edges
     *
     *  Edge don't have to know how many segments it has
     *  because we can handle it at the last step
     */
    class Edge
    {

    public:

        enum Status
        {
            Valid,
            Temporary,
        };

    private:

        Point* point1;
        Point* point2;
        Status status;
        unsigned length;

    public:

        Edge( Point* p1, Point* p2, Status s);

        unsigned GetLength();
        unsigned GetPosX1();
        unsigned GetPosY1();
        unsigned GetPosX2();
        unsigned GetPosY2();
        unsigned GetSCCCounter();

        Point* GetPoint1();
        Point* GetPoint2();

        bool IsTemporary();
        bool IsInOneSCC();
        bool IsInM2Layer();
        bool IsInM3Layer();
        bool IsInBothLayers();

        void PseudoLink();
        void RealLink();
        void FinalLink();
    };

private:

    /**
     *  Description for marker
     */
    class Marker
    {

    private:

        unsigned id;
        unsigned counter;
        std::list<Point*> marked_points;

    public:

        Marker( unsigned id);
        ~Marker() = default;
        Marker( const Marker& other);
        Marker( Marker&& other_tmp) = default;
        Marker& operator=( const Marker& other);
        Marker& operator=( Marker&& other_tmp) = default;

        void AddPoint( Point* point);
        void InitByPoint( Point* point);

        unsigned GetId();
        unsigned GetCounter();
    };

    /**
     *  Description for cells
     *
     *  Cell is used to determine hanan points and for
     *  solution checker TODO
     */
    class Cell
    {

    private:

        bool is_hanan_x;
        bool is_hanan_y;
        bool is_pin;

    public:

        Cell();

        bool IsHanan();

        void SetHananX( bool is_hanan_x);
        void SetHananY( bool is_hanan_y);
        void SetPin( bool is_pin);
    };

    /** SMT Description */

    std::list<Point*> existing_points;
    std::list<Point*> hanan_points;
    std::list<Edge*> edges;
    std::list<Edge*> existing_edges;
    std::list<Marker*> markers;
    std::list<Edge*> extra_edges;

    unsigned grid_size;
    unsigned pin_count; // possible redundant
    unsigned num_of_points;
    unsigned current_MST_length;

    bool finalized;

    void AddExistingPoint( unsigned x, unsigned y, Point::PointType t, Edge::Status s);
    void AddPseudoPoint( unsigned x, unsigned y);
    void AddTemporaryPoint( unsigned x, unsigned y);
    void AddHananPoint( unsigned x, unsigned y);
    void AddEdge( Point* p1, Point* p2, Edge::Status s);
    void AddEdge( Point* p1, Point* p2);
    void AddExistingEdge( Edge* edge);

    void DeleteTemporaryEdges();
    void DeleteTemporaryPoint();
    void DeleteExistingEdges();

    void ResetMarkers();

    void FinalizeSMT();
    unsigned CalculateMST( bool to_finalize);
    unsigned CalculateMST();
    bool SMTIteration();
    void CollectHananPoints();

    template<typename T> std::list<T*> DuplicateListOfPointers( const std::list<T*>& to_copy);
    template<typename T> std::list<T> MakeSafeCopyForListOfPointers( const std::list<T*>& to_copy);
    template<typename T> void ClearListOfPointers( std::list<T*>& to_clear);

    void PerformCopy( const SMT& other);

    void Destroy();

public:

    SMT( unsigned N, unsigned M);
    ~SMT();
    SMT( const SMT& other);
    SMT( SMT&& other_tmp) = default;
    SMT& operator=( const SMT& other);
    SMT& operator=( SMT&& other_tmp);

    unsigned GetGridSize();
    unsigned GetPinCount();
    void AddPin( unsigned x, unsigned y);
    unsigned BuildSMT();

    std::list<Point> GetPointsList();
    std::list<Edge> GetEdgesList();
};

#endif
