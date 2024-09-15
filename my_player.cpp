#include "my_player.h"
#include <cstdlib>
#include <iostream>
#include <algorithm> 
#include "vector"
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
        
        //на случай если я 0 и (0,0) занято пусть будет рандомная точка с малым весом
        do {
            result = {
                .x = rand_int(b.min.x, b.max.x),
                .y = rand_int(b.min.y, b.max.y),
            };
        } while (game.get_state().field->get_value(result) != Mark::None);
        my_moves.push_back(result);
        my_moves_weights.push_back(0.005);
        while (field_iterator->has_value())                 // анализ точек, которые уже в поле
        {
            Point now = field_iterator->get_point();
            Mark now_mark = game.get_state().field->get_value(now);
            bool my_or_not = (now_mark == game.get_state().current_move);
             
            int weight1 = line_analysis1(game, now, now_mark);
            int weight2 = line_analysis2(game, now, now_mark);
            int weight3 = line_analysis3(game, now, now_mark);
            int weight4 = line_analysis4(game, now, now_mark);
            
            // четверки 

            if (weight1 == 4) {     // если найдена 4, то независимо от того, чея линия, закрываем
                int dx, dy;
                dx = dy = 1;
                while (((now.y + dy) <= b.max.y) && (dy < 5) &&
                    ((now.x - dx) >= b.min.x))        // северо-запад
                {
                    if (placeIsFree({ (now.x - dx), (now.y + dy) }, game)) {
                        return { (now.x - dx), (now.y + dy) };
                    }
                    dx++;
                    dy++;
                };
                dx = dy = 1;
                while (((now.y - dy) >= b.min.y) && (dy < 5) &&
                    ((now.x + dx) <= b.max.x))        // юго-восток
                {
                    if (placeIsFree({ (now.x + dx), (now.y - dy) }, game)) {
                        return { (now.x + dx), (now.y - dy) };
                    }
                    dx++;
                    dy++;
                };
            }
            if (weight2 == 4) {     // если найдена 4, то независимо от того, чея линия, закрываем
                int dy;
                dy = 1;
                while (((now.y + dy) <= b.max.y) && (dy < 5))        // вверх
                {
                    if (placeIsFree({ (now.x), (now.y + dy) }, game)) {
                        return { (now.x), (now.y + dy) };
                    }
                    dy++;
                };
                dy = 1;
                while (((now.y - dy) >= b.min.y) && (dy < 5))        // вниз
                {
                    if (placeIsFree({ (now.x), (now.y - dy) }, game)) {
                        return { (now.x), (now.y - dy) };
                    }
                    dy++;
                };
            }
            if (weight3 == 4) {     // если найдена 4, то независимо от того, чея линия, закрываем
                int dx, dy;
                dx = dy = 1;
                while (((now.y + dy) <= b.max.y) && (dy < 5) &&
                    ((now.x + dx) <= b.max.x))        // северо-восток
                {
                    if (placeIsFree({ (now.x + dx), (now.y + dy) }, game)) {
                        return { (now.x + dx), (now.y + dy) };
                    }
                    dx++;
                    dy++;
                };
                dx = dy = 1;
                while (((now.y - dy) >= b.min.y) && (dy < 5) &&
                    ((now.x - dx) >= b.min.x))        // юго-запад
                {
                    if (placeIsFree({ (now.x - dx), (now.y - dy) }, game)) {
                        return { (now.x - dx), (now.y - dy) };
                    }
                    dx++;
                    dy++;
                };
            }
            if (weight4 == 4) {     // если найдена 4, то независимо от того, чея линия, закрываем
                int dx;
                dx = 1;
                while (((now.x + dx) <= b.max.x) && (dx < 5))        // влево
                {
                    if (placeIsFree({ (now.x + dx), (now.y) }, game)) {
                        return { (now.x + dx), (now.y) };
                    }
                    dx++;
                };
                dx = 1;
                while (((now.x - dx) >= b.min.x) && (dx < 5))        // вправо
                {
                    if (placeIsFree({ (now.x - dx), (now.y) }, game)) {
                        return { (now.x - dx), (now.y) };
                    }
                    dx++;
                };
            }

            // тройки 

            if (weight1 == 3) {
                int dx, dy;
                dx = dy = 1;
                while (((now.y + dy) <= b.max.y) && (dy < 4) &&
                    ((now.x - dx) >= b.min.x))        // северо-запад
                {
                    if (placeIsFree({ (now.x - dx), (now.y + dy) }, game)) {
                        my_moves.push_back({ (now.x - dx), (now.y + dy) });
                        if (my_or_not){ my_moves_weights.push_back(0.95); }
                        else { my_moves_weights.push_back(0.85); }
                        break;
                    }
                    dx++;
                    dy++;
                };
                dx = dy = 1;
                while (((now.y - dy) >= b.min.y) && (dy < 4) &&
                    ((now.x + dx) <= b.max.x))        // юго-восток
                {
                    if (placeIsFree({ (now.x + dx), (now.y - dy) }, game)) {
                        my_moves.push_back({ (now.x + dx), (now.y - dy) });
                        if (my_or_not) { my_moves_weights.push_back(0.95); }
                        else { my_moves_weights.push_back(0.85); }
                        break;
                    }
                    dx++;
                    dy++;
                };
            }
            if (weight2 == 3) {
                int dy;
                dy = 1;
                while (((now.y + dy) <= b.max.y) && (dy < 4))        // вверх
                {
                    if (placeIsFree({ (now.x), (now.y + dy) }, game)) {
                        my_moves.push_back({ (now.x), (now.y + dy) });
                        if (my_or_not) { my_moves_weights.push_back(0.90); }
                        else { my_moves_weights.push_back(0.80); }
                        break;
                    }
                    dy++;
                };
                dy = 1;
                while (((now.y - dy) >= b.min.y) && (dy < 4))        // вниз
                {
                    if (placeIsFree({ (now.x), (now.y - dy) }, game)) {
                        my_moves.push_back({ (now.x), (now.y - dy) });
                        if (my_or_not) { my_moves_weights.push_back(0.90); }
                        else { my_moves_weights.push_back(0.80); }
                        break;
                    }
                    dy++;
                };
            }
            if (weight3 == 3) {
                int dx, dy;
                dx = dy = 1;
                while (((now.y + dy) <= b.max.y) && (dy < 4) &&
                    ((now.x + dx) <= b.max.x))        // северо-восток
                {
                    if (placeIsFree({ (now.x + dx), (now.y + dy) }, game)) {
                        my_moves.push_back({ (now.x + dx), (now.y + dy) });
                        if (my_or_not) { my_moves_weights.push_back(0.90); }
                        else { my_moves_weights.push_back(0.80); }
                        break;
                    }
                    dx++;
                    dy++;
                };
                dx = dy = 1;
                while (((now.y - dy) >= b.min.y) && (dy < 4) &&
                    ((now.x - dx) >= b.min.x))        // юго-запад
                {
                    if (placeIsFree({ (now.x - dx), (now.y - dy) }, game)) {
                        my_moves.push_back({ (now.x - dx), (now.y - dy) });
                        if (my_or_not) { my_moves_weights.push_back(0.90); }
                        else { my_moves_weights.push_back(0.80); }
                        break;
                    }
                    dx++;
                    dy++;
                };
            }
            if (weight4 == 3) {
                int dx;
                dx = 1;
                while (((now.x + dx) <= b.max.x) && (dx < 4))        // влево
                {
                    if (placeIsFree({ (now.x + dx), (now.y) }, game)) {
                        my_moves.push_back({ (now.x + dx), (now.y) });
                        if (my_or_not) { my_moves_weights.push_back(0.90); }
                        else { my_moves_weights.push_back(0.80); }
                        break;
                    }
                    dx++;
                };
                dx = 1;
                while (((now.x - dx) >= b.min.x) && (dx < 4))        // вправо
                {
                    if (placeIsFree({ (now.x - dx), (now.y) }, game)) {
                        my_moves.push_back({ (now.x - dx), (now.y) });
                        if (my_or_not) { my_moves_weights.push_back(0.90); }
                        else { my_moves_weights.push_back(0.80); }
                        break;
                    }
                    dx++;
                };    
            }

            
            if (my_or_not) {    // игнорируем 2-ки и 1-чки противника

                // двойки

                if (weight1 == 2) {
                    int dx, dy;
                    dx = dy = 1;
                    while (((now.y + dy) <= b.max.y) && (dy < 3) &&
                        ((now.x - dx) >= b.min.x))        // северо-запад
                    {
                        if (placeIsFree({ (now.x - dx), (now.y + dy) }, game)) {
                            my_moves.push_back({ (now.x - dx), (now.y + dy) });
                            my_moves_weights.push_back(0.45);
                            break;
                        }
                        dx++;
                        dy++;
                    };
                    dx = dy = 1;
                    while (((now.y - dy) >= b.min.y) && (dy < 3) &&
                        ((now.x + dx) <= b.max.x))        // юго-восток
                    {
                        if (placeIsFree({ (now.x + dx), (now.y - dy) }, game)) {
                            my_moves.push_back({ (now.x + dx), (now.y - dy) });
                            my_moves_weights.push_back(0.45);
                            break;
                        }
                        dx++;
                        dy++;
                    };
                }
                if (weight2 == 2) {
                    int dy;
                    dy = 1;
                    while (((now.y + dy) <= b.max.y) && (dy < 3))        // вверх
                    {
                        if (placeIsFree({ (now.x), (now.y + dy) }, game)) {
                            my_moves.push_back({ (now.x), (now.y + dy) });
                            my_moves_weights.push_back(0.40);
                            break;
                        }
                        dy++;
                    };
                    dy = 1;
                    while (((now.y - dy) >= b.min.y) && (dy < 3))        // вниз
                    {
                        if (placeIsFree({ (now.x), (now.y - dy) }, game)) {
                            my_moves.push_back({ (now.x), (now.y - dy) });
                            my_moves_weights.push_back(0.40);
                            break;
                        }
                        dy++;
                    };
                }
                if (weight3 == 2) {
                    int dx, dy;
                    dx = dy = 1;
                    while (((now.y + dy) <= b.max.y) && (dy < 3) &&
                        ((now.x + dx) <= b.max.x))        // северо-восток
                    {
                        if (placeIsFree({ (now.x + dx), (now.y + dy) }, game)) {
                            my_moves.push_back({ (now.x + dx), (now.y + dy) });
                            my_moves_weights.push_back(0.40);
                            break;
                        }
                        dx++;
                        dy++;
                    };
                    dx = dy = 1;
                    while (((now.y - dy) >= b.min.y) && (dy < 3) &&
                        ((now.x - dx) >= b.min.x))        // юго-запад
                    {
                        if (placeIsFree({ (now.x - dx), (now.y - dy) }, game)) {
                            my_moves.push_back({ (now.x - dx), (now.y - dy) });
                            my_moves_weights.push_back(0.40);
                            break;
                        }
                        dx++;
                        dy++;
                    };
                }
                if (weight4 == 2) {
                    int dx;
                    dx = 1;
                    while (((now.x + dx) <= b.max.x) && (dx < 3))        // влево
                    {
                        if (placeIsFree({ (now.x + dx), (now.y) }, game)) {
                            my_moves.push_back({ (now.x + dx), (now.y) });
                            my_moves_weights.push_back(0.40);
                            break;
                        }
                        dx++;
                    };
                    dx = 1;
                    while (((now.x - dx) >= b.min.x) && (dx < 3))        // вправо
                    {
                        if (placeIsFree({ (now.x - dx), (now.y) }, game)) {
                            my_moves.push_back({ (now.x - dx), (now.y) });
                            my_moves_weights.push_back(0.40);
                            break;
                        }
                        dx++;
                    };
                }

                // единички 

                if (weight1 == 1) {
                    int dx, dy;
                    dx = dy = 1;
                    while (((now.y + dy) <= b.max.y) && (dy < 2) &&
                        ((now.x - dx) >= b.min.x))        // северо-запад
                    {
                        if (placeIsFree({ (now.x - dx), (now.y + dy) }, game)) {
                            my_moves.push_back({ (now.x - dx), (now.y + dy) });
                            my_moves_weights.push_back(0.15);
                            break;
                        }
                        dx++;
                        dy++;
                    };
                    dx = dy = 1;
                    while (((now.y - dy) >= b.min.y) && (dy < 2) &&
                        ((now.x + dx) <= b.max.x))        // юго-восток
                    {
                        if (placeIsFree({ (now.x + dx), (now.y - dy) }, game)) {
                            my_moves.push_back({ (now.x + dx), (now.y - dy) });
                            my_moves_weights.push_back(0.15);
                            break;
                        }
                        dx++;
                        dy++;
                    };
                }
                if (weight2 == 1) {
                    int dy;
                    dy = 1;
                    while (((now.y + dy) <= b.max.y) && (dy < 2))        // вверх
                    {
                        if (placeIsFree({ (now.x), (now.y + dy) }, game)) {
                            my_moves.push_back({ (now.x), (now.y + dy) });
                            my_moves_weights.push_back(0.10);
                            break;
                        }
                        dy++;
                    };
                    dy = 1;
                    while (((now.y - dy) >= b.min.y) && (dy < 2))        // вниз
                    {
                        if (placeIsFree({ (now.x), (now.y - dy) }, game)) {
                            my_moves.push_back({ (now.x), (now.y - dy) });
                            my_moves_weights.push_back(0.10);
                            break;
                        }
                        dy++;
                    };
                }
                if (weight3 == 1) {
                    int dx, dy;
                    dx = dy = 1;
                    while (((now.y + dy) <= b.max.y) && (dy < 2) &&
                        ((now.x + dx) <= b.max.x))        // северо-восток
                    {
                        if (placeIsFree({ (now.x + dx), (now.y + dy) }, game)) {
                            my_moves.push_back({ (now.x + dx), (now.y + dy) });
                            my_moves_weights.push_back(0.10);
                            break;
                        }
                        dx++;
                        dy++;
                    };
                    dx = dy = 1;
                    while (((now.y - dy) >= b.min.y) && (dy < 2) &&
                        ((now.x - dx) >= b.min.x))        // юго-запад
                    {
                        if (placeIsFree({ (now.x - dx), (now.y - dy) }, game)) {
                            my_moves.push_back({ (now.x - dx), (now.y - dy) });
                            my_moves_weights.push_back(0.10);
                            break;
                        }
                        dx++;
                        dy++;
                    };
                }
                if (weight4 == 1) {
                    int dx;
                    dx = 1;
                    while (((now.x + dx) <= b.max.x) && (dx < 2))        // влево
                    {
                        if (placeIsFree({ (now.x + dx), (now.y) }, game)) {
                            my_moves.push_back({ (now.x + dx), (now.y) });
                            my_moves_weights.push_back(0.10);
                            break;
                        }
                        dx++;
                    };
                    dx = 1;
                    while (((now.x - dx) >= b.min.x) && (dx < 2))        // вправо
                    {
                        if (placeIsFree({ (now.x - dx), (now.y) }, game)) {
                            my_moves.push_back({ (now.x - dx), (now.y) });
                            my_moves_weights.push_back(0.10);
                            break;
                        }
                        dx++;
                    };
                }
            }

            field_iterator->step();
        }
        double maxw = -1.0;
        int maxw_index = -1;
        for (int i = 0; i < my_moves_weights.size(); i++) {
            if (my_moves_weights[i] > maxw) {
                maxw = my_moves_weights[i];
                maxw_index = i;
            }
        }
        result = my_moves[maxw_index];
        return result;

    }
}

int NewPlayer::line_analysis1(const GameView& game, const Point& point, Mark now_mark) //диагональ (слева-сверху->снизу-справа)
{
    int count = 1;
    int dx, dy;
    dx = dy = 1;
    while (((point.y + dy) <= game.get_settings().field_size.max.y) && (dy < 5) &&
        ((point.x - dx) >= game.get_settings().field_size.min.x))        // северо-запад
    {
        if (placeIsMy({ (point.x - dx), (point.y + dy) }, game, now_mark)) {
            count += 1;
        }
        else
        {
            break;
        }
        dy++;
        dx++;
    }
    dy = dx = 1;
    while (((point.y - dy) >= game.get_settings().field_size.min.y) && (dy < 5) && 
        ((point.x + dx) <= game.get_settings().field_size.max.x))        // юго-восток
    {
        if (placeIsMy({ (point.x + dx), (point.y - dy) }, game, now_mark)) {
            count += 1;
        }
        else {
            break;
        }
        dy++;
        dx++;
    }
    return count;    
}

int NewPlayer::line_analysis2(const GameView& game, const Point& point, Mark now_mark)      // вертикаль
{
    int count = 1;
    int dx, dy;
    dx = dy = 1;
    while (((point.y + dy) <= game.get_settings().field_size.max.y) && (dy < 5))        // вверх
    {
        if (placeIsMy({ (point.x), (point.y + dy) }, game, now_mark)) {            
            count += 1;
        }
        else
        {
            break;
        }
        dy++;
    }
    dy = 1;
    while (((point.y - dy) >= game.get_settings().field_size.min.y) && (dy < 5))        // вниз
    {
        if (placeIsMy({ (point.x), (point.y - dy) }, game, now_mark)) {            
            count += 1;
        }
        else {
            break;
        }
        dy++;
    }
    return count;
}

int NewPlayer::line_analysis3(const GameView& game, const Point& point, Mark now_mark)      //диагональ (справа-сверху->снизу-слева)
{
    int count = 1;
    int dx, dy;
    dx = dy = 1;
    while (((point.y + dy) <= game.get_settings().field_size.max.y) && (dy < 5) &&
        ((point.x + dx) <= game.get_settings().field_size.max.x))        // северо-восток
    {
        if (placeIsMy({ (point.x + dx), (point.y + dy) }, game, now_mark)) {
            count += 1;
        }
        else
        {
            break;
        }
        dy++;
        dx++;
    }
    dy = dx = 1;
    while (((point.y - dy) >= game.get_settings().field_size.min.y) && (dy < 5) && 
        ((point.x - dx) >= game.get_settings().field_size.min.x))        // юго-запад
    {
        if (placeIsMy({ (point.x - dx), (point.y - dy) }, game, now_mark)) {
            count += 1;
        }
        else {
            break;
        }
        dy++;
        dx++;
    }
    return count;
}

int NewPlayer::line_analysis4(const GameView& game, const Point& point, Mark now_mark)
{
    int count = 1;
    int dx, dy;
    dx = dy = 1;
    while (((point.x + dx) <= game.get_settings().field_size.max.x) && (dx < 5))        // влево
    {
        if (placeIsMy({ (point.x + dx), (point.y) }, game, now_mark)) {
            count += 1;
        }
        else
        {
            break;
        }
        dx++;
    }
    dx = 1;
    while (((point.x - dx) >= game.get_settings().field_size.min.x) && (dx < 5))        // вправо
    {
        if (placeIsMy({ (point.x - dx), (point.y) }, game, now_mark)) {
            count += 1;
        }
        else {
            break;
        }
        dx++;
    }
    return count;
}

bool NewPlayer::insideField(const Point& point, const Boundary& b)
{
    return (point.x >= b.min.x && point.y >= b.min.y && point.x <= b.max.x && point.y >= b.max.y);
}

bool NewPlayer::placeIsFree(const Point& point, const GameView& game)
{
    return game.get_state().field->get_value(point) == Mark::None;
}

bool NewPlayer::placeIsMy(const Point& point, const GameView& game, Mark mark)
{
    return game.get_state().field->get_value(point) == mark;
}
//-----------------------------------------------------
