#pragma optimize("O3")  
#include <bits/stdc++.h>
#include <chrono>
using namespace std;
using namespace std::chrono;

// BIG WARNING !
// The destructor of Box has not been implemented yet because of recursive pointers
// This WILL lead to memory leaks
// This should be fixed when creation of levels will become a part of the code

long long int get_ms()
{
    return duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    ).count();
}

const vector<vector<int>> DIR = {
    {1, 0},
    {-1, 0},
    {0, 1},
    {0, -1},
};
const vector<string> DIR_NAMES = {"S", "N", "E", "W"};

template <typename T>
ostream& operator<<(ostream &ost, const vector<T> &tab)
{
    for (T v : tab) ost << v << " ";
    return ost;
}

template <typename T>
ostream& operator<<(ostream &ost, const vector<vector<T>> &tab)
{
    for (vector<T> v : tab) ost << v << "\n";
    return ost;
}

struct Box
{
    enum Type {
        WALL,
        EMPTY,
        PLAYER,
        SIMPLE_BOX,
        UNIVERSE,
        LEVEL
    };

    Type type;
    vector<vector<Box*>> grid;
    Box* container;
    int lig;
    int col;
    bool has_player_end;
    bool has_box_end;
    string name;

    Box(Type _type, vector<vector<Box*>> _grid, Box* _cont = nullptr, int _lig = -1, int _col = -1, string _name = ""):
        type{_type},
        grid{_grid},
        container{_cont},
        lig{_lig},
        col{_col},
        has_player_end{false},
        has_box_end{false},
        name{_name}
    {}

    Box(Type _type, int height, int width, Box* _cont = nullptr, int _lig = -1, int _col = -1, string _name = ""):
        Box(_type,
            {},
            _cont,
            _lig,
            _col,
            _name
        )
    {
        reinit_grid(height, width);
    }

    // GETTERS
    int height() const
    {
        return grid.size();
    }

    int width() const
    {
        if (grid.empty()) return 0;
        return grid[0].size();
    }

    bool is_enterable() const
    {
        return type == LEVEL;
    }

    bool is_eatable() const
    {
        return type == LEVEL || type == PLAYER || type == SIMPLE_BOX;
    }

    bool is_movable() const
    {
        return type == LEVEL || type == PLAYER || type == SIMPLE_BOX;
    }

    bool is_container() const
    {
        return type == LEVEL || type == UNIVERSE;
    }

    bool counts_for_box_end() const
    {
        return type == LEVEL || type == SIMPLE_BOX;
    }

    // SETTERS
    void place_box_at(Box* box, int lig, int col)
    {
        if (box != grid[lig][col]) delete grid[lig][col];
        grid[lig][col] = box;
    }

    void reinit_grid(int h, int w)
    {
        for (int lig = 0; lig < height(); lig++)
        {
            for (int col = 0; col < width(); col++)
            {
                delete grid[lig][col];
            }
        }
        grid = vector<vector<Box*>>(h, vector<Box*>(w, nullptr));
        for (int lig = 0; lig < h; lig++)
        {
            for (int col = 0; col < w; col++)
            {
                grid[lig][col] = EmptyBox(this, lig, col);
            }
        }
    }

    // FACTORIES
    static Box* Wall(Box* _cont = nullptr, int _lig = -1, int _col = -1)
    {
        return new Box(WALL, {}, _cont, _lig, _col, "#");
    }

    static Box* EmptyBox(Box* _cont = nullptr, int _lig = -1, int _col = -1)
    {
        return new Box(EMPTY, {}, _cont, _lig, _col, ".");
    }

    static Box* Player(Box* _cont = nullptr, int _lig = -1, int _col = -1)
    {
        return new Box(PLAYER, {}, _cont, _lig, _col, "p");
    }

    static Box* SimpleBox(Box* _cont = nullptr, int _lig = -1, int _col = -1)
    {
        return new Box(SIMPLE_BOX, {}, _cont, _lig, _col, "b");
    }

    static Box* Universe()
    {
        Box* univ = Level(7, 7, "U");
        univ->type = UNIVERSE;
        for (int i = 0; i < 7; i++)
        {
            delete univ->grid[i][0];
            delete univ->grid[i][6];
            univ->grid[i][0] = Wall(univ, i, 0);
            univ->grid[i][6] = Wall(univ, i, 6);
        }
        for (int i = 1; i < 6; i++)
        {
            delete univ->grid[0][i];
            delete univ->grid[6][i];
            univ->grid[0][i] = Wall(univ, 0, i);
            univ->grid[6][i] = Wall(univ, 6, i);
        }
        return univ;
    }

    static Box* Level(int height, int width, string name, Box* _cont = nullptr, int _lig = -1, int _col = -1)
    {
        Box* level = new Box(LEVEL, height, width, _cont, _lig, _col, name);
        for (int lig = 0; lig < height; lig++)
            for (int col = 0; col < width; col++)
                level->grid[lig][col] = EmptyBox(level, lig, col);
        return level;
    }

    // ACTIONS
    bool is_in_grid(int l, int c) const
    {
        return l >= 0 && l < height() && c >= 0 && c < width();
    }

    // Returns the position you would have after entering this box
    void pos_after_enter(int dlig, int dcol, int &Nlig, int &Ncol) const
    {
        if (dlig == 0) Nlig = height() / 2;
        else if (dlig < 0) Nlig = height() + dlig;
        else Nlig = dlig - 1;
        if (dcol == 0) Ncol = width() / 2;
        else if (dcol < 0) Ncol = width() + dcol;
        else Ncol = dcol - 1;
    }

    bool move(int dlig, int dcol)
    {
        if (type == EMPTY || type == WALL) return false;
        Box* Ncontainer = container;
        int Nlig = lig + dlig, Ncol = col + dcol;
        while (!Ncontainer->is_in_grid(Nlig, Ncol))
        {
            Nlig = Ncontainer->lig + dlig;
            Ncol = Ncontainer->col + dcol;
            Ncontainer = Ncontainer->container;
        }
        bool can_enter = enter_cell(Ncontainer, Nlig, Ncol, dlig, dcol);
        return can_enter;
    }

    bool enter_cell(Box* box, int Nlig, int Ncol, int dlig, int dcol)
    {
        bool can_move = box->grid[Nlig][Ncol]->move(dlig, dcol);
        if (box->grid[Nlig][Ncol]->type == WALL) return false;
        if (box->grid[Nlig][Ncol]->type == EMPTY)
        {
            bool me_had_player_end = has_player_end;
            bool me_had_box_end = has_box_end;
            has_player_end = box->grid[Nlig][Ncol]->has_player_end;
            has_box_end = box->grid[Nlig][Ncol]->has_box_end;
            delete box->grid[Nlig][Ncol];
            box->grid[Nlig][Ncol] = this;
            container->grid[lig][col] = EmptyBox(container, lig, col);
            container->grid[lig][col]->has_player_end = me_had_player_end;
            container->grid[lig][col]->has_box_end = me_had_box_end;
            container = box;
            lig = Nlig;
            col = Ncol;
            return true;
        }
        if (box->grid[Nlig][Ncol]->is_enterable())
        {
            int NNlig, NNcol;
            box->grid[Nlig][Ncol]->pos_after_enter(dlig, dcol, NNlig, NNcol);
            bool can_enter = enter_cell(box->grid[Nlig][Ncol], NNlig, NNcol, dlig, dcol);
            if (can_enter) return true;
        }
        if (is_enterable() && box->grid[Nlig][Ncol]->is_eatable())
        {
            int NNlig, NNcol;
            pos_after_enter(-dlig, -dcol, NNlig, NNcol);
            bool can_enter = box->grid[Nlig][Ncol]->enter_cell(this, NNlig, NNcol, -dlig, -dcol);
            if (can_enter)
            {
                enter_cell(box, Nlig, Ncol, dlig, dcol);
                return true;
            }
        }
        return false;
    }
};

struct BoxPosition
{
    Box* box;
    int lig;
    int col;
};

struct Level
{
    Box* universe;
    Box* player;
    long long int nb_cells = 0;
    map<Box*, long long int> box_to_id_cell;
    vector<BoxPosition> id_cell_to_box;
    vector<Box*> all_movables;
    BoxPosition player_end = {nullptr, -1, -1};
    vector<BoxPosition> box_ends;

    bool move_player(int dlig, int dcol)
    {
        return player->move(dlig, dcol);
    }

    long long int hash() const
    {
        long long int h = 0, p = 1;
        for (Box* box : all_movables)
        {
            long long int box_hash = box_to_id_cell.at(box->container) + box->container->width() * box->lig + box->col;
            h += p * box_hash;
            p *= nb_cells;
        }
        return h;
    }

    void set_from_hash(long long int h)
    {
        vector<BoxPosition> new_pos;
        while (h > 0)
        {
            long long int id_pos = h % nb_cells;
            h /= nb_cells;
            new_pos.push_back(id_cell_to_box[id_pos]);
        }
        for (Box* box : all_movables)
        {
            box->container->grid[box->lig][box->col] = Box::EmptyBox(box->container, box->lig, box->col);
            if (box->has_player_end)
            {
                box->has_player_end = false;
                box->container->grid[box->lig][box->col]->has_player_end = true;
            }
            if (box->has_box_end)
            {
                box->has_box_end = false;
                box->container->grid[box->lig][box->col]->has_box_end = true;
            }
        }
        for (int i = 0; i < all_movables.size(); i++)
        {
            if (new_pos[i].box->grid[new_pos[i].lig][new_pos[i].col]->has_player_end)
            {
                all_movables[i]->has_player_end = true;
            }
            if (new_pos[i].box->grid[new_pos[i].lig][new_pos[i].col]->has_box_end)
            {
                all_movables[i]->has_box_end = true;
            }
            delete new_pos[i].box->grid[new_pos[i].lig][new_pos[i].col];
            new_pos[i].box->grid[new_pos[i].lig][new_pos[i].col] = all_movables[i];
            all_movables[i]->container = new_pos[i].box;
            all_movables[i]->lig = new_pos[i].lig;
            all_movables[i]->col = new_pos[i].col;
        }
    }

    bool is_finished() const
    {
        if (player_end.box != nullptr && player_end.box->grid[player_end.lig][player_end.col]->type != Box::PLAYER) return false;
        for (BoxPosition bp : box_ends)
        {
            if (!bp.box->grid[bp.lig][bp.col]->counts_for_box_end()) return false;
        }
        return true;
    }
};

Level ask_for_level()
{
    Level whole_level;
    whole_level.universe = Box::Universe();
    map<string, Box*> asked;
    vector<Box*> to_ask = {whole_level.universe};
    while (!to_ask.empty())
    {
        Box* box = to_ask.back();
        to_ask.pop_back();
        asked[box->name] = box;
        cout << box->name << " :\n";

        // Read map
        string line;
        vector<string> box_map;
        while (getline(cin, line) && !line.empty())
        {
            box_map.push_back(line);
        }

        // Process map
        box->reinit_grid(0, 0);
        for (int lig = 0; lig < box_map.size(); lig++)
        {
            line = box_map[lig];
            if (line.back() != ' ' && line.back() != '=' && line.back() != '-') line += " ";
            box->grid.push_back({});
            string name = "";
            int col = 0;
            for (char c : line)
            {
                if (c == ' ' || c == '=' || c == '-')
                {
                    Box* new_box;
                    if (name == "#" || name == "x") new_box = Box::Wall(box, lig, col);
                    else if (name == ".") new_box = Box::EmptyBox(box, lig, col);
                    else if (name == "p")
                    {
                        new_box = Box::Player(box, lig, col);
                        whole_level.player = new_box;
                        whole_level.all_movables.push_back(new_box);
                    }
                    else if (name == "b")
                    {
                        new_box = Box::SimpleBox(box, lig, col);
                        whole_level.all_movables.push_back(new_box);
                    }
                    else
                    {
                        if (asked.count(name) == 0)
                        {
                            new_box = Box::Level(0, 0, name, box, lig, col);
                            to_ask.push_back(new_box);
                            whole_level.all_movables.push_back(new_box);
                        }
                        else new_box = asked[name];
                    }
                    if (c == '=')
                    {
                        new_box->has_player_end = true;
                        whole_level.player_end = BoxPosition{box, lig, col};
                    }
                    if (c == '-')
                    {
                        new_box->has_box_end = true;
                        whole_level.box_ends.push_back(BoxPosition{box, lig, col});
                    }
                    box->grid[lig].push_back(new_box);
                    name = "";
                    col++;
                }
                else
                {
                    name += c;
                }
            }
        }
        if (box->is_container())
        {
            whole_level.box_to_id_cell[box] = whole_level.nb_cells;
            whole_level.nb_cells += box->height() * box->width();
            for (int lig = 0; lig < box->height(); lig++)
            {
                for (int col = 0; col < box->width(); col++)
                {
                    whole_level.id_cell_to_box.push_back(BoxPosition{box, lig, col});
                }
            }
        }
    }
    return whole_level;
}

ostream& operator<<(ostream& ost, Box* top_box)
{
    set<Box*> printed;
    vector<Box*> to_print = {top_box};
    while (!to_print.empty())
    {
        Box* box = to_print.back();
        to_print.pop_back();
        printed.insert(box);
        ost << box->name << " (" << box->lig << "," << box->col << ") :\n";
        for (int lig = 0; lig < box->height(); lig++)
        {
            for (int col = 0; col < box->width(); col++)
            {
                Box* child = box->grid[lig][col];
                if (child)
                {
                    ost << child->name;
                    if (child->has_player_end) ost << "=";
                    else if (child->has_box_end) ost << "-";
                    else ost << " ";
                    if (child->type == Box::LEVEL && printed.count(child) == 0) to_print.push_back(child);
                }
                else ost << "  ";
            }
            ost << "\n";
        }
        ost << "\n";
    }
    return ost;
}

ostream& operator<<(ostream& ost, const Level &level)
{
    ost << "UNIVERSE PRINT :\n";
    ost << level.universe;
    // ost << "PLAYER PRINT :\n";
    // ost << "pos = (" << level.player->lig << "," << level.player->col << ")\n";
    // ost << level.player->container;
    // ost << "PLAYER END PRINT :\n";
    // ost << "pos = (" << level.player_end.lig << "," << level.player_end.col << ")\n";
    // ost << level.player_end.box;
    // ost << "BOX END PRINT :\n";
    // for (BoxPosition bp : level.box_ends)
    // {
    //     ost << "pos = (" << bp.lig << "," << bp.col << ")\n";
    //     ost << bp.box;
    // }
    // ost << "NB CELLS : " << level.nb_cells << "\n";
    // ost << "BOX TO ID :\n";
    // for (auto it = level.box_to_id_cell.begin(); it != level.box_to_id_cell.end(); it++)
    // {
    //     ost << it->first->name << " : " << it->second << "\n";
    // }
    // ost << "ALL MOVABLES :\n" << level.all_movables << "\n";
    ost << "HASH : " << level.hash() << "\n";
    return ost;
}

vector<string> find_best_path(Level &level)
{
    // BFS
    map<long long int, int> dist, lastdir;
    map<long long int, long long int> last_hash;
    long long int start_hash = level.hash();
    vector<long long int> aFaire = {start_hash}, aRajouter;
    dist[start_hash] = 0;
    lastdir[start_hash] = -1;
    long long int finish_found = -1;
    int d = 0;
    if (level.is_finished()) finish_found = start_hash;
    while (finish_found == -1 && !aFaire.empty())
    {
        d++;
        for (long long int hash : aFaire)
        {
            for (int idir = 0; idir < DIR.size(); idir++)
            {
                vector<int> dir = DIR[idir];
                level.set_from_hash(hash);
                level.move_player(dir[0], dir[1]);
                long long int next_hash = level.hash();
                if (dist.count(next_hash) == 0)
                {
                    dist[next_hash] = d;
                    lastdir[next_hash] = idir;
                    last_hash[next_hash] = hash;
                    aRajouter.push_back(next_hash);
                    if (level.is_finished()) finish_found = next_hash;
                }
            }
        }
        aFaire = aRajouter;
        aRajouter.clear();
    }

    level.set_from_hash(finish_found);
    cout << "FINISH :\n" << level << "\n";

    // Calculate found path
    level.set_from_hash(start_hash);
    if (finish_found == -1) return {};
    long long int curr_hash = finish_found;
    vector<string> path = {"."};
    while (curr_hash != start_hash)
    {
        path.push_back(DIR_NAMES[lastdir[curr_hash]]);
        curr_hash = last_hash[curr_hash];
    }
    reverse(path.begin(), path.end());
    cout << "hash map size : " << dist.size() << "\n";
    return path;
}

int main()
{
    Level whole_level = ask_for_level();
    cout << whole_level;
    // whole_level.set_from_hash(424213776);
    // cout << whole_level;
    // cout << "MOVE 1 : " << whole_level.move_player(0, 1) << "\n\n";
    // cout << whole_level;
    // cout << "MOVE 2 : " << whole_level.move_player(-1, 0) << "\n\n";
    // cout << whole_level;
    // cout << "MOVE 3 : " << whole_level.move_player(-1, 0) << "\n\n";
    // cout << whole_level;
    // cout << "MOVE 4 : " << whole_level.move_player(-1, 0) << "\n\n";
    // cout << whole_level;
    long long int tps_dep = get_ms();
    vector<string> best_path = find_best_path(whole_level);
    long long int tps_fin = get_ms();

    cout << "BEST PATH (" << best_path.size() << " moves) : " << best_path << "\n";
    cout << "Execution time : " << tps_fin - tps_dep << "ms\n";
}