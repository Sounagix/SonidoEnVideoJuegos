#include <fmod.hpp>
#include <fmod_errors.h>
#include <iostream>
#include <thread>
#include <cstdio>
#include <conio.h>
#include <math.h>
#include <chrono>
#include <ctime>   
#include "Source.h"
#include <vector>
#include <iomanip>
#include <windows.h>
#include <list>

#pragma warning(disable : 4996)
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define EXIT 27
#define ENTER 13		
#define ADD 43			
#define SUB 45			
//	Movimiento del listener
#define W 119			
#define A 97			
#define S 115			
#define D 100	
//	Movimiento del source
#define J 106	
#define K 107	
#define L 108	
#define I 105	

#define Z 122
#define X 120
#define C 99
#define V 118
#define B 98
#define N 110
#define M 109	
#define COMA 44	

//	teclas para mover las direcciones de los conos
#define ARRIBA		56
#define ABAJO		50
#define IZQUIERDA	52
#define DERECHA		54

using namespace FMOD;
System* syst;

class BaseSound;
class Wall;

enum soundType
{
	sound3D,
	sound2D,
};

enum orientation {
	forward,
	back,
	left,
	right,
	oNone

};

enum movType {
	listener,
	source,
	mNone
};

// Determina el tipo de Loop con el metodo
enum loop {
	freeLoop = -1,	// loop indefinido, por defecto es el argumento inicializado en función playLoop(int l = -1)
	OnceLoop = 0,	// se reproduce una sola vez	
	ThreeLoop = 2	// se reproduce 3 veces
};

struct distance
{
	orientation d;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};


struct Comp
{
	Source::RutaId r;
	bool loop = false;
	soundType sT;
};


struct WallComp
{
	int nummaxPoligons, maxVertices, x, z;
};



//	Lista de todos los sonidos cargados
std::vector<BaseSound*> playList;
//	index para los efectos 
int selectionH = 0;
//	index para las sonidos cargados
int selectionV = 0;
//	index para el submenu
int subMenu = 0;
//	Si el subMenu está activo
bool subMenuActive = false;

//	Máxima anchura que el emisor se puede desplazar
const int MAX_WIDTH = 20;
const int MIN_WIDTH = -20;
//	Máxima altura que el emisor se puede desplazar
const int MAX_HEIGHT = 10;
const int MIN_HEIGHT = -10;

//Handle
HANDLE console_color;

std::vector<Wall*> muros;



void ERRCHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK) {
		std::cout << FMOD_ErrorString(result) << std::endl;
		exit(-1);
	}
}

void init(FMOD_RESULT res) {
	if (syst == NULL) {

		res = System_Create(&syst);
		ERRCHECK(res);

		res = syst->init(128, FMOD_INIT_NORMAL, 0);
		ERRCHECK(res);
	}
}

//	Sonido base para cualquier sonido que se quiera crear
class BaseSound {
protected:
	//	Sonido a reproducir
	Sound* sonido;
	//	Canal para reproducir
	Channel* canal;
	//	Nombre del sonido
	std::string nombre;
	//	frame rate
	float timeRate = 0.7f;
	//	tiempo actual
	float currTime = 0.0f;
	//
	float lastTime = 0.0f;

public:
	//	Constructor generico
	BaseSound() {};

	//	Destructor generico
	~BaseSound() {
		std::cout << "Base\n";
		sonido->release();
	}

	//	Reproduce un sound 
	virtual void play() {
		FMOD_RESULT res = syst->playSound(sonido, 0, false, &canal);
		ERRCHECK(res);
	}

	// Reproduce un sound y si loop es -1 se reproduce en loop
	// Utiliza el enum loop para gestionar el tipo de loop
	void playLoop(int l = -1) {
		FMOD_RESULT res = syst->playSound(sonido, 0, false, &canal);
		canal->setLoopCount(l);
		ERRCHECK(res);
	}

	//	Setea a un canal como pausa o no en función de un booleano
	void setPause() {
		FMOD_RESULT res;
		bool status = false;
		res = canal->getPaused(&status);
		ERRCHECK(res);
		canal->setPaused(!status);
	}

	//	Cambia el volumen de un sonido *TODO
	void setVolume(float v) {
		canal->setVolume(v);
	}

	//	Para un source
	void stop() {
		if (!isPlaying()) return;
		FMOD_RESULT res;
		res = canal->stop();
		ERRCHECK(res);
	}

	//	Cambia el estado de un canal a mute en función de un bool
	void setMute(bool status) {
		canal->setMute(status);
	}

	//	Cambia el pitch de un canal en función de un valor(float) -> para devolver a un pitch normal -> 1.0f
	void setPitch(float value) {
		canal->setPitch(value);
	}

	////	Cambia el frame rate de un sonido
	//void setFrameRate(float value) {
	//	timeRate = value;
	//}

	virtual bool update(float deltaTime) = 0;

	//	Determina si un sonido se está reproduciendo
	bool isPlaying() {
		bool active;
		canal->isPlaying(&active);
		return active;
	}

	//	Aplica el efecto de fadeIn
	void fadeIn() {
		play();
		unsigned long long parentclock;
		FMOD_RESULT res = canal->getDSPClock(NULL, &parentclock);
		res = canal->addFadePoint(parentclock, 0.0f);
		res = canal->addFadePoint(parentclock + 100000, 1.0f);
	}

	//	Apica el efecto de fadeOut
	void fadeOut(/*unsigned long long value*/) {
		if (!isPlaying()) return;
		unsigned long long parentclock;
		FMOD_RESULT res = canal->getDSPClock(NULL, &parentclock);
		float vol;
		canal->getVolume(&vol);
		res = canal->addFadePoint(parentclock, vol);
		res = canal->addFadePoint(parentclock + 100000, 0.0f);
	}

	// Devuelve el nombre del sonido
	inline std::string getName() {
		return nombre;
	}

	virtual float getEfect(Source::EffectId e) {
		switch (e)
		{
		case Source::EffectId::Pitch:
		{
			float p;
			canal->getPitch(&p);
			return p;
		}
		case Source::EffectId::Volumen:
		{
			float p;
			canal->getVolume(&p);
			return p;
		}
		//case Source::EffectId::Movement:
		//{
		//	Sound3D* s = nullptr;
		//	s = dynamic_cast<Sound3D*>(this);
		//	FMOD_VECTOR v = s->getPosition();
		//	std::cout << v.x << v.y << v.z;
		//	return -1;
		//}
		default:
			break;
		}
	}

	virtual void modEfect(Source::EffectId e, float value) {
		switch (e)
		{
		case Source::EffectId::Pitch:
		{
			float p;
			canal->getPitch(&p);
			p += value;
			canal->setPitch(p);
			break;
		}
		case Source::EffectId::Volumen:
		{
			float v;
			canal->getVolume(&v);
			v += value;
			canal->setVolume(v);
			break;
		}
		default:
			break;
		}
	}

	unsigned int getTime() {
		unsigned int l;
		sonido->getLength(&l, FMOD_TIMEUNIT_MS);
		//unsigned long long parentclock;
		//unsigned int time;
		//FMOD_RESULT res = canal->getPosition(&time, FMOD_TIMEUNIT_MS);
		//std::cout << time;
		return l;
	}
};

class Sound3D : public BaseSound {
private:
	bool movActive = false;
	float angleToRotate = 1.0f;
	float oldDx = 0.0f;
	float oldDz = 0.0f;
	FMOD_VECTOR lastPos;
public:
	Sound3D(const char* ruta, std::string n = "undefined", bool loop = false) {

		nombre = n;
		FMOD_RESULT res;
		if (loop)
			res = syst->createSound(ruta, FMOD_3D | FMOD_LOOP_NORMAL, 0, &sonido);
		else
			res = syst->createSound(ruta, FMOD_3D, 0, &sonido);


		ERRCHECK(res);

		playList.push_back(this);
	}

	virtual ~Sound3D() {
		std::cout << "3D\n";
	}

	//	Cuando se de play a un sound3D con parametros para 
	virtual void play(float in, float out, float outGain) {
		FMOD_RESULT res = syst->playSound(sonido, 0, false, &canal);
		ERRCHECK(res);
		setSourceConeAngle(in, out, outGain);
	}

	//	Setea los conos correspondiente al canal
	void setSourceConeAngle(float insCone = -1.0f, float outCone = -1.0f, float outSideVol = -1.0f) {
		bool playing;
		canal->isPlaying(&playing);
		if (playing) {
			std::tuple<float, float, float> t = getConeInfo();
			if (insCone == -1.0f) {
				insCone = std::get<0>(t);
			}
			if (outCone == -1.0f) {
				outCone = std::get<1>(t);
			}
			if (outSideVol == -1.0f) {
				outSideVol = std::get<2>(t);
			}
			FMOD_RESULT res = canal->set3DConeSettings(insCone, outCone, outSideVol);
			ERRCHECK(res);
		}
	}

	//	Setea la orientación en un espacio 2D sobre un sonido 3D
	void setSourceOrientation(orientation ori) {

		FMOD_VECTOR
			orient;
		switch (ori)
		{
		case forward:
			orient = { 0.0,0.0,1.0 };
			break;
		case back:
			orient = { 0.0,0.0,-1.0 };
			break;
		case left:
			orient = { -1.0,0.0,0.0 };
			break;
		case right:
			orient = { 1.0,0.0,0.0 };
			break;
		default:
			break;
		}
		canal->set3DConeOrientation(&orient);
	}

	//	Setea la posición del source
	void setSourcePos(distance d) {
		FMOD_VECTOR
			pos,
			vel;
		canal->get3DAttributes(&pos, &vel);
		lastPos.x = pos.x;
		lastPos.y = pos.y;
		lastPos.z = pos.z;
		pos.x += d.x;
		pos.y = 0.0f;
		pos.z += d.z;
		canal->set3DAttributes(&pos, &vel);
	}
	//	Devuelve la posición del listener
	FMOD_VECTOR getSourcePos() {
		FMOD_VECTOR
			pos,
			vel;
		canal->get3DAttributes(&pos, &vel);
		return pos;
	}

	//	In , out , outVol
	std::tuple<float, float, float> getConeInfo() {
		FMOD_RESULT res;
		float in, out, outVol;
		res = canal->get3DConeSettings(&in, &out, &outVol);
		ERRCHECK(res);
		return std::tuple<float, float, float>(in, out, outVol);
	}

	FMOD_VECTOR getOrientation() {
		FMOD_VECTOR
			ori;
		canal->get3DConeOrientation(&ori);
		return ori;
	}

	void setDistance(float min, float max) {
		//float minDistance;
		//float maxDistance;
		//if (min == -1.0f) {
		//	canal->get3DMinMaxDistance(&minDistance, &maxDistance);
		//	
		//}
		canal->set3DMinMaxDistance(min, max);
	}

	//	min, max
	std::pair<float, float> getDistance() {
		float minDistance;
		float maxDistance;
		canal->get3DMinMaxDistance(&minDistance, &maxDistance);
		return std::pair<float, float>(minDistance,maxDistance);
	}

	//	TODO faltan cositas
	void rotateAround() {
		angleToRotate += 25.0;
		double radAngle = angleToRotate * (3.14159265359 / 180);

		FMOD_VECTOR
			listPos,
			listVel,
			forw,
			up;

		syst->get3DListenerAttributes(0, &listPos, &listVel, &forw, &up);
		FMOD_VECTOR
			sourcePos,
			sourceVel;
		canal->get3DAttributes(&sourcePos, &sourceVel);
	

		//int d = sqrt(powf((listPos.x - sourcePos.x),2) + powf((listPos.z - sourcePos.z), 2));

		//float distToMoveX = sqrt(powf((listPos.x - sourcePos.x), 2)) / 2; //(listPos.x - sourcePos.x) / 2;
		//float distToMoveY = sqrt(powf((listPos.z - sourcePos.z), 2)) / 2;//(listPos.z - sourcePos.z) / 2;

		//float xP = distToMoveX * cos(radAngle) - distToMoveY * sin(radAngle);
		//float yP = distToMoveY * sin(radAngle) + distToMoveX * cos(radAngle);

		int xP = 5 * cos(radAngle) - 5 * sin(radAngle);
		int yP = 5 * sin(radAngle) + 5 * cos(radAngle);


		FMOD_VECTOR
			posToMove = { xP ,0 ,yP };

		canal->set3DAttributes(&posToMove, &sourceVel);
	}

	virtual bool update(float deltaTime) override {
		//return true;
		if (isPlaying()) {
			FMOD_VECTOR
				pos,
				vel;
			canal->get3DAttributes(&pos, &vel);

			vel.x = (pos.x - lastPos.x) / deltaTime;
			vel.y = (pos.y - lastPos.y) / deltaTime;
			vel.z = (pos.z - lastPos.z) / deltaTime;
			canal->set3DAttributes(&pos,&vel);
		}
		lastTime = deltaTime;
		if (movActive && deltaTime - currTime >= timeRate) {
			currTime = deltaTime;
			rotateAround();
			return false;
		}
		return true;
	};

	void activeMov() {
		movActive = !movActive;
	}

	void setDopplerLevel(float value) {
		float currLevel;
		canal->get3DDopplerLevel(&currLevel);
		currLevel += value;
		if (currLevel >= 0 && currLevel < 6) {
			FMOD_RESULT res = canal->set3DDopplerLevel(currLevel);
			ERRCHECK(res);
		}
	}

	float getDopplerLevel() {
		float lvl;
		canal->get3DDopplerLevel(&lvl);
		return lvl;
	}

	FMOD_VECTOR getVelocity() {
		FMOD_VECTOR
			pos,
			vel;
		canal->get3DAttributes(&pos, &vel);
		return vel;
	}
};

class Sound2D : public BaseSound {
public:
	Sound2D(const char* ruta, std::string n = "undefined", bool loop = false) {
		nombre = n;
		FMOD_RESULT res;
		if (loop)
			res = syst->createSound(ruta, FMOD_2D | FMOD_LOOP_NORMAL, 0, &sonido);
		else
			res = syst->createSound(ruta, FMOD_2D, 0, &sonido);

		ERRCHECK(res);
		playList.push_back(this);
	}
	virtual ~Sound2D() {
		std::cout << "2D\n";
	};

	virtual bool update(float deltaTime) override { return true; };
};

//	TODO
class Reverb {
private:
	Reverb* reverb;
public:
	Reverb() {

	}
};

class Wall {

private:
	Geometry* geo;
public:
	FMOD_VECTOR geoPos;

	Wall(int maxPoligons,int maxVertices){
		FMOD_RESULT res;
		res = syst->createGeometry(maxPoligons, maxVertices, &geo);
		ERRCHECK(res);
		muros.push_back(this);
	};

	void translate(FMOD_VECTOR newPos) {
		FMOD_VECTOR
			pos;
		geo->getPosition(&pos);

		pos.x += newPos.x;
		pos.y += newPos.y;
		pos.z += newPos.z;

		geoPos = pos;

		geo->setPosition(&geoPos);
	}

	void setPosition(FMOD_VECTOR newPos) {
		geoPos = newPos;
		geo->setPosition(&geoPos);
	}
};

orientation toOrientation(FMOD_VECTOR v) {
	if (v.x == 0) {
		if (v.z == 1) {
			return orientation::forward;
		}
		else if (v.z == -1) {
			return orientation::back;
		}
	}
	else if (v.z == 0) {
		if (v.x == 1) {
			return orientation::right;
		}
		else if (v.x == -1) {
			return orientation::left;
		}
	}
	return orientation::oNone;
}

//	Detiene todos los sonidos de la playList
void stopPlaylist() {
	for (auto s : playList) {
		s->stop();
	}
}

//	Setea una posición del listener
void setListenerPos(distance d) {
	FMOD_VECTOR
		pos,
		vel,
		forw,
		upD;

	syst->get3DListenerAttributes(0, &pos, &vel, &forw, &upD);
	pos.x += d.x;
	pos.z += d.z;
	syst->set3DListenerAttributes(0, &pos, &vel, &forw, &upD);
}

//	Devuelve la posición del listener
FMOD_VECTOR getListenerPos() {
	FMOD_VECTOR
		pos,
		vel,
		forw,
		up;

	syst->get3DListenerAttributes(0, &pos, &vel, &forw, &up);
	return pos;
}

//	Convierte los milisegundos en tiempo standart
void elapsedTime(unsigned int t) {
	int mins = t / 10000;
	int secs = t / 1000;
	std::cout << mins << ":" << secs;
}

bool posValida(int x, int z, distance d) {
	int posX, posZ;
	posX = x + d.x;
	posZ = z + d.z;
	return posX < MAX_WIDTH&& posX > MIN_WIDTH && posZ < MAX_HEIGHT&& posZ > MIN_HEIGHT ? true : false;
}

//	Inicializa al listener
void initListener() {

	FMOD_RESULT res;
	FMOD_VECTOR
		listenerPos = { 0,0,0 },		// posicion del listener
		listenerVel = { 0,0,0 },		// velocidad del listener
		at = { 0, 0, 1 },				// vector at: hacia donde mira
		up = { 0, 1, 0 };				// vector up: hacia la ``coronilla''
	// colocamos listener
	res = syst->set3DListenerAttributes(0, &listenerPos, &listenerVel, &at, &up);
	ERRCHECK(res);
}

//	Imprime los valores que se muestran debajo del grid de puntos
void graficaStats() {
	Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
	SetConsoleTextAttribute(console_color, 9);

	std::cout << "Listener(L): asdw " << (int)getListenerPos().x << "," << (int)getListenerPos().z << "   ";
	std::cout << "Source(S): jkli "<< (int)s->getSourcePos().x << "," << (int)s->getSourcePos().z << "   ";
	std::cout << "Orientacion: 8624 " << s->getOrientation().x << "," << s->getOrientation().z << "  ";
	std::cout << "Vel:  " << s->getVelocity().x << "," << s->getVelocity().z << "  ";
	
	SetConsoleTextAttribute(console_color, 7);
	std::cout << "x symemetry	";
	std::cout << "1-2: reverbs	 ";
	std::cout << "z exit \n";

	if (subMenu == Source::EffectIdSubMenu::MinDistance3D) {
		SetConsoleTextAttribute(console_color, 12);
	}
	std::cout << "minD:	" << s->getDistance().first << "  ";
	SetConsoleTextAttribute(console_color, 7);

	if (subMenu == Source::EffectIdSubMenu::MaxDistance3D) {
		SetConsoleTextAttribute(console_color, 12);
	}
	std::cout << "maxD: " << s->getDistance().second <<"  ";
	SetConsoleTextAttribute(console_color, 7);
	
	if (subMenu == Source::EffectIdSubMenu::ConeIn) {
		SetConsoleTextAttribute(console_color, 12);
	}
	std::cout << "ConeI: " << std::get<0>(s->getConeInfo()) << "   ";
	SetConsoleTextAttribute(console_color, 7);

	if (subMenu == Source::EffectIdSubMenu::ConeOut) {
		SetConsoleTextAttribute(console_color, 12);
	}
	std::cout << "ConeO: " << std::get<1>(s->getConeInfo()) << "   ";
	SetConsoleTextAttribute(console_color, 7);

	if (subMenu == Source::EffectIdSubMenu::Doppler) {
		SetConsoleTextAttribute(console_color, 12);
	}
	std::cout << "Doppler: " << s->getDopplerLevel() << " \n";
	SetConsoleTextAttribute(console_color, 7);

}

//	Muestra en pantalla los sonidos cargar y los efectos activos
void graficaPlayList() {
	system("CLS");
	std::cout << "Sonidos cargados " << playList.size() << std::endl;
	int s = playList.size();
	for (int i = 0; i < s; i++) {
		if (i == selectionV) {
			SetConsoleTextAttribute(console_color, 12);
			std::cout << ">" << playList.at(i)->getName();
			SetConsoleTextAttribute(console_color, 7);
			std::cout << "  ";
			elapsedTime(playList.at(i)->getTime());
			std::cout << "\n";
		}
		else
			std::cout << playList.at(i)->getName() << "\n";
	}

	std::cout << "\n\n\n\n";
	for (size_t i = 0; i < Source::efectos.size(); i++)
	{
		if (i == selectionH) {
			SetConsoleTextAttribute(console_color, 12);
			std::cout << ">" << Source::efectos[i].name << "   ";
			SetConsoleTextAttribute(console_color, 7);
		}
		else
			std::cout << Source::efectos[i].name << "   ";

	}
	std::cout << "\n\n";
}

//	Imprime el tablero
void graficaTablero() {
	Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
	std::cout << "Pos List " << " x: " << getListenerPos().x << " z: " << getListenerPos().z << std::endl;
	std::cout << "Pos Sour " << " x: " << s->getSourcePos().x << " z: " << s->getSourcePos().z << std::endl;

	std::tuple<float, float, float> t = s->getConeInfo();

	std::cout << "Cone in " << (std::get<0>(t));
	std::cout << "	Cone out " << (std::get<1>(t));
	std::cout << "	Cone out Volumen " << (std::get<2>(t)) << "\n";


	for (int x = MIN_HEIGHT; x < MAX_HEIGHT; x++) {
		for (int y = MIN_WIDTH; y < MAX_WIDTH; y++) {

			if (getListenerPos().x == x && getListenerPos().z == y) {
				std::cout << "L ";
			}
			else if ((int)s->getSourcePos().x == y && (int)s->getSourcePos().z == x) {
				std::cout << "S ";
			}
			else
			{
				std::cout << ". ";
			}
		}
		std::cout << std::endl;
	}
}

void graficaTableroElemental() {
	FMOD_VECTOR
		lPos = getListenerPos();
	Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
	if (s == nullptr) return;
	FMOD_VECTOR
		SPos = s->getSourcePos();

	for (int x = MIN_HEIGHT; x < MAX_HEIGHT; x++) {
		for (int y = MIN_WIDTH; y < MAX_WIDTH; y++) {

			if (!muros.empty()) {
				for (int i = 0; i < muros.size(); i++) {
					if ((int)muros[i]->geoPos.x == y && (int)muros[i]->geoPos.z == x) {
						std::cout << "=";
					}
				}
			}
			if ((int)lPos.x == y && (int)lPos.z == x) {
				std::cout << "L ";
			}
			else if ((int)SPos.x == y && (int)SPos.z == x) {
				//FMOD_VECTOR
				//	orient = s->getOrientation();
				//switch (toOrientation(orient))
				//{
				//case orientation::back: {
				//	break;
				//}
				//case orientation::forward: {
				//	break;
				//}				
				//case orientation::left: {
				//	break;
				//}				
				//case orientation::right: {
				//	break;
				//}
				//default:
				//	break;
				//}
				std::cout << "S ";
			}
			else
			{
				std::cout << ". ";
			}
		}
		std::cout << std::endl;
	}
}

void graficaSubMenuMovimiento3D() {
	for (int i = 0; i < Source::subMenu.size(); i++) {
		if (i == subMenu) {
			SetConsoleTextAttribute(console_color, 12);
			std::cout << ">" << (Source::subMenu[subMenu].name) << "  ";
			SetConsoleTextAttribute(console_color, 7);
		}
		else
		{
			std::cout << Source::subMenu[i].name << "  ";
		}
	}
	std::cout << "\n";
}

void graficaMovimiento3D() {
	system("CLS");
	graficaTableroElemental();
	graficaStats();
	graficaSubMenuMovimiento3D();
}

void muestraEfecto() {

	graficaPlayList();
	switch (Source::efectos[selectionH].effect)
	{
	case Source::EffectId::Pitch: {

		float pot = playList.at(selectionV)->getEfect(Source::efectos[selectionH].effect);
		if (pot > 100.0f)
			pot = 1.0f;
		else if (pot <= 0)
			pot = 0;
		std::cout << (pot) / 100.0f;

		break;
	}
	case Source::EffectId::Volumen: {
		float pot = playList.at(selectionV)->getEfect(Source::efectos[selectionH].effect);
		if (pot > 1.0f)
			pot = 1.0f;
		else if (pot <= 0)
			pot = 0;
		//std::cout << (pot - 0) / 1.0f;
		int norm = pot * 10;
		/*std::cout << ">" << std::setfill('-') << std::setw(norm);*/
		std::cout << std::setfill('x') << std::setw(norm);
		std::cout << norm << std::endl;
		break;
	}
	case Source::EffectId::Movement: { // TODO*
		float pot = playList.at(selectionV)->getEfect(Source::efectos[selectionH].effect);
		if (pot > 1.0f)
			pot = 1.0f;
		else if (pot <= 0)
			pot = 0;
		//std::cout << (pot - 0) / 1.0f;
		int norm = pot * 10;
		/*std::cout << ">" << std::setfill('-') << std::setw(norm);*/
		std::cout << std::setfill('x') << std::setw(norm);
		std::cout << norm << std::endl;
		break;
	}
	case Source::EffectId::Posicional: {
		graficaTablero();
		break;
	}
	case Source::EffectId::Move3DElemt: {
		graficaMovimiento3D();
		break;
	}
	default:
		break;
	}
}

void modificaEfecto(float value, distance* d = nullptr, movType t = movType::mNone)
{
	switch (Source::efectos[selectionH].effect)
	{
	case Source::EffectId::Pitch: {
		playList.at(selectionV)->modEfect(Source::efectos[selectionH].effect, value * 0.5f);
		break;
	}
	case Source::EffectId::Volumen: {
		playList.at(selectionV)->modEfect(Source::efectos[selectionH].effect, value * 0.1f);
		break;
	}
	case Source::EffectId::Movement: {
		setListenerPos(*d);
		break;
	}
	case Source::EffectId::Posicional: {
		Sound3D* sd = dynamic_cast<Sound3D*>(playList.at(selectionV));
		if (sd != nullptr && posValida((int)round(sd->getSourcePos().x), (int)round(sd->getSourcePos().y), *d)) {
			sd->setSourcePos(*d);
			sd->setSourceOrientation(d->d);
		}
		break;
	}
	case Source::FadeIn: {
		playList[selectionH]->fadeIn();
		break;
	}
	case Source::FadeOut: {
		playList[selectionH]->fadeOut();
		break;
	}
	case Source::EffectId::Move3DElemt: {
		switch (t)
		{
		case listener: {
			setListenerPos(*d); 
			break;
		}
		case source: {
			Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
			if (s != nullptr) {
				s->setSourcePos(*d);
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
	muestraEfecto();
}

bool gestionaTeclas(int c) {
	switch (c)
	{
	case KEY_DOWN: {
		selectionV += 1;
		if (selectionV >= playList.size())
			selectionV = 0;
		graficaPlayList();
		break;
	}
	case KEY_UP: {
		selectionV -= 1;
		if (selectionV < 0)
			selectionV = playList.size() - 1;
		graficaPlayList();
		break;
	}
	case KEY_LEFT: {
		if (subMenuActive) {
			subMenu -= 1;
			if (subMenu < 0) {
				subMenu = Source::subMenu.size() - 1;
			}
			graficaMovimiento3D();
		}
		else
		{
			selectionH -= 1;
			if (selectionH < 0)
				selectionH = Source::efectos.size() - 1;
			graficaPlayList();
		}
		break;
	}
	case KEY_RIGHT: {
		if (subMenuActive) {
			subMenu += 1;
			if (subMenu >= Source::subMenu.size()) {
				subMenu = 0;
			}
			graficaMovimiento3D();
		}
		else {
			selectionH += 1;
			if (selectionH >= Source::efectos.size())
				selectionH = 0;
			graficaPlayList();
		}
		break;
	}
	case ARRIBA: {
		if (subMenuActive && subMenu == Source::EffectIdSubMenu::ConeOrientation) {
			Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
			FMOD_VECTOR
				pos = s->getSourcePos();

			if (pos.x > MIN_WIDTH && pos.x < MAX_WIDTH && pos.z > MIN_HEIGHT && pos.z < MAX_WIDTH) {
				orientation forward = orientation::forward;
				s->setSourceOrientation(forward);
			}
			graficaMovimiento3D();
		}
		break;
	}
	case ABAJO: {
		if (subMenuActive && subMenu == Source::EffectIdSubMenu::ConeOrientation) {
			Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
			FMOD_VECTOR
				pos = s->getSourcePos();

			if (pos.x > MIN_WIDTH && pos.x < MAX_WIDTH && pos.z > MIN_HEIGHT && pos.z < MAX_WIDTH) {
				orientation back = orientation::back;
				s->setSourceOrientation(back);
			}
			graficaMovimiento3D();
		}
		break;
	}
	case IZQUIERDA: {
		if (subMenuActive && subMenu == Source::EffectIdSubMenu::ConeOrientation) {
			Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
			FMOD_VECTOR
				pos = s->getSourcePos();

			if (pos.x > MIN_WIDTH && pos.x < MAX_WIDTH && pos.z > MIN_HEIGHT && pos.z < MAX_WIDTH) {
				orientation left = orientation::left;
				s->setSourceOrientation(left);
			}
			graficaMovimiento3D();
		}
		break;
	}
	case DERECHA: {
		if (subMenuActive && subMenu == Source::EffectIdSubMenu::ConeOrientation) {
			Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
			FMOD_VECTOR
				pos = s->getSourcePos();

			if (pos.x > MIN_WIDTH && pos.x < MAX_WIDTH && pos.z > MIN_HEIGHT && pos.z < MAX_WIDTH) {
				orientation right = orientation::right;
				s->setSourceOrientation(right);
			}
			graficaMovimiento3D();
		}
		break;
	}
	case ENTER: {
		switch (selectionH)
		{
		case Source::EffectId::Play_Pause: {	//Para dar play o pause
			if (!(playList[selectionV])->isPlaying()) {
				playList[selectionV]->play();
			}
			else {
				playList[selectionV]->setPause();
			}
			break;
		}
		case Source::EffectId::Stop: {	//Para para un track
			if ((playList[selectionV])->isPlaying()) {
				playList[selectionV]->stop();
			}
			break;
		}
		case Source::EffectId::FadeOut: {
			playList[selectionV]->fadeOut();
			break;
		}
		case Source::EffectId::FadeIn: {
			playList[selectionV]->fadeIn();
			break;
		}
		case Source::EffectId::Move3DElemt: {
			subMenuActive = true;
			stopPlaylist();
			playList[selectionV]->play();
			//distance d = {
			//	orientation::oNone, -5.0f, 0.0f, 0.0f
			//};
			//setListenerPos(d);
			Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
			if (s == nullptr) ERRCHECK(FMOD_RESULT::FMOD_ERR_DSP_INUSE);
			s->activeMov();
			distance sD = {
				orientation::oNone, 0.0f, 0.0f, 10.0f
			};
			s->setSourcePos(sD);
			graficaTableroElemental();
			graficaStats();
			graficaSubMenuMovimiento3D();
			break;
		}
		default:
			break;
		}
		break;
	}
	case ADD: {

		if (subMenuActive) {
			switch (subMenu)
			{
			case Source::EffectIdSubMenu::ConeIn: {
				Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
				if (s == nullptr) return true;
				std::tuple<float, float, float> t = s->getConeInfo();
				float cone = std::get<0>(t) += 1.0f;
				if (cone <= std::get<1>(t))
					s->setSourceConeAngle(cone);
				break;
			}
			case Source::EffectIdSubMenu::ConeOut: {
				Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
				if (s == nullptr) return true;
				std::tuple<float, float, float> t = s->getConeInfo();
				float cone = std::get<1>(t) += 1.0f;
				if (cone >= std::get<0>(t))
					s->setSourceConeAngle(-1.0f, cone);
				break;
			}
			case Source::EffectIdSubMenu::MinDistance3D: {
				Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
				if (s == nullptr) return true;
				s->setDistance(s->getDistance().first + 1.0f, s->getDistance().second);
				break;
			}
			case Source::EffectIdSubMenu::MaxDistance3D: {
				Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
				if (s == nullptr) return true;
				s->setDistance(s->getDistance().first, s->getDistance().second + 1.0f);
				break;
			}
			case Source::EffectIdSubMenu::Doppler: {
				Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
				if (s == nullptr) return true;
				s->setDopplerLevel(1.0f);
				break;
			}
			default:
				break;
			}

			muestraEfecto();

		}
		else if ((selectionH == Source::EffectId::Play_Pause
			|| selectionH == Source::EffectId::Stop
			|| selectionH == Source::EffectId::FadeOut
			|| selectionH == Source::EffectId::FadeIn) == true) return true;
		else
		{
			modificaEfecto(1.0f);
		}

		break;
	}
	case SUB: {
		if (subMenuActive) {
			switch (subMenu)
			{
			case Source::EffectIdSubMenu::ConeIn: {
				Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
				if (s == nullptr) return true;
				std::tuple<float, float, float> t = s->getConeInfo();
				float cone = std::get<0>(t) -= 1.0f;
				if (cone <= std::get<1>(t))
					s->setSourceConeAngle(cone);
				break;
			}
			case Source::EffectIdSubMenu::ConeOut: {
				Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
				if (s == nullptr) return true;
				std::tuple<float, float, float> t = s->getConeInfo();
				float cone = std::get<1>(t) -= 1.0f;
				if (cone >= std::get<0>(t))
					s->setSourceConeAngle(-1.0f, cone);
				break;
			}
			case Source::EffectIdSubMenu::MaxDistance3D: {
				Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
				if (s == nullptr) return true;
				s->setDistance(s->getDistance().first, s->getDistance().second - 1.0f);
				break;
			}
			case Source::EffectIdSubMenu::MinDistance3D: {
				Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
				if (s == nullptr) return true;
				s->setDistance(s->getDistance().first - 1.0f, s->getDistance().second);
				break;
			}
			case Source::EffectIdSubMenu::Doppler: {
				Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
				if (s == nullptr) return true;
				s->setDopplerLevel(-1.0f);
				break;
			}
			default:
				break;
			}
			muestraEfecto();

		}
		else if ((selectionH == Source::EffectId::Play_Pause
			|| selectionH == Source::EffectId::Stop
			|| selectionH == Source::EffectId::FadeOut
			|| selectionH == Source::EffectId::FadeIn) == true) return true;
		else {
			modificaEfecto(-1.0);
		}

		break;
	}
	case W: {
		//	Para el efecto movimiento
		if (selectionH == Source::EffectId::Movement) {
			distance forward = {
				orientation::forward, 0.0f, 0.0f, -1.0f
			};
			modificaEfecto(0, &forward);
		}
		else if (selectionH == Source::EffectId::Move3DElemt) {
			distance forward = {
				orientation::forward, 0.0f, 0.0f, -1.0f
			};
			modificaEfecto(0, &forward, movType::listener);
		}
		break;
	}
	case A: {
		if (selectionH == Source::EffectId::Movement) {
			distance left = {
				orientation::left, -1.0f, 0.0f, 0.0f
			};
			modificaEfecto(0, &left);
		}
		else if (selectionH == Source::EffectId::Move3DElemt) {
			distance left = {
				orientation::left, -1.0f, 0.0f, 0.0f
			};
			modificaEfecto(0, &left, movType::listener);
		}
		break;
	}
	case S: {
		if (selectionH == Source::EffectId::Movement) {
			distance back = {
				orientation::back, 0.0f, 0.0f, 1.0f
			};
			modificaEfecto(0, &back);
		}
		else if (selectionH == Source::EffectId::Move3DElemt) {
			distance back = {
				orientation::back, 0.0f, 0.0f, 1.0f
			};
			modificaEfecto(0, &back, movType::listener);
		}
		break;
	}
	case D: {
		if (selectionH == Source::EffectId::Movement) {
			distance right = {
				orientation::right, 1.0f, 0.0f, 0.0f
			};
			modificaEfecto(0, &right);
		}
		else if (selectionH == Source::EffectId::Move3DElemt) {
			distance right = {
				orientation::right, 1.0f, 0.0f, 0.0f
			};
			modificaEfecto(0, &right, movType::listener);
		}
		break;
	}
	case J: {
		if (selectionH == Source::EffectId::Move3DElemt) {
			distance left = {
				orientation::right, -1.0f, 0.0f, 0.0f
			};
			modificaEfecto(0, &left, movType::source);
		}
		break;
	}
	case K: {
		if (selectionH == Source::EffectId::Move3DElemt) {
			distance back = {
				orientation::back, 0.0f, 0.0f, 1.0f
			};
			modificaEfecto(0, &back, movType::source);
		}
		break;
	}
	case L: {
		if (selectionH == Source::EffectId::Move3DElemt) {
			distance right = {
				orientation::right, 1.0f, 0.0f, 0.0f
			};
			modificaEfecto(0, &right, movType::source);
		}
		break;
	}
	case I: {
		if (selectionH == Source::EffectId::Move3DElemt) {
			distance forward = {
				orientation::forward, 0.0f, 0.0f, -1.0f
			};
			modificaEfecto(0, &forward, movType::source);
		}
		break;
	}
	case Z: {
		//0,2,4,5,7,9,11,12 escala de frecuencias
		//no hace nada al ser la primera nota: DO
		std::cout << "DO" << std::endl;
		break;

	}
	case X: {
		//nota: RE
		playList[0]->setPitch(std::pow(2, (2 / 12.0f)));
		std::cout << "RE" << std::endl;
		break;
	}
	case C: {
		//nota: MI
		playList[0]->setPitch(std::pow(2, (4 / 12.0f)));
		std::cout << "MI" << std::endl;
		break;
	}
	case V: {
		//nota: FA
		playList[0]->setPitch(std::pow(2, (5 / 12.0f)));
		std::cout << "FA" << std::endl;
		break;
	}
	case B: {
		//nota: SOL
		playList[0]->setPitch(std::pow(2, (7 / 12.0f)));
		std::cout << "SOL" << std::endl;
		break;
	}
	case N: {
		//nota: LA
		std::cout << "LA" << std::endl;
		playList[0]->setPitch(std::pow(2, (9 / 12.0f)));
		break;
	}
	case M: {
		//nota: SI
		std::cout << "SI" << std::endl;
		playList[0]->setPitch(std::pow(2, (11 / 12.0f)));
		break;
	}
	case COMA: {
		//nota: DO ALTO
		playList[0]->setPitch(std::pow(2, (12 / 12.0f)));
		std::cout << "DO*" << std::endl;
		break;
	}
	case EXIT: {
		std::cout << "\n";
		return false;
		break;
	}
	default:
		std::cout << c << std::endl;
		break;
	}
	return true;
}

void cargaMuros(const std::vector<WallComp> t) {
	for (int i = 0; i < t.size(); i++) {
		Wall* currWall = new Wall(t.at(i).nummaxPoligons, t.at(i).maxVertices);
		FMOD_VECTOR
			pos = { t[i].x,0,t[i].z };
		currWall->setPosition(pos);
	}
}

void cargaSonidos(const std::vector<Comp> s) {
	for (int i = 0; i < s.size(); i++) {
		switch (s[i].sT)
		{
		case soundType::sound2D:
		{
			Sound2D* s2D = new Sound2D(Source::sonidos[s[i].r].ruta.c_str(),
				Source::sonidos[s[i].r].nombre, s[i].loop);
			break;
		}
		case soundType::sound3D:
		{
			Sound3D* s3D = new Sound3D(Source::sonidos[s[i].r].ruta.c_str(),
				Source::sonidos[s[i].r].nombre, s[i].loop);
			break;
		}
		default:
			break;
		}
	}
}

double randMToN(double mm, double nn)
{
	return mm + (rand() / (RAND_MAX / (nn - mm)));
}

int main() {

	if (syst == NULL) {
		FMOD_RESULT res;
		res = System_Create(&syst);
		ERRCHECK(res);

		res = syst->init(128, FMOD_INIT_NORMAL, 0);
		ERRCHECK(res);
	}
	initListener();
	console_color = GetStdHandle(STD_OUTPUT_HANDLE);

#pragma region Practica3
#pragma region Apartado1
	//Sound2D* battle = new Sound2D(Source::sonidos[Source::Battle].ruta.c_str());
	//Sound3D* gun1 = new Sound3D(Source::sonidos[Source::Gun1].ruta.c_str());
	//Sound3D* gun2 = new Sound3D(Source::sonidos[Source::Gun2].ruta.c_str());
	//battle->playLoop();

	//bool run = true;
	//char tecla;
	//std::time_t t = std::time(0);   
	//std::tm* now = std::localtime(&t);


	//double randomTime = rand() % 10;
	//int timeToPlay = randomTime;
	//int currSec = 0;
	//int tmc = 0;
	//while (run)
	//{
	//	t = std::time(0); 
	//	now = std::localtime(&t);
	//	if (now->tm_sec > tmc) {
	//		currSec++;
	//		tmc = now->tm_sec;
	//	}

	//	if (currSec >= timeToPlay) {
	//		int rnd = rand() % 2;
	//		if (rnd == 0) {
	//			gun1->play();
	//			gun1->setPosition(rand() % 500);
	//		}
	//		else {
	//			gun2->play();
	//			gun2->setPosition(rand() % 500);
	//		}

	//		t = std::time(0);
	//		randomTime = rand() % 10;
	//		timeToPlay = randomTime;
	//		currSec = 0;
	//	}

	//	syst->update();

	//	if (_kbhit()) {
	//		tecla = _getch();
	//		std::cout << tecla << std::endl;
	//		run = !(tecla == 'q');
	//	}
	//}
#pragma endregion

#pragma region Apartado2
	/*Sound2D* motor = new Sound2D(Source::sonidos[Source::Motor].ruta.c_str(), Source::sonidos[Source::Motor].name, true);
	playList.push_back(motor);
	motor->playLoop();

	Sound3D* gun1 = new Sound3D(Source::sonidos[Source::Gun1].ruta.c_str());
	playList.push_back(gun1);


	bool run = true;
	char tecla = ' ';
	grafica();

	int c = 0;
	while (run)
	{
		switch ((c = getch()))
		{
		case KEY_DOWN:
		{
			selectionV += 1;
			if (selectionV >= playList.size())
				selectionV = 0;
			grafica();
			break;
		}
		case KEY_UP:
		{
			selectionV -= 1;
			if (selectionV < 0)
				selectionV = playList.size() - 1;
			grafica();
			break;
		}
		case KEY_LEFT:
		{
			selectionH -= 1;
			if (selectionH < 0)
				selectionH = Source::efectos.size() - 1;
			grafica();
			break;
		}
		case KEY_RIGHT:
		{
			selectionH += 1;
			if (selectionH >= Source::efectos.size())
				selectionH = 0;
			grafica();
			break;
		}
		case ENTER:
		{
			muestraEfecto();
			break;
		}
		case ADD:
		{
			modificaEfecto(1.0f);
			break;
		}
		case SUB:
		{
			modificaEfecto(-1.0);
			break;
		}
		case EXIT:
		{
			run = false;
			break;
		}
		default:
			std::cout << c << std::endl;
			break;
		}
		syst->update();
	}*/
#pragma endregion

#pragma region Apartado3

	//std::vector<Comp> s = {
	//	Comp{Source::FootStep, true, soundType::sound3D},
	//};
	//cargaSonidos(s);

	//grafica();

	//bool run = true;
	//while (run)
	//{
	//	if (_kbhit()) {
	//		int c;
	//		run = gestionaTeclas((c = getch()));
	//	}
	//	syst->update();
	//}
#pragma endregion

#pragma region Apartado4
	/*std::vector<Comp> s = {
			Comp{Source::RifleMod01, false, soundType::sound2D},
			Comp{Source::RifleMod02, false, soundType::sound2D},
			Comp{Source::RifleMod03, false, soundType::sound2D},
			Comp{Source::RifleMod04, false, soundType::sound2D},
	};
	cargaSonidos(s);

	grafica();

	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);

	double randomTime = rand() % 10;
	int timeToPlay = randomTime;
	int currSec = 0;
	int tmc = 0;

	bool run = true;
	while (run)
	{
		t = std::time(0);
		now = std::localtime(&t);

		if (now->tm_sec > tmc) {
			currSec++;
			tmc = now->tm_sec;
		}
		if (currSec >= timeToPlay) {
			int rnd = rand() % 4;
			playList[rnd]->setPitch(randMToN(0.01f, 100.0f));
			playList[rnd]->play();
			t = std::time(0);
			randomTime = rand() % (4-1);

			timeToPlay = randomTime;
			currSec = 0;
		}

		if (_kbhit()) {
			int c;
			run = gestionaTeclas((c = getch()));
		}
		syst->update();
	}*/
#pragma endregion

#pragma region Apartado5

	//std::vector<Comp> s = { 
	//	Comp{ Source::Piano, false, soundType::sound2D }, 
	//};

	//cargaSonidos(s);
	//grafica();

	///*std::time_t t = std::time(0);
	//std::tm* now = std::localtime(&t);*/

	//std::string nota = "";
	//bool run = true;
	//while (run)
	//{
	//	if (_kbhit()) {
	//		int c;
	//		run = gestionaTeclas((c = getch()));
	//		
	//		if (c == Z || c == X || c == C || c == V || c == B || c == N || c == M || c == COMA) {
	//			playList[0]->play();
	//		}
	//	}
	//	syst->update();
	//}

#pragma endregion

#pragma region Apartado6
	//std::vector<Comp> s = {
	//		Comp{ Source::Scooter, true, soundType::sound3D },
	//};
	//cargaSonidos(s);
	//grafica();

	//bool run = true;
	//while (run)
	//{
	//	if (_kbhit()) {
	//		int c;
	//		run = gestionaTeclas((c = getch()));
	//	}
	//	syst->update();
	//}

#pragma endregion
#pragma endregion

#pragma region Practica4

#pragma region Apartado1
	//	PlayList
	std::vector<Comp> s = {
		Comp{ Source::RutaId::Siren , true, soundType::sound3D },
	};

	//	Los muros
	std::vector<WallComp> t = {
		WallComp{1,10,-5,0},
		WallComp{1,10,-4,0},
		WallComp{1,10,-3,0},
		WallComp{1,10,-2,0},
	};

	cargaSonidos(s);
	cargaMuros(t);
	graficaPlayList();

	bool run = true;
	auto start = std::chrono::system_clock::now();
	while (run)
	{
		auto end = std::chrono::system_clock::now();
		if (_kbhit()) {
			int c;
			run = gestionaTeclas((c = getch()));
		}
		std::chrono::duration<double> elapsed_seconds = end - start;
		std::time_t end_time = std::chrono::system_clock::to_time_t(end);
		if (!playList[selectionV]->update(elapsed_seconds.count()))
			graficaMovimiento3D();
		syst->update();
	}
#pragma endregion

#pragma endregion

	FMOD_RESULT res = syst->release();
	ERRCHECK(res);
	system("PAUSE");
	return 0;
}