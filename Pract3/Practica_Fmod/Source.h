#pragma once
#include <vector>
#include <string>

class Source
{
public:
	enum RutaId : int {
		Battle = 0,
		FootStep,
		Gun1,
		Gun2,
		Motor,
		Piano,
		Rifle,
		Scooter,
		Talking,
		Size
	};

	enum EffectId : int
	{
		Pitch = 0,
		Volumen,
	};

	struct SoundID
	{
		public:
			RutaId nombre;
			std::string ruta;
			std::string name;
	};

	struct Effect
	{
	public:
		EffectId effect;
		std::string name;
	};

	static std::vector<SoundID> sonidos;
	static std::vector<Effect> efectos;
};

