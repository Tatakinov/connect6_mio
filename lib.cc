#include "lib.h"

#include "saori.h"
#include "util.h"

#include "connect6.h"

namespace {
    Connect6<score_t> c;
}

bool __load(std::string path) {
    return true;
}

bool __unload() {
    return true;
}

std::string __request(std::string request) {
    saori::Request req = saori::Request::parse(request);
    // if (!req["Argument0"]) { と等価
    if (!req(0)) {
        saori::Response res{204, "No Content"};
        return res;
    }
    saori::Response res{200, "OK"};
    if (req(0).value() == "init") {
        c.init();
    }
    else if (req(0).value() == "isGameOver") {
        auto s = c.isGameOver();
        switch (s) {
            case State::Win:
                res() = "True";
                res(0) = c.isBlack() ? "Black" : "White";
                break;
            case State::Draw:
                res() = "True";
                res(0) = "Draw";
                break;
            case State::Lose:
                res() = "True";
                res(0) = c.isBlack() ? "White" : "Black";
                break;
            default:
                res() = "False";
                res(0) = "Continue";
                break;
        }
    }
    else if (req(0).value() == "put") {
        int x, y;
        {
            std::istringstream iss(req(1).value());
            iss >> x;
        }
        {
            std::istringstream iss(req(2).value());
            iss >> y;
        }
        c.move(y * kSize + x);
        res() = "true";
    }
    else if (req(0).value() == "teban") {
        res() = c.isBlack() ? "Black" : "White";
    }
    else if (req(0).value() == "genHits") {
        auto moves = c.generateMoves();
        res() = moves.size();
        for (int i = 0; i < moves.size(); i++) {
            int x = moves[i] % kSize;
            int y = moves[i] / kSize;
            std::ostringstream oss;
            oss << x << "," << y;
            res(i) = oss.str();
        }
    }
    else if (req(0).value() == "search") {
        int factor = 1;
        if (req(1)) {
            std::istringstream iss(req(1).value());
            iss >> factor;
            if (factor < 1) {
                factor = 1;
            }
            else if (factor > 8) {
                factor = 8;
            }
        }
        c.setRandomMoveFactor(factor);
        auto v = c.search(1);
        int x = v.move % kSize;
        int y = v.move / kSize;
        std::ostringstream oss;
        oss << x << "," << y;
        res() = v.move;
        res(0) = oss.str();
        res(1) = v.score;
    }
    else if (req(0).value() == "board") {
        res() = c.board();
    }

    return res;
}
