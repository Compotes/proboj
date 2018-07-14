#!/usr/bin/env python3
import sys
from functools import total_ordering

from consts import MAPA_NEVIEM

def load_list(load_element):
    size = int(input())
    res = []
    for _ in range(size):
        res.append(load_element())
    return res


class Teren:
    def __init__(self, data):
        self.data = data

    @classmethod
    def from_stdin(self):
        load_int = lambda: int(input())
        load_list_of_int = lambda: load_list(load_int)
        data = load_list(load_list_of_int)
        return Teren(data)

    @classmethod
    def from_compressed(cls):
        width, height, *zakodovany_teren = load_list(lambda: int(input()))
        new_map = [[MAPA_NEVIEM] * width for _ in range(height)]

        x = y = 0
        for i in range(0, len(zakodovany_teren), 2):
            for j in range(zakodovany_teren[i+1]):
                new_map[y][x] = zakodovany_teren[i]
                x += 1
                if x == width:
                    x = 0
                    y += 1
        return Teren(new_map)


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
        self.priblizny_teren = None

    @classmethod
    def from_stdin(cls):
        pocet_hracov = int(input())
        w = int(input())
        h = int(input())
        priblizny_teren = Teren.from_stdin()

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
        hraci = load_list(Hrac.from_stdin)
        manici = load_list(Manik.from_stdin)
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
    def __init__(self, kto, typ, ciel, parameter=0):
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


class Manik:
    def __init__(self, id, x, y, ktoreho_hraca, typ, zlato, zelezo, spenat,
                 kovac_energia):
        self.id = id
        self.x = x
        self.y = y
        self.ktoreho_hraca = ktoreho_hraca
        self.typ = typ
        self.zlato = zlato
        self.zelezo = zelezo
        self.spenat = spenat
        self.kovac_energia = kovac_energia
        self.pozicia = Bod(self.x, self.y)

    @classmethod
    def from_stdin(cls):
        id = int(input())
        x = int(input())
        y = int(input())
        ktoreho_hraca = int(input())
        typ = int(input())
        zlato = int(input())
        zelezo = int(input())
        spenat = int(input())
        kovac_energia = int(input())

        return Manik(
            id, x, y, ktoreho_hraca, typ, zlato, zelezo, spenat, kovac_energia
        )


class Hrac:
    def __init__(self, skore, mapovanie):
        self.skore = skore
        self.mapovanie = mapovanie

    @classmethod
    def from_stdin(cls):
        skore = int(input())
        mapovanie = load_list(lambda: int(input()))
        return Hrac(skore, mapovanie)
