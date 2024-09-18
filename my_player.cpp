#include "my_player.h"
#include <cstdlib>
#include <iostream>
#include <algorithm> 
#include <math.h>
using namespace std;


static field_index_t rand_int(field_index_t min, field_index_t max) {
    return min + rand() % (max - min + 1);
}

Point RandomPlayer::play(const GameView& game) {
    Boundary b = game.get_settings().field_size;
    Point result;
    do {
        result = {
            .x = rand_int(b.min.x, b.max.x),
            .y = rand_int(b.min.y, b.max.y),
        };
    } while(game.get_state().field->get_value(result) != Mark::None);
    return result;
}

void BasicObserver::notify(const GameView&, const Event& event) {
    if (event.get_type() == MoveEvent::TYPE) {
        auto &data = get_data<MoveEvent>(event);
        _out << "Move:\tx = " <<  data.point.x 
            << ",\ty = " << data.point.y << ":\t";
        _print_mark(data.mark) << '\n';
        return;
    }
    if (event.get_type() == PlayerJoinedEvent::TYPE) {
        auto &data = get_data<PlayerJoinedEvent>(event);
        _out << "Player '" << data.name << "' joined as ";
        _print_mark(data.mark) << '\n';
        return;
    }
    if (event.get_type() == GameStartedEvent::TYPE) {
        _out << "Game started\n";
        return;
    }
    if (event.get_type() == WinEvent::TYPE) {
        auto &data = get_data<WinEvent>(event);
        _out << "Player playing ";
        _print_mark(data.winner_mark) << " has won\n";
        return;
    }
    if (event.get_type() == DrawEvent::TYPE) {
        auto &data = get_data<DrawEvent>(event);
        _out << "Draw happened, reason: '" << data.reason << "'\n";
        return;
    }
    if (event.get_type() == DisqualificationEvent::TYPE) {
        auto &data = get_data<DisqualificationEvent>(event);
        _out << "Player playing ";
        _print_mark(data.player_mark) << " was disqualified, reason: '" 
            << data.reason << "'\n";
        return;
    }
}

std::ostream& BasicObserver::_print_mark(Mark m) {
    if (m == Mark::Cross) return _out << "X";
    if (m == Mark::Zero) return _out << "O";
    return _out << "?";
}

Point SillyPlayer::play(const GameView& game) {
    Point result{};
    Boundary b = game.get_settings().field_size;
    for (int row = b.min.x; row < b.max.x; row++) {
        for (int col = b.min.y; col < b.max.y; col++) {
            Point cur = { .x = row, .y = col, };
            if (game.get_state().field->get_value(cur) == Mark::None) {
                result = { .x = row, .y = col, };
            }
            else { continue; }
        }
    }
    return result;
}

//-----------------------------------------------------
void NewPlayer::notify(const GameView& game, const Event& event)
{
}

std::string NewPlayer::get_name() const
{
    return _name;
}

Point NewPlayer::play(const GameView& game)
{
    Point result{};
    Boundary b = game.get_settings().field_size;        //первый ход в центр поля, если свободен

    int cRow = (b.min.x + b.max.x) / 2;
    int cCol = (b.min.y + b.max.y) / 2;
    Point center = { .x = cRow, .y = cCol };

    if (game.get_state().field->get_value(center) == Mark::None) {
        return center;
    }
    else {

        auto field_iterator = game.get_state().field->get_iterator();
        vector <Point> my_moves;
        vector<double> my_moves_weights(my_moves.size(), 0.0);
        Mark my = game.get_state().current_move;

        //на случай если я 0 и (0,0) занято пусть будет рандомная точка с малым весом
        do {
            result = {
                .x = rand_int((b.min.x + 5), (b.max.x - 5)),
                .y = rand_int((b.min.y + 5), (b.max.y - 5)),
            };
        } while (game.get_state().field->get_value(result) != Mark::None);

        my_moves.push_back(result);
        my_moves_weights.push_back(0.005);
        while (field_iterator->has_value())                 // анализ точек, которые уже в поле
        {
            Point now = field_iterator->get_point();
            Mark now_mark = game.get_state().field->get_value(now);
            bool my_or_not = (now_mark == game.get_state().current_move);
             
            
            int prov1 = all_line_analysis(game, now, now_mark, -1, 1);
            int prov2 = all_line_analysis(game, now, now_mark, 0, 1);
            int prov3 = all_line_analysis(game, now, now_mark, 1, 1);
            int prov4 = all_line_analysis(game, now, now_mark, -1, 0);
            
            line_checker(game, now, prov1, -1, 1, my_moves, my, now_mark, my_moves_weights);
            line_checker(game, now, prov2, 0, 1, my_moves, my, now_mark, my_moves_weights);
            line_checker(game, now, prov3, 1, 1, my_moves, my, now_mark, my_moves_weights);
            line_checker(game, now, prov4, -1, 0, my_moves, my, now_mark, my_moves_weights);
           
            field_iterator->step();
        }
        result = my_moves[find_max_index(my_moves_weights)];
        return result;
    }
}

int NewPlayer::all_line_analysis(const GameView& game, const Point& point, Mark now_mark, int dx, int dy)
{
    int count = 1;

    int granminx = game.get_settings().field_size.min.x;
    int granmaxx = game.get_settings().field_size.max.x;
    int granminy = game.get_settings().field_size.min.y;
    int granmaxy = game.get_settings().field_size.max.y;
    
    int new_dx, new_dy;
    new_dx = new_dy = 0;
    new_dx += dx;
    new_dy += dy;
    
    while (if_for_while(new_dx, new_dy, point, granmaxx, granminx, granmaxy, granminy, 5))
    {
        if (placeIsMy({ (point.x + new_dx), (point.y + new_dy) }, game, now_mark)) {
            count += 1;
        }
        else { break; }
        new_dx += dx;
        new_dy += dy;
    }
    new_dx = new_dy = 0;
    new_dx -= dx;
    new_dy -= dy;

    while (if_for_while(new_dx, new_dy, point, granmaxx, granminx, granmaxy, granminy, 5))
    {
        if (placeIsMy({ (point.x + new_dx), (point.y + new_dy) }, game, now_mark)) {
            count += 1;
        }
        else { return count; }
        new_dx -= dx;
        new_dy -= dy;
    }
    return count;
}

void NewPlayer::line_checker(const GameView& game, const Point& point, int col, int dx, int dy, vector <Point>& my_moves,
    Mark my, Mark now, vector<double>& my_moves_weights)
{
    if ((col == 4) || (col == 3) || (col == 2)) { 
        nearest_one(game, point, dx, dy, col, my_moves, my_moves_weights, (my == now));
    }

    if (my == now) {
        if (col == 1) {
            nearest_one(game, point, dx, dy, col, my_moves, my_moves_weights, (my == now));
        }
    }
}

void NewPlayer::nearest_one(const GameView& game, const Point& point, int dx, int dy, int my_i, vector <Point>& my_moves,
    vector<double>& my_moves_weights, bool my_or_not)
{
    int granminx = game.get_settings().field_size.min.x;
    int granmaxx = game.get_settings().field_size.max.x;
    int granminy = game.get_settings().field_size.min.y;
    int granmaxy = game.get_settings().field_size.max.y;
    
    Mark my = game.get_state().current_move;

    bool mainline = ((dx == -1) && (dy == 1)) ? true : false;
    int new_dx, new_dy;
    new_dx = new_dy = 0;
    new_dx += dx;
    new_dy += dy;
    while (if_for_while(new_dx, new_dy, point, granmaxx, granminx, granmaxy, granminy, my_i))
    {
        if (placeIsMy({ (point.x + new_dx), (point.y + new_dy) }, game, my)) {
            break;
        }
        if (placeIsFree({ (point.x + new_dx), (point.y + new_dy) }, game)) {
            my_moves.push_back({ (point.x + new_dx), (point.y + new_dy) });
            calculated(my_i, my_moves_weights, my_or_not, mainline);
        }
        new_dx += dx;
        new_dy += dy;
    }
    new_dx = new_dy = 0;
    new_dx -= dx;
    new_dy -= dy;
    while (if_for_while(new_dx, new_dy, point, granmaxx, granminx, granmaxy, granminy, my_i))
    {
        if (placeIsMy({ (point.x + new_dx), (point.y + new_dy) }, game, my)) {
            return;
        }
        if (placeIsFree({ (point.x + new_dx), (point.y + new_dy) }, game)) {
            my_moves.push_back({ (point.x + new_dx), (point.y + new_dy) });
            calculated(my_i, my_moves_weights, my_or_not, mainline);
        }
        new_dx -= dx;
        new_dy -= dy;
    }
}

void NewPlayer::calculated(int my_i, std::vector<double>& my_moves_weights, bool my_or_not, bool mainline)
{
    if ((my_i == 4) && my_or_not) {
        my_moves_weights.push_back(1.0);
        return;
    }
    if ((my_i == 4) && !(my_or_not)) {
        my_moves_weights.push_back(0.95);
        return;
    }
    if ((my_i == 3) && my_or_not) {
        mainline ? my_moves_weights.push_back(0.85) : my_moves_weights.push_back(0.80);
        return;
    }
    if ((my_i == 3) && !(my_or_not)) {
        mainline ? my_moves_weights.push_back(0.75) : my_moves_weights.push_back(0.70);
        return;
    }
    if ((my_i == 2) && my_or_not) {
        mainline ? my_moves_weights.push_back(0.45) : my_moves_weights.push_back(0.40);
        return;
    }
    if ((my_i == 2) && !(my_or_not)) {
        mainline ? my_moves_weights.push_back(0.35) : my_moves_weights.push_back(0.30);
        return;
    }
    if ((my_i == 1) && my_or_not) {
        mainline ? my_moves_weights.push_back(0.25) : my_moves_weights.push_back(0.20);
        return;
    }
}

bool NewPlayer::if_for_while(int new_dx, int new_dy, const Point& point, int granmaxx, int granminx, int granmaxy, int granminy, int my_i)
{
    return (
        ((point.x + new_dx) <= granmaxx) && ((abs(new_dx) < (my_i + 1)) || (abs(new_dy) < (my_i + 1)))
        && ((point.x + new_dx) >= granminx) && ((point.y + new_dy) >= granminy)
        && ((point.y + new_dy) <= granmaxy)
        );
}

int NewPlayer::find_max_index(std::vector<double>& my_moves_weights)
{
    double maxw = -1.0;
    int maxw_index = -1;
    for (int i = 0; i < my_moves_weights.size(); i++) {
        if (my_moves_weights[i] > maxw) {
            maxw = my_moves_weights[i];
            maxw_index = i;
        }
    }
    return maxw_index;
}

bool NewPlayer::placeIsFree(const Point& point, const GameView& game)
{
    return game.get_state().field->get_value(point) == Mark::None;
}

bool NewPlayer::placeIsMy(const Point& point, const GameView& game, Mark mark)
{
    if (mark == Mark::Cross) {
        return (int)game.get_state().field->get_value(point) == 1;
    }
    return (int)game.get_state().field->get_value(point) == 0; 
}
//-----------------------------------------------------
// было 630 строчек кода, стало 227.
//функция play занимает теперь 52 строчек (должен был уложиться в 100).
