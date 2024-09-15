#pragma once
#include <ostream>
#include <vector>
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

    int line_analysis1(const GameView& game, const Point& point, Mark now_mark); //проверка по линиям (диагональ1).сделано
    int line_analysis2(const GameView& game, const Point& point, Mark now_mark); //проверка по линиям (вертикаль).сделано
    int line_analysis3(const GameView& game, const Point& point, Mark now_mark); //проверка по линиям (диагональ2).сделано 
    int line_analysis4(const GameView& game, const Point& point, Mark now_mark); //проверка по линиям (горизонталь).сделано
    
    bool insideField(const Point& point, const Boundary& b);        //проверка, что ход выбран внутри поля. сделано но хз зачем
    bool placeIsFree(const Point& point, const GameView& game);     //проверка, что место свободно.сделано
    bool placeIsMy(const Point& point, const GameView& game, Mark mark);       //проверка, что поле занято мной. сделано
};

//-----------------------------------------------------