#include <fmod.hpp>
#include <fmod_errors.h>
#include <iostream>
#include <thread>
#include <cstdio>
#include <conio.h>

using namespace FMOD;
System * syst;


enum Fmod_Tipo : int
{
	F2D = 0, F3D
};

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


class BaseSound {
private:
	//	Sonido a reproducir
	Sound* sonido;
	//	Canal para reproducir
	Channel* canal;
public:
	//	Constructor generico
	BaseSound(const char* ruta, int loop, Fmod_Tipo tipo) {
		FMOD_RESULT res;
		if(tipo == F2D)
			res = syst->createSound(ruta, FMOD_DEFAULT, 0, &sonido);
		else 
			res = syst->createSound(ruta, FMOD_3D, 0, &sonido);

		std::cout << "Sonido creado \n";
		ERRCHECK(res);
		canal->setLoopCount(loop);
	}
	//	Destructor generico
	~BaseSound() {
		sonido->release();
	}
	//	Reproduce un sound y si loop es -1 se reproduce en loop
	void play(int loop = 1) {
		FMOD_RESULT res = syst->playSound(sonido, 0, false, &canal);
		ERRCHECK(res);
		canal->setLoopCount(loop);
	}

};


int main() {

	if (syst == NULL) {
		FMOD_RESULT res;
		res = System_Create(&syst);
		ERRCHECK(res);

		res = syst->init(128, FMOD_INIT_NORMAL, 0);
		ERRCHECK(res);
	}
	
	BaseSound* battle = new BaseSound("../muestras/Battle.wav", -1, F2D);
	BaseSound* gun1 = new BaseSound("../muestras/Gun1.wav", 1, F3D);
	BaseSound* gun2 = new BaseSound("../muestras/Gun2.wav", 1, F3D);

	

	bool run = true;
	char tecla;
	while (run) 
	{
		battle->play();
		gun1->play();
		gun2->play();
		syst->update();

		tecla = _getch();
		run = !(tecla == 1);
	}
	
	FMOD_RESULT res = syst->release();
	ERRCHECK(res);
	return 0;
}