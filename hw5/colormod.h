//
// Created by alex on 5/11/18.
//


// this is extra class to colorify cerr and cout prints, therefore no comments, it's taken from internet

#ifndef HW5_COLORMOD_H
#define HW5_COLORMOD_H
#include <ostream>


namespace Color {
    enum Code {
        FG_RED      = 31,
        FG_GREEN    = 32,
        FG_BLUE     = 34,
        FG_DEFAULT  = 39,
        BG_RED      = 41,
        BG_GREEN    = 42,
        BG_BLUE     = 44,
        BG_DEFAULT  = 49
    };
    class Modifier {
        Code code;
    public:
        Modifier(Code pCode) : code(pCode) {}
        friend std::ostream&
        operator<<(std::ostream& os, const Modifier& mod) {
            return os << "\033[" << mod.code << "m";
        }
    };
}
#endif //HW5_COLORMOD_H
