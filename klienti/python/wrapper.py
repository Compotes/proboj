#!/usr/bin/env python3
import sys
from functools import total_ordering


def load_list(load_element):
    size = int(input())
    res = []
    for _ in range(size):
        res.append(load_element())
    return res


class Teren:
    def __init__(self):
        self.data = None

    def load(self):
        load_int = lambda: int(input())
        load_list_of_int = lambda: load_list(load_int)
        self.data = load_list(load_list_of_int)

    def __str__(self):
        flat = [len(self.data)]
        for row in self.data:
            flat.append(len(row))
            flat.extend(col for col in row)

        return '{}'.format(
            '\n'.join(map(str, flat))
        )


class Mapa:
    def __init__(self):
        self.pocet_hracov = None
        self.w = None
        self.h = None
        self.priblizny_teren = Teren()

    def load(self):
        self.pocet_hracov = int(input())
        self.w = int(input())
        self.h = int(input())
        self.priblizny_teren.load()

    def __str__(self):
        return '{}\n{}\n{}\n{}'.format(
            self.pocet_hracov,
            self.w,
            self.h,
            self.priblizny_teren
        )


class Stav:
    def __init__(self, hraci, manici, cas, dalsi_id):
        self.hraci = hraci
        self.manici = manici
        self.cas = cas
        self.dalsi_id = dalsi_id

    @classmethod
    def form_stdin(cls):
        hraci = load_list(lambda: int(input()))
        manici = load_list(lambda: int(input()))
        dalsi_id = int(input())
        cas = int(input())
        return Stav(hraci, manici, cas, dalsi_id)

    def __str__(self):
        return '{}\n{}\n{}\n{}\n{}\n{}'.format(
            len(self.hraci),
            '\n'.join(map(str, self.hraci)),
            len(self.manici),
            '\n'.join(map(str, self.manici)),
            self.dalsi_id,
            self.cas
        )


class Prikaz:
    def __init__(kto, typ, ciel, parameter):
        self.kto = kto
        self.typ = typ
        self.ciel = ciel
        self.parameter = parameter

    def __str__(self):
        return '{}\n{}\n{}\n{}'.format(
            self.kto,
            self.typ,
            self.ciel,
            self.parameter
        )

@total_ordering
class Bod:
    def __init__(self, x=0, y=0):
        self.x = x
        self.y = y

    def __eq__(self, bod):
        return self.x == bod.x and self.y == bod.y

    def __lt__(self, bod):
        return self.y < bod.y or (self.y == bod.y and self.x < bod.x)

    def __str__(self):
        return '{}\n{}'.format(self.x, self.y)
