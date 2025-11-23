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

Texture2D texturaCarroIzquierda;
Texture2D texturaCarroDerecha;

Music ost;
Sound sonidoChoque;
Sound sonidoRana;

enum TipoTerreno
{
	TIERRA,
	CALLE,
	AGUA
};

struct Celda
{
	Color color;
	Texture2D textura;
	bool solido;
	bool ocupado;
};

struct Fila
{
	TipoTerreno terreno;
	vector<Celda> celdas;
	int posicion;
	bool vaHaciaDerecha;
	bool fueVisitado;
};

struct Carro
{
	Vector2 posicion;
	Vector2 tamano;
	float velocidad;
	Texture2D textura;
};

struct Jugador
{
	Vector2 posicion;
	Vector2 tamano;
	bool estaVivo;
	int puntuacion;
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
vector<Texture2D> texturasCarroIzquierda;
vector<Texture2D> texturasCarroDerecha;

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
void llenarCalleDeCarros(Fila& fila);


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
	nuevaFila.fueVisitado = false;

	if (proximoTipoFila == CALLE)
		proximaDireccionCalle = !proximaDireccionCalle;

	for (int i = 0; i < numeroCeldas; i++)
		nuevaFila.celdas.push_back(generarCelda(proximoTipoFila));

	filasDeTipoRestantes--;
	return nuevaFila;
}


void reiniciarJuego()
{
	__asm
	{
		mov eax, 200
		mov ebx, 700
		cvtsi2ss xmm0, eax
		cvtsi2ss xmm1, ebx
		movss jugador1.posicion.x, xmm0
		movss jugador1.posicion.y, xmm1

		mov eax, 450
		mov ebx, 700
		cvtsi2ss xmm0, eax
		cvtsi2ss xmm1, ebx
		movss jugador2.posicion.x, xmm0
		movss jugador2.posicion.y, xmm1

		mov al, 1
		mov bl, 1
		mov jugador1.estaVivo, al
		mov jugador2.estaVivo, bl

		mov eax, -10
		mov ebx, -10
		mov jugador1.puntuacion, eax
		mov jugador2.puntuacion, ebx

		lea esi, jugador1.texturaArriba
		lea edi, jugador1.texturaActual
		mov ecx, 20
		mov eax, 0
		COPIAR_TEXTURA_JUGADOR1_ARRIBA2:
		mov bl, [esi + eax]
			mov[edi + eax], bl
			inc eax
		loop COPIAR_TEXTURA_JUGADOR1_ARRIBA2
		lea esi, jugador2.texturaArriba
		lea edi, jugador2.texturaActual
		mov ecx, 20
		mov eax, 0
		COPIAR_TEXTURA_JUGADOR2_ARRIBA2:
			mov bl, [esi+eax]
			mov [edi+eax], bl
			inc eax
		loop COPIAR_TEXTURA_JUGADOR2_ARRIBA2

		mov eax, 0
		mov filasDeTipoRestantes, eax

		mov al, 1
		mov proximaDireccionCalle, al

		mov eax, 0
		cvtsi2ss xmm0, eax
		movss temporizadorScroll, xmm0

		mov eax, 0
		cvtsi2ss xmm0, eax
		movss temporizadorSpawnCarro, xmm0

		mov al, 0
		mov juegoIniciado, al

		mov al, 1
		mov esPrimerScroll, al
	}

	proximoTipoFila = CALLE;

	carros.clear();

	mapa.clear();
	indicesFilasCalle.clear();

	for (int i = numeroCeldas - 1; i >= 0; i--)
		mapa.push_back(crearFila(i));
	proximoIndiceGenerar = -1;
	for (Fila& fila : mapa)
		if (fila.terreno == CALLE)
			llenarCalleDeCarros(fila);

}

void llenarCalleDeCarros(Fila& fila)
{
	Carro nuevoCarro;
	float posY, posX;
	bool seSolapan;
	int indice, numCarrosAGenerar;
	numCarrosAGenerar = GetRandomValue(2, 4);

	for (int i = 0; i < numCarrosAGenerar; i++)
	{
		nuevoCarro;
		nuevoCarro.tamano = { (float)tamanoCelda * 2, (float)tamanoCelda };
		posY = (float)fila.posicion * tamanoCelda;

		int maxCeldaX = numeroCeldas - (int)(nuevoCarro.tamano.x / tamanoCelda);
		int celdaXAleatoria = GetRandomValue(0, maxCeldaX);
		posX = (float)celdaXAleatoria * tamanoCelda;

		nuevoCarro.posicion = { posX, posY };
		nuevoCarro.velocidad = fila.vaHaciaDerecha ? velocidadCarro : -velocidadCarro;
		if (fila.vaHaciaDerecha)
		{
			indice = GetRandomValue(0, texturasCarroDerecha.size() - 1);
			nuevoCarro.textura = texturasCarroDerecha[indice];
		}
		else
		{
			indice = GetRandomValue(0, texturasCarroIzquierda.size() - 1);
			nuevoCarro.textura = texturasCarroIzquierda[indice];
		}

		Rectangle rectNuevoCarro = { nuevoCarro.posicion.x, nuevoCarro.posicion.y, nuevoCarro.tamano.x, nuevoCarro.tamano.y };

		seSolapan = false;
		for (const Carro& carroExistente : carros)
		{
			if (carroExistente.posicion.y != posY) 
				continue;

			Rectangle rectCarroExistente = { carroExistente.posicion.x, carroExistente.posicion.y, carroExistente.tamano.x, carroExistente.tamano.y };

			Rectangle rectNuevoConEspacio = rectNuevoCarro;
			rectNuevoConEspacio.x -= tamanoCelda;
			rectNuevoConEspacio.width += tamanoCelda * 2;

			if (CheckCollisionRecs(rectNuevoConEspacio, rectCarroExistente))
			{
				seSolapan = true;
				break;
			}
		}

		if (!seSolapan)
			carros.push_back(nuevoCarro);
	}
}

void actualizarJuego()
{
	int indiceTexturas, indiceAleatorio, indiceEnMapa;
	Vector2 mousePos;
	float tiempoLimiteActual;
	bool vaHaciaDerecha;

	if (juegoIniciado)
	{
		UpdateMusicStream(ost);
		if (GetMusicTimePlayed(ost) >= GetMusicTimeLength(ost))
		{
			StopMusicStream(ost);
			PlayMusicStream(ost);
		}
	}
	if (!juegoIniciado)
	{
		mousePos = GetMousePosition();

		Rectangle rectFacil = { botonFacil.posicion.x, botonFacil.posicion.y, botonFacil.tamano.x, botonFacil.tamano.y };
		Rectangle rectNormal = { botonNormal.posicion.x, botonNormal.posicion.y, botonNormal.tamano.x, botonNormal.tamano.y };
		Rectangle rectDificil = { botonDificil.posicion.x, botonDificil.posicion.y, botonDificil.tamano.x, botonDificil.tamano.y };

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
			if (CheckCollisionPointRec(mousePos, rectFacil) || IsKeyPressed(KEY_ONE))
			{
				__asm
				{
					mov eax, 0x3F000000 //0.5
					movd xmm0, eax
					movss tiempoScroll, xmm0
					
					mov eax, 100
					cvtsi2ss xmm0, eax 
					movss velocidadCarro, xmm0

					call reiniciarJuego

					mov al, 1
					mov juegoIniciado, al
				}
				PlayMusicStream(ost);
			}
			else if (CheckCollisionPointRec(mousePos, rectNormal) || IsKeyPressed(KEY_TWO))
			{
				__asm
				{
					mov eax, 0x3E999999 // 0.3
					movd xmm0, eax
					movss tiempoScroll, xmm0

					mov eax, 150
					cvtsi2ss xmm0, eax
					movss velocidadCarro, xmm0

					call reiniciarJuego

					mov al, 1
					mov juegoIniciado, al
				}
				PlayMusicStream(ost);
			}
			else if (CheckCollisionPointRec(mousePos, rectDificil) || IsKeyPressed(KEY_THREE))
			{
				__asm
				{
					mov eax, 0x3E4CCCCD // 0.2
					movd xmm0, eax
					movss tiempoScroll, xmm0

					mov eax, 200
					cvtsi2ss xmm0, eax
					movss velocidadCarro, xmm0

					call reiniciarJuego

					mov al, 1
					mov juegoIniciado, al
				}
				PlayMusicStream(ost);
			}
		return;
	}
	if (jugador1.posicion.y >= altoVentana)
	{
		if (jugador1.estaVivo)
			PlaySound(sonidoRana);
		__asm
		{
			mov al, 0
			mov jugador1.estaVivo, al
		}
		jugador1.estaVivo = false;
	}
	if (jugador2.posicion.y >= altoVentana)
	{
		if (jugador2.estaVivo)
			PlaySound(sonidoRana);
		__asm
		{
			mov al, 0
			mov jugador2.estaVivo, al
		}
	}

	if (!jugador1.estaVivo && !jugador2.estaVivo)
	{
		StopMusicStream(ost);
		if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER))
			__asm
			{
				call reiniciarJuego
			}
		return;
	}
	__asm
	{
		call GetFrameTime
		fadd temporizadorScroll
		fstp temporizadorScroll

		movss xmm0, tiempoScroll
		movss tiempoLimiteActual, xmm0

		cmp esPrimerScroll, 0
		je PRIMER_SCROLL
			mov eax, 0x40666666; 3.6 < -ajustado a la música
			movd xmm0, eax
			movss tiempoLimiteActual, xmm0
		PRIMER_SCROLL:
	}
	if (temporizadorScroll >= tiempoLimiteActual)
	{
		__asm
		{
			je PRIMER_SCROLL2
				mov eax, 0
				cvtsi2ss xmm0, eax
				movss temporizadorScroll, xmm0
				mov al, 0
				mov esPrimerScroll, al
				jmp YA_PRIMER_SCROLL2
			PRIMER_SCROLL2:
			fld temporizadorScroll
			fsub tiempoScroll
			fstp temporizadorScroll
			YA_PRIMER_SCROLL2:
		}
		mapa.push_back(crearFila(proximoIndiceGenerar));

		Fila& filaRecienCreada = mapa.back();
		if (filaRecienCreada.terreno == CALLE)
			llenarCalleDeCarros(filaRecienCreada);

		for (Fila& fila : mapa)
			fila.posicion++;
		for (Carro& carro : carros)
			carro.posicion.y += tamanoCelda;

		jugador1.posicion.y += tamanoCelda;
		jugador2.posicion.y += tamanoCelda;


		for (int i = mapa.size() - 1; i >= 0; i--)
			if (mapa[i].posicion >= numeroCeldas)
				mapa.erase(mapa.begin() + i);
		for (int i = carros.size() - 1; i >= 0; i--)
			if (carros[i].posicion.y >= altoVentana)
				carros.erase(carros.begin() + i);
	}

	if (jugador1.estaVivo)
	{
		if (IsKeyPressed(KEY_W))
		{
			__asm
			{
				lea esi, jugador1.texturaArriba
				lea edi, jugador1.texturaActual
				mov ecx, 20
				mov eax, 0
				COPIAR_TEXTURA_JUGADOR1_ARRIBA:
					mov bl, [esi+eax]
					mov [edi+eax], bl
					inc eax
				loop COPIAR_TEXTURA_JUGADOR1_ARRIBA
			}
			if (jugador1.posicion.y != 0 &&
				!(jugador2.posicion.y == jugador1.posicion.y - tamanoCelda && jugador2.posicion.x == jugador1.posicion.x && jugador2.estaVivo))
				__asm 
				{
					fld jugador1.posicion.y
					fisub tamanoCelda
					fstp jugador1.posicion.y
				}
				
		}
		if (IsKeyPressed(KEY_S))
		{
			__asm
			{
				lea esi, jugador1.texturaAbajo
				lea edi, jugador1.texturaActual
				mov ecx, 20
				mov eax, 0
				COPIAR_TEXTURA_JUGADOR1_ABAJO:
				mov bl, [esi + eax]
					mov[edi + eax], bl
					inc eax
					loop COPIAR_TEXTURA_JUGADOR1_ABAJO
			}
			if (jugador1.posicion.y != GetScreenHeight() - tamanoCelda &&
				!(jugador2.posicion.y == jugador1.posicion.y + tamanoCelda && jugador2.posicion.x == jugador1.posicion.x && jugador2.estaVivo))
				__asm
				{
					fld jugador1.posicion.y
					fiadd tamanoCelda
					fstp jugador1.posicion.y
				}
		}
		if (IsKeyPressed(KEY_A))
		{
			__asm
			{
				lea esi, jugador1.texturaIzquierda
				lea edi, jugador1.texturaActual
				mov ecx, 20
				mov eax, 0
				COPIAR_TEXTURA_JUGADOR1_IZQUIERDA:
				mov bl, [esi + eax]
					mov[edi + eax], bl
					inc eax
					loop COPIAR_TEXTURA_JUGADOR1_IZQUIERDA
			}
			if (jugador1.posicion.x != 0 &&
				!(jugador2.posicion.y == jugador1.posicion.y && jugador2.posicion.x == jugador1.posicion.x - tamanoCelda && jugador2.estaVivo))
				__asm
				{
					fld jugador1.posicion.x
					fisub tamanoCelda
					fstp jugador1.posicion.x
				}
		}
		if (IsKeyPressed(KEY_D))
		{
			__asm
			{
				lea esi, jugador1.texturaDerecha
				lea edi, jugador1.texturaActual
				mov ecx, 20
				mov eax, 0
				COPIAR_TEXTURA_JUGADOR1_DERECHA:
				mov bl, [esi + eax]
					mov[edi + eax], bl
					inc eax
					loop COPIAR_TEXTURA_JUGADOR1_DERECHA
			}
			if (jugador1.posicion.x != GetScreenWidth() - tamanoCelda &&
				!(jugador2.posicion.y == jugador1.posicion.y && jugador2.posicion.x == jugador1.posicion.x + tamanoCelda && jugador2.estaVivo))
				__asm
				{
					fld jugador1.posicion.x
					fiadd tamanoCelda
					fstp jugador1.posicion.x
				}

		}
	}

	if (jugador2.estaVivo)
	{
		if (IsKeyPressed(KEY_UP))
		{
			__asm
			{
				lea esi, jugador2.texturaArriba
				lea edi, jugador2.texturaActual
				mov ecx, 20
				mov eax, 0
				COPIAR_TEXTURA_JUGADOR2_ARRIBA:
				mov bl, [esi + eax]
					mov[edi + eax], bl
					inc eax
					loop COPIAR_TEXTURA_JUGADOR2_ARRIBA
			}
			if (jugador2.posicion.y != 0 &&
				!(jugador1.posicion.y == jugador2.posicion.y - tamanoCelda && jugador1.posicion.x == jugador2.posicion.x && jugador1.estaVivo))
				__asm
				{
					fld jugador2.posicion.y
					fisub tamanoCelda
					fstp jugador2.posicion.y
				}
		}
		if (IsKeyPressed(KEY_DOWN))
		{
			__asm
			{
				lea esi, jugador2.texturaAbajo
				lea edi, jugador2.texturaActual
				mov ecx, 20
				mov eax, 0
				COPIAR_TEXTURA_JUGADOR2_ABAJO:
				mov bl, [esi + eax]
					mov[edi + eax], bl
					inc eax
					loop COPIAR_TEXTURA_JUGADOR2_ABAJO
			}
			if (jugador2.posicion.y != GetScreenHeight() - tamanoCelda &&
				!(jugador1.posicion.y == jugador2.posicion.y + tamanoCelda && jugador1.posicion.x == jugador2.posicion.x && jugador1.estaVivo))
				__asm
				{
					fld jugador2.posicion.y
					fiadd tamanoCelda
					fstp jugador2.posicion.y
				}
		}
		if (IsKeyPressed(KEY_LEFT))
		{
			__asm
			{
				lea esi, jugador2.texturaIzquierda
				lea edi, jugador2.texturaActual
				mov ecx, 20
				mov eax, 0
				COPIAR_TEXTURA_JUGADOR2_IZQUIERDA:
				mov bl, [esi + eax]
					mov[edi + eax], bl
					inc eax
					loop COPIAR_TEXTURA_JUGADOR2_IZQUIERDA
			}
			if (jugador2.posicion.x != 0 &&
				!(jugador1.posicion.y == jugador2.posicion.y && jugador1.posicion.x == jugador2.posicion.x - tamanoCelda && jugador1.estaVivo))
				__asm
				{
					fld jugador2.posicion.x
					fisub tamanoCelda
					fstp jugador2.posicion.x
				}
		}
		if (IsKeyPressed(KEY_RIGHT))
		{
			__asm
			{
				lea esi, jugador2.texturaDerecha
				lea edi, jugador2.texturaActual
				mov ecx, 20
				mov eax, 0
				COPIAR_TEXTURA_JUGADOR2_DERECHA:
				mov bl, [esi + eax]
					mov[edi + eax], bl
					inc eax
					loop COPIAR_TEXTURA_JUGADOR2_DERECHA
			}
			if (jugador2.posicion.x != GetScreenWidth() - tamanoCelda &&
				!(jugador1.posicion.y == jugador2.posicion.y && jugador1.posicion.x == jugador2.posicion.x + tamanoCelda && jugador1.estaVivo))
				__asm
				{
					fld jugador2.posicion.x
					fiadd tamanoCelda
					fstp jugador2.posicion.x
				}
		}
	}

	indicesFilasCalle.clear();
	for (int i = 0; i < mapa.size(); i++)
		if (mapa[i].terreno == CALLE && mapa[i].posicion >= 0 && mapa[i].posicion < numeroCeldas)
			indicesFilasCalle.push_back(i);

	temporizadorSpawnCarro += GetFrameTime();

	if (temporizadorSpawnCarro >= tiempoSpawnCarro && !indicesFilasCalle.empty())
	{
		int indiceFilaY;
		float posY, posX;
		bool vaHaciaDerecha;
		bool seSolapan;

		Carro nuevoCarro;

		temporizadorSpawnCarro = 0.0f;
		tiempoSpawnCarro = (float)GetRandomValue(5, 15) / 10.0f;

		nuevoCarro.tamano = { (float)tamanoCelda * 2, (float)tamanoCelda };

		indiceAleatorio = GetRandomValue(0, indicesFilasCalle.size() - 1);
		indiceEnMapa = indicesFilasCalle[indiceAleatorio];
		Fila& filaDeCalle = mapa[indiceEnMapa];

		indiceFilaY = filaDeCalle.posicion;
		posY = (float)indiceFilaY * tamanoCelda;
		vaHaciaDerecha = filaDeCalle.vaHaciaDerecha;

		if (vaHaciaDerecha)
			posX = -nuevoCarro.tamano.x;
		else
			posX = (float)anchoVentana;

		nuevoCarro.posicion = { posX, posY };
		nuevoCarro.velocidad = vaHaciaDerecha ? velocidadCarro : -velocidadCarro;
		if (vaHaciaDerecha)
		{
			indiceTexturas = GetRandomValue(0, texturasCarroDerecha.size() - 1);
			nuevoCarro.textura = texturasCarroDerecha[indiceTexturas];
		}
		else
		{
			indiceTexturas = GetRandomValue(0, texturasCarroIzquierda.size() - 1);
			nuevoCarro.textura = texturasCarroIzquierda[indiceTexturas];
		}

		Rectangle rectNuevoCarro = { nuevoCarro.posicion.x, nuevoCarro.posicion.y, nuevoCarro.tamano.x, nuevoCarro.tamano.y };

		seSolapan = false;

		for (const Carro& carroExistente : carros)
		{
			if (carroExistente.posicion.y != posY) 
				continue;

			Rectangle rectCarroExistente = { carroExistente.posicion.x, carroExistente.posicion.y, carroExistente.tamano.x, carroExistente.tamano.y };

			Rectangle rectNuevoConMargen = rectNuevoCarro;
			rectNuevoConMargen.x -= tamanoCelda;
			rectNuevoConMargen.width += tamanoCelda * 2;

			if (CheckCollisionRecs(rectNuevoConMargen, rectCarroExistente))
			{
				seSolapan = true;
				break;
			}
		}

		if (!seSolapan)
			carros.push_back(nuevoCarro);
	}

	for (int i = carros.size() - 1; i >= 0; i--)
	{
		carros[i].posicion.x += carros[i].velocidad * GetFrameTime();
		//bool fueraIzquierda = (carros[i].velocidad < 0 && carros[i].posicion.x + carros[i].tamano.x < 0);
		//bool fueraDerecha = (carros[i].velocidad > 0 && carros[i].posicion.x > anchoVentana);

		if ((carros[i].velocidad < 0 && carros[i].posicion.x + carros[i].tamano.x < 0) || 
			(carros[i].velocidad > 0 && carros[i].posicion.x > anchoVentana))
			carros.erase(carros.begin() + i);
	}

	Rectangle jugador1Hurtbox = { jugador1.posicion.x + 10, jugador1.posicion.y + 10, jugador1.tamano.x - 20, jugador1.tamano.y - 20 };
	Rectangle jugador2Hurtbox = { jugador2.posicion.x + 10, jugador2.posicion.y + 10, jugador2.tamano.x - 20, jugador2.tamano.y - 20 };

	for (const Carro& carro : carros)
	{
		Rectangle carroHitbox = { carro.posicion.x, carro.posicion.y, carro.tamano.x, carro.tamano.y };

		if (jugador1.estaVivo && CheckCollisionRecs(jugador1Hurtbox, carroHitbox))
		{
			jugador1.estaVivo = false;
			PlaySound(sonidoChoque);
		}
		if (jugador2.estaVivo && CheckCollisionRecs(jugador2Hurtbox, carroHitbox))
		{
			jugador2.estaVivo = false;
			PlaySound(sonidoChoque);
		}
	}

	for (int i = 0; i < mapa.size(); i++)
	{
		if (mapa[i].terreno == TIERRA && !mapa[i].fueVisitado)
		{
			float filaY = mapa[i].posicion * (float)tamanoCelda;

			bool j1EnFila = (jugador1.estaVivo && jugador1.posicion.y == filaY);
			bool j2EnFila = (jugador2.estaVivo && jugador2.posicion.y == filaY);

			if (j1EnFila || j2EnFila)
			{
				if (j1EnFila)
					jugador1.puntuacion += 10;

				if (j2EnFila)
					jugador2.puntuacion += 10;

				mapa[i].fueVisitado = true;
			}
		}
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

	for (const Carro& carro : carros)
		DrawTexturePro(
			carro.textura,
			{ 0.0f, 0.0f, (float)carro.textura.width, (float)carro.textura.height },
			{ carro.posicion.x, carro.posicion.y, carro.tamano.x, carro.tamano.y },
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
		{ 0.0f, 0.0f, 750, 750 },
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

void dibujarUI()
{
	DrawText(TextFormat("Verde: %d", jugador1.puntuacion), 10, 10, 30, WHITE);

	const char* textoJ2 = TextFormat("Rojo: %d", jugador2.puntuacion);
	int anchoTextoJ2 = MeasureText(textoJ2, 30);
	DrawText(textoJ2, anchoVentana - anchoTextoJ2 - 10, 10, 30, WHITE);

	if (!jugador1.estaVivo && !jugador2.estaVivo)
	{
		if (jugador1.puntuacion > jugador2.puntuacion)
		{
			const char* texto = "VERDE GANA";
			int anchoTexto = MeasureText(texto, 60);
			DrawText(texto, (anchoVentana - anchoTexto) / 2, altoVentana / 2 - 60, 60, WHITE);
		}
		if (jugador1.puntuacion < jugador2.puntuacion)
		{
			const char* texto = "ROJO GANA";
			int anchoTexto = MeasureText(texto, 60);
			DrawText(texto, (anchoVentana - anchoTexto) / 2, altoVentana / 2 - 60, 60, WHITE);
		}
		if (jugador1.puntuacion == jugador2.puntuacion)
		{
			const char* texto = "EMPATE";
			int anchoTexto = MeasureText(texto, 60);
			DrawText(texto, (anchoVentana - anchoTexto) / 2, altoVentana / 2 - 60, 60, WHITE);
		}
	}
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

	texturasCarroIzquierda.push_back(LoadTexture("red_car_left.png"));
	texturasCarroIzquierda.push_back(LoadTexture("blue_car_left.png"));
	texturasCarroIzquierda.push_back(LoadTexture("green_car_left.png"));
	texturasCarroIzquierda.push_back(LoadTexture("white_car_left.png"));
	texturasCarroIzquierda.push_back(LoadTexture("orange_car_left.png"));
	texturasCarroIzquierda.push_back(LoadTexture("purple_car_left.png"));
	texturasCarroIzquierda.push_back(LoadTexture("brown_car_left.png"));
	texturasCarroIzquierda.push_back(LoadTexture("yellow_car_left.png"));

	texturasCarroDerecha.push_back(LoadTexture("red_car_right.png"));
	texturasCarroDerecha.push_back(LoadTexture("blue_car_right.png"));
	texturasCarroDerecha.push_back(LoadTexture("green_car_right.png"));
	texturasCarroDerecha.push_back(LoadTexture("white_car_right.png"));
	texturasCarroDerecha.push_back(LoadTexture("orange_car_right.png"));
	texturasCarroDerecha.push_back(LoadTexture("purple_car_right.png"));
	texturasCarroDerecha.push_back(LoadTexture("brown_car_right.png"));
	texturasCarroDerecha.push_back(LoadTexture("yellow_car_right.png"));

	fondoMenu.posicion = { 0, 0 };
	fondoMenu.tamano = { (float)anchoVentana, (float)altoVentana };

	botonFacil.tamano = { 200.0f, 100.0f };
	botonFacil.posicion = { 275.0f, 350.0f };

	botonNormal.tamano = { 200.0f, 100.0f };
	botonNormal.posicion = { 275.0f, 470.0f };

	botonDificil.tamano = { 200.0f, 100.0f };
	botonDificil.posicion = { 275.0f, 590.0f };

	ost = LoadMusicStream("froggerost.mp3");
	SetMusicVolume(ost, 0.3f);
	sonidoChoque = LoadSound("choque.mp3");
	sonidoRana = LoadSound("sonidoRana.mp3");
}

int main()
{
	InitWindow(anchoVentana, altoVentana, "Frogger x86");
	InitAudioDevice();
	SetTargetFPS(144);

	inicializarJuego();

	while (WindowShouldClose() == false)
	{
		actualizarJuego();

		BeginDrawing();
		ClearBackground(DARKBROWN);

		if (!juegoIniciado)
			dibujarMenu();
		else
		{
			dibujarJuego();
			dibujarUI();
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
	UnloadMusicStream(ost);
	UnloadSound(sonidoChoque);
	UnloadSound(sonidoRana);
	CloseAudioDevice();

	for (Texture2D textura : texturasCarroIzquierda)
		UnloadTexture(textura);
	for (Texture2D textura : texturasCarroDerecha)
		UnloadTexture(textura);

	CloseWindow();
	return 0;
}