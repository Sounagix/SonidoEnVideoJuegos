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
		Movement,
		Volumen,
	};

	struct SoundID
	{
		public:
			RutaId tipo;
			std::string ruta;
			std::string nombre;
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
