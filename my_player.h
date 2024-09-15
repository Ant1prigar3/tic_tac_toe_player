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
    std::string _name;      // ��� ������

public:

    NewPlayer(const std::string& name) : _name(name) {}                     // �����������
    virtual void notify(const GameView& game, const Event& event) override;
    virtual std::string get_name() const override;                  // ��������� �����
    virtual void assign_mark(Mark player_mark) override {};
    virtual Point play(const GameView& game) override;              //���������� ���� � �������� ������.

    int line_analysis1(const GameView& game, const Point& point, Mark now_mark); //�������� �� ������ (���������1).�������
    int line_analysis2(const GameView& game, const Point& point, Mark now_mark); //�������� �� ������ (���������).�������
    int line_analysis3(const GameView& game, const Point& point, Mark now_mark); //�������� �� ������ (���������2).������� 
    int line_analysis4(const GameView& game, const Point& point, Mark now_mark); //�������� �� ������ (�����������).�������
    
    bool insideField(const Point& point, const Boundary& b);        //��������, ��� ��� ������ ������ ����. ������� �� �� �����
    bool placeIsFree(const Point& point, const GameView& game);     //��������, ��� ����� ��������.�������
    bool placeIsMy(const Point& point, const GameView& game, Mark mark);       //��������, ��� ���� ������ ����. �������
};

//-----------------------------------------------------