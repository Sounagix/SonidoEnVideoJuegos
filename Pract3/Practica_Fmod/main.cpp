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

#pragma warning(disable : 4996)
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define EXIT 27
#define ENTER 13		
#define ADD 43			
#define SUB 45			
#define W 119			
#define A 97			
#define S 115			
#define D 100	

#define Z 90
#define X 88
#define C 67
#define V 86
#define B 66
#define N 78
//#define M 77	//SE USA EL VALOR EN KEY_RIGHT
//#define ,		LA COMA HAY QUE DEFINIRLA, LO PONE EN EL ENUNCIADO COMO "DO alto"


using namespace FMOD;
System* syst;

class BaseSound;

// Constantes - apartado 5
const char* TECLAS = "zxcvbnm,";	// do; re; mi; fa; sol; la; si; ',' = do alto

enum soundType
{
	sound3D,
	sound2D,
};

enum dir {
	up,
	down,
	left,
	right,
};

// Determina el tipo de Loop con el metodo
enum loop {
	freeLoop = -1,	// loop indefinido, por defecto es el argumento inicializado en función playLoop(int l = -1)
	OnceLoop = 0,	// se reproduce una sola vez	
	ThreeLoop = 2	// se reproduce 3 veces
};

struct distance
{
	dir d;
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



std::vector<BaseSound*> playList;
int selectionH = 0;
int selectionV = 0;


void ERRCHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK) {
		std::cout << FMOD_ErrorString(result) << std::endl;
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
	float timeRate = 0.0f;
	//	tiempo actual
	float currTime = 0.0f;

public:
	//	Constructor generico
	BaseSound() {};

	//	Destructor generico
	~BaseSound() {
		std::cout << "Base\n";
		sonido->release();
	}

	//	Reproduce un sound 
	void play() {
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
	void setPause(bool status) {
		canal->setPaused(status);
	}

	//	Cambia el volumen de un sonido *TODO
	void setVolume(float v) {
		canal->setVolume(v);
	}

	//	Cambia el estado de un canal a mute en función de un bool
	void setMute(bool status) {
		canal->setMute(status);
	}

	//	Cambia el pitch de un canal en función de un valor(float) -> para devolver a un pitch normal -> 1.0f
	void setPitch(float value) {
		canal->setPitch(value);
	}

	//	Cambia el frame rate de un sonido
	void setFrameRate(float value) {
		timeRate = value;
	}

	virtual void update(float deltaTime) = 0;

	//	Determina si un sonido se está reproduciendo
	bool isPlaying() {
		bool active;
		canal->isPlaying(&active);
		return active;
	}

	// Devuelve el nombre del sonido
	std::string getName() {
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
};

class Sound3D : public BaseSound {
public:
	Sound3D(const char* ruta, std::string n = "undefined", bool loop = false, unsigned int dist = 5) {

		nombre = n;
		FMOD_RESULT res;
		if (loop)
			res = syst->createSound(ruta, FMOD_3D | FMOD_LOOP_NORMAL, 0, &sonido);
		else
			res = syst->createSound(ruta, FMOD_3D, 0, &sonido);

		play();

		ERRCHECK(res);

		FMOD_VECTOR
			listenerPos = { 0,0,0 },	// posicion del listener
			listenerVel = { 0,0,0 },	// velocidad del listener
			up = { 0,1,0 },				// vector up: hacia la ``coronilla''
			at = { 1,0,0 };				// vector at: hacia donde mira
		// colocamos listener
		syst->set3DListenerAttributes(0, &listenerPos, &listenerVel, &up, &at);

		playList.push_back(this);
	}

	virtual ~Sound3D() {
		std::cout << "3D\n";
	}

	virtual void update(float deltaTime) override {
		currTime += deltaTime;
		if (currTime >= timeRate)
			play();
	};

	//	Setea una posición a un sonido 3d
	void setPosition(distance d) {
		FMOD_VECTOR
			pos,
			vel,
			forw,
			upD;

		syst->get3DListenerAttributes(0, &pos, &vel, &forw, &upD);

		d.x += pos.x;
		d.y += pos.y;
		d.z += pos.z;


		FMOD_VECTOR
			listenerPos = { d.x,d.y,d.z },// posicion del listener
			listenerVel = { 0,0,0 },		// velocidad del listener
			up = { 0,1,0 },					// vector up: hacia la ``coronilla''
			at = { 1,0,0 };					// vector at: hacia donde mira
		// colocamos listener

		syst->set3DListenerAttributes(0, &listenerPos, &listenerVel, &up, &at);
	}

	FMOD_VECTOR getPosition() {
		FMOD_VECTOR
			pos,
			vel;
		canal->get3DAttributes(&pos, &vel);
		return pos;
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

	virtual void update(float deltaTime) override {};

};


void grafica() {
	system("CLS");
	std::cout << "Sonidos cargados " << playList.size() << std::endl;
	int s = playList.size();
	for (int i = 0; i < s; i++) {
		if (i == selectionV)
			std::cout << ">" << playList.at(i)->getName() << "\n";
		else
			std::cout << playList.at(i)->getName() << "\n";
	}

	std::cout << "\n\n\n\n";
	for (size_t i = 0; i < Source::efectos.size(); i++)
	{
		if (i == selectionH)
			std::cout << ">" << Source::efectos[i].name << "   ";
		else
			std::cout << Source::efectos[i].name << "   ";

	}
	std::cout << "\n\n";
}

void muestraEfecto() {

	grafica();
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
	default:
		break;
	}
	//std::cout << playList.at(selectionV)->getEfect(Source::efectos[selectionH].effect);
}

void modificaEfecto(float value, distance* d = nullptr)
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
		static_cast<Sound3D*>(playList.at(selectionV))->setPosition(*d);
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
		grafica();
		break;
	}
	case KEY_UP: {
		selectionV -= 1;
		if (selectionV < 0)
			selectionV = playList.size() - 1;
		grafica();
		break;
	}
	case KEY_LEFT: {
		selectionH -= 1;
		if (selectionH < 0)
			selectionH = Source::efectos.size() - 1;
		grafica();
		break;
	}
	case KEY_RIGHT: {
		selectionH += 1;
		if (selectionH >= Source::efectos.size())
			selectionH = 0;
		grafica();
		break;
	}
	case ENTER: {
		muestraEfecto();
		break;
	}
	case ADD: {
		if (selectionH != 2)
			modificaEfecto(1.0f);
		break;
	}
	case SUB: {
		if (selectionH != 2)
			modificaEfecto(-1.0);
		break;
	}
	case W: {
		if (selectionH == 2) {
			distance forward = {
				dir::right, 1.0f, 0.0f, 0.0f
			};
			modificaEfecto(0, &forward);
		}
		break;
	}
	case A: {
		if (selectionH == 2) {
			distance forward = {
				dir::left,0.0f, 0.0f,1.0f
			};
			modificaEfecto(0, &forward);
		}
		break;
	}
	case S: {
		if (selectionH == 2) {
			distance forward = {
				dir::left,-1.0f, 0.0f,0.0f
			};
			modificaEfecto(0, &forward);
		}
		break;
	}
	case D: {
		if (selectionH == 2) {
			distance forward = {
				dir::left,0.0f, 0.0f,-1.0f
			};
			modificaEfecto(0, &forward);
		}
		break;
	}	
	case Z: {
		break;
	}case X: {
		break;
	}case C: {
		break;
	}case V: {
		break;
	}case B: {
		break;
	}case N: {
		break; 
	}
	/*}case M: {
		break; 
	}*/

	/*}case, : {
		break; 
	}*/

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

void cargaSonidos(const std::vector<Comp> s) {
	for (int i = 0; i < s.size(); i++) {
		switch (s[i].sT)
		{
		case soundType::sound2D:
		{
			Sound2D* s2D = new Sound2D(Source::sonidos[s[i].r].ruta.c_str(),
				Source::sonidos[s[i].r].nombre, s[i].loop);
			//playList.push_back(s2D);
			break;
		}
		case soundType::sound3D:
		{
			Sound3D* s3D = new Sound3D(Source::sonidos[s[i].r].ruta.c_str(),
				Source::sonidos[s[i].r].nombre, s[i].loop);
			//playList.push_back(s3D);
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

	short int frecuencia[8] = { 0,2,4,5,7,9,11,12 };
	float pitch[8];

	std::vector<Comp> s;
	for (int i = 0; i < 8; i++) {
		pitch[i] = std::pow(2, (frecuencia[i] / 12.0f));
		std::cout << pitch[i] << std::endl;
		s.push_back(Comp{ Source::Piano, false, soundType::sound2D });
	}

	cargaSonidos(s);
	grafica();

	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);


	bool run = true;
	while (run)
	{
		if (_kbhit()) {
			int c;
			run = gestionaTeclas((c = getch()));
		}
		syst->update();
	}




#pragma endregion




	FMOD_RESULT res = syst->release();
	ERRCHECK(res);
	system("PAUSE");
	return 0;
}