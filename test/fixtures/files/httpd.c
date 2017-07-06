#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/filio.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define REGISTRO 0
#define PREGUNTOMUS 1
#define DESCARTE 2
#define GRANDE   3
#define CHICA    4
#define PARES    5
#define JUEGO    6
#define PUNTO    7


struct sMensaje {
	char		id;
	union TipoMensaje {
		struct C_CREAPART {
			char		NombrePartida[16];
			char		Vacas;
			char		JuegosPorVaca;
			char		PuntosPorJuego[2];
			char		Privada;
			char		Jugador   [16];
			char		Companero [16];
			char		Jugador1  [16];
			char		Jugador3  [16];
		}		c_CreaPart;
		struct RESPART {
			char		SeAcepta;
			char		Texto     [32];
		}		ResPart;
		struct C_APUNPART {
			char		NombrePartida[16];
			char		NombreJugador[16];
		}		c_ApunPart;
		struct PARTCOMP {
			char		Vacas;
			char		JuegosPorVaca;
			char		PuntosPorJuego[2];
			char		NumeroJugador;
			char		Jugador0  [16];
			char		Jugador1  [16];
			char		Jugador2  [16];
			char		Jugador3  [16];
		}		PartComp;
		struct C_CHAT {
			char		Texto     [64];
		}		c_Chat;
		struct S_CHAT {
			char		Jugador   [16];
			char		Texto     [64];
		}		s_Chat;
		struct C_PARTCANC {
			char		nada;
		}		c_PartCanc;
		struct S_PARTCANC {
			char		Texto     [64];
		}		s_PartCanc;
	}		a;
	char		SaltodeLinea;
}              *Mensaje;


struct sPartida {
	char		NombrePartida[17];
	int		EsPublica;
	char		Jugadores [4][17];
	int		Canutos    [4];
	int		CuantasVacas;
	int		JuegosPorVaca;
	int		PuntosPorJuego;
	int		BazaActual;
	int		PuntosActual[2];
	int		JuegosActual[2];
	int		VacasActual[2];
	int		Envidado;
	int		HayOrdago;
	int		PorSumar;
	int		Mano;
	int		QuienHabla;
	int		QuienEnvido;
	int		EnvidadoFinal[8];
	int		QuienGanoPares;
	int		QuienGanoJuego;
	int		Cartas     [4][4];
	int		Mazo       [40];
	int		PorDondeVaElMazo;
	time_t		UltimaAccion;
	int		Anterior;
	int		Siguiente;
}              *dummyya;



#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

long		bloquea = 1;

int		MaxPartidas;
int		PuertoWeb;
int		PuertoMus;
int		PosPrimeraPartida = -1;
struct sPartida *Partidas;
int		NumPartidas;
int		SocketWeb , SocketMus;
int		SirveWeb = 0;
#define MAXWEBPEND 200
#define MAXMUSPEND 200
#define TIMEOUT 600
int		SocketWebPendientes[MAXWEBPEND];
int		CuantosWebPendientes = 0;
int		SocketMusPendientes[MAXMUSPEND];
int		CuantosMusPendientes = 0;

char           *CabeceraHTTP = "HTTP/1.0 200 OK\nContent-Type: text/html\n\n<html><head><title>test</title></head><body><H1><Center> HELLO</H1><HR></Center>";
char           *FinalHTTP = "<br><hr></center></body></html>";
char		Buffer    [65536];
char		Buffer2   [20480];
char		BufferEnv [1024];
int		one = 1;


void 
Envia(int Canuto, char *Texto)
{
	memset(BufferEnv, ' ', sizeof(struct sMensaje));
	memcpy(BufferEnv, Texto, strlen(Texto));
	BufferEnv[sizeof(struct sMensaje) - 1] = '\n';
	send(Canuto, BufferEnv, sizeof(struct sMensaje), MSG_NOSIGNAL);
}

void 
EliminaPartida(int Pos)
{
	int		j;
	fprintf(stderr,"Partida eliminada %d [%s]\n", Pos, Partidas[Pos].NombrePartida);
	if (PosPrimeraPartida == Pos)
		PosPrimeraPartida = Partidas[Pos].Siguiente;

	if (Partidas[Pos].Anterior > -1)
		Partidas[Partidas[Pos].Anterior].Siguiente = Partidas[Pos].Siguiente;
	if (Partidas[Pos].Siguiente > -1)
		Partidas[Partidas[Pos].Siguiente].Anterior = Partidas[Pos].Anterior;

	for (j = 0; j < 4; j++) {
		close(Partidas[Pos].Canutos[j]);
		Partidas[Pos].Canutos[j] = -1;
		Partidas[Pos].Jugadores[j][0] = 0;
	}
	Partidas[Pos].NombrePartida[0] = 0;
	NumPartidas--;
}


void 
PreguntaAccion(int Pos)
{
	int		j;


	if (Partidas[Pos].BazaActual > PUNTO || Partidas[Pos].BazaActual < GRANDE)
		return;

	sprintf(Buffer, "A%1d%1d%1d%1d%1d%1d%1d%2d%2d", Partidas[Pos].QuienHabla, Partidas[Pos].BazaActual - 2,
		Partidas[Pos].Envidado > 0 ? 0 : 1, Partidas[Pos].Envidado > 0 && Partidas[Pos].HayOrdago == 0 ? 1 : 0,
		Partidas[Pos].Envidado > 0 ? 1 : 0, Partidas[Pos].HayOrdago == 0 ? 1 : 0, Partidas[Pos].Envidado,
		Partidas[Pos].PorSumar);
	for (j = 0; j < 4; j++)
		Envia(Partidas[Pos].Canutos[j], Buffer);
}

void 
MensajeTrasAccion(int Quien, char Mensaje, int Envidado, int Pos)
{
	int		j;
	char		MiBuffer  [20];

	sprintf(MiBuffer, "E%1d%c%2d%2d%2d", Quien, Mensaje, Envidado, Partidas[Pos].PuntosActual[0],
		Partidas[Pos].PuntosActual[1]);
	for (j = 0; j < 4; j++)
		Envia(Partidas[Pos].Canutos[j], MiBuffer);
}


void 
NuevasCartas(int Pos)
{
	int		i         , k, temp;

	for (i = 1; i <= 40; i++)
		Partidas[Pos].Mazo[i - 1] = i;
	for (i = 1; i <= 40; i++) {
		k = random() % 40;
		temp = Partidas[Pos].Mazo[k];
		Partidas[Pos].Mazo[k] = Partidas[Pos].Mazo[i - 1];
		Partidas[Pos].Mazo[i - 1] = temp;
	}
	for (i = 0; i < 4; i++) {
		Partidas[Pos].Cartas[i][0] = Partidas[Pos].Mazo[i * 4 + 0];
		Partidas[Pos].Cartas[i][1] = Partidas[Pos].Mazo[i * 4 + 1];
		Partidas[Pos].Cartas[i][2] = Partidas[Pos].Mazo[i * 4 + 2];
		Partidas[Pos].Cartas[i][3] = Partidas[Pos].Mazo[i * 4 + 3];
	}
	Partidas[Pos].PorDondeVaElMazo = 16;
}


int 
TieneUnoPares(int Pos, int Jugador)
{
	int		j         , k, u, tCarta[4];

	for (k = 0; k < 4; k++) {
		tCarta[k] = (Partidas[Pos].Cartas[Jugador][k] % 10) * 10;
		if (tCarta[k] == 30 || tCarta[k] == 0)
			tCarta[k] = 100;
		if (tCarta[k] == 20)
			tCarta[k] = 10;
	}
	for (k = 0; k < 3; k++)
		for (u = k + 1; u < 4; u++)
			if (tCarta[k] == tCarta[u])
				return 1;
	return 0;
}


int 
ValePares(int Pos, int Pareja)
{
	int		j         , k, u, tCarta[4], Puntos[2];

	Puntos[0] = Puntos[1] = 0;
	for (j = 0; j < 2; j++) {
		for (k = 0; k < 4; k++) {
			tCarta[k] = (Partidas[Pos].Cartas[Pareja + (j * 2)][k] % 10) * 10;
			if (tCarta[k] == 30 || tCarta[k] == 0)
				tCarta[k] = 100;
			if (tCarta[k] == 20)
				tCarta[k] = 10;
		}

		if (tCarta[0] == tCarta[1] && tCarta[1] == tCarta[2] && tCarta[2] == tCarta[3])
			Puntos[j] = 3;
		else if ((tCarta[0] == tCarta[1] && tCarta[2] == tCarta[3]) ||
			(tCarta[0] == tCarta[2] && tCarta[1] == tCarta[3]) ||
			 (tCarta[0] == tCarta[3] && tCarta[1] == tCarta[2]))
			Puntos[j] = 3;
		else if (tCarta[0] == tCarta[1] && tCarta[1] == tCarta[2])
			Puntos[j] = 2;
		else if (tCarta[0] == tCarta[1] && tCarta[1] == tCarta[3])
			Puntos[j] = 2;
		else if (tCarta[2] == tCarta[3] && tCarta[1] == tCarta[2])
			Puntos[j] = 2;
		else if (tCarta[2] == tCarta[3] && tCarta[0] == tCarta[2])
			Puntos[j] = 2;
		else
			for (k = 0; k < 3; k++)
				for (u = k + 1; u < 4; u++)
					if (tCarta[k] == tCarta[u])
						Puntos[j] = 1;
	}
	return Puntos[0] + Puntos[1];
}


int 
TieneUnoJuego(int Pos, int Jugador)
{
	int		j         , k, u, tCarta[4], Puntos;
	Puntos = 0;
	for (k = 0; k < 4; k++) {
		switch (Partidas[Pos].Cartas[Jugador][k] % 10) {
		case 8:
		case 9:
		case 0:
		case 3:
			Puntos += 10;
			break;
		case 7:
			Puntos += 7;
			break;
		case 6:
			Puntos += 6;
			break;
		case 5:
			Puntos += 5;
			break;
		case 4:
			Puntos += 4;
			break;
		case 1:
		case 2:
			Puntos += 1;
			break;
		}
	}
	return (Puntos > 30 ? 1 : 0);
}



int 
ValeJuego(int Pos, int Pareja)
{
	int		j         , k, tCarta[4], Puntos[2];

	Puntos[0] = Puntos[1] = 0;
	for (j = 0; j < 2; j++) {
		for (k = 0; k < 4; k++) {
			switch (Partidas[Pos].Cartas[Pareja + (j * 2)][k] % 10) {
			case 8:
			case 9:
			case 0:
			case 3:
				Puntos[j] += 10;
				break;
			case 7:
				Puntos[j] += 7;
				break;
			case 6:
				Puntos[j] += 6;
				break;
			case 5:
				Puntos[j] += 5;
				break;
			case 4:
				Puntos[j] += 4;
				break;
			case 1:
			case 2:
				Puntos[j] += 1;
				break;
			}
		}
		switch (Puntos[j]) {
		case 31:
			Puntos[j] = Puntos[j] = 3;
			break;
		case 32:
		case 40:
		case 39:
		case 38:
		case 37:
		case 36:
		case 35:
		case 34:
		case 33:
			Puntos[j] = 2;
			break;
		default:
			Puntos[j] = 0;
			break;
		}
	}
	if (Puntos[0] + Puntos[1] == 0)
		return 1;
	return Puntos[0] + Puntos[1];
}


void 
FinJuego(int Pos)
{
	int		j;

	if (Partidas[Pos].VacasActual[0] == Partidas[Pos].CuantasVacas ||
	    Partidas[Pos].VacasActual[1] == Partidas[Pos].CuantasVacas) {
		fprintf(stderr,"Partida concluida: %d\n", Pos);
		sprintf(Buffer2, "K ");
		Buffer2[1] = (Partidas[Pos].VacasActual[0] == Partidas[Pos].CuantasVacas ? '1' : '2');
		for (j = 0; j < 4; j++)
			Envia(Partidas[Pos].Canutos[j], Buffer2);
		EliminaPartida(Pos);
		return;
	}
	Partidas[Pos].PuntosActual[0] = Partidas[Pos].PuntosActual[1] = 0;
	if (Partidas[Pos].JuegosActual[0] == Partidas[Pos].JuegosPorVaca ||
	    Partidas[Pos].JuegosActual[0] == Partidas[Pos].JuegosPorVaca)
		Partidas[Pos].JuegosActual[0] = Partidas[Pos].JuegosActual[1] = 0;
	Partidas[Pos].BazaActual = PREGUNTOMUS;
	Partidas[Pos].Envidado = Partidas[Pos].HayOrdago = Partidas[Pos].PorSumar = 0;
	Partidas[Pos].Mano = (Partidas[Pos].Mano + 1) % 4;
	Partidas[Pos].QuienHabla = Partidas[Pos].Mano;
	Partidas[Pos].QuienEnvido = 0;
	for (j = 0; j < 8; j++)
		Partidas[Pos].EnvidadoFinal[j] = 0;
	Partidas[Pos].QuienGanoPares = Partidas[Pos].QuienGanoJuego = 0;
	NuevasCartas(Pos);

	for (j = 0; j < 4; j++) {
		sprintf(Buffer, "I%1d%1d%1d%1d%2d%2d%1d%2d%2d%2d%2d", Partidas[Pos].VacasActual[0], Partidas[Pos].VacasActual[1],
			Partidas[Pos].JuegosActual[0], Partidas[Pos].JuegosActual[1], Partidas[Pos].PuntosActual[0],
			Partidas[Pos].PuntosActual[1], Partidas[Pos].Mano, Partidas[Pos].Cartas[j][0], Partidas[Pos].Cartas[j][1],
		    Partidas[Pos].Cartas[j][2], Partidas[Pos].Cartas[j][3]);
		Envia(Partidas[Pos].Canutos[j], Buffer);
	}


	Buffer[0] = 'Q';
	Buffer[1] = 0;
	Envia(Partidas[Pos].Canutos[Partidas[Pos].Mano], Buffer);
}

void 
FinRonda(int Pos)
{
	int		j         , k, gGrande, gChica, gPares, gJuego;
	int		pGrande   , pChica, pPares, pJuego;
	int		ggJuego   , ggVaca;



	gGrande = gChica = gPares = gJuego = 0;
	pGrande = pChica = pPares = pJuego = 0;
	ggJuego = ggVaca = 0;

	if (Partidas[Pos].EnvidadoFinal[GRANDE] > 0) {
		gGrande = QuienGanaBaza(Pos, GRANDE);
		if (gGrande > 0) {
			pGrande = Partidas[Pos].EnvidadoFinal[GRANDE];
			Partidas[Pos].PuntosActual[gGrande - 1] += pGrande;
		}
	}
	if (Partidas[Pos].PuntosActual[0] < Partidas[Pos].PuntosPorJuego &&
	    Partidas[Pos].PuntosActual[1] < Partidas[Pos].PuntosPorJuego) {
		if (Partidas[Pos].EnvidadoFinal[CHICA] > 0) {
			gChica = QuienGanaBaza(Pos, CHICA);
			if (gChica > 0) {
				pChica = Partidas[Pos].EnvidadoFinal[CHICA];
				Partidas[Pos].PuntosActual[gChica - 1] += pChica;
			}
		}
		if (Partidas[Pos].PuntosActual[0] < Partidas[Pos].PuntosPorJuego &&
		    Partidas[Pos].PuntosActual[1] < Partidas[Pos].PuntosPorJuego) {
			gPares = Partidas[Pos].QuienGanoPares > 0 ? Partidas[Pos].QuienGanoPares : QuienGanaBaza(Pos, PARES);
			if (gPares > 0) {
				pPares = Partidas[Pos].EnvidadoFinal[PARES] + ValePares(Pos, gPares - 1);
				Partidas[Pos].PuntosActual[gPares - 1] += pPares;
			}
			if (Partidas[Pos].PuntosActual[0] < Partidas[Pos].PuntosPorJuego &&
			    Partidas[Pos].PuntosActual[1] < Partidas[Pos].PuntosPorJuego) {
				gJuego = Partidas[Pos].QuienGanoJuego > 0 ? Partidas[Pos].QuienGanoJuego : QuienGanaBaza(Pos, JUEGO);
				if (gJuego == 0)
					gJuego = QuienGanaBaza(Pos, PUNTO);
				pJuego = Partidas[Pos].EnvidadoFinal[JUEGO] + Partidas[Pos].EnvidadoFinal[PUNTO] + ValeJuego(Pos, gJuego - 1);
			}
		}
	}
	if (Partidas[Pos].PuntosActual[0] >= Partidas[Pos].PuntosPorJuego) {
		ggJuego = 1;
		Partidas[Pos].JuegosActual[0]++;
		if (Partidas[Pos].JuegosActual[0] >= Partidas[Pos].JuegosPorVaca) {
			ggVaca = 1;
			Partidas[Pos].VacasActual[0]++;
		}
	} else if (Partidas[Pos].PuntosActual[1] >= Partidas[Pos].PuntosPorJuego) {
		ggJuego = 2;
		Partidas[Pos].JuegosActual[1]++;
		if (Partidas[Pos].JuegosActual[1] >= Partidas[Pos].JuegosPorVaca) {
			ggVaca = 2;
			Partidas[Pos].VacasActual[1]++;
		}
	}
	sprintf(Buffer, "P%2d%2d%2d%2d%2d%2d%2d%2d%2d%2d%2d%2d%2d%2d%2d%2d%1d%2d%1d%2d%1d%2d%1d%1d%2d%1d%1d%1d",
		Partidas[Pos].Cartas[0][0], Partidas[Pos].Cartas[0][1], Partidas[Pos].Cartas[0][2], Partidas[Pos].Cartas[0][3],
		Partidas[Pos].Cartas[1][0], Partidas[Pos].Cartas[1][1], Partidas[Pos].Cartas[1][2], Partidas[Pos].Cartas[1][3],
		Partidas[Pos].Cartas[2][0], Partidas[Pos].Cartas[2][1], Partidas[Pos].Cartas[2][2], Partidas[Pos].Cartas[2][3],
		Partidas[Pos].Cartas[3][0], Partidas[Pos].Cartas[3][1], Partidas[Pos].Cartas[3][2], Partidas[Pos].Cartas[3][3],
		gGrande, pGrande, gChica, pChica, gPares, pPares, ValePares(Pos, gPares), gJuego, pJuego, ValeJuego(Pos, gJuego),
		ggJuego, ggVaca);
	for (j = 0; j < 4; j++)
		Envia(Partidas[Pos].Canutos[j], Buffer);



	if (Partidas[Pos].PuntosActual[0] >= Partidas[Pos].PuntosPorJuego ||
	    Partidas[Pos].PuntosActual[1] >= Partidas[Pos].PuntosPorJuego) {
		FinJuego(Pos);
		return;
	}
	Partidas[Pos].Envidado = Partidas[Pos].HayOrdago = Partidas[Pos].PorSumar = 0;
	for (j = 0; j < 8; j++)
		Partidas[Pos].EnvidadoFinal[j] = 0;
	NuevasCartas(Pos);
	Partidas[Pos].Mano = (Partidas[Pos].Mano + 1) % 4;
	Partidas[Pos].QuienHabla = Partidas[Pos].Mano;
	Partidas[Pos].BazaActual = PREGUNTOMUS;
	Partidas[Pos].QuienGanoPares = Partidas[Pos].QuienGanoJuego = 0;

	for (j = 0; j < 4; j++) {
		sprintf(Buffer, "I%1d%1d%1d%1d%2d%2d%1d%2d%2d%2d%2d", Partidas[Pos].VacasActual[0], Partidas[Pos].VacasActual[1],
			Partidas[Pos].JuegosActual[0], Partidas[Pos].JuegosActual[1], Partidas[Pos].PuntosActual[0],
			Partidas[Pos].PuntosActual[1], Partidas[Pos].Mano, Partidas[Pos].Cartas[j][0], Partidas[Pos].Cartas[j][1],
		    Partidas[Pos].Cartas[j][2], Partidas[Pos].Cartas[j][3]);
		Envia(Partidas[Pos].Canutos[j], Buffer);
	}

	Buffer[0] = 'Q';
	Buffer[1] = 0;
	Envia(Partidas[Pos].Canutos[Partidas[Pos].Mano], Buffer);
}

int 
TenemosPares(int Pos)
{
	if ((TieneUnoPares(Pos, 0) || TieneUnoPares(Pos, 2)) &&
	    (TieneUnoPares(Pos, 1) || TieneUnoPares(Pos, 3)))
		return 1;
	return 0;
}

int 
TenemosJuego(int Pos)
{
	if ((TieneUnoJuego(Pos, 0) || TieneUnoJuego(Pos, 2)) &&
	    (TieneUnoJuego(Pos, 1) || TieneUnoJuego(Pos, 3)))
		return 1;
	return 0;
}

int 
QuienGanaBaza(int Pos, int Baza)
{
	int		j         , k, u, Mayor1, Mayor2, tCarta[4], Sietes, Sotas;
	long		Puntos    [4];

	Puntos[0] = Puntos[1] = Puntos[2] = Puntos[3] = 0;
	switch (Baza) {
	case GRANDE:
		for (j = 0; j < 4; j++) {
			for (k = 0; k < 4; k++) {
				switch (Partidas[Pos].Cartas[j][k] % 10) {
				case 1:
				case 2:
					Puntos[j] += 0;
					break;
				case 4:
					Puntos[j] += 5;
					break;
				case 5:
					Puntos[j] += 25;
					break;
				case 6:
					Puntos[j] += 125;
					break;
				case 7:
					Puntos[j] += 625;
					break;
				case 8:
					Puntos[j] += 3125;
					break;
				case 9:
					Puntos[j] += 15625;
					break;
				case 0:
				case 3:
					Puntos[j] += 78125;
					break;
				}
			}
			if (Partidas[Pos].Mano == j)
				Puntos[j] += 3;
			else if (((Partidas[Pos].Mano + 1) % 4) == j)
				Puntos[j] += 2;
			else if (((Partidas[Pos].Mano + 2) % 4) == j)
				Puntos[j]++;
		}
		Mayor1 = Puntos[0] > Puntos[2] ? 0 : 2;
		Mayor2 = Puntos[1] > Puntos[3] ? 1 : 3;
		if (Puntos[Mayor1] > Puntos[Mayor2])
			return 1;
		return 2;

		break;

	case CHICA:
		for (j = 0; j < 4; j++) {
			for (k = 0; k < 4; k++) {
				switch (Partidas[Pos].Cartas[j][k] % 10) {
				case 0:
				case 3:
					Puntos[j] += 0;
					break;
				case 9:
					Puntos[j] += 5;
					break;
				case 8:
					Puntos[j] += 25;
					break;
				case 7:
					Puntos[j] += 125;
					break;
				case 6:
					Puntos[j] += 625;
					break;
				case 5:
					Puntos[j] += 3125;
					break;
				case 4:
					Puntos[j] += 15625;
					break;
				case 1:
				case 2:
					Puntos[j] += 78125;
					break;
				}
			}
			if (Partidas[Pos].Mano == j)
				Puntos[j] += 3;
			else if (((Partidas[Pos].Mano + 1) % 4) == j)
				Puntos[j] += 2;
			else if (((Partidas[Pos].Mano + 2) % 4) == j)
				Puntos[j]++;
		}
		Mayor1 = Puntos[0] > Puntos[2] ? 0 : 2;
		Mayor2 = Puntos[1] > Puntos[3] ? 1 : 3;
		if (Puntos[Mayor1] > Puntos[Mayor2])
			return 1;
		return 2;

		break;

	case PARES:
		for (j = 0; j < 4; j++) {
			for (k = 0; k < 4; k++) {
				tCarta[k] = (Partidas[Pos].Cartas[j][k] % 10) * 10;
				if (tCarta[k] == 30 || tCarta[k] == 0)
					tCarta[k] = 100;
				if (tCarta[k] == 20)
					tCarta[k] = 10;
			}

			if (tCarta[0] == tCarta[1] && tCarta[1] == tCarta[2] && tCarta[2] == tCarta[3])
				Puntos[j] = 5000 + tCarta[0] + tCarta[1] + tCarta[2] + tCarta[3];
			else if ((tCarta[0] == tCarta[1] && tCarta[2] == tCarta[3]) ||
				 (tCarta[0] == tCarta[2] && tCarta[1] == tCarta[3]) ||
			 (tCarta[0] == tCarta[3] && tCarta[1] == tCarta[1]))
				Puntos[j] = 5000 + tCarta[0] + tCarta[1] + tCarta[2] + tCarta[3];
			else if (tCarta[0] == tCarta[1] && tCarta[1] == tCarta[2])
				Puntos[j] = 2000 + tCarta[0] + tCarta[1] + tCarta[2];
			else if (tCarta[0] == tCarta[1] && tCarta[1] == tCarta[3])
				Puntos[j] = 2000 + tCarta[0] + tCarta[1] + tCarta[3];
			else if (tCarta[2] == tCarta[3] && tCarta[1] == tCarta[2])
				Puntos[j] = 2000 + tCarta[1] + tCarta[2] + tCarta[3];
			else if (tCarta[2] == tCarta[3] && tCarta[0] == tCarta[2])
				Puntos[j] = 2000 + tCarta[0] + tCarta[2] + tCarta[3];
			else
				for (k = 0; k < 3; k++)
					for (u = k + 1; u < 4; u++)
						if (tCarta[k] == tCarta[u])
							Puntos[j] = 1000 + tCarta[k] + tCarta[u];

			if (Partidas[Pos].Mano == j)
				Puntos[j] += 3;
			else if (((Partidas[Pos].Mano + 1) % 4) == j)
				Puntos[j] += 2;
			else if (((Partidas[Pos].Mano + 2) % 4) == j)
				Puntos[j]++;
		}
		if (Puntos[0] < 1000 && Puntos[1] < 1000 && Puntos[2] < 1000 && Puntos[3] < 1000)
			return 0;
		Mayor1 = Puntos[0] > Puntos[2] ? 0 : 2;
		Mayor2 = Puntos[1] > Puntos[3] ? 1 : 3;
		if (Puntos[Mayor1] > Puntos[Mayor2])
			return 1;
		return 2;


		break;

	case JUEGO:
		for (j = 0; j < 4; j++) {
			for (k = 0; k < 4; k++) {
				switch (Partidas[Pos].Cartas[j][k] % 10) {
				case 8:
				case 9:
				case 0:
				case 3:
					Puntos[j] += 10;
					break;
				case 7:
					Puntos[j] += 7;
					break;
				case 6:
					Puntos[j] += 6;
					break;
				case 5:
					Puntos[j] += 5;
					break;
				case 4:
					Puntos[j] += 4;
					break;
				case 1:
				case 2:
					Puntos[j] += 1;
					break;
				}
			}
			switch (Puntos[j]) {
			case 31:
				Puntos[j] = 100;
				break;
			case 32:
				Puntos[j] = 90;
				break;
			case 40:
				Puntos[j] = 80;
				break;
			case 39:
				Puntos[j] = 70;
				break;
			case 38:
				Puntos[j] = 60;
				break;
			case 37:
				Puntos[j] = 50;
				break;
			case 36:
				Puntos[j] = 40;
				break;
			case 35:
				Puntos[j] = 30;
				break;
			case 34:
				Puntos[j] = 20;
				break;
			case 33:
				Puntos[j] = 10;
				break;
			default:
				Puntos[j] = 0;
				break;
			}


			Sietes = Sotas = 0;
			for (k = 0; k < 4; k++) {
				if (Partidas[Pos].Cartas[j][k] % 10 == 7)
					Sietes++;
				if (Partidas[Pos].Cartas[j][k] % 10 == 8)
					Sotas++;
			}

			if (Sietes == 3 && Sotas == 1)
				Puntos[j] += 100;


			if (Partidas[Pos].Mano == j)
				Puntos[j] += 3;
			else if (((Partidas[Pos].Mano + 1) % 4) == j)
				Puntos[j] += 2;
			else if (((Partidas[Pos].Mano + 2) % 4) == j)
				Puntos[j]++;
		}
		if (Puntos[0] < 10 && Puntos[1] < 10 && Puntos[2] < 10 && Puntos[3] < 10)
			return 0;
		Mayor1 = Puntos[0] > Puntos[2] ? 0 : 2;
		Mayor2 = Puntos[1] > Puntos[3] ? 1 : 3;
		if (Puntos[Mayor1] > Puntos[Mayor2])
			return 1;
		return 2;

		break;

	case PUNTO:
		for (j = 0; j < 4; j++) {
			for (k = 0; k < 4; k++) {
				switch (Partidas[Pos].Cartas[j][k] % 10) {
				case 8:
				case 9:
				case 0:
				case 3:
					Puntos[j] += 10;
					break;
				case 7:
					Puntos[j] += 7;
					break;
				case 6:
					Puntos[j] += 6;
					break;
				case 5:
					Puntos[j] += 5;
					break;
				case 4:
					Puntos[j] += 4;
					break;
				case 1:
				case 2:
					Puntos[j] += 1;
					break;
				}
			}
			if (Puntos[j] > 30)
				Puntos[j] = 0;
			else
				Puntos[j] *= 10;

			if (Partidas[Pos].Mano == j)
				Puntos[j] += 3;
			else if (((Partidas[Pos].Mano + 1) % 4) == j)
				Puntos[j] += 2;
			else if (((Partidas[Pos].Mano + 2) % 4) == j)
				Puntos[j]++;
		}
		Mayor1 = Puntos[0] > Puntos[2] ? 0 : 2;
		Mayor2 = Puntos[1] > Puntos[3] ? 1 : 3;
		if (Puntos[Mayor1] > Puntos[Mayor2])
			return 1;
		return 2;

		break;
	}
}

void 
SiguienteBaza(int Pos)
{
	int		j;
	char		cTem      [100];


	Partidas[Pos].BazaActual++;

	Partidas[Pos].Envidado = 0;
	Partidas[Pos].PorSumar = 0;
	Partidas[Pos].QuienHabla = Partidas[Pos].Mano;


	if (Partidas[Pos].BazaActual == PARES) {
		for (j = 0; j < 4; j++) {
			sprintf(cTem, "L%1d%s", j, (TieneUnoPares(Pos, j) ? "Pares SI" : "Pares NO"));
			Envia(Partidas[Pos].Canutos[0], cTem);
			Envia(Partidas[Pos].Canutos[1], cTem);
			Envia(Partidas[Pos].Canutos[2], cTem);
			Envia(Partidas[Pos].Canutos[3], cTem);
		}
	} else if (Partidas[Pos].BazaActual == JUEGO) {
		for (j = 0; j < 4; j++) {
			sprintf(cTem, "L%1d%s", j, (TieneUnoJuego(Pos, j) ? "Juego SI" : "Juego NO"));
			Envia(Partidas[Pos].Canutos[0], cTem);
			Envia(Partidas[Pos].Canutos[1], cTem);
			Envia(Partidas[Pos].Canutos[2], cTem);
			Envia(Partidas[Pos].Canutos[3], cTem);
		}
	}
	if (Partidas[Pos].BazaActual == PARES)
		if (!TenemosPares(Pos))
			SiguienteBaza(Pos);
		else {
			if (TieneUnoPares(Pos, Partidas[Pos].Mano))
				Partidas[Pos].QuienHabla = Partidas[Pos].Mano;
			else if (TieneUnoPares(Pos, (Partidas[Pos].Mano + 1) % 4))
				Partidas[Pos].QuienHabla = (Partidas[Pos].Mano + 1) % 4;
			else
				Partidas[Pos].QuienHabla = (Partidas[Pos].Mano + 2) % 4;
		}


	if (Partidas[Pos].BazaActual == JUEGO)
		if (!TenemosJuego(Pos))
			SiguienteBaza(Pos);
		else {
			if (TieneUnoJuego(Pos, Partidas[Pos].Mano))
				Partidas[Pos].QuienHabla = Partidas[Pos].Mano;
			else if (TieneUnoJuego(Pos, (Partidas[Pos].Mano + 1) % 4))
				Partidas[Pos].QuienHabla = (Partidas[Pos].Mano + 1) % 4;
			else
				Partidas[Pos].QuienHabla = (Partidas[Pos].Mano + 2) % 4;
		}


	if (Partidas[Pos].BazaActual == PUNTO)
		if (TieneUnoJuego(Pos, 0) || TieneUnoJuego(Pos, 1) ||
		    TieneUnoJuego(Pos, 2) || TieneUnoJuego(Pos, 3))
			SiguienteBaza(Pos);


	Partidas[Pos].QuienHabla = Partidas[Pos].Mano;

	if (Partidas[Pos].BazaActual == PARES)
		if (!TieneUnoPares(Pos, Partidas[Pos].Mano)) {
			Partidas[Pos].QuienHabla = (Partidas[Pos].Mano + 1) % 4;
			if (!TieneUnoPares(Pos, (Partidas[Pos].Mano + 1) % 4))
				Partidas[Pos].QuienHabla = (Partidas[Pos].Mano + 2) % 4;
		}
	if (Partidas[Pos].BazaActual == JUEGO)
		if (!TieneUnoJuego(Pos, Partidas[Pos].Mano)) {
			Partidas[Pos].QuienHabla = (Partidas[Pos].Mano + 1) % 4;
			if (!TieneUnoJuego(Pos, (Partidas[Pos].Mano + 1) % 4))
				Partidas[Pos].QuienHabla = (Partidas[Pos].Mano + 2) % 4;
		}
	if (Partidas[Pos].BazaActual > PUNTO)
		FinRonda(Pos);
}



void 
TrataMensaje(int Pos, int Quien, char *Mensaje)
{
	int		j         , k, Temp, s, CartaTira, Salir;
	char		MiBuffer  [100];

	Partidas[Pos].UltimaAccion = time(NULL);

	switch (Mensaje[0]) {
	case 'G':
		sprintf(Buffer2, "J%1d%c", Quien, Partidas[Pos].Jugadores[Quien], Mensaje[1]);
		for (j = 0; j < 4; j++)
			if (Partidas[Pos].Canutos[j] > -1)
				Envia(Partidas[Pos].Canutos[j], Buffer2);
		return;
	case 'F':
		sprintf(Buffer2, "BJugador %d [%s] Abandona. La partida finaliza.", Quien, Partidas[Pos].Jugadores[Quien]);
		for (j = 0; j < 4; j++)
			if (Partidas[Pos].Canutos[j] > -1)
				Envia(Partidas[Pos].Canutos[j], Buffer2);
		EliminaPartida(Pos);
		return;
	case 'M':
		memcpy(Buffer2, Mensaje + 1, 64);
		Buffer2[64] = 0;
		sprintf(Buffer, "L%1d%s", Quien, Buffer2);
		for (j = 0; j < 4; j++)
			if (Partidas[Pos].Canutos[j] > -1)
				Envia(Partidas[Pos].Canutos[j], Buffer);
		break;
	case 'T':
		if (Partidas[Pos].BazaActual == PREGUNTOMUS && Quien == Partidas[Pos].QuienHabla) {
			sprintf(MiBuffer, "X%1d%c", Quien, Mensaje[1]);
			for (j = 0; j < 4; j++)
				Envia(Partidas[Pos].Canutos[j], MiBuffer);
			if (Mensaje[1] == '1') {
				Partidas[Pos].QuienHabla = (Quien + 1) % 4;
				if (Partidas[Pos].QuienHabla == Partidas[Pos].Mano) {
					Partidas[Pos].BazaActual = DESCARTE;
					Envia(Partidas[Pos].Canutos[Partidas[Pos].QuienHabla], "D");
				} else
					Envia(Partidas[Pos].Canutos[Partidas[Pos].QuienHabla], "Q");
			} else {
				Partidas[Pos].QuienHabla = Partidas[Pos].Mano;
				Partidas[Pos].BazaActual = GRANDE;
				Partidas[Pos].Envidado = Partidas[Pos].PorSumar = Partidas[Pos].HayOrdago = 0;
				PreguntaAccion(Pos);
			}
		}
		break;
	case 'U':
		if (Partidas[Pos].BazaActual == DESCARTE && Quien == Partidas[Pos].QuienHabla) {
			if ((Mensaje[1] - '0') <= 0 || (Mensaje[1] - '0') > 4) {
				Envia(Partidas[Pos].Canutos[Quien], "D");
				return;
			}
			for (j = 0; j < (Mensaje[1] - '0'); j++) {
				CartaTira = ((Mensaje[(j * 2) + 2] - '0') * 10) + (Mensaje[(j * 2) + 3] - '0');
				if (CartaTira < 0)
					CartaTira = Mensaje[(j * 2) + 3] - '0';
				if (Partidas[Pos].Cartas[Quien][0] == CartaTira)
					s = 0;
				else if (Partidas[Pos].Cartas[Quien][1] == CartaTira)
					s = 1;
				else if (Partidas[Pos].Cartas[Quien][2] == CartaTira)
					s = 2;
				else if (Partidas[Pos].Cartas[Quien][3] == CartaTira)
					s = 3;
				else {
					Envia(Partidas[Pos].Canutos[Quien], "D");
					return;
				}
			}

			for (j = 0; j < (Mensaje[1] - '0'); j++) {
				CartaTira = ((Mensaje[(j * 2) + 2] - '0') * 10) + (Mensaje[(j * 2) + 3] - '0');
				if (CartaTira < 0)
					CartaTira = Mensaje[(j * 2) + 3] - '0';
				if (Partidas[Pos].Cartas[Quien][0] == CartaTira)
					s = 0;
				else if (Partidas[Pos].Cartas[Quien][1] == CartaTira)
					s = 1;
				else if (Partidas[Pos].Cartas[Quien][2] == CartaTira)
					s = 2;
				else if (Partidas[Pos].Cartas[Quien][3] == CartaTira)
					s = 3;
				Temp = Partidas[Pos].Cartas[Quien][s];
				Partidas[Pos].Cartas[Quien][s] = Partidas[Pos].Mazo[Partidas[Pos].PorDondeVaElMazo];
				Partidas[Pos].Mazo[Partidas[Pos].PorDondeVaElMazo] = Temp;
				Partidas[Pos].PorDondeVaElMazo++;
				if (Partidas[Pos].PorDondeVaElMazo == 40) {
					Partidas[Pos].PorDondeVaElMazo = 16;
					for (s = 16; s < 40; s++) {
						k = (random() % 24) + 16;
						Temp = Partidas[Pos].Mazo[s];
						Partidas[Pos].Mazo[s] = Partidas[Pos].Mazo[k];
						Partidas[Pos].Mazo[k] = Temp;
					}
				}
			}
			sprintf(MiBuffer, "V%1d%c%2d", Quien, Mensaje[1], 40 - Partidas[Pos].PorDondeVaElMazo);
			for (j = 0; j < 4; j++)
				Envia(Partidas[Pos].Canutos[j], MiBuffer);
			sprintf(MiBuffer, "W%2d%2d%2d%2d", Partidas[Pos].Cartas[Quien][0], Partidas[Pos].Cartas[Quien][1],
				Partidas[Pos].Cartas[Quien][2], Partidas[Pos].Cartas[Quien][3]);
			Envia(Partidas[Pos].Canutos[Quien], MiBuffer);
			Partidas[Pos].QuienHabla = (Partidas[Pos].QuienHabla + 1) % 4;
			if (Partidas[Pos].QuienHabla == Partidas[Pos].Mano) {
				Partidas[Pos].BazaActual = PREGUNTOMUS;
				Envia(Partidas[Pos].Canutos[Partidas[Pos].QuienHabla], "Q");
			} else
				Envia(Partidas[Pos].Canutos[Partidas[Pos].QuienHabla], "D");
		}
		break;

	case 'H':
		if (Quien != Partidas[Pos].QuienHabla || Partidas[Pos].BazaActual < GRANDE)
			return;
		if (Mensaje[1] == 'E' && Partidas[Pos].PorSumar) {
			PreguntaAccion(Pos);
			return;
		}
		if (Mensaje[1] == 'S' && Partidas[Pos].PorSumar == 0) {
			PreguntaAccion(Pos);
			return;
		}
		if (Mensaje[1] == 'V' && Partidas[Pos].PorSumar == 0) {
			PreguntaAccion(Pos);
			return;
		}
		if (Mensaje[1] == 'O' && Partidas[Pos].HayOrdago) {
			PreguntaAccion(Pos);
			return;
		}
		switch (Mensaje[1]) {
		case 'P':
			if (Partidas[Pos].Envidado == 0) {
				Partidas[Pos].QuienHabla = (Partidas[Pos].QuienHabla + 1) % 4;
				MensajeTrasAccion(Quien, Mensaje[1], 0, Pos);

				if (Partidas[Pos].BazaActual == PARES && !TieneUnoPares(Pos, Partidas[Pos].QuienHabla)
				    && Partidas[Pos].QuienHabla != Partidas[Pos].Mano) {
					Partidas[Pos].QuienHabla = (Partidas[Pos].QuienHabla + 1) % 4;
					if (Partidas[Pos].BazaActual == PARES && !TieneUnoPares(Pos, Partidas[Pos].QuienHabla)
					    && Partidas[Pos].QuienHabla != Partidas[Pos].Mano)
						Partidas[Pos].QuienHabla = (Partidas[Pos].QuienHabla + 1) % 4;
				}
				if (Partidas[Pos].BazaActual == JUEGO && !TieneUnoJuego(Pos, Partidas[Pos].QuienHabla)
				    && Partidas[Pos].QuienHabla != Partidas[Pos].Mano) {
					Partidas[Pos].QuienHabla = (Partidas[Pos].QuienHabla + 1) % 4;
					if (Partidas[Pos].BazaActual == JUEGO && !TieneUnoJuego(Pos, Partidas[Pos].QuienHabla)
					    && Partidas[Pos].QuienHabla != Partidas[Pos].Mano)
						Partidas[Pos].QuienHabla = (Partidas[Pos].QuienHabla + 1) % 4;
				}
				if (Partidas[Pos].QuienHabla == Partidas[Pos].Mano)
					if (Partidas[Pos].BazaActual >= JUEGO) {
						FinRonda(Pos);
						return;
					} else {
						if (Partidas[Pos].BazaActual <= CHICA)
							Partidas[Pos].EnvidadoFinal[Partidas[Pos].BazaActual] = 1;
						SiguienteBaza(Pos);
					}
				if (Partidas[Pos].BazaActual <= PUNTO)
					PreguntaAccion(Pos);
				else
					return;
			} else {
				Partidas[Pos].QuienHabla = (Partidas[Pos].QuienHabla + 2) % 4;

				Temp = (Partidas[Pos].QuienEnvido + Partidas[Pos].Mano) % 2 ? Partidas[Pos].Mano : Partidas[Pos].Mano + 1;
				if ((Partidas[Pos].QuienHabla == Temp) ||
				    (Partidas[Pos].BazaActual == PARES && !TieneUnoPares(Pos, Partidas[Pos].QuienHabla)) ||
				    (Partidas[Pos].BazaActual == JUEGO && !TieneUnoJuego(Pos, Partidas[Pos].QuienHabla))) {
					Partidas[Pos].PuntosActual[Partidas[Pos].QuienEnvido % 2] += Partidas[Pos].PorSumar;

					MensajeTrasAccion(Quien, Mensaje[1], 0, Pos);
					if (Partidas[Pos].BazaActual == PARES)
						Partidas[Pos].QuienGanoPares = (Partidas[Pos].QuienEnvido % 2) + 1;
					if (Partidas[Pos].BazaActual >= JUEGO)
						Partidas[Pos].QuienGanoJuego = (Partidas[Pos].QuienEnvido % 2) + 1;
					if (Partidas[Pos].PuntosActual[Partidas[Pos].QuienEnvido % 2] >= Partidas[Pos].PuntosPorJuego) {
						FinJuego(Pos);
						return;
					} else {
						MensajeTrasAccion(Quien, Mensaje[1], 0, Pos);
						Partidas[Pos].Envidado = Partidas[Pos].PorSumar = Partidas[Pos].HayOrdago = 0;
						Partidas[Pos].QuienHabla = Partidas[Pos].Mano;
						SiguienteBaza(Pos);
						if (Partidas[Pos].BazaActual > PUNTO)
							return;
					}
				}
				PreguntaAccion(Pos);
			}
			break;

		case 'E':
			MiBuffer[2] = 0;
			MiBuffer[0] = Mensaje[2];
			MiBuffer[1] = Mensaje[3];
			Partidas[Pos].Envidado = atoi(MiBuffer);
			if (Partidas[Pos].Envidado == 0)
				Partidas[Pos].Envidado = 2;
			Partidas[Pos].PorSumar = 1;
			Partidas[Pos].QuienEnvido = Quien;
			MensajeTrasAccion(Quien, Mensaje[1], Partidas[Pos].Envidado, Pos);
			Partidas[Pos].QuienHabla = Partidas[Pos].Mano;

			if ((Quien - Partidas[Pos].Mano) % 2 == 0)
				Partidas[Pos].QuienHabla = (Partidas[Pos].Mano + 1) % 4;
			else
				Partidas[Pos].QuienHabla = Partidas[Pos].Mano;

			if (Partidas[Pos].BazaActual == PARES && !TieneUnoPares(Pos, Partidas[Pos].QuienHabla) ||
			    Partidas[Pos].BazaActual == JUEGO && !TieneUnoJuego(Pos, Partidas[Pos].QuienHabla))
				Partidas[Pos].QuienHabla = (Partidas[Pos].QuienHabla + 2) % 4;
			PreguntaAccion(Pos);
			break;

		case 'S':
			MiBuffer[2] = 0;
			MiBuffer[0] = Mensaje[2];
			MiBuffer[1] = Mensaje[3];
			Partidas[Pos].PorSumar = Partidas[Pos].Envidado;
			Partidas[Pos].Envidado = Partidas[Pos].Envidado + (atoi(MiBuffer) ? atoi(MiBuffer) : 1);
			Partidas[Pos].QuienEnvido = Quien;
			MensajeTrasAccion(Quien, Mensaje[1], atoi(MiBuffer), Pos);
			if ((Quien - Partidas[Pos].Mano) % 2 == 0)
				Partidas[Pos].QuienHabla = (Partidas[Pos].Mano + 1) % 4;
			else
				Partidas[Pos].QuienHabla = Partidas[Pos].Mano;

			if (Partidas[Pos].BazaActual == PARES && !TieneUnoPares(Pos, Partidas[Pos].QuienHabla) ||
			    Partidas[Pos].BazaActual == JUEGO && !TieneUnoJuego(Pos, Partidas[Pos].QuienHabla))
				Partidas[Pos].QuienHabla = (Partidas[Pos].QuienHabla + 2) % 4;
			PreguntaAccion(Pos);
			break;

		case 'O':
			Partidas[Pos].HayOrdago = 1;
			Partidas[Pos].QuienHabla = Partidas[Pos].Mano;
			Partidas[Pos].QuienEnvido = Quien;
			Partidas[Pos].PorSumar = Partidas[Pos].Envidado;
			if (Partidas[Pos].PorSumar == 0)
				Partidas[Pos].PorSumar = 1;
			Partidas[Pos].Envidado = Partidas[Pos].PuntosPorJuego;
			MensajeTrasAccion(Quien, Mensaje[1], 0, Pos);
			if ((Quien - Partidas[Pos].Mano) % 2 == 0)
				Partidas[Pos].QuienHabla = (Partidas[Pos].Mano + 1) % 4;
			else
				Partidas[Pos].QuienHabla = Partidas[Pos].Mano;
			PreguntaAccion(Pos);

			if (Partidas[Pos].BazaActual == PARES && !TieneUnoPares(Pos, Partidas[Pos].QuienHabla) ||
			    Partidas[Pos].BazaActual == JUEGO && !TieneUnoJuego(Pos, Partidas[Pos].QuienHabla))
				Partidas[Pos].QuienHabla = (Partidas[Pos].QuienHabla + 2) % 4;
			break;

		case 'V':
			if (Partidas[Pos].HayOrdago) {
				j = QuienGanaBaza(Pos, Partidas[Pos].BazaActual);
				Partidas[Pos].PuntosActual[j] = Partidas[Pos].PuntosPorJuego;
				MensajeTrasAccion(Quien, Mensaje[1], 0, Pos);
				FinRonda(Pos);
			} else {
				MensajeTrasAccion(Quien, Mensaje[1], 0, Pos);
				Partidas[Pos].EnvidadoFinal[Partidas[Pos].BazaActual] = Partidas[Pos].Envidado;

				SiguienteBaza(Pos);
				if (Partidas[Pos].BazaActual > PUNTO) {
					FinRonda(Pos);
					return;
				}
				PreguntaAccion(Pos);
			}
			break;
		}
		break;
	}
}

void 
PonEnListaWeb(int s)
{
	if (CuantosWebPendientes == MAXWEBPEND) {
		close(s);
		return;
	}
	ioctl(s, FIONBIO, &bloquea);

	SocketWebPendientes[CuantosWebPendientes++] = s;
}

void 
PonEnListaMus(int s)
{
	if (CuantosMusPendientes == MAXMUSPEND) {
		close(s);
		return;
	}
	ioctl(s, FIONBIO, &bloquea);

	SocketMusPendientes[CuantosMusPendientes++] = s;
}

void 
SacaDeListaWeb(int Pos)
{
	int		j;
	if (Pos < 0)
		return;
	for (j = Pos; j < CuantosWebPendientes - 1; j++)
		SocketWebPendientes[j] = SocketWebPendientes[j + 1];
	CuantosWebPendientes--;
}

void 
SacaDeListaMus(int Pos)
{
	int		j;
	if (Pos < 0)
		return;
	for (j = Pos; j < CuantosMusPendientes - 1; j++)
		SocketMusPendientes[j] = SocketMusPendientes[j + 1];
	CuantosMusPendientes--;
}


void 
EmpiezaPartida(int Pos)
{
	int		i         , temp, k;

	Partidas[Pos].BazaActual = PREGUNTOMUS;
	Partidas[Pos].PuntosActual[0] = 0;
	Partidas[Pos].PuntosActual[1] = 0;
	Partidas[Pos].JuegosActual[0] = 0;
	Partidas[Pos].JuegosActual[1] = 0;
	Partidas[Pos].VacasActual[0] = 0;
	Partidas[Pos].VacasActual[1] = 0;
	Partidas[Pos].Envidado = 0;
	Partidas[Pos].HayOrdago = 0;
	Partidas[Pos].PorSumar = 0;
	for (i = 0; i < 8; i++)
		Partidas[Pos].EnvidadoFinal[i] = 0;
	Partidas[Pos].QuienGanoPares = Partidas[Pos].QuienGanoJuego = 0;
	Partidas[Pos].Mano = random() % 4;
	Partidas[Pos].QuienHabla = Partidas[Pos].Mano;


	NuevasCartas(Pos);


	for (i = 0; i < 4; i++) {
		sprintf(Buffer, "C%1d%1d%2d%1d%16s%16s%16s%16s", Partidas[Pos].CuantasVacas, Partidas[Pos].JuegosPorVaca,
			Partidas[Pos].PuntosPorJuego, i, Partidas[Pos].Jugadores[0], Partidas[Pos].Jugadores[1],
		    Partidas[Pos].Jugadores[2], Partidas[Pos].Jugadores[3]);
		Envia(Partidas[Pos].Canutos[i], Buffer);
	}


	for (i = 0; i < 4; i++) {
		sprintf(Buffer, "I00000000%1d%2d%2d%2d%2d", Partidas[Pos].Mano, Partidas[Pos].Cartas[i][0],
			Partidas[Pos].Cartas[i][1], Partidas[Pos].Cartas[i][2], Partidas[Pos].Cartas[i][3]);
		Envia(Partidas[Pos].Canutos[i], Buffer);
	}


	Buffer[0] = 'Q';
	Buffer[1] = 0;
	Envia(Partidas[Pos].Canutos[Partidas[Pos].Mano], Buffer);
}

void 
RespondeHTTPPendiente(int Pos)
{
	int		j         , kk, faltan, tmp;
	char		tmpCad    [64], *p1, *p2;
	FILE           *f;


	Buffer[400] = 0;
	p1 = strstr(Buffer, "GET");
	if (p1 == NULL)
		p1 = strstr(Buffer, "Get");
	if (p1 == NULL)
		p1 = strstr(Buffer, "get");
	if (p1 != NULL) {
		j = 5;
		kk = 0;
		if (j < strlen(p1))
			while (p1[j] != ' ' && p1[j])
				tmpCad[kk++] = p1[j++];
		tmpCad[kk] = 0;
	}
	fprintf(stderr,"Sirve Web [%s]\n", tmpCad);

	if (SirveWeb && strcmp(tmpCad, "/") && tmpCad[0]) {
		if (strstr(tmpCad, ".jar") || strstr(tmpCad, ".class"))
			p1 = "application/x-java";
		else if (strstr(tmpCad, ".jpg") || strstr(tmpCad, ".jpeg"))
			p1 = "image/jpeg";
		else if (strstr(tmpCad, ".gif"))
			p1 = "image/gif";
		else if (strstr(tmpCad, ".txt"))
			p1 = "text/plain";
		else
			p1 = "text/html";

		sprintf(Buffer, "./web/%s", tmpCad);
		f = fopen(Buffer, "rb");
		if (f != NULL) {
			sprintf(Buffer, "HTTP/1.0 200 OK\nContent-type: %s%c%c", p1, 10, 10);
			send(SocketWebPendientes[Pos], Buffer, strlen(Buffer), MSG_NOSIGNAL);
			do {
				j = fread(Buffer, 1, 65535, f);
				if (j)
					send(SocketWebPendientes[Pos], Buffer, j, MSG_NOSIGNAL);
			} while (j == 65535);
		} else {
			strcpy(Buffer, "HTTP/1.0 404 FILE NOT FOUND\nContent-type: text/html\n\n<h1>404 FILE NOT FOUND.</h1>");
			send(SocketWebPendientes[Pos], Buffer, strlen(Buffer), MSG_NOSIGNAL);
		}
		if (f != NULL)
			fclose(f);
	} else {
		send(SocketWebPendientes[Pos], CabeceraHTTP, strlen(CabeceraHTTP), MSG_NOSIGNAL);
		sprintf(Buffer, "Puerto por el que escucha este servidor de partidas: %d <br> Numero maximo de partidas simultaneas: %d<br> Numero de partidas disputandose actualmente: %d<br>", PuertoMus, MaxPartidas, NumPartidas);
		strcat(Buffer, "<br>Listado de partidas en curso:<br><UL>");
		send(SocketWebPendientes[Pos], Buffer, strlen(Buffer), MSG_NOSIGNAL);

		for (j = 0; j < MaxPartidas; j++) {
			if (Partidas[j].NombrePartida[0]) {
				strcpy(Buffer, "<br><LI><B>Nombre Partida: ");
				strcat(Buffer, Partidas[j].NombrePartida);
				faltan = 4;
				for (kk = 0; kk < 4; kk++)
					if (Partidas[j].Canutos[kk] > -1)
						faltan--;
				sprintf(tmpCad, "Faltan %d jugador(es) para completar la partida.", faltan);
				sprintf(Buffer2, "</B></li><LI>Numero de Partida: %d</li><li>Creada por: %s</li> <li>%s es una partida privada.</li><li>%s</LI><br><br>",
					j, Partidas[j].Jugadores[0], Partidas[j].EsPublica ? "NO" : "SI", (Partidas[j].Canutos[1] > -1 &&
													   Partidas[j].Canutos[2] > -1 && Partidas[j].Canutos[3] > -1) ? "Jugadores ya inscritos. Partida en marcha." :
					tmpCad);
				strcat(Buffer, Buffer2);
				send(SocketWebPendientes[Pos], Buffer, strlen(Buffer), MSG_NOSIGNAL);
			}
		}
		send(SocketWebPendientes[Pos], FinalHTTP, strlen(FinalHTTP), MSG_NOSIGNAL);
	}
	close(SocketWebPendientes[Pos]);
	SacaDeListaWeb(Pos);
}


void 
RespondeMusPendiente(int Pos)
{
	int		j         , k, conta;
	char		Cad       [200];
	Mensaje = (struct sMensaje *)Buffer;
	if (Mensaje->id == 'A') {
		strncpy(Buffer2, Mensaje->a.c_ApunPart.NombrePartida, 16);
		Buffer2[16] = 0;
		k = -1;
		j = PosPrimeraPartida;
		while (j >= 0 && k < 0) {
			if (!strncmp(Mensaje->a.c_ApunPart.NombrePartida, Partidas[j].NombrePartida, 16))
				k = j;
			else
				j = Partidas[j].Siguiente;
		}
		if (k == -1) {
			strcpy(Buffer2, "R0Nombre de partida no encontrado");
			Envia(SocketMusPendientes[Pos], Buffer2);
			close(SocketMusPendientes[Pos]);
			SacaDeListaMus(Pos);
			return;
		}
		conta = 0;
		if (!Partidas[k].EsPublica) {
			for (j = 1; j < 4; j++)
				if (!strncmp(Partidas[k].Jugadores[j], Mensaje->a.c_ApunPart.NombreJugador, 16)) {
					Partidas[k].Canutos[j] = SocketMusPendientes[Pos];
					conta = j;
				}
			if (conta) {
				strcpy(Buffer2, "R1");
				Envia(SocketMusPendientes[Pos], Buffer2);
				SacaDeListaMus(Pos);
				if (Partidas[k].Canutos[1] >= 0 && Partidas[k].Canutos[2] >= 0 && Partidas[k].Canutos[3] >= 0)
					EmpiezaPartida(k);
				return;
			} else {
				strcpy(Buffer2, "R0Nombre de jugador no encontrado");
				Envia(SocketMusPendientes[Pos], Buffer2);
				close(SocketMusPendientes[Pos]);
				SacaDeListaMus(Pos);
				return;
			}
		} else {
			conta = 0;
			for (j = 1; j < 4; j++)
				if (Partidas[k].Canutos[j] == -1) {
					conta = 1;
					Partidas[k].Canutos[j] = SocketMusPendientes[Pos];
					strncpy(Partidas[k].Jugadores[j], Mensaje->a.c_ApunPart.NombreJugador, 16);
					Partidas[k].Jugadores[j][16] = 0;
					j = 4;
				}
			if (conta) {
				strcpy(Buffer2, "R1");
				Envia(SocketMusPendientes[Pos], Buffer2);
				SacaDeListaMus(Pos);
				if (Partidas[k].Canutos[0] > -1 && Partidas[k].Canutos[1] > -1 && Partidas[k].Canutos[2] > -1 &&
				    Partidas[k].Canutos[3] > -1)
					EmpiezaPartida(k);
				return;
			} else {
				strcpy(Buffer2, "R0Partida ya completa");
				Envia(SocketMusPendientes[Pos], Buffer2);
				close(SocketMusPendientes[Pos]);
				SacaDeListaMus(Pos);
				return;
			}
		}
	} else if (Mensaje->id == 'N') {
		if (NumPartidas == MaxPartidas) {
			strcpy(Buffer2, "R0No hay sitio para mas partidas  ");
			Envia(SocketMusPendientes[Pos], Buffer2);
			close(SocketMusPendientes[Pos]);
			SacaDeListaMus(Pos);
			return;
		}
		k = 0;
		while (Partidas[k].NombrePartida[0] && k < MaxPartidas)
			k++;
		conta = -1;
		j = PosPrimeraPartida;
		while (j > -1) {
			conta = j;
			j = Partidas[j].Siguiente;
		}
		if (conta > -1)
			Partidas[conta].Siguiente = k;
		else
			PosPrimeraPartida = k;
		Partidas[k].Siguiente = -1;
		Partidas[k].Anterior = conta;
		Partidas[k].EsPublica = (Mensaje->a.c_CreaPart.Privada == '1' ? 0 : 1);
		Partidas[k].Canutos[0] = SocketMusPendientes[Pos];
		Partidas[k].Canutos[1] = Partidas[k].Canutos[2] = Partidas[k].Canutos[3] = -1;
		strncpy(Partidas[k].Jugadores[0], Mensaje->a.c_CreaPart.Jugador, 16);
		Partidas[k].Jugadores[0][16] = 0;
		Partidas[k].CuantasVacas = Mensaje->a.c_CreaPart.Vacas - '0';
		Partidas[k].JuegosPorVaca = Mensaje->a.c_CreaPart.JuegosPorVaca - '0';
		Partidas[k].PuntosPorJuego = (Mensaje->a.c_CreaPart.PuntosPorJuego[0] - '0') * 10 +
			(Mensaje->a.c_CreaPart.PuntosPorJuego[1] - '0');
		if (!Partidas[k].EsPublica) {
			strncpy(Partidas[k].Jugadores[1], Mensaje->a.c_CreaPart.Jugador1, 16);
			strncpy(Partidas[k].Jugadores[2], Mensaje->a.c_CreaPart.Companero, 16);
			strncpy(Partidas[k].Jugadores[3], Mensaje->a.c_CreaPart.Jugador3, 16);
			Partidas[k].Jugadores[1][16] = 0;
			Partidas[k].Jugadores[2][16] = 0;
			Partidas[k].Jugadores[3][16] = 0;
		}
		strncpy(Partidas[k].NombrePartida, Mensaje->a.c_CreaPart.NombrePartida, 16);
		Partidas[k].NombrePartida[16] = 0;
		Partidas[k].UltimaAccion = time(NULL);
		strcpy(Buffer2, "R1");
		Envia(SocketMusPendientes[Pos], Buffer2);
		SacaDeListaMus(Pos);
		printf("Creada Partida %s:[%s] en %d por %s\n", (Partidas[k].EsPublica ? "Publica" : "Privada"), Partidas[k].NombrePartida, k, Partidas[k].Jugadores[0]);
		NumPartidas++;
		return;
	} else {
		close(SocketMusPendientes[Pos]);
		SacaDeListaMus(Pos);
	}
}



int 
main(int argc, char **argv)
{
	int		j         , k, l, p;
	int		PonAyuda, PonAyudb;
	int		NuevoSocket;
	struct sockaddr_in host;
	time_t		Ahora;
	time_t		Ahorb;


	PonAyuda = 0;
	MaxPartidas = 5;
	PuertoWeb = 0;
	PuertoMus = 0;
	k = 0;

	srandom(time(NULL));


	for (j = 0; j < argc; j++) {
		if (argv[j][0] == '-') {
			switch (argv[j][1]) {
			case 'w':
				k = atoi(argv[j + 1]);
				if (k > 0)
					PuertoWeb = k;
				break;
			case 'p':
				k = atoi(argv[j + 1]);
				if (k > 0)
					PuertoMus = k;
				break;
			case 'm':
				k = atoi(argv[j + 1]);
				if (k > 0)
					MaxPartidas = k;
				break;
			case 'd':
				SirveWeb = 1;
				break;
			default:
				PonAyuda = 1;
			}
		}
	}
	if (PonAyuda || !PuertoWeb ) {
		fprintf(stderr, "Usage: httpd -w PuertoWeb\n");
		exit(0);
	}
	Partidas = (struct sPartida *)calloc(MaxPartidas, sizeof(struct sPartida));
	if (Partidas == NULL) {
		printf("Error: Imposible asignar memoria para %d partidas.\n", MaxPartidas);
		exit(-1);
	}
	SocketWeb = socket(AF_INET, SOCK_STREAM, 0);
	if (SocketWeb < 0 ) {
		printf("Error: Imposible asignar recursos de comunicaciones (sockets)\n");
		exit(-2);
	}
	one = 1;
	setsockopt(SocketWeb, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

	if (ioctl(SocketWeb, FIONBIO, &bloquea) < 0 ) {
		printf("Error: Este sistema no me permite usar comunicaciones asincronas.\n");
		exit(-3);
	}
	memset(&host, 0, sizeof(host));
	host.sin_family = AF_INET;
	host.sin_port = htons(PuertoWeb);
	host.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(SocketWeb, (struct sockaddr *)&host, sizeof(host))) {
		printf("Error: Sistema de comunicaciones (bind Web).\n");
		exit(-3);
	}
	if (listen(SocketWeb, 50)) {
		printf("Error: Sistema de comunicaciones (listen Web).\n");
		exit(-4);
	}
	printf("Acepta un maximo de %d partidas simultaneas\n\n", MaxPartidas);
	printf("Puerto información Web: %d\n", PuertoWeb);
	printf("Preparado para recibir peticiones!\n");
	while (1) {
		sleep(1);

		NuevoSocket = accept(SocketWeb, NULL, 0);
		if (NuevoSocket >= 0)
			PonEnListaWeb(NuevoSocket);
		else if (errno != EAGAIN && errno != EWOULDBLOCK) {
			printf("Error: No puedo Seguir escuchando peticiones Web\n.");
			exit(-7);
		}
		for (j = CuantosWebPendientes - 1; j >= 0; j--) {
			k = recv(SocketWebPendientes[j], Buffer, 20480, 0);
			if (k > 0)
				RespondeHTTPPendiente(j);
			else if ((errno != EAGAIN && errno != EWOULDBLOCK) || k == 0) {
				close(SocketWebPendientes[j]);
				SacaDeListaWeb(j);
			}
		}


		Ahora = time(NULL);
		j = PosPrimeraPartida;
		while (j > -1) {
			if (Ahora - Partidas[j].UltimaAccion > TIMEOUT) {
				printf("%d [%s] TimeOut\n", j, Partidas[j].NombrePartida);
				for (k = 0; k < 4; k++)
					if (Partidas[j].Canutos[k] > -1)
						Envia(Partidas[j].Canutos[k], "BTimeOut");
				k = Partidas[j].Siguiente;
				EliminaPartida(j);
				j = k;
			} else {
				l = j;
				j = Partidas[j].Siguiente;
				for (k = 0; k < 4; k++)
					if (Partidas[l].Canutos[k] > -1) {
						p = recv(Partidas[l].Canutos[k], Buffer, sizeof(struct sMensaje), 0);
						if (p > 0)
							TrataMensaje(l, k, Buffer);
						else if (p == 0)
							TrataMensaje(l, k, "F");
					}
			}
		}
	}

	free(Partidas);
	return 1;
}
