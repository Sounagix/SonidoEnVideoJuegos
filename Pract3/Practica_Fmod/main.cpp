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

#define Z 122
#define X 120
#define C 99
#define V 118
#define B 98
#define N 110
#define M 109	
#define COMA 44	

using namespace FMOD;
System* syst;

class BaseSound;

enum soundType
{
	sound3D,
	sound2D,
};

enum orientation {
	forward,
	back,
	left,
	right
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


//	Lista de todos los sonidos cargados
std::vector<BaseSound*> playList;
//	index para los efectos 
int selectionH = 0;
//	index para las sonidos cargados
int selectionV = 0;

//	Máxima anchura que el emisor se puede desplazar
const int MAX_WIDTH = 20;
const int MIN_WIDTH = -20;
//	Máxima altura que el emisor se puede desplazar
const int MAX_HEIGHT = 10;
const int MIN_HEIGHT = -10;


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
	float timeRate = 0.0f;
	//	tiempo actual
	float currTime = 0.0f;

	bool  _statusPause = false;

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

	void stop() {
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

	void fadeIn() {
		unsigned long long parentclock;
		FMOD_RESULT res = canal->getDSPClock(NULL, &parentclock);
		res = canal->addFadePoint(parentclock, 0.0f);
		res = canal->addFadePoint(parentclock + 500000, 1.0f);
	}

	//	Todavia en testeo
	void fadeOut(/*unsigned long long value*/) {
		////if (!isPlaying()) return;
		//FMOD_RESULT res;
		////res = canal->setLoopCount(0);
		////ERRCHECK(res);
		//unsigned int numPoints = 0;
		//unsigned long long dspClock = 0;
		//float volume = 0;
		//res = canal->getFadePoints(&numPoints, &dspClock, &volume);
		//dspClock += value;
		//ERRCHECK(res);
		//res = canal->addFadePoint(dspClock, volume);
		//ERRCHECK(res);

		unsigned long long parentclock;
		FMOD_RESULT res = canal->getDSPClock(NULL, &parentclock);
		float vol;
		canal->getVolume(&vol);
		res = canal->addFadePoint(parentclock, vol);
		res = canal->addFadePoint(parentclock + 500000, 0.0f);
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

	unsigned int getTime() {
		unsigned int l;
		sonido->getLength(&l, FMOD_TIMEUNIT_MS);
		return l;
	}
};

class Sound3D : public BaseSound {
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

	virtual void update(float deltaTime) override {

	};

	//	Setea los conos correspondiente al canal
	void setSourceConeAngle(float insCone = 0.0f, float outCone = 0.0f, float outSideVol = 0.0f) {
		bool playing;
		canal->isPlaying(&playing);
		if (playing)
			canal->set3DConeSettings(insCone, outCone, outSideVol);
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

		pos.x += d.x;
		pos.y = 0.0f;
		pos.z += d.z;

		canal->set3DAttributes(&pos, &vel);
	}

	//	Setea una posición del listener
	void setListenerPos(distance d) {
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

void initListener() {

	FMOD_RESULT res;
	FMOD_VECTOR
		listenerPos = { 0,0,0 },		// posicion del listener
		listenerVel = { 0,0,0 },		// velocidad del listener
		up = { 0, 1, 0 },				// vector up: hacia la ``coronilla''
		at = { 0, 0, 1 };				// vector at: hacia donde mira
	// colocamos listener
	res = syst->set3DListenerAttributes(0, &listenerPos, &listenerVel, &up, &at);
	ERRCHECK(res);
}

//	Muestra en pantalla los sonidos cargar y los efectos activos
void grafica() {
	system("CLS");
	std::cout << "Sonidos cargados " << playList.size() << std::endl;
	int s = playList.size();
	for (int i = 0; i < s; i++) {
		if (i == selectionV) {
			std::cout << ">" << playList.at(i)->getName();
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
	case Source::EffectId::Posicional: {

		Sound3D* s = dynamic_cast<Sound3D*>(playList[selectionV]);
		std::cout << "Pos List " << " x: " << s->getListenerPos().x << " z: " << s->getListenerPos().z << std::endl;
		std::cout << "Pos Sour " << " x: " << s->getSourcePos().x << " z: " << s->getSourcePos().z << std::endl;

		std::tuple<float, float, float> t = s->getConeInfo();

		std::cout << "Cone in " << (std::get<0>(t));
		std::cout << "	Cone out " << (std::get<1>(t));
		std::cout << "	Cone out Volumen " << (std::get<2>(t)) << "\n";


		for (int x = MIN_HEIGHT; x < MAX_HEIGHT; x++) {
			for (int y = MIN_WIDTH; y < MAX_WIDTH; y++) {

				if ((int)s->getListenerPos().x == x && (int)s->getListenerPos().z == y) {
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
		static_cast<Sound3D*>(playList.at(selectionV))->setListenerPos(*d);
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
		default:
			break;
		}
		break;
	}
	case ADD: {
		if ((	selectionH == Source::EffectId::Play_Pause
			||	selectionH == Source::EffectId::Stop 
			||	selectionH == Source::EffectId::FadeOut
			||	selectionH == Source::EffectId::FadeIn) == true) return true;

		modificaEfecto(1.0f);
		break;
	}
	case SUB: {
		if ((selectionH == Source::EffectId::Play_Pause
			|| selectionH == Source::EffectId::Stop
			|| selectionH == Source::EffectId::FadeOut
			|| selectionH == Source::EffectId::FadeIn) == true) return true;

		modificaEfecto(-1.0);
		break;
	}
	case W: {
		//	Para el efecto movimiento
		if ((selectionH == 4 || selectionH == 5) && playList[selectionV]->isPlaying()) {
			distance forward = {
				orientation::forward, 0.0f, 0.0f, -1.0f
			};
			modificaEfecto(0, &forward);
		}
		break;
	}
	case A: {
		if ((selectionH == 4 || selectionH == 5) && playList[selectionV]->isPlaying()) {
			distance left = {
				orientation::left, -1.0f, 0.0f, 0.0f
			};
			modificaEfecto(0, &left);
		}
		break;
	}
	case S: {
		if ((selectionH == 4 || selectionH == 5) && playList[selectionV]->isPlaying()) {
			distance back = {
				orientation::back, 0.0f, 0.0f, 1.0f
			};
			modificaEfecto(0, &back);
		}
		break;
	}
	case D: {
		if ((selectionH == 4 || selectionH == 5) && playList[selectionV]->isPlaying()) {
			distance right = {
				orientation::right, 1.0f, 0.0f, 0.0f
			};
			modificaEfecto(0, &right);
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
	initListener();

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
	std::vector<Comp> s = {
		Comp{ Source::Talking, false, soundType::sound3D },
	};
	cargaSonidos(s);
	grafica();

	bool run = true;
	while (run)
	{
		if (_kbhit()) {
			int c;
			run = gestionaTeclas((c = getch()));
			playList[selectionV]->fadeOut();
		}
		syst->update();
	}
#pragma endregion

#pragma endregion

	FMOD_RESULT res = syst->release();
	ERRCHECK(res);
	system("PAUSE");
	return 0;
}