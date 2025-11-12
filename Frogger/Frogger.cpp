#include <raylib.h>
#include <iostream>
#include <vector>

using namespace std;

const int tamanoCelda = 50;
const int numeroCeldas = 15;
const int anchoVentana = 750;
const int altoVentana = 750;

float tiempoScroll = 0.0f;
float temporizadorScroll = 0.0f;
float tiempoSpawnCarro = 1.0f;
float velocidadCarro = 0.0f;
float temporizadorSpawnCarro = 0.0f;
int intervaloCalles = 0;
bool juegoIniciado = false;
bool esPrimerScroll = true;

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

struct Boton
{
	Vector2 posicion;
	Vector2 tamano;
	Texture2D textura;
};

struct Fondo
{
	Vector2 posicion;
	Vector2 tamano;
	Texture2D textura;
};

Jugador jugador1;
Jugador jugador2;
Boton botonFacil;
Boton botonNormal;
Boton botonDificil;
Fondo fondoMenu;
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
void dibujarMenu();
void PoblarFilaDeCalle(Fila& fila);


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
	for (Fila& fila : mapa)
	{
		if (fila.terreno == CALLE)
		{
			PoblarFilaDeCalle(fila);
		}
	}
	temporizadorScroll = 0.0f;
	temporizadorSpawnCarro = 0.0f;
	juegoIniciado = false;
	esPrimerScroll = true;
}

void PoblarFilaDeCalle(Fila& fila)
{
	int numCarrosAGenerar = GetRandomValue(2, 4);
	for (int i = 0; i < numCarrosAGenerar; i++)
	{
		Carro nuevoCarro;
		nuevoCarro.tamano = { (float)tamanoCelda * 2, (float)tamanoCelda };
		float posY = (float)fila.posicion * tamanoCelda;

		int maxCeldaX = numeroCeldas - (int)(nuevoCarro.tamano.x / tamanoCelda);
		int celdaXAleatoria = GetRandomValue(0, maxCeldaX);
		float posX = (float)celdaXAleatoria * tamanoCelda;

		nuevoCarro.posicion = { posX, posY };
		nuevoCarro.velocidad = fila.vaHaciaDerecha ? velocidadCarro : -velocidadCarro;

		Rectangle rectNuevoCarro = { nuevoCarro.posicion.x, nuevoCarro.posicion.y, nuevoCarro.tamano.x, nuevoCarro.tamano.y };

		bool haySolapamiento = false;
		for (const Carro& carroExistente : carros)
		{
			if (carroExistente.posicion.y != posY) continue;

			Rectangle rectCarroExistente = { carroExistente.posicion.x, carroExistente.posicion.y, carroExistente.tamano.x, carroExistente.tamano.y };

			Rectangle rectNuevoConEspacio = rectNuevoCarro;
			rectNuevoConEspacio.x -= tamanoCelda;
			rectNuevoConEspacio.width += tamanoCelda * 2;

			if (CheckCollisionRecs(rectNuevoConEspacio, rectCarroExistente))
			{
				haySolapamiento = true;
				break;
			}
		}

		if (!haySolapamiento)
		{
			carros.push_back(nuevoCarro);
		}
	}
}

void actualizarJuego()
{
	if (!juegoIniciado)
	{
		Vector2 mousePos = GetMousePosition();

		Rectangle rectFacil = { botonFacil.posicion.x, botonFacil.posicion.y, botonFacil.tamano.x, botonFacil.tamano.y };
		Rectangle rectNormal = { botonNormal.posicion.x, botonNormal.posicion.y, botonNormal.tamano.x, botonNormal.tamano.y };
		Rectangle rectDificil = { botonDificil.posicion.x, botonDificil.posicion.y, botonDificil.tamano.x, botonDificil.tamano.y };

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		{
			if (CheckCollisionPointRec(mousePos, rectFacil))
			{
				tiempoScroll = 1.0f;
				velocidadCarro = 150.0f;
				reiniciarJuego();
				juegoIniciado = true;
			}
			else if (CheckCollisionPointRec(mousePos, rectNormal))
			{
				tiempoScroll = 0.5f;
				velocidadCarro = 200.0f;
				reiniciarJuego();
				juegoIniciado = true;
			}
			else if (CheckCollisionPointRec(mousePos, rectDificil))
			{
				tiempoScroll = 0.2f;
				velocidadCarro = 300.0f;
				reiniciarJuego();
				juegoIniciado = true;
			}
		}
		return;
	}
	if (jugador1.posicion.y >= altoVentana)
		jugador1.estaVivo = false;
	if (jugador2.posicion.y >= altoVentana)
		jugador2.estaVivo = false;

	if (!jugador1.estaVivo && !jugador2.estaVivo)
	{
		if (IsKeyPressed(KEY_R))
		{
			reiniciarJuego();
		}
		return;
	}

	temporizadorScroll += GetFrameTime();
	float tiempoLimiteActual = tiempoScroll;
	if (esPrimerScroll)
		tiempoLimiteActual = 3.0f;
	if (temporizadorScroll >= tiempoLimiteActual)
	{
		if (esPrimerScroll)
		{
			temporizadorScroll = 0.0f;
			esPrimerScroll = false;
		}
		else
		{
			temporizadorScroll -= tiempoScroll;
		}
		mapa.push_back(crearFila(proximoIndiceGenerar));

		Fila& filaRecienCreada = mapa.back();
		if (filaRecienCreada.terreno == CALLE)
		{
			PoblarFilaDeCalle(filaRecienCreada);
		}

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

	temporizadorSpawnCarro += GetFrameTime();

	if (temporizadorSpawnCarro >= tiempoSpawnCarro && !indicesFilasCalle.empty())
	{
		temporizadorSpawnCarro = 0.0f;
		tiempoSpawnCarro = (float)GetRandomValue(5, 15) / 10.0f;

		Carro nuevoCarro;
		nuevoCarro.tamano = { (float)tamanoCelda * 2, (float)tamanoCelda };

		int indiceAleatorio = GetRandomValue(0, indicesFilasCalle.size() - 1);
		int indiceEnMapa = indicesFilasCalle[indiceAleatorio];
		Fila& filaDeCalle = mapa[indiceEnMapa];

		int indiceFilaY = filaDeCalle.posicion;
		float posY = (float)indiceFilaY * tamanoCelda;
		bool vaHaciaDerecha = filaDeCalle.vaHaciaDerecha;

		float posX;
		if (vaHaciaDerecha)
			posX = -nuevoCarro.tamano.x;
		else
			posX = (float)anchoVentana;

		nuevoCarro.posicion = { posX, posY };
		nuevoCarro.velocidad = vaHaciaDerecha ? velocidadCarro : -velocidadCarro;

		Rectangle rectNuevoCarro = { nuevoCarro.posicion.x, nuevoCarro.posicion.y, nuevoCarro.tamano.x, nuevoCarro.tamano.y };

		bool haySolapamiento = false;

		for (const Carro& carroExistente : carros)
		{
			if (carroExistente.posicion.y != posY) continue;

			Rectangle rectCarroExistente = { carroExistente.posicion.x, carroExistente.posicion.y, carroExistente.tamano.x, carroExistente.tamano.y };

			Rectangle rectNuevoConEspacio = rectNuevoCarro;
			rectNuevoConEspacio.x -= tamanoCelda;
			rectNuevoConEspacio.width += tamanoCelda * 2;

			if (CheckCollisionRecs(rectNuevoConEspacio, rectCarroExistente))
			{
				haySolapamiento = true;
				break;
			}
		}

		if (!haySolapamiento)
		{
			carros.push_back(nuevoCarro);
		}
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

void dibujarMenu()
{
	DrawTexturePro(
		fondoMenu.textura,
		{ 0.0f, 0.0f, (float)fondoMenu.textura.width, (float)fondoMenu.textura.height },
		{ 0.0f, 0.0f, 750, 750},
		{ 0.0f, 0.0f },
		0.0f,
		WHITE
	);

	DrawTexturePro(
		botonFacil.textura,
		{ 0.0f, 0.0f, (float)botonFacil.textura.width, (float)botonFacil.textura.height },
		{ botonFacil.posicion.x, botonFacil.posicion.y, botonFacil.tamano.x, botonFacil.tamano.y },
		{ 0.0f, 0.0f },
		0.0f,
		WHITE
	);

	DrawTexturePro(
		botonNormal.textura,
		{ 0.0f, 0.0f, (float)botonNormal.textura.width, (float)botonNormal.textura.height },
		{ botonNormal.posicion.x, botonNormal.posicion.y, botonNormal.tamano.x, botonNormal.tamano.y },
		{ 0.0f, 0.0f },
		0.0f,
		WHITE
	);

	DrawTexturePro(
		botonDificil.textura,
		{ 0.0f, 0.0f, (float)botonDificil.textura.width, (float)botonDificil.textura.height },
		{ botonDificil.posicion.x, botonDificil.posicion.y, botonDificil.tamano.x, botonDificil.tamano.y },
		{ 0.0f, 0.0f },
		0.0f,
		WHITE
	);
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

	fondoMenu.textura = LoadTexture("baseMenu.png");
	botonFacil.textura = LoadTexture("botonFacil.png");
	botonNormal.textura = LoadTexture("botonNormal.png");
	botonDificil.textura = LoadTexture("botonDificil.png");

	fondoMenu.posicion = { 0, 0 };
	fondoMenu.tamano = { (float)anchoVentana, (float)altoVentana };
	float btnAncho = 200.0f;
	float btnAlto = 100.0f;
	float btnPosX = (anchoVentana - btnAncho) / 2.0f;

	botonFacil.tamano = { btnAncho, btnAlto };
	botonFacil.posicion = { btnPosX, 350.0f };

	botonNormal.tamano = { btnAncho, btnAlto };
	botonNormal.posicion = { btnPosX, botonFacil.posicion.y + btnAlto + 20.0f};

	botonDificil.tamano = { btnAncho, btnAlto };
	botonDificil.posicion = { btnPosX, botonNormal.posicion.y + btnAlto + 20.0f};
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
		ClearBackground(DARKBROWN);

		if (!juegoIniciado)
		{
			dibujarMenu();
		}
		else
		{
			dibujarJuego();
		}


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
	UnloadTexture(fondoMenu.textura);
	UnloadTexture(botonFacil.textura);
	UnloadTexture(botonNormal.textura);
	UnloadTexture(botonDificil.textura);

	CloseWindow();
	return 0;
}