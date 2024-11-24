#include "os_key_map.hpp"
#include <ospp/event.h>

namespace input
{
//  ----------------------------------------------------------------------------
void initialize_os_key_map(bimap<key_code, int>& key_map) {
    key_map.clear();

    key_map.map(key_code::comma, os::key::code::comma);
    key_map.map(key_code::down, os::key::code::down);
    key_map.map(key_code::enter, os::key::code::enter);
    key_map.map(key_code::equal, os::key::code::equals);
    key_map.map(key_code::escape, os::key::code::escape);
    key_map.map(key_code::left, os::key::code::left);
    key_map.map(key_code::left_bracket, os::key::code::leftbracket);
    key_map.map(key_code::left_shift, os::key::code::lshift);
    key_map.map(key_code::minus, os::key::code::minus);
    key_map.map(key_code::right, os::key::code::right);
    key_map.map(key_code::right_bracket, os::key::code::rightbracket);
    key_map.map(key_code::semicolon, os::key::code::semicolon);
    key_map.map(key_code::space, os::key::code::space);
    key_map.map(key_code::tab, os::key::code::tab);
    key_map.map(key_code::up, os::key::code::up);
    // //  Function
    key_map.map(key_code::f1,  os::key::code::f1);
    key_map.map(key_code::f2,  os::key::code::f2);
    key_map.map(key_code::f3,  os::key::code::f3);
    key_map.map(key_code::f4,  os::key::code::f4);
    key_map.map(key_code::f5,  os::key::code::f5);
    key_map.map(key_code::f6,  os::key::code::f6);
    key_map.map(key_code::f7,  os::key::code::f7);
    key_map.map(key_code::f8,  os::key::code::f8);
    key_map.map(key_code::f9,  os::key::code::f9);
    key_map.map(key_code::f10, os::key::code::f10);
    key_map.map(key_code::f11, os::key::code::f11);
    key_map.map(key_code::f12, os::key::code::f12);
    // //  Digits
    key_map.map(key_code::d0, os::key::code::digit0);
    key_map.map(key_code::d1, os::key::code::digit1);
    key_map.map(key_code::d2, os::key::code::digit2);
    key_map.map(key_code::d3, os::key::code::digit3);
    key_map.map(key_code::d4, os::key::code::digit4);
    key_map.map(key_code::d5, os::key::code::digit5);
    key_map.map(key_code::d6, os::key::code::digit6);
    key_map.map(key_code::d7, os::key::code::digit7);
    key_map.map(key_code::d8, os::key::code::digit8);
    key_map.map(key_code::d9, os::key::code::digit9);
    // //  Letters
    key_map.map(key_code::a, os::key::code::a);
    key_map.map(key_code::b, os::key::code::b);
    key_map.map(key_code::c, os::key::code::c);
    key_map.map(key_code::d, os::key::code::d);
    key_map.map(key_code::e, os::key::code::e);
    key_map.map(key_code::f, os::key::code::f);
    key_map.map(key_code::g, os::key::code::g);
    key_map.map(key_code::h, os::key::code::h);
    key_map.map(key_code::i, os::key::code::i);
    key_map.map(key_code::j, os::key::code::j);
    key_map.map(key_code::k, os::key::code::k);
    key_map.map(key_code::l, os::key::code::l);
    key_map.map(key_code::m, os::key::code::m);
    key_map.map(key_code::n, os::key::code::n);
    key_map.map(key_code::o, os::key::code::o);
    key_map.map(key_code::p, os::key::code::p);
    key_map.map(key_code::q, os::key::code::q);
    key_map.map(key_code::r, os::key::code::r);
    key_map.map(key_code::s, os::key::code::s);
    key_map.map(key_code::t, os::key::code::t);
    key_map.map(key_code::u, os::key::code::u);
    key_map.map(key_code::v, os::key::code::v);
    key_map.map(key_code::w, os::key::code::w);
    key_map.map(key_code::x, os::key::code::x);
    key_map.map(key_code::y, os::key::code::y);
    key_map.map(key_code::z, os::key::code::z);
}
}
