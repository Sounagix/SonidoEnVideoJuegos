#include "Source.h"


std::vector<Source::SoundID> Source::sonidos{
	{ Battle	, "../muestras/Battle.wav"	,"Battle"},
	{ FootStep	, "../muestras/footstep.wav","FootStep"},
	{ Gun1		, "../muestras/Gun1.wav"	,"Gun1"},
	{ Gun2		, "../muestras/Gun2.wav"	,"Gun2"},
	{ Motor		, "../muestras/motor.wav"	,"Motor"},
	{ Piano		, "../muestras/piano.wav"	,"Piano"},
	{ Rifle		, "../muestras/rifle.wav"	,"Rifle"},
	{ Scooter	, "../muestras/scooter.wav"	,"Scooter"},
	{ Talking	, "../muestras/talking.wav"	,"Talking"},
};

std::vector<Source::Effect> Source::efectos{
	{ Pitch		,"Pitch"},
	{ Volumen	,"Volumen"},
	{ Movement	,"Movimiento"},
};
