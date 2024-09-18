#pragma once
#include <ostream>
#include "vector"
#include "player.h"


/**
 * This is example player, that plays absolutely randomly.
 */
class RandomPlayer : public Player {
    std::string _name;
public:
    RandomPlayer(const std::string& name): _name(name) {}
    std::string get_name() const override { return _name; }
    Point play(const GameView& game) override;
    void assign_mark(Mark player_mark) override { /*does nothing*/ }
    void notify(const GameView&, const Event&) override { /*does nothing*/ }
};


/**
 * Simple observer, that logs every event in given output stream.
 */
class BasicObserver : public Observer {
    std::ostream& _out;

    std::ostream& _print_mark(Mark mark);
public:
    BasicObserver(std::ostream& out_stream): _out(out_stream) {}
    void notify(const GameView&, const Event& event) override;
};

//this player places dots in rows sequentially
class SillyPlayer : public Player {
    std::string _name;
public:
    SillyPlayer(const std::string& name) : _name(name) {};
    std::string get_name() const override { return _name; }
    Point play(const GameView& game) override;
    void assign_mark(Mark player_mark) override {};
    void notify(const GameView&, const Event&) override {};
};

//-----------------------------------------------------

class NewPlayer : public Player
{
    std::string _name;      // имя игрока

public:

    NewPlayer(const std::string& name) : _name(name) {}                     // конструктор
    virtual void notify(const GameView& game, const Event& event) override;
    virtual std::string get_name() const override;                  // получение имени
    virtual void assign_mark(Mark player_mark) override {};
    virtual Point play(const GameView& game) override;              //перебираем ходы и выбираем лучший.

    int all_line_analysis(const GameView& game, const Point& point, Mark now_mark, int dx, int dy);
    void line_checker(const GameView& game, const Point& point, int col, int dx, int dy, std::vector<Point>& my_moves,
        Mark my, Mark now, std::vector<double>& my_moves_weights);
    void nearest_one(const GameView& game, const Point& point, int dx, int dy, int my_i, std::vector <Point>& my_moves,
        std::vector<double>& my_moves_weights, bool my_or_not);
    void calculated(int my_i, std::vector<double>& my_moves_weights, bool my_or_not, bool mainline);
    bool if_for_while(int new_dx, int new_dy, const Point& point, int granmaxx, int granminx, int granmaxy, int granminy, int my_i);
    int find_max_index(std::vector<double>& my_moves_weights);
    bool placeIsFree(const Point& point, const GameView& game);     //проверка, что место свободно.
    bool placeIsMy(const Point& point, const GameView& game, Mark mark);       //проверка, что поле занято мной.
};

//-----------------------------------------------------
