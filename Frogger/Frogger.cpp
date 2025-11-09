#include <raylib.h>
#include <iostream>
#include <vector>

using namespace std;

const int tamanoCelda = 50;
const int numeroCeldas = 15;
const int anchoVentana = 750;
const int altoVentana = 750;

const float tiempoScroll = 0.3f;
float temporizadorScroll = 0.0f;

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
	bool estaVivo;
	Texture2D texturaActual;
	Texture2D texturaArriba;
	Texture2D texturaAbajo;
	Texture2D texturaIzquierda;
	Texture2D texturaDerecha;
};

Jugador jugador1;
Jugador jugador2;
vector<Fila> mapa;
vector<Carro> carros;
vector<int> indicesFilasCalle;

TipoTerreno proximoTipoFila = CALLE;
int filasDeTipoRestantes = 0;
bool proximaDireccionCalle = true;
int proximoIndiceGenerar = 0;


Celda generarCelda(TipoTerreno nuevoTerreno);
Fila crearFila(int posicionY);
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

Fila crearFila(int posicionY)
{
	Fila nuevaFila;

	if (filasDeTipoRestantes <= 0)
	{
		if (proximoTipoFila == TIERRA)
		{
			proximoTipoFila = CALLE;
			filasDeTipoRestantes = GetRandomValue(3, 5);
		}
		else
		{
			proximoTipoFila = TIERRA;
			filasDeTipoRestantes = 1;
		}
	}

	nuevaFila.terreno = proximoTipoFila;
	nuevaFila.posicion = posicionY;
	nuevaFila.vaHaciaDerecha = proximaDireccionCalle;

	if (proximoTipoFila == CALLE)
	{
		proximaDireccionCalle = !proximaDireccionCalle;
	}

	for (int i = 0; i < numeroCeldas; i++)
	{
		nuevaFila.celdas.push_back(generarCelda(proximoTipoFila));
	}

	filasDeTipoRestantes--;
	return nuevaFila;
}


void reiniciarJuego()
{
	jugador1.posicion = { 4 * (float)tamanoCelda, (numeroCeldas - 1) * (float)tamanoCelda };
	jugador2.posicion = { 9 * (float)tamanoCelda, (numeroCeldas - 1) * (float)tamanoCelda };

	jugador1.texturaActual = jugador1.texturaArriba;
	jugador2.texturaActual = jugador2.texturaArriba;

	jugador1.estaVivo = true;
	jugador2.estaVivo = true;
	carros.clear();

	mapa.clear();
	indicesFilasCalle.clear();

	proximoTipoFila = CALLE;
	filasDeTipoRestantes = 0;
	proximaDireccionCalle = true;

	for (int i = numeroCeldas - 1; i >= 0; i--)
	{
		mapa.push_back(crearFila(i));
	}
	proximoIndiceGenerar = -1;

	temporizadorScroll = 0.0f;
}

void actualizarJuego()
{
	if (jugador1.posicion.y >= altoVentana)
		jugador1.estaVivo = false;
	if (jugador2.posicion.y >= altoVentana)
		jugador2.estaVivo = false;

	if (!jugador1.estaVivo && !jugador2.estaVivo)
	{
		if (IsKeyPressed(KEY_ENTER))
		{
			reiniciarJuego();
		}
		return;
	}

	temporizadorScroll += GetFrameTime();
	if (temporizadorScroll >= tiempoScroll)
	{
		temporizadorScroll -= tiempoScroll;
		mapa.push_back(crearFila(proximoIndiceGenerar));

		for (Fila& fila : mapa)
		{
			fila.posicion++;
		}
		for (Carro& carro : carros)
		{
			carro.posicion.y += tamanoCelda;
		}

		jugador1.posicion.y += tamanoCelda;
		jugador2.posicion.y += tamanoCelda;


		for (int i = mapa.size() - 1; i >= 0; i--)
		{
			if (mapa[i].posicion >= numeroCeldas)
			{
				mapa.erase(mapa.begin() + i);
			}
		}
		for (int i = carros.size() - 1; i >= 0; i--)
		{
			if (carros[i].posicion.y >= altoVentana)
			{
				carros.erase(carros.begin() + i);
			}
		}
	}

	if (jugador1.estaVivo)
	{
		if (IsKeyPressed(KEY_W))
		{
			jugador1.texturaActual = jugador1.texturaArriba;
			if (jugador1.posicion.y != 0 &&
				!(jugador2.posicion.y == jugador1.posicion.y - tamanoCelda && jugador2.posicion.x == jugador1.posicion.x && jugador2.estaVivo))
				jugador1.posicion.y -= tamanoCelda;
		}
		if (IsKeyPressed(KEY_S))
		{
			jugador1.texturaActual = jugador1.texturaAbajo;
			if (jugador1.posicion.y != GetScreenHeight() - tamanoCelda &&
				!(jugador2.posicion.y == jugador1.posicion.y + tamanoCelda && jugador2.posicion.x == jugador1.posicion.x && jugador2.estaVivo))
				jugador1.posicion.y += tamanoCelda;
		}
		if (IsKeyPressed(KEY_A))
		{
			jugador1.texturaActual = jugador1.texturaIzquierda;
			if (jugador1.posicion.x != 0 &&
				!(jugador2.posicion.y == jugador1.posicion.y && jugador2.posicion.x == jugador1.posicion.x - tamanoCelda && jugador2.estaVivo))
				jugador1.posicion.x -= tamanoCelda;
		}
		if (IsKeyPressed(KEY_D))
		{
			jugador1.texturaActual = jugador1.texturaDerecha;
			if (jugador1.posicion.x != GetScreenWidth() - tamanoCelda &&
				!(jugador2.posicion.y == jugador1.posicion.y && jugador2.posicion.x == jugador1.posicion.x + tamanoCelda && jugador2.estaVivo))
				jugador1.posicion.x += tamanoCelda;
		}
	}

	if (jugador2.estaVivo)
	{
		if (IsKeyPressed(KEY_UP))
		{
			jugador2.texturaActual = jugador2.texturaArriba;
			if (jugador2.posicion.y != 0 &&
				!(jugador1.posicion.y == jugador2.posicion.y - tamanoCelda && jugador1.posicion.x == jugador2.posicion.x && jugador1.estaVivo))
				jugador2.posicion.y -= tamanoCelda;
		}
		if (IsKeyPressed(KEY_DOWN))
		{
			jugador2.texturaActual = jugador2.texturaAbajo;
			if (jugador2.posicion.y != GetScreenHeight() - tamanoCelda &&
				!(jugador1.posicion.y == jugador2.posicion.y + tamanoCelda && jugador1.posicion.x == jugador2.posicion.x && jugador1.estaVivo))
				jugador2.posicion.y += tamanoCelda;
		}
		if (IsKeyPressed(KEY_LEFT))
		{
			jugador2.texturaActual = jugador2.texturaIzquierda;
			if (jugador2.posicion.x != 0 &&
				!(jugador1.posicion.y == jugador2.posicion.y && jugador1.posicion.x == jugador2.posicion.x - tamanoCelda && jugador1.estaVivo))
				jugador2.posicion.x -= tamanoCelda;
		}
		if (IsKeyPressed(KEY_RIGHT))
		{
			jugador2.texturaActual = jugador2.texturaDerecha;
			if (jugador2.posicion.x != GetScreenWidth() - tamanoCelda &&
				!(jugador1.posicion.y == jugador2.posicion.y && jugador1.posicion.x == jugador2.posicion.x + tamanoCelda && jugador1.estaVivo))
				jugador2.posicion.x += tamanoCelda;
		}
	}

	indicesFilasCalle.clear();
	for (int i = 0; i < mapa.size(); i++)
	{
		if (mapa[i].terreno == CALLE && mapa[i].posicion >= 0 && mapa[i].posicion < numeroCeldas)
		{
			indicesFilasCalle.push_back(i);
		}
	}

	const int chanceGenerar = 10;
	if (GetRandomValue(1, 100) <= chanceGenerar && !indicesFilasCalle.empty())
	{
		Carro nuevoCarro;
		nuevoCarro.tamano = { (float)tamanoCelda * 2, (float)tamanoCelda };

		int indiceAleatorio = GetRandomValue(0, indicesFilasCalle.size() - 1);
		int indiceEnMapa = indicesFilasCalle[indiceAleatorio];
		Fila& filaDeCalle = mapa[indiceEnMapa];

		int indiceFilaY = filaDeCalle.posicion;
		bool vaHaciaDerecha = filaDeCalle.vaHaciaDerecha;
		float velocidadAleatoria = 150.0;

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

	Rectangle jugador1Hurtbox = { jugador1.posicion.x + 10, jugador1.posicion.y + 10, jugador1.tamano.x - 20, jugador1.tamano.y - 20 };
	Rectangle jugador2Hurtbox = { jugador2.posicion.x + 10, jugador2.posicion.y + 10, jugador2.tamano.x - 20, jugador2.tamano.y - 20 };

	for (const Carro& carro : carros)
	{
		Rectangle carroHitbox = { carro.posicion.x, carro.posicion.y, carro.tamano.x, carro.tamano.y };

		if (jugador1.estaVivo && CheckCollisionRecs(jugador1Hurtbox, carroHitbox))
			jugador1.estaVivo = false;
		if (jugador2.estaVivo && CheckCollisionRecs(jugador2Hurtbox, carroHitbox))
			jugador2.estaVivo = false;
	}
}

void dibujarJuego()
{
	for (const Fila& fila : mapa)
	{
		float posY = fila.posicion * (float)tamanoCelda;
		if (posY > altoVentana || posY < -tamanoCelda)
			continue;

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
		jugador1.texturaActual,
		{ 0.0f, 0.0f, (float)jugador1.texturaActual.width, (float)jugador1.texturaActual.height },
		{ jugador1.posicion.x, jugador1.posicion.y, jugador1.tamano.x, jugador1.tamano.y },
		{ 0.0f, 0.0f },
		0.0f,
		WHITE
	);

	DrawTexturePro(
		jugador2.texturaActual,
		{ 0.0f, 0.0f, (float)jugador2.texturaActual.width, (float)jugador2.texturaActual.height },
		{ jugador2.posicion.x, jugador2.posicion.y, jugador2.tamano.x, jugador2.tamano.y },
		{ 0.0f, 0.0f },
		0.0f,
		WHITE
	);
}

void dibujarUI()
{
	
}


void inicializarJuego()
{
	jugador1.tamano = { (float)tamanoCelda, (float)tamanoCelda };
	jugador1.texturaArriba = LoadTexture("green_frog_up.png");
	jugador1.texturaAbajo = LoadTexture("green_frog_down.png");
	jugador1.texturaIzquierda = LoadTexture("green_frog_left.png");
	jugador1.texturaDerecha = LoadTexture("green_frog_right.png");

	jugador2.tamano = { (float)tamanoCelda, (float)tamanoCelda };
	jugador2.texturaArriba = LoadTexture("red_frog_up.png");
	jugador2.texturaAbajo = LoadTexture("red_frog_down.png");
	jugador2.texturaIzquierda = LoadTexture("red_frog_left.png");
	jugador2.texturaDerecha = LoadTexture("red_frog_right.png");

	reiniciarJuego();
}

int main()
{
	InitWindow(anchoVentana, altoVentana, "Frogger x86");
	SetTargetFPS(144);

	inicializarJuego();

	while (WindowShouldClose() == false)
	{
		actualizarJuego();

		BeginDrawing();
		ClearBackground(SKYBLUE);

		dibujarJuego();

		dibujarUI();

		EndDrawing();
	}

	UnloadTexture(jugador1.texturaArriba);
	UnloadTexture(jugador1.texturaAbajo);
	UnloadTexture(jugador1.texturaIzquierda);
	UnloadTexture(jugador1.texturaDerecha);
	UnloadTexture(jugador2.texturaArriba);
	UnloadTexture(jugador2.texturaAbajo);
	UnloadTexture(jugador2.texturaIzquierda);
	UnloadTexture(jugador2.texturaDerecha);

	CloseWindow();
	return 0;
}