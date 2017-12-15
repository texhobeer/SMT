#ifndef SMT__SMT_H
#define SMT__SMT_H

#include<list>

/**
 *  Description for Steiner Minimal Tree
 */
class SMT
{

public:

    class Point;

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

        void AddPoint( Point* point);
        void InitByPoint( Point* point);

        unsigned GetId();
        unsigned GetCounter();
    };

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
            Pins_M3
        };

    private:

        unsigned posX;
        unsigned posY;
        PointType type;
        Marker* marker;
        /** TODO: decide on list of edges */

    public:

        Point( unsigned x, unsigned y, PointType t);

        unsigned GetPosX();
        unsigned GetPosY();
        Marker* GetMarker();
        unsigned GetSCCId();
        unsigned GetSCCCounter();

        void SetMarker( Marker* m);

        bool IsPin();
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
            Temporary
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

        bool IsTemporary();
        bool IsInOneSCC();

        void Link();
        unsigned GetSCCCounter();
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

private:
public:
    std::list<Point*> existing_points;
    std::list<Point*> hanan_points;
    std::list<Edge*> edges;
    std::list<Marker*> markers;

    unsigned grid_size;
    unsigned pin_count; // possible redundant
    unsigned num_of_points;
    unsigned current_MST_length;

    void AddExistingPoint( unsigned x, unsigned y, Point::PointType t, Edge::Status s);
    void AddPseudoPoint( unsigned x, unsigned y);
    void AddTemporaryPoint( unsigned x, unsigned y);
    void AddHananPoint( unsigned x, unsigned y);
    void AddEdge( Point* p1, Point* p2, Edge::Status s);
    void AddEdge( Point* p1, Point* p2);

    void DeleteTemporaryEdges();
    void DeleteTemporaryPoint();

    void ResetMarkers();

    unsigned CalculateMST();
    bool SMTIteration();
    void CollectHananPoints();

public:

    SMT( unsigned N, unsigned M);
    ~SMT();
    unsigned GetGridSize();
    unsigned GetPinCount();
    void AddPin( unsigned x, unsigned y);
    unsigned BuildSMT();

    std::list<Point> GetPointsList();
    std::list<Edge> GetEdgesList();
};

#endif
