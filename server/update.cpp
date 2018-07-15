
#include <ostream>
#include <vector>
#include <queue>
#include <set>
#include <algorithm>
#include <cmath>
using namespace std;

#include "common.h"
#include "update.h"

// clockwise zhora
const int DX[] = { 0, 1, 0, -1 };
const int DY[] = { -1, 0, 1, 0 };

const int kUtok[MANIK_POCET_TYPOV][MANIK_POCET_TYPOV] = {
  {  5,  0,  0,  0,  0,  0,  0,  0,  0 },
  { 10, 50, 65,  5, 65, 60, 80, 10,  5 },
  { 80, 30, 40, 20, 75, 70, 80, 50, 25 },
  { 30, 15,  5,  5, 20, 10, 25, 15,  5 },
  { 60, 20, 20,  5, 30, 40, 80, 60,  0 },
  { 50, 45, 40, 15, 70, 25, 70, 20, 10 },
  { 20,  5,  5,  0,  5, 10,  5, 10,  0 },
  { 10,  5,  5,  0,  5, 10,  5, 20,  0 },
  { 30, 10, 10,  0, 30, 20,  5, 30,  0 },
};
const int kTazba[MANIK_POCET_TYPOV] =
  { 50, 0, 10, 0, 0, 0, 0, 0, 0 };
const int kRozhlad[MANIK_POCET_TYPOV] =
  { 6, 6, 6, 6, 12, 6, 6, 6, 6 };
const int kNormalnyKrok[MANIK_POCET_TYPOV] =
  { 1, 1, 1, 1, 1, 1, 1, 0, 0 };
const int kSpenatovyKrok[MANIK_POCET_TYPOV] =
  { 3, 3, 3, 3, 3, 3, 3, 1, 1 };
const int kCenaZlato[MANIK_POCET_TYPOV] =
  { 3, 6, 2, 1, 2, 5, 0, 5, 20 };
const int kCenaZelezo[MANIK_POCET_TYPOV] =
  { 3, 2, 6, 5, 2, 1, 3, 14, 30 };
const int kCenaEnergia[MANIK_POCET_TYPOV] =
  { 3, 6, 5, 6, 7, 6, 2, 8, 50 };

const int kUrodnostNizka = 10;
const int kUrodnostVysoka = 70;

const int kSkoreFrag = 10;
const int kSkoreFragKovac = 200;

const int kPopulacnyLimit = 100;
const int kZiskaneZlato = 7;
const int kZiskaneZelezo = 7;
const int kRychlostKuchtenia = 2;
const int kSkautHladuje = 30;
const int kZaciatocneZlato = 10;
const int kZaciatocneZelezo = 10;

const int kMaximalnaDlzkaHry = 2000;


static ostream* g_observation;
void zapniObservation(ostream* observation) { g_observation = observation; }

#define OBSERVE(s,...) do {                                                    \
    if (!g_observation) break;                                                 \
    *g_observation << (s);                                                     \
    int __m[] = { __VA_ARGS__ };                                               \
    for (unsigned __i = 0; __i < sizeof(__m)/sizeof(*__m); __i++)              \
      *g_observation << " " << __m[__i];                                       \
    *g_observation << endl;                                                    \
  } while(0)


static void spravManika(Stav& stav, Bod p, int hrac, int typ) {
  Manik novy = Manik();   // tento zapis vsetko inicializuje na nuly
  novy.id = (stav.dalsiId++);
  novy.x = p.x;
  novy.y = p.y;
  novy.ktorehoHraca = hrac;
  novy.typ = typ;
  stav.manici.push_back(novy);
}


Stav zaciatokHry(const Mapa& mapa) {
  Stav stav = Stav();   // tento zapis vsetko inicializuje na nuly

  vector<Bod> starty;
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if (mapa.pribliznyTeren.get(x, y) == MAPA_START) {
      starty.push_back(Bod(x, y));
    }
  }
  random_shuffle(starty.begin(), starty.end());

  stav.teren.vyprazdni(mapa.w, mapa.h, MAPA_SUTER);
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    switch (mapa.pribliznyTeren.get(x, y)) {
      case MAPA_VOLNO:
      case MAPA_START:
        stav.teren.set(x, y, MAPA_VOLNO);
        break;
      case MAPA_ZLATO:
      case MAPA_ZELEZO:
        stav.teren.set(x, y, (rand() % 100 < kUrodnostVysoka) ? mapa.pribliznyTeren.get(x, y) : MAPA_SUTER);
        break;
      case MAPA_SUTER:
        stav.teren.set(x, y, (rand() % 100 < kUrodnostNizka) ?
            (rand() % 2 ? MAPA_ZLATO : MAPA_ZELEZO) :
            MAPA_SUTER);
        break;
    }
  }

  for (int i = 0; i < mapa.pocetHracov && i < (int)starty.size(); i++) {
    spravManika(stav, starty[i], i, MANIK_KOVAC);
    stav.manici.rbegin()->zlato = kZaciatocneZlato;
    stav.manici.rbegin()->zelezo = kZaciatocneZelezo;
  }

  stav.hraci.resize(mapa.pocetHracov);
  for (int i = 0; i < mapa.pocetHracov; i++) {
    Hrac& h = stav.hraci[i];

    // zoberieme nahodnu permutaciu, kde mapovanie[i] == 0
    h.mapovanie.resize(mapa.pocetHracov);
    for (int j = 0; j < mapa.pocetHracov; j++) h.mapovanie[j] = j;
    random_shuffle(h.mapovanie.begin() + 1, h.mapovanie.end());
    swap(h.mapovanie[0], h.mapovanie[i]);
  }

  return stav;
}


void prehladajBfs(const Teren& teren, Bod start, Teren& vzdialenost) {
  int inf = teren.w() * teren.h() * 2;
  vzdialenost.vyprazdni(teren.w(), teren.h(), inf);
  queue<Bod> Q;
  vzdialenost.set(start, 0);
  Q.push(start);
  while (!Q.empty()) {
    Bod p = Q.front();
    Q.pop();
    for (int d = 0; d < 4; d++) {
      Bod n(p.x + DX[d], p.y + DY[d]);
      if (teren.get(n) == MAPA_OKRAJ) continue;
      if (vzdialenost.get(n) != inf) continue;
      vzdialenost.set(n, vzdialenost.get(p) + 1);
      if (priechodne(teren.get(n))) Q.push(n);
    }
  }
}


void prehladajLokalneBfs(const Teren& teren, Bod start, int rozsahLimit, map<Bod,int>& vzdialenost) {
  vzdialenost.clear();
  queue<Bod> Q;
  vzdialenost[start] = 0;
  Q.push(start);
  while (!Q.empty()) {
    Bod p = Q.front(); Q.pop();
    if (vzdialenost[p] == rozsahLimit) continue;
    for (int d = 0; d < 4; d++) {
      Bod n(p.x + DX[d], p.y + DY[d]);
      if (teren.get(n) == MAPA_OKRAJ) continue;
      if (vzdialenost.count(n)) continue;
      vzdialenost[n] = vzdialenost[p] + 1;
      if (priechodne(teren.get(n))) Q.push(n);
    }
  }
}


static bool jeMrtvy(const Manik& m) {
  return m.id == -1;
}


#define SPOLOCNE_PRIKAZOVE_KONTROLY_FUJ_MAKRO()                                \
    if (!maniciPodlaId.count(p->kto)) continue;                                \
    Manik &m = stav.manici[maniciPodlaId[p->kto]];                             \
    if (m.ktorehoHraca != hrac) continue;                                      \
    if (uzTahali.count(p->kto)) continue;                                      \
    uzTahali.insert(p->kto);                                                   \


void odsimulujKolo(const Mapa& mapa, Stav& stav, const vector<Odpoved>& akcie) {
  OBSERVE("odsimulujKolo.zacina", stav.cas, stav.cas + 1);
  set<int> uzTahali;
  map<int,int> maniciPodlaId;
  map<Bod,int> plnePolicka;
  vector<int> populacia(stav.hraci.size(), 0);
  FOREACH (it, stav.manici) {
    maniciPodlaId[it->id] = it - stav.manici.begin();
    plnePolicka[it->pozicia()] = it->id;
    populacia[it->ktorehoHraca]++;
  }

  // Vykoname PRIKAZ_KUJ
  for (int hrac = 0; hrac < mapa.pocetHracov; hrac++) {
    FOREACH (p, akcie[hrac]) if (p->typPrikazu == PRIKAZ_KUJ) {
      SPOLOCNE_PRIKAZOVE_KONTROLY_FUJ_MAKRO();
      if (populacia[hrac] >= kPopulacnyLimit) continue;
      if (m.typ != MANIK_KOVAC) continue;
      if (abs(p->ciel.x - m.x) + abs(p->ciel.y - m.y) != 1) continue;
      if (plnePolicka.count(p->ciel)) continue;
      if (!priechodne(stav.teren.get(p->ciel))) continue;
      if (p->parameter < 0 || p->parameter >= MANIK_POCET_TYPOV) continue;
      if (m.zlato < kCenaZlato[p->parameter]) continue;
      if (m.zelezo < kCenaZelezo[p->parameter]) continue;
      if (m.kovacEnergia < kCenaEnergia[p->parameter]) continue;
      OBSERVE("PRIKAZ_KUJ", p->kto, p->ciel.x, p->ciel.y, p->parameter, (int)stav.manici.size());
      m.zlato -= kCenaZlato[p->parameter];
      m.zelezo -= kCenaZelezo[p->parameter];
      m.kovacEnergia = 0;
      plnePolicka[p->ciel] = stav.dalsiId;
      maniciPodlaId[stav.dalsiId] = stav.manici.size();
      populacia[hrac]++;
      spravManika(stav, p->ciel, hrac, p->parameter);
    }
  }

  // Vykoname PRIKAZ_DAJ_*
  for (int hrac = 0; hrac < mapa.pocetHracov; hrac++) {
    FOREACH (p, akcie[hrac]) {
      if (p->typPrikazu != PRIKAZ_DAJ_ZLATO &&
        p->typPrikazu != PRIKAZ_DAJ_ZELEZO &&
        p->typPrikazu != PRIKAZ_DAJ_SPENAT) continue;
      SPOLOCNE_PRIKAZOVE_KONTROLY_FUJ_MAKRO();
      if (abs(p->ciel.x - m.x) + abs(p->ciel.y - m.y) != 1) continue;
      if (p->parameter < 0) continue;
      if (!plnePolicka.count(p->ciel)) continue;
      Manik &d = stav.manici[maniciPodlaId[plnePolicka[p->ciel]]];
      if (p->typPrikazu == PRIKAZ_DAJ_ZLATO && m.zlato >= p->parameter) {
        OBSERVE("PRIKAZ_DAJ_ZLATO", m.id, d.id, p->parameter);
        m.zlato -= p->parameter;
        d.zlato += p->parameter;
      }
      if (p->typPrikazu == PRIKAZ_DAJ_ZELEZO && m.zelezo >= p->parameter) {
        OBSERVE("PRIKAZ_DAJ_ZELEZO", m.id, d.id, p->parameter);
        m.zelezo -= p->parameter;
        d.zelezo += p->parameter;
      }
      if (p->typPrikazu == PRIKAZ_DAJ_SPENAT && m.spenat >= p->parameter) {
        OBSERVE("PRIKAZ_DAJ_SPENAT", m.id, d.id, p->parameter);
        m.spenat -= p->parameter;
        d.spenat += p->parameter;
      }
    }
  }

  // Vykoname PRIKAZ_UTOC
  set<int> mrtviId;
  for (int hrac = 0; hrac < mapa.pocetHracov; hrac++) {
    FOREACH (p, akcie[hrac]) if (p->typPrikazu == PRIKAZ_UTOC) {
      SPOLOCNE_PRIKAZOVE_KONTROLY_FUJ_MAKRO();
      bool dosiahnem = false;
      for (int d = 0; d < 4; d++) {
        if (p->ciel.x == m.x+DX[d] && p->ciel.y == m.y+DY[d]) dosiahnem = true;
        if (m.typ == MANIK_STRELEC &&
            priechodne(stav.teren.get(m.x+DX[d], m.y+DY[d])) &&
            p->ciel.x == m.x+DX[d]*2 && p->ciel.y == m.y+DY[d]*2) {
          dosiahnem = true;
        }
      }
      if (!dosiahnem) continue;
      if (!plnePolicka.count(p->ciel)) {
        OBSERVE("PRIKAZ_UTOC.tazba", p->kto, p->ciel.x, p->ciel.y);
        if (rand() % 100 < kTazba[m.typ]) {
          if (stav.teren.get(p->ciel) == MAPA_SUTER) {
            OBSERVE("teren", p->ciel.x, p->ciel.y, MAPA_VOLNO);
            stav.teren.set(p->ciel, MAPA_VOLNO);
          } else if (stav.teren.get(p->ciel) == MAPA_ZLATO) {
            OBSERVE("teren", p->ciel.x, p->ciel.y, MAPA_VOLNO);
            stav.teren.set(p->ciel, MAPA_VOLNO);
            m.zlato += kZiskaneZlato;
          } else if (stav.teren.get(p->ciel) == MAPA_ZELEZO) {
            OBSERVE("teren", p->ciel.x, p->ciel.y, MAPA_VOLNO);
            stav.teren.set(p->ciel, MAPA_VOLNO);
            m.zelezo += kZiskaneZelezo;
          }
        }
        if (m.typ == MANIK_LOVEC) {
          if (stav.teren.get(p->ciel) == MAPA_VOLNO) {
            OBSERVE("PRIKAZ_UTOC.pasca", p->kto, p->ciel.x, p->ciel.y);
            OBSERVE("teren", p->ciel.x, p->ciel.y, MAPA_PASCA);
            stav.teren.set(p->ciel, MAPA_PASCA);
            mrtviId.insert(m.id);
          }
        }
      } else {
        Manik &d = stav.manici[maniciPodlaId[plnePolicka[p->ciel]]];
        OBSERVE("PRIKAZ_UTOC.utok", m.id, d.id);
        if (rand() % 100 < kUtok[m.typ][d.typ]) {
          OBSERVE("PRIKAZ_UTOC.zabil", m.id, d.id);
          mrtviId.insert(d.id);
          m.zlato += d.zlato / 2;
          m.zelezo += d.zelezo / 2;
          m.spenat += d.spenat / 2;
          d.zlato = d.zelezo = d.spenat = 0;
          if (m.ktorehoHraca != d.ktorehoHraca) {
            stav.hraci[m.ktorehoHraca].skore += (d.typ == MANIK_KOVAC ? kSkoreFragKovac : kSkoreFrag);
          }
        }
      }
    }
  }

  // Upraceme mrtvych
  maniciPodlaId.clear();
  FOREACH(it, stav.manici) {
    if (mrtviId.count(it->id)) {
      it->id = -1;
      plnePolicka.erase(it->pozicia());
    }
  }
  stav.manici.erase(remove_if(stav.manici.begin(), stav.manici.end(), jeMrtvy), stav.manici.end());
  FOREACH (it, stav.manici) {
    maniciPodlaId[it->id] = it - stav.manici.begin();
  }

  // Vykoname PRIKAZ_CHOD
  map<Bod,Bod> chceIst;
  map<Bod,vector<Bod> > semChcePrist;
  set<Bod> mozeIst;
  for (int hrac = 0; hrac < mapa.pocetHracov; hrac++) {
    FOREACH (p, akcie[hrac]) if (p->typPrikazu == PRIKAZ_CHOD) {
      SPOLOCNE_PRIKAZOVE_KONTROLY_FUJ_MAKRO();
      bool dosiahnem = false;
      for (int d = 0; d < 4; d++) {
        for (int k = 1; k <= (m.spenat ? kSpenatovyKrok[m.typ] : kNormalnyKrok[m.typ]); k++) {
          Bod n(m.x+DX[d]*k, m.y+DY[d]*k);
          if (!priechodne(stav.teren.get(n))) break;
          if (p->ciel == n) dosiahnem = true;
          if (plnePolicka.count(n) &&
              stav.manici[maniciPodlaId[plnePolicka[n]]].ktorehoHraca != hrac) break;
        }
      }
      if (!dosiahnem) continue;
      chceIst[m.pozicia()] = p->ciel;
      semChcePrist[p->ciel].push_back(m.pozicia());
    }
  }
  // Vybavime komponenty co koncia cyklom
  int visitId = 0;
  map<Bod,int> visited;
  FOREACH(it, chceIst) {
    Bod pos = it->first;
    while (!visited.count(pos) && chceIst.count(pos)) {
      visited[pos] = visitId;
      pos = chceIst[pos];
    }
    if (visited.count(pos) && visited[pos] == visitId) {
      Bod cyklopos = pos;
      do {
        mozeIst.insert(cyklopos);
        cyklopos = chceIst[cyklopos];
      } while (cyklopos != pos);
    }
    visitId++;
  }
  // Vybavime komponenty co koncia prazdnym polickom
  FOREACH(it, semChcePrist) {
    Bod pos = it->first;
    if (chceIst.count(pos) || plnePolicka.count(pos)) continue;
    while (semChcePrist.count(pos)) {
      pos = semChcePrist[pos][rand() % semChcePrist[pos].size()];
      mozeIst.insert(pos);
    }
  }
  // Konecne vsetkych naozaj pohneme
  FOREACH(it, stav.manici) {
    if (mozeIst.count(it->pozicia())) {
      Bod ciel = chceIst[it->pozicia()];
      int vzdialenost = abs(it->x - ciel.x) + abs(it->y - ciel.y);
      it->x = ciel.x; it->y = ciel.y;
      OBSERVE("PRIKAZ_CHOD", it->id, it->x, it->y);
      if (vzdialenost > kNormalnyKrok[it->typ]) {
        it->spenat--;
        if (it->typ == MANIK_SKAUT && rand() % 100 < kSkautHladuje) it->spenat++;
      }
    }
  }

  // Vybavime pasce
  maniciPodlaId.clear();
  FOREACH(it, stav.manici) {
    if (stav.teren.get(it->pozicia()) == MAPA_PASCA) {
      it->id = -1;
      plnePolicka.erase(it->pozicia());
      OBSERVE("teren", it->x, it->y, MAPA_VOLNO);
      stav.teren.set(it->pozicia(), MAPA_VOLNO);
    }
  }
  stav.manici.erase(remove_if(stav.manici.begin(), stav.manici.end(), jeMrtvy), stav.manici.end());

  // Pridame kovacom energiu a kucharom spenat
  FOREACH(it, stav.manici) {
    if (it->typ == MANIK_KOVAC) it->kovacEnergia++;
    if (it->typ == MANIK_KUCHAR && stav.cas % kRychlostKuchtenia) it->spenat++;
  }

  OBSERVE("odsimulujKolo.konci", stav.cas, stav.cas + 1);
  stav.cas++;
}


void zistiCoVidi(const Stav& stav, int hrac, Teren &viditelne) {
  viditelne.vyprazdni(stav.teren.w(), stav.teren.h(), MAPA_NEVIEM);
  FOREACH(it, stav.manici) if (it->ktorehoHraca == hrac) {
    map<Bod,int> vzdialenost;
    prehladajLokalneBfs(stav.teren, it->pozicia(), kRozhlad[it->typ], vzdialenost);
    FOREACH(itp, vzdialenost) {
      Bod p = itp->first;
      viditelne.set(p, stav.teren.get(p) == MAPA_PASCA ? MAPA_VOLNO : stav.teren.get(p));
    }
  }
}


void zamaskujStav(const Mapa& mapa, const Stav& stav, int hrac, const Teren& viditelne, Stav& novy) {
  const vector<int>& mapovanie = stav.hraci[hrac].mapovanie;

  novy.hraci.resize(mapa.pocetHracov);
  for (int i = 0; i < mapa.pocetHracov; i++) {
    novy.hraci[mapovanie[i]].skore = stav.hraci[i].skore;
  }

  FOREACH(it, stav.manici) {
    if (viditelne.get(it->pozicia()) != MAPA_NEVIEM) {
      novy.manici.push_back(*it);
      novy.manici.rbegin()->ktorehoHraca = mapovanie[it->ktorehoHraca];
    }
  }

  novy.dalsiId = stav.dalsiId;
  novy.cas = stav.cas;
}


void odmaskujOdpoved(const Mapa& mapa, const Stav& stav, int hrac, Odpoved& odpoved) {
  // this function intentionally left blank
}


void zakodujViditelnyTeren(const Teren &vstup, vector<int> &vystup) {
  vystup.clear();
  vystup.push_back(vstup.w());
  vystup.push_back(vstup.h());
  int last = MAPA_OKRAJ;
  for (int y = 0; y < vstup.h(); y++) for (int x = 0; x < vstup.w(); x++) {
    if (vstup.get(x, y) == last) {
      vystup[vystup.size()-1]++;
    } else {
      last = vstup.get(x, y);
      vystup.push_back(last);
      vystup.push_back(1);
    }
  }
}


void dekodujViditelnyTeren(const vector<int> &vstup, Teren &vystup) {
  vystup.vyprazdni(vstup[0], vstup[1], MAPA_NEVIEM);
  int x = 0, y = 0;
  for (unsigned i = 2; i < vstup.size(); i += 2) {
    for (int n = 0; n < vstup[i+1]; n++) {
      vystup.set(x, y, vstup[i]);
      x++;
      if (x == vstup[0]) {
        x = 0;
        y++;
      }
    }
  }
}


vector<int> ktoriZiju(const Mapa& mapa, const Stav& stav) {
  set<int> zijuci;
  FOREACH(it, stav.manici) {
    zijuci.insert(it->ktorehoHraca);
  }
  return vector<int>(zijuci.begin(), zijuci.end());
}


bool hraSkoncila(const Mapa& mapa, const Stav& stav) {
  set<int> zijuciKovaci;
  FOREACH(it, stav.manici) {
    if (it->typ == MANIK_KOVAC) {
      zijuciKovaci.insert(it->ktorehoHraca);
    }
  }
  return zijuciKovaci.size() <= 1 || stav.cas >= kMaximalnaDlzkaHry;
}


int zistiSkore(const Stav& stav, int hrac) {
  return stav.hraci[hrac].skore;
}
