/*!
 * @copyright
 * Copyright (c) 2015, Tymoteusz Blazejczyk
 *
 * @copyright
 * All rights reserved.
 *
 * @copyright
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * @copyright
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * @copyright
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * @copyright
 * * Neither the name of json-cxx nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * @copyright
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file json/rpc/list.cpp
 *
 * @brief JSON double-linked list interface
 * */

#include <json/rpc/list.hpp>

using namespace json::rpc;

void List::push(ListItem* pitem) {
    if (nullptr == pitem) { return; }

    if (nullptr == mp_first) {
        pitem->mp_prev = nullptr;
        pitem->mp_next = nullptr;
        mp_first = pitem;
        mp_last = pitem;
    }
    else {
        mp_last->mp_next = pitem;
        pitem->mp_prev = mp_last;
        pitem->mp_next = nullptr;
        mp_last = pitem;
    }
}

ListItem* List::remove(ListItem* pitem) {
    if (nullptr == pitem) { return nullptr; }

    if ((pitem == mp_first) && (pitem == mp_last)) {
        mp_first = nullptr;
        mp_last = nullptr;
    }
    else if (pitem == mp_first) {
        mp_first = pitem->mp_next;
    }
    else if (pitem == mp_last) {
        mp_last = pitem->mp_prev;
    }
    else if ((nullptr != pitem->mp_prev) && (nullptr != pitem->mp_next)) {
        ListItem* pafter = pitem->mp_next;
        ListItem* pbefore = pitem->mp_prev;

        pafter->mp_prev = pbefore;
        pbefore->mp_next = pafter;
    }

    pitem->mp_prev = nullptr;
    pitem->mp_next = nullptr;

    return pitem;
}

void List::splice(List& list) {
    if (nullptr != list.mp_first) {
        if (nullptr != mp_first) {
            mp_last->mp_next = list.mp_first;
            mp_last = list.mp_last;
        }
        else {
            mp_first = list.mp_first;
            mp_last = list.mp_last;
        }
        list.clear();
    }
}
