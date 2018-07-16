#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <ostream>
#include <queue>
#include <set>
#include <algorithm>
#include <cmath>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"


Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
Teren viditelnyTeren;
Teren objavenyTeren;
vector<Prikaz> prikazy;
set<Bod> starts;
bool rob_banikov = true;
int vlacik = 0;
int pocet_banikov = 0, pocet_banikov_old = 0;
bool vitaz = false;

int kovacX = -1, kovacY = -1;

bool vlacik_cesty(int x, int y){
	for ( auto it = starts.begin(); it != starts.end(); ++it ){
		if(it->x == x || it->y == y) return true;
	}
	return false;
}

void bfs(const Teren& teren, Bod start, Teren& vzdialenost) {
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
      if (teren.get(n) == MAPA_START) continue;
      if (vzdialenost.get(n) != inf) continue;
      vzdialenost.set(n, vzdialenost.get(p) + 1);
      if (teren.get(n) == MAPA_VOLNO) Q.push(n);
    }
  }
}


// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
	Bod start2, start1 = Bod(-1, -1);
	fprintf(stderr, "INIT\n");
	objavenyTeren.vyprazdni(mapa.w, mapa.h, MAPA_NEVIEM);
	for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
		if (mapa.pribliznyTeren.get(x, y) == MAPA_START){
			objavenyTeren.set(x, y, MAPA_START);
			starts.insert(Bod(x, y));
			if(start1.x == -1) start1 = Bod(x, y);
			else start2 = Bod(x, y);
		}
	}
	cerr << "koniec initu\n";
	FOREACH(it, stav.manici) {
		if (it->ktorehoHraca == 0 && it->typ == MANIK_KOVAC) {
			kovacX = it->x; kovacY = it->y;
		}
	}
	if(abs(start1.x - start2.x) + abs(start1.y - start2.y) < 50) {
		rob_banikov = false;
		vlacik = 1;
	}
}


// pomocna funkcia co ked uz vieme kam ten manik chce ist tak ho tam posle
static void chodKuMiestu(const Manik &m, Bod ciel) {
	Teren vzdialenost;
	bfs(objavenyTeren, ciel, vzdialenost);
	int smer = -1;
	for (int d = 0; d < 4; d++) {
		Bod n(m.x + DX[d], m.y + DY[d]);
		if (priechodne(objavenyTeren.get(n)) && vzdialenost.get(n) < vzdialenost.get(m.pozicia())) {
			smer = d;
		}
	}
	if(smer != -1){
		prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, Bod(m.x + DX[smer], m.y + DY[smer])));
		objavenyTeren.set(Bod(m.x + DX[smer], m.y + DY[smer]), MAPA_START);
	}else{
		for (int d = 0; d < 4; d++) {
		Bod n(m.x + DX[d], m.y + DY[d]);
		if (priechodne(objavenyTeren.get(n)) && abs(n.x - ciel.x) + abs(n.y - ciel.y) <= abs(m.x - ciel.x) + abs(m.y - ciel.y)) {
			smer = d;
		}
	}
	if(smer != -1){
		prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, Bod(m.x + DX[smer], m.y + DY[smer])));
		objavenyTeren.set(Bod(m.x + DX[smer], m.y + DY[smer]), MAPA_START);
	}


	else {
		if(m.x > ciel.x) {
			if(priechodne(objavenyTeren.get(m.x-1, m.y))){
				prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, m.x-1, m.y));
			} else {
				prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, m.x-1, m.y));
			}
		} else if(m.x < ciel.x) {
			if(priechodne(objavenyTeren.get(m.x+1, m.y))){
				prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, m.x+1, m.y));
			} else {
				prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, m.x+1, m.y));
			}
		} else {
			if(m.y > ciel.y) {
				if(priechodne(objavenyTeren.get(m.x, m.y-1))){
					prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, m.x, m.y-1));
				} else {
					prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, m.x, m.y-1));
				}
			} else if(m.y < ciel.y) {
				if(priechodne(objavenyTeren.get(m.x, m.y+1))){
					prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, m.x, m.y+1));
				} else {
					prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, m.x, m.y+1));
				}
			}
		}
	}
}
}

void coRobiVlacik(const Manik &m) {
	for (int d = 0; d < 4; d++) {
		int nx = m.x + DX[d], ny = m.y + DY[d];
		FOREACH(it, stav.manici) if(it->ktorehoHraca != 0 && it->x == nx && it->y == ny && (vitaz || it->typ != MANIK_KOVAC)){
			prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, nx, ny));
			return;
		}
		// ak som hned vedla kovaca a mam mu co dat, dam mu to.
		if (nx == kovacX && ny == kovacY && m.zlato) {
			prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZLATO, nx, ny, m.zlato));
			return;
		}
		if (nx == kovacX && ny == kovacY && m.zelezo) {
			prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZELEZO, nx, ny, m.zelezo));
			return;
		}
	}

	// ak som uz vytazil vela surovin, idem nazad za kovacom.
	if ((m.zlato + m.zelezo >= 60) && kovacX != -1) {
		chodKuMiestu(m, Bod(kovacX, kovacY));
		return;
	}
	vector <Bod> possible;
	int bestdist = mapa.w * mapa.h;
	for ( auto it = starts.begin(); it != starts.end(); ++it ){
		if(abs(it->x-m.x)+abs(it->y-m.y) < bestdist){
			possible.clear();
			possible.push_back(Bod(it->x, it->y));
			bestdist = abs(it->x-m.x)+abs(it->y-m.y);
		}else if(abs(it->x-m.x)+abs(it->y-m.y) == bestdist) possible.push_back(Bod(it->x, it->y));
	}
	if (!possible.empty()) {
		Bod bestp = possible[rand()%possible.size()];
		if (abs(bestp.x - m.x) + abs(bestp.y - m.y) == 1) {
			FOREACH(it, stav.manici) if(it->ktorehoHraca != 0 && it->x == bestp.x && it->y == bestp.y ){
				if(vitaz || it->typ != MANIK_KOVAC)  prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, bestp));
				return;
			}
			starts.erase(Bod(bestp.x, bestp.y));
		}
		// inak sa k nemu priblizim
		chodKuMiestu(m, bestp);
		return;
	}

}


void coRobiKovac(const Manik &m) {
	// hlupy klient proste furt stavia banikov kolko moze...
	kovacX = m.x;
	kovacY = m.y;
	int d = rand() % 4;
	int vyrabam = rand()%7;
	if(rob_banikov){
		if(pocet_banikov_old < 3 || vyrabam < 4) {
			for(int i = 0; i < 4; i++){
				int counter = 0;
				FOREACH(it, stav.manici) {
					if (it->x == (m.x + DX[(i+d)%4]) && it->y == (m.y + DY[(i+d)%4])) {
						counter++;
						break;
					}
				}
				if(!counter){
					prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + DX[(i+d)%4], m.y + DY[(i+d)%4], MANIK_BANIK));
				}
			}
		} else {
			for(int i = 0; i < 4; i++){
				int counter = 0;
				FOREACH(it, stav.manici) {
					if (it->x == (m.x + DX[(i+d)%4]) && it->y == (m.y + DY[(i+d)%4])) {
						counter++;
						break;
					}
				}
				if(!counter){
					prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + DX[(i+d)%4], m.y + DY[(i+d)%4], MANIK_STRAZNIK));
				}
			}
		}
	} else if(vyrabam < 2 || vlacik) {
		vector <Bod> possible;
		int bestdist = mapa.w * mapa.h;
		for ( auto it = starts.begin(); it != starts.end(); ++it ){
			if(abs(it->x-m.x)+abs(it->y-m.y) < bestdist){
				possible.clear();
				possible.push_back(Bod(it->x, it->y));
				bestdist = abs(it->x-m.x)+abs(it->y-m.y);
			}else if(abs(it->x-m.x)+abs(it->y-m.y) == bestdist) possible.push_back(Bod(it->x, it->y));
		}
		bestdist = mapa.w * mapa.h;
		if (!possible.empty()) {
			Bod bestp = possible[rand()%possible.size()];
			int d = rand() % 4;
			for(int i = 0; i < 4; i++){
				if (abs(bestp.x-(m.x + DX[(i+d)%4]))+abs(bestp.y-(m.y + DY[(i+d)%4]) < bestdist)) {
					prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, (m.x + DX[(i+d)%4]), (m.y + DY[(i+d)%4]), MANIK_MLATIC));
					//vlacik--;
					break;
				}
			}

		}
	} else if(vyrabam < 5){
		for(int i = 0; i < 4; i++){
			int counter = 0;
			FOREACH(it, stav.manici) {
				if (it->x == (m.x + DX[(i+d)%4]) && it->y == (m.y + DY[(i+d)%4])) {
					counter++;
					break;
				}
			}
			if(!counter){
				prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + DX[(i+d)%4], m.y + DY[(i+d)%4], MANIK_SEKAC));
			}
		}
	} else {
		for(int i = 0; i < 4; i++){
			int counter = 0;
			FOREACH(it, stav.manici) {
				if (it->x == (m.x + DX[(i+d)%4]) && it->y == (m.y + DY[(i+d)%4])) {
					counter++;
					break;
				}
			}
			if(!counter){
				prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + DX[(i+d)%4], m.y + DY[(i+d)%4], MANIK_STRAZNIK));
			}
		}
	}
}

void coRobiUtok(const Manik &m) {
	for (int d = 0; d < 4; d++) {
		int nx = m.x + DX[d], ny = m.y + DY[d];
		FOREACH(it, stav.manici) if(it->ktorehoHraca != 0 && it->x == nx && it->y == ny &&(vitaz || it->typ != MANIK_KOVAC)){
			prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, nx, ny));
			return;
		}
		// ak som hned vedla kovaca a mam mu co dat, dam mu to.
		if (nx == kovacX && ny == kovacY && m.zlato) {
			prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZLATO, nx, ny, m.zlato));
			return;
		}
		if (nx == kovacX && ny == kovacY && m.zelezo) {
			prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZELEZO, nx, ny, m.zelezo));
			return;
		}
	}

	// ak som uz vytazil vela surovin, idem nazad za kovacom.
	if ((m.zlato + m.zelezo >= 60) && kovacX != -1) {
		chodKuMiestu(m, Bod(kovacX, kovacY));
		return;
	}
	Teren vzdialenost;
	bfs(objavenyTeren, m.pozicia(), vzdialenost);
	vector <Bod> possible;
	int bestdist = mapa.w * mapa.h;

	FOREACH(it, stav.manici){
		if(it->ktorehoHraca != 0 && (vitaz || it->typ != MANIK_KOVAC)){
			if (vzdialenost.get(it->x, it->y) < bestdist) {
				possible.clear();
				possible.push_back(Bod(it->x, it->y));
				bestdist = vzdialenost.get(it->x, it->y);
			} else if (vzdialenost.get(it->x, it->y) == bestdist){
				possible.push_back(Bod(it->x, it->y));
			}
		}
	}
	if (!possible.empty()) {
		Bod bestp = possible[rand()%possible.size()];
		if (abs(bestp.x - m.x) + abs(bestp.y - m.y) == 1) {
			prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, bestp));
			return;
		}
		// inak sa k nemu priblizim
		chodKuMiestu(m, bestp);
		return;
	}
	bestdist = 0;
	// ak nie, tak idem za najblizsim sutrom a snad niekde nieco najdem...
	for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
		//if (objavenyTeren.get(x, y) == MAPA_VOLNO){
			if (vzdialenost.get(x, y) > bestdist) {
				possible.clear();
				possible.push_back(Bod(x, y));
				bestdist = vzdialenost.get(x, y);
			} else if (vzdialenost.get(x, y) == bestdist){
				possible.push_back(Bod(x, y));
			}

		//}
	}
	if (!possible.empty()) {
		Bod bestp = possible[rand()%possible.size()];
		// inak sa k nemu priblizim
		chodKuMiestu(m, bestp);
		return;
	}
}

void coRobiObrana(const Manik &m) {
	for (int d = 0; d < 4; d++) {
		int nx = m.x + DX[d], ny = m.y + DY[d];
		FOREACH(it, stav.manici) if(it->ktorehoHraca != 0 && it->x == nx && it->y == ny){
			if(vitaz || it->typ != MANIK_KOVAC) prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, nx, ny));
			return;
		}
		// ak som hned vedla kovaca a mam mu co dat, dam mu to.
		if (nx == kovacX && ny == kovacY && m.zlato) {
			prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZLATO, nx, ny, m.zlato));
			return;
		}
		if (nx == kovacX && ny == kovacY && m.zelezo) {
			prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZELEZO, nx, ny, m.zelezo));
			return;
		}
	}

	// ak som uz vytazil vela surovin, idem nazad za kovacom.
	if ((m.zlato + m.zelezo >= 60) && kovacX != -1) {
		chodKuMiestu(m, Bod(kovacX, kovacY));
		return;
	}
	Teren vzdialenost;
	bfs(objavenyTeren, m.pozicia(), vzdialenost);
	vector <Bod> possible;
	int bestdist = 20;

	FOREACH(it, stav.manici){
		if(it->ktorehoHraca != 0 && (vitaz || it->typ != MANIK_KOVAC) && (it->y-kovacY)*(it->y-kovacY)+(it->x-kovacX)*(it->x-kovacX) <= 50){
			if (vzdialenost.get(it->x, it->y) < bestdist) {
				possible.clear();
				possible.push_back(Bod(it->x, it->y));
				bestdist = vzdialenost.get(it->x, it->y);
			} else if (vzdialenost.get(it->x, it->y) == bestdist){
				possible.push_back(Bod(it->x, it->y));
			}
		}
	}
	if (!possible.empty()) {
		Bod bestp = possible[rand()%possible.size()];
		if (abs(bestp.x - m.x) + abs(bestp.y - m.y) == 1) {
			prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, bestp));
			return;
		}
		// inak sa k nemu priblizim
		chodKuMiestu(m, bestp);
		return;
	}
	bestdist = mapa.w * mapa.h;
	// ak nie, tak idem za najblizsim sutrom a snad niekde nieco najdem...
	for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
		if ((objavenyTeren.get(x,y) != MAPA_START || (x == m.x && y == m.y)) && !vlacik_cesty(x,y) && y%3 != 0 && (x-1)%11 != 0 && (y-kovacY)*(y-kovacY)+(x-kovacX)*(x-kovacX) >= 18 && (y-kovacY)*(y-kovacY)+(x-kovacX)*(x-kovacX) <= 36){
			if (vzdialenost.get(x, y) < bestdist) {
				possible.clear();
				possible.push_back(Bod(x, y));
				bestdist = vzdialenost.get(x, y);
			} else if (vzdialenost.get(x, y) == bestdist){
				possible.push_back(Bod(x, y));
			}

		}
	}
	if (!possible.empty()) {
		Bod bestp = possible[rand()%possible.size()];
		// inak sa k nemu priblizim
		chodKuMiestu(m, bestp);
		return;
	}
}


void coRobiBanik(const Manik &m) {

	for (int d = 0; d < 4; d++) {
		int nx = m.x + DX[d], ny = m.y + DY[d];
		// ak som hned vedla zlata alebo zeleza, tazim.
		if (objavenyTeren.get(nx, ny) == MAPA_ZLATO || objavenyTeren.get(nx, ny) == MAPA_ZELEZO) {
			prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, nx, ny));
			return;
		}
		// ak som hned vedla kovaca a mam mu co dat, dam mu to.
		if (nx == kovacX && ny == kovacY && m.zlato) {
			prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZLATO, nx, ny, m.zlato));
			return;
		}
		if (nx == kovacX && ny == kovacY && m.zelezo) {
			prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZELEZO, nx, ny, m.zelezo));
			return;
		}
	}

	// ak som uz vytazil vela surovin, idem nazad za kovacom.
	if ((m.zlato + m.zelezo >= min(stav.cas/20+1,100)) && kovacX != -1) {
		chodKuMiestu(m, Bod(kovacX, kovacY));
		return;
	}

	// ak vidime nejake zlato alebo zelezo, idem k nemu.
	Teren vzdialenost;
	bfs(objavenyTeren, m.pozicia(), vzdialenost);
	int bestdist = 6;
	vector <Bod> possible;

	for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
		if (objavenyTeren.get(x, y) == MAPA_ZLATO || objavenyTeren.get(x, y) == MAPA_ZELEZO) {
			if (vzdialenost.get(x, y) < bestdist) {
				possible.clear();
				possible.push_back(Bod(x, y));
				bestdist = vzdialenost.get(x, y);
			} else if (vzdialenost.get(x, y) == bestdist){
				possible.push_back(Bod(x, y));
			}
		}
	}
	if (!possible.empty()) {
		chodKuMiestu(m, possible[rand()%possible.size()]);
		return;
	}

	bestdist = mapa.w * mapa.h;
	// ak nie, tak idem za najblizsim sutrom a snad niekde nieco najdem...
	for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
		if (objavenyTeren.get(x, y) == MAPA_SUTER && (y%3 == 0 || (x-1)%11 == 0 || (y-kovacY)*(y-kovacY)+(x-kovacX)*(x-kovacX) <= 36 || vlacik_cesty(x,y))){
			if (vzdialenost.get(x, y) < bestdist) {
				possible.clear();
				possible.push_back(Bod(x, y));
				bestdist = vzdialenost.get(x, y);
			} else if (vzdialenost.get(x, y) == bestdist){
				possible.push_back(Bod(x, y));
			}

		}
	}
	if (!possible.empty()) {
		Bod bestp = possible[rand()%possible.size()];
		if (abs(bestp.x - m.x) + abs(bestp.y - m.y) == 1) {
			prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, bestp));
			return;
		}
		// inak sa k nemu priblizim
		chodKuMiestu(m, bestp);
		return;
	}

	/*if ((m.zlato + m.zelezo > 0) && kovacX != -1) {
		chodKuMiestu(m, Bod(kovacX, kovacY));
		return;
	}*/



	bestdist = mapa.w * mapa.h;
	FOREACH(it, stav.manici){
		if(it->typ == MANIK_BANIK && it->id != m.id){
			if (vzdialenost.get(it->x, it->y) < bestdist) {
				possible.clear();
				possible.push_back(Bod(it->x, it->y));
				bestdist = vzdialenost.get(it->x, it->y);
			} else if (vzdialenost.get(it->x, it->y) == bestdist){
				possible.push_back(Bod(it->x, it->y));
			}
		}
	}
	if (!possible.empty()) {
		Bod bestp = possible[rand()%possible.size()];
		if (abs(bestp.x - m.x) + abs(bestp.y - m.y) == 1) {
			prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, bestp));
			return;
		}
		// inak sa k nemu priblizim
		chodKuMiestu(m, bestp);
		return;
	}

}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat,
// co tato funkcia rozhodne pomocou: prikazy.push_back(Prikaz(...));
void zistiTah() {
	// (sem patri vas kod)
	int skore_max = 0;
	for(int i = 1; i < stav.hraci.size(); i++){
		if(stav.hraci[i].skore > skore_max){
			skore_max = zistiSkore(stav, i);
		}
	}
	if(skore_max-200 > zistiSkore(stav, 0)) vitaz = false;
	else vitaz = true;
	fprintf(stderr, "zistiTah zacina %d\n", stav.cas);

	// zapamatame si teren co vidime a doteraz sme nevideli
	for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
		if (viditelnyTeren.get(x, y) != MAPA_NEVIEM) {
			objavenyTeren.set(x, y, viditelnyTeren.get(x, y));
		}
	}
	FOREACH(it, stav.manici) {
		if(it->ktorehoHraca == 0){
			if(it->typ == MANIK_KOVAC || it->typ == MANIK_STRAZNIK) objavenyTeren.set(it->x, it->y, MAPA_START);
			starts.erase(Bod(it->x, it->y));
		} else {
			if(it->typ == MANIK_KOVAC){
				starts.insert(Bod(it->x, it->y));
			}
		}
	}
	// kazdemu nasmu manikovi povieme co ma robit (na to mame pomocne funkcie)
	pocet_banikov = 0;
	FOREACH(it, stav.manici) {
		if (it->ktorehoHraca != 0) continue;
		switch (it->typ) {
			case MANIK_KOVAC:
				coRobiKovac(*it);
				break;

			case MANIK_BANIK:
				coRobiBanik(*it);
				pocet_banikov++;
				break;

			case MANIK_MLATIC:
				coRobiVlacik(*it);
				break;
			case MANIK_SEKAC:
				coRobiUtok(*it);
				break;
			case MANIK_STRAZNIK:
				coRobiObrana(*it);
				break;
		}
	}
	if(pocet_banikov >= 50) rob_banikov = false;
	pocet_banikov_old = pocet_banikov;

	fprintf(stderr, "prikazov %d\n", (int)prikazy.size());
}


int main() {
	// v tejto funkcii su vseobecne veci, nemusite ju menit (ale mozte).

	unsigned int seed = time(NULL) * getpid();
	srand(seed);

	nacitaj(cin, mapa);
	fprintf(stderr, "START pid=%d, seed=%u\n", getpid(), seed);
	inicializuj();

	while (cin.good()) {
		vector<int> zakodovanyTeren;
		nacitaj(cin, zakodovanyTeren);
		dekodujViditelnyTeren(zakodovanyTeren, viditelnyTeren);
		nacitaj(cin, stav);
		prikazy.clear();
		zistiTah();
		uloz(cout, prikazy);
		cout << ".\n" << flush;   // bodka a flush = koniec odpovede
	}

	return 0;
}

