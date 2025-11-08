#include <raylib.h>
#include <iostream>
#include <vector>

using namespace std;

const int tamanoCelda = 50;
const int numeroCeldas = 15;

const int anchoVentana = 750;
const int altoVentana = 750;

enum TipoTerreno
{
	TIERRA,
	CALLE,
	AGUA
};

struct Celda
{
	Color color;
	bool solido;
	bool ocupado;
};

struct Fila
{
	TipoTerreno terreno;
	vector<Celda> celdas;
	int posicion;
	bool vaHaciaDerecha;
};

struct Carro
{
	Vector2 posicion;
	Vector2 tamano;
	float velocidad;
};

struct Jugador
{
	Vector2 posicion;
	Vector2 tamano;
	Texture2D textura;
};

Jugador jugador1;
vector<Fila> mapa;
vector<Carro> carros;
vector<int> indicesFilasCalle;
bool jugadorEstaVivo;


Celda generarCelda(TipoTerreno nuevoTerreno);
void inicializarJuego();
void reiniciarJuego();
void actualizarJuego();
void dibujarJuego();
void dibujarUI();


Celda generarCelda(TipoTerreno nuevoTerreno)
{
	switch (nuevoTerreno)
	{
	case TIERRA:
		return { GREEN, false, false };
	case CALLE:
		return { DARKGRAY, false, false };
	case AGUA:
		return { BLUE, false, false };
	}
	return { BLACK, false, false };
}

void reiniciarJuego()
{
	jugador1.posicion = { (numeroCeldas / 2) * (float)tamanoCelda, (numeroCeldas - 1) * (float)tamanoCelda };
	carros.clear();
	jugadorEstaVivo = true;

}

void actualizarJuego()
{
	if (!jugadorEstaVivo)
	{
		if (IsKeyPressed(KEY_ENTER))
		{
			reiniciarJuego();
		}
		return;
	}

	if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
		jugador1.posicion.y -= tamanoCelda;
	if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
		jugador1.posicion.y += tamanoCelda;
	if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
		jugador1.posicion.x -= tamanoCelda;
	if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
		jugador1.posicion.x += tamanoCelda;



	const int chanceGenerar = 3;
	if (GetRandomValue(1, 100) <= chanceGenerar && !indicesFilasCalle.empty())
	{
		Carro nuevoCarro;
		nuevoCarro.tamano = { (float)tamanoCelda * 2, (float)tamanoCelda };

		int indiceAleatorio = GetRandomValue(0, indicesFilasCalle.size() - 1);
		int indiceEnMapa = indicesFilasCalle[indiceAleatorio];
		Fila& filaDeCalle = mapa[indiceEnMapa];

		int indiceFilaY = filaDeCalle.posicion;
		bool vaHaciaDerecha = filaDeCalle.vaHaciaDerecha;
		float velocidadAleatoria = (float)GetRandomValue(80, 200);

		if (vaHaciaDerecha)
		{
			nuevoCarro.posicion = { -nuevoCarro.tamano.x, (float)indiceFilaY * tamanoCelda };
			nuevoCarro.velocidad = velocidadAleatoria;
		}
		else
		{
			nuevoCarro.posicion = { (float)anchoVentana, (float)indiceFilaY * tamanoCelda };
			nuevoCarro.velocidad = -velocidadAleatoria;
		}
		carros.push_back(nuevoCarro);
	}

	for (int i = carros.size() - 1; i >= 0; i--)
	{
		carros[i].posicion.x += carros[i].velocidad * GetFrameTime();
		bool fueraIzquierda = (carros[i].velocidad < 0 && carros[i].posicion.x + carros[i].tamano.x < 0);
		bool fueraDerecha = (carros[i].velocidad > 0 && carros[i].posicion.x > anchoVentana);

		if (fueraIzquierda || fueraDerecha)
		{
			carros.erase(carros.begin() + i);
		}
	}

	Rectangle jugadorHurtbox = { jugador1.posicion.x + 10, jugador1.posicion.y + 10, jugador1.tamano.x - 20, jugador1.tamano.y - 20 };
	for (const Carro& carro : carros)
	{
		Rectangle carroHitbox = { carro.posicion.x, carro.posicion.y, carro.tamano.x, carro.tamano.y };
		if (CheckCollisionRecs(jugadorHurtbox, carroHitbox))
		{
			jugadorEstaVivo = false;
			break;
		}
	}
}

void dibujarJuego()
{
	for (const Fila& fila : mapa)
	{
		float posY = fila.posicion * (float)tamanoCelda;
		for (int i = 0; i < fila.celdas.size(); i++)
		{
			float posX = i * (float)tamanoCelda;
			DrawRectangle(posX, posY, tamanoCelda, tamanoCelda, fila.celdas[i].color);
		}
	}

	for (const Carro& carro : carros)
	{
		DrawRectangleV(carro.posicion, carro.tamano, RED);
	}

	DrawTexturePro(
		jugador1.textura,
		{ 0.0f, 0.0f, (float)jugador1.textura.width, (float)jugador1.textura.height },
		{ jugador1.posicion.x, jugador1.posicion.y, jugador1.tamano.x, jugador1.tamano.y },
		{ 0.0f, 0.0f },
		0.0f,
		WHITE
	);
}



void inicializarJuego()
{
	jugador1.tamano = { (float)tamanoCelda, (float)tamanoCelda };
	jugador1.textura = LoadTexture("green_frog.png");

	int indiceFilaActual = numeroCeldas - 1;

	mapa.clear();
	indicesFilasCalle.clear();

	Fila filaInicio;
	filaInicio.posicion = indiceFilaActual;
	filaInicio.terreno = TIERRA;
	for (int i = 0; i < numeroCeldas; i++)
		filaInicio.celdas.push_back(generarCelda(TIERRA));
	mapa.push_back(filaInicio);
	indiceFilaActual--;

	bool vaHaciaDerecha = true;

	for (int i = 0; i < 4; i++)
	{
		Fila filaCalle;
		filaCalle.posicion = indiceFilaActual;
		filaCalle.terreno = CALLE;
		filaCalle.vaHaciaDerecha = vaHaciaDerecha;
		for (int j = 0; j < numeroCeldas; j++)
			filaCalle.celdas.push_back(generarCelda(CALLE));
		mapa.push_back(filaCalle);
		indicesFilasCalle.push_back(mapa.size() - 1);
		indiceFilaActual--;
		vaHaciaDerecha = !vaHaciaDerecha;
	}

	Fila fila2;
	fila2.posicion = indiceFilaActual;
	fila2.terreno = TIERRA;
	for (int i = 0; i < numeroCeldas; i++)
		fila2.celdas.push_back(generarCelda(TIERRA));
	mapa.push_back(fila2);
	indiceFilaActual--;

	for (int i = 0; i < 4; i++)
	{
		Fila filaCalle;
		filaCalle.posicion = indiceFilaActual;
		filaCalle.terreno = CALLE;
		filaCalle.vaHaciaDerecha = vaHaciaDerecha;
		for (int j = 0; j < numeroCeldas; j++)
			filaCalle.celdas.push_back(generarCelda(CALLE));
		mapa.push_back(filaCalle);
		indicesFilasCalle.push_back(mapa.size() - 1);
		indiceFilaActual--;
		vaHaciaDerecha = !vaHaciaDerecha;
	}

	Fila fila3;
	fila3.posicion = indiceFilaActual;
	fila3.terreno = TIERRA;
	for (int i = 0; i < numeroCeldas; i++)
		fila3.celdas.push_back(generarCelda(TIERRA));
	mapa.push_back(fila3);
	indiceFilaActual--;

	for (int i = 0; i < 3; i++)
	{
		Fila filaCalle;
		filaCalle.posicion = indiceFilaActual;
		filaCalle.terreno = CALLE;
		filaCalle.vaHaciaDerecha = vaHaciaDerecha;
		for (int j = 0; j < numeroCeldas; j++)
			filaCalle.celdas.push_back(generarCelda(CALLE));
		mapa.push_back(filaCalle);
		indicesFilasCalle.push_back(mapa.size() - 1);
		indiceFilaActual--;
		vaHaciaDerecha = !vaHaciaDerecha;
	}

	Fila fila5;
	fila5.posicion = indiceFilaActual;
	fila5.terreno = AGUA;
	for (int i = 0; i < numeroCeldas; i++)
		fila5.celdas.push_back(generarCelda(AGUA));
	mapa.push_back(fila5);

	reiniciarJuego();
}

int main()
{
	InitWindow(anchoVentana, altoVentana, "Frogger x86");
	SetTargetFPS(60);

	inicializarJuego();

	while (WindowShouldClose() == false)
	{
		actualizarJuego();

		BeginDrawing();
		ClearBackground(SKYBLUE);

		dibujarJuego();

		EndDrawing();
	}

	UnloadTexture(jugador1.textura);
	CloseWindow();
	return 0;
}