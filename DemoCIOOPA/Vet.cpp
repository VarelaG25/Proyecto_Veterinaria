// Vet.cpp : Define el punto de entrada de la aplicación.
//

#include "framework.h"
#include "Vet.h"
#include <string>
#include <commctrl.h>
#include <commdlg.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>
#define MAX_LOADSTRING 100
#define IDC_TIMER1 1000
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996)
using namespace std;

// Estructuras
struct PACIENTES {
	//int id;
	WCHAR nombreCliente[255];
	WCHAR telefono[50];
	WCHAR especie[100];
	WCHAR motivo[255];
	WCHAR estatus[50];
	WCHAR costo[50];
	WCHAR fecha[20];
	WCHAR hora[20];
	WCHAR nombreUsuario[255];
};

struct USUARIO {
	WCHAR nombreDoctor[255];
	WCHAR nombreUsuario[255];
	WCHAR contrasena[255];
	WCHAR cedula[100];
	WCHAR ruta[300];
};

struct NODOLA {
	USUARIO* dato;
	PACIENTES* datoP;
	NODOLA* siguiente;
};

struct LISTACLIENTES {
	NODOLA* origen;
	NODOLA* fin;
};

// Variables globales:
HINSTANCE hInst;                                // instancia actual
WCHAR szTitle[MAX_LOADSTRING];                  // Texto de la barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // nombre de clase de la ventana principal
USUARIO usuarioActual;
LISTACLIENTES ListaClientes;

// Definir el tamaño máximo de pacientes
const int maxPacientes = 100;

// Crear un arreglo dinámico para almacenar pacientes
PACIENTES** listaPacientes = new PACIENTES * [maxPacientes];
int numPacientes = 0;

// Variables globales o definidas en el ámbito superior
int contadorUsuarioActual = 1;
vector<int> indicesFiltrados;

ofstream archivoCredenciales;

//CALLBACKS
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    principalCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	ventanaPerfil(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	ventanaCitas(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	ventanaAgenda(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Declaraciones de funciones adelantadas incluidas en este módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
bool menu(int opcion, HWND ventana);
void ConvertirCharToWCHAR(const char* source, WCHAR* dest, int destSize);
NODOLA* nuevoNodo(PACIENTES* dato) {
	NODOLA* nodo = new NODOLA;
	nodo->datoP = dato;
	nodo->siguiente = nullptr;
	return nodo;
}

// Buscar nombre
NODOLA* buscarNombre(const char* buscar) {
	if (ListaClientes.origen == nullptr)
		return nullptr;

	// Convertir la cadena de caracteres estrechos a anchos
	WCHAR nombreBuscar[255];
	ConvertirCharToWCHAR(buscar, nombreBuscar, 255);

	NODOLA* indice = ListaClientes.origen;
	while (indice != nullptr) {
		if (wcscmp(indice->datoP->nombreCliente, nombreBuscar) == 0)
			break;
		indice = indice->siguiente;
	}
	return indice;
}

// AÑADIR USUARIO NUEVO
USUARIO* crearUsuario(const WCHAR* nombre, const WCHAR* username, const WCHAR* contrasena, const WCHAR* cedula, const WCHAR* ruta) {
	// Verificar las longitudes de las cadenas
	if (wcslen(nombre) >= 255 || wcslen(username) >= 255 || wcslen(contrasena) >= 255 || wcslen(cedula) >= 100 || wcslen(ruta) >= 300) {
		// Manejar el error, por ejemplo, lanzar una excepción, imprimir un mensaje de error, etc.
		// También puedes ajustar los tamaños según tus necesidades.
		return nullptr;
	}

	USUARIO* nuevo = new USUARIO;
	wcscpy_s(nuevo->nombreDoctor, nombre);
	wcscpy_s(nuevo->nombreUsuario, username);
	wcscpy_s(nuevo->contrasena, contrasena);
	wcscpy_s(nuevo->cedula, cedula);
	wcscpy_s(nuevo->ruta, ruta);
	return nuevo;
}

// AÑADIR PACIENTE A LA ESTRUCTURA
PACIENTES* crearPaciente(const WCHAR* nombreCliente, const WCHAR* fecha, const WCHAR* hora, const WCHAR* tel, const WCHAR* especie, const WCHAR* motivo, const WCHAR* costo, const WCHAR* estatus, const WCHAR* nombreUsuario) {
	// Verificar las longitudes de las cadenas
	if (wcslen(nombreCliente) >= 255 || wcslen(fecha) >= 20 || wcslen(hora) >= 20 || wcslen(tel) >= 50 || wcslen(especie) >= 100 || wcslen(motivo) >= 255 || wcslen(costo) >= 50 || wcslen(estatus) >= 50) {
		// Manejar el error, por ejemplo, lanzar una excepción, imprimir un mensaje de error, etc.
		// También puedes ajustar los tamaños según tus necesidades.
		return nullptr;
	}

	PACIENTES* nuevoP = new PACIENTES;
	wcscpy_s(nuevoP->nombreCliente, nombreCliente);
	wcscpy_s(nuevoP->fecha, fecha);
	wcscpy_s(nuevoP->hora, hora);
	wcscpy_s(nuevoP->telefono, tel);
	wcscpy_s(nuevoP->especie, especie);
	wcscpy_s(nuevoP->motivo, motivo);
	wcscpy_s(nuevoP->costo, costo);
	wcscpy_s(nuevoP->estatus, estatus);
	wcscpy_s(nuevoP->nombreUsuario, nombreUsuario);
	return nuevoP;
}

// Declaracion de funciones de paciente-cliente
void agregarPacienteMedio(const char* buscar, PACIENTES* dato);
void agregarPacienteFinal(PACIENTES* dato);

// Función para guardar credenciales
void guardarCredenciales(const USUARIO* usuario, HWND hWnd) {
	// Verifica si el archivo se abrió correctamente
	if (archivoCredenciales.is_open()) {
		// Escribe la estructura USUARIO en el archivo
		archivoCredenciales.write(reinterpret_cast<const char*>(usuario), sizeof(USUARIO));
	}
	else {
		// Manejo de errores al abrir el archivo
		MessageBox(hWnd, L"No se pudo abrir el archivo para escritura.", L"Error", MB_OK | MB_ICONERROR);
	}
}

bool verificarCredenciales(const WCHAR* nombreUsuario, const WCHAR* contrasena, ifstream& archivo) {
	USUARIO usuario;

	while (archivo.read(reinterpret_cast<char*>(&usuario), sizeof(USUARIO))) {
		wprintf(L"Comparando: %s - %s\n", usuario.nombreUsuario, nombreUsuario);
		if (wcscmp(usuario.nombreUsuario, nombreUsuario) == 0 && wcscmp(usuario.contrasena, contrasena) == 0) {
			usuarioActual = usuario;  // Si necesitas asignar el usuario actual
			return true;
		}
	}
	return false;
}

// Función para guardar citas en un archivo
void guardarCitas() {
	// Abre el archivo para escritura en binario
	ofstream archivoCitas("citas.bin", ios::binary);

	// Verifica si el archivo se abrió correctamente
	if (!archivoCitas.is_open()) {
		MessageBox(nullptr, L"No se pudo abrir el archivo para escritura.", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	// Escribe la cantidad de citas
	archivoCitas.write(reinterpret_cast<const char*>(&numPacientes), sizeof(int));

	// Escribe cada cita en el archivo
	for (int i = 0; i < numPacientes; ++i) {
		archivoCitas.write(reinterpret_cast<const char*>(listaPacientes[i]), sizeof(PACIENTES));
	}

	// Cierra el archivo
	archivoCitas.close();
}

// Función para cargar citas desde un archivo
void cargarCitas() {
	// Abre el archivo para lectura en binario
	ifstream archivoCitas("citas.bin", ios::binary);

	// Verifica si el archivo se abrió correctamente
	if (!archivoCitas.is_open()) {
		// El archivo no existe, crea uno nuevo
		ofstream nuevoArchivo("citas.bin", ios::binary);

		// Verifica si el nuevo archivo se creó correctamente
		if (!nuevoArchivo.is_open()) {
			MessageBox(nullptr, L"No se pudo crear el archivo para lectura.", L"Error", MB_OK | MB_ICONERROR);
			return;
		}

		// Cierra el nuevo archivo
		nuevoArchivo.close();

		// Intenta abrir el archivo nuevamente para lectura
		archivoCitas.open("citas.bin", ios::binary);

		// Verifica si el archivo se abrió correctamente después de la creación
		if (!archivoCitas.is_open()) {
			MessageBox(nullptr, L"No se pudo abrir el archivo para lectura.", L"Error", MB_OK | MB_ICONERROR);
			return;
		}
	}

	// Lee la cantidad de citas
	archivoCitas.read(reinterpret_cast<char*>(&numPacientes), sizeof(int));

	// Lee cada cita desde el archivo
	for (int i = 0; i < numPacientes; ++i) {
		listaPacientes[i] = new PACIENTES;
		archivoCitas.read(reinterpret_cast<char*>(listaPacientes[i]), sizeof(PACIENTES));
	}

	// Cierra el archivo
	archivoCitas.close();
}

void FiltrarCitasPorUsuario(const WCHAR* nombreUsuario, HWND hWnd) {
	// Limpiar el vector
	indicesFiltrados.clear();

	// Borra todos los elementos actuales en el ListBox
	SendMessage(GetDlgItem(hWnd, LIST_AGENDA), LB_RESETCONTENT, 0, 0);

	// Llena la ListBox solo con las citas que coinciden con el nombre de usuario actual
	for (int i = 0; i < numPacientes; ++i) {
		if (wcscmp(listaPacientes[i]->nombreUsuario, nombreUsuario) == 0) {
			indicesFiltrados.push_back(i); // Agregar el índice filtrado

			// Obtener el índice directo del paciente en la lista general
			int indiceDirecto = indicesFiltrados.back();

			wstring infoCita = L"[" + to_wstring(indiceDirecto + 1) + L"] " + listaPacientes[indiceDirecto]->nombreCliente;
			SendMessage(hWnd, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(infoCita.c_str()));
		}
	}
}

void modificarInfoUsuario(const WCHAR* usuario, const WCHAR* nombre, const WCHAR* cedula, const WCHAR* contrasenaActual, const WCHAR* contrasenaNueva, HWND hwnd) {
	// Verificar si se han llenado todos los campos
	if (wcslen(usuario) == 0 || wcslen(nombre) == 0 || wcslen(cedula) == 0 || wcslen(contrasenaActual) == 0 || wcslen(contrasenaNueva) == 0) {
		MessageBox(hwnd, L"Por favor, complete todos los campos.", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	// Verificar que la contraseña actual coincida con la contraseña del usuario actual
	if (wcscmp(usuarioActual.contrasena, contrasenaActual) != 0) {
		MessageBox(hwnd, L"La contrasena actual no es correcta.", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	// Modificar la información del usuario actual
	wcscpy_s(usuarioActual.nombreDoctor, nombre);
	wcscpy_s(usuarioActual.nombreUsuario, usuario);
	wcscpy_s(usuarioActual.cedula, cedula);
	wcscpy_s(usuarioActual.contrasena, contrasenaNueva);

	// Mostrar un mensaje de éxito
	MessageBox(hwnd, L"Informacion del usuario modificada exitosamente.", L"Exito", MB_OK | MB_ICONINFORMATION);
}


 WCHAR* rutaImagen = usuarioActual.ruta;

//
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	// TODO: Colocar código aquí.

	// Abre el archivo para escritura en binario al inicio de la aplicación
	ofstream archivoCitas("citas.bin", ios::binary | ios::app);

	// Verificar si el archivo se abrió correctamente
	if (!archivoCitas.is_open()) {
		// Si el archivo no existe, intenta crearlo
		archivoCitas.open("citas.bin", ios::binary);
		if (!archivoCitas.is_open()) {
			MessageBox(nullptr, L"No se pudo abrir ni crear el archivo para escritura.", L"Error", MB_OK | MB_ICONERROR);
			return 1; // Salir con código de error
		}
	}

	// Abre el archivo para escritura en binario al inicio de la aplicación
	archivoCredenciales.open("credenciales.bin", ios::out | ios::binary | ios::app);

	// Verificar si el archivo se abrió correctamente
	if (!archivoCredenciales.is_open()) {
		MessageBox(nullptr, L"No se pudo abrir el archivo para escritura.", L"Error", MB_OK | MB_ICONERROR);
		return 1; // Salir con código de error
	}

	// Inicializar cadenas globales
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MENU, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Realizar la inicialización de la aplicación:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MENU));

	MSG msg;

	// Bucle principal de mensajes:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	// Cerrar el archivo al final de la aplicación
	guardarCitas();
	archivoCitas.close();
	archivoCredenciales.close();
	return (int)msg.wParam;
}

//
//  FUNCIÓN: MyRegisterClass()
//
//  PROPÓSITO: Registra la clase de ventana.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VET));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MENU);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCIÓN: InitInstance(HINSTANCE, int)
//
//   PROPÓSITO: Guarda el identificador de instancia y crea la ventana principal
//
//   COMENTARIOS:
//
//        En esta función, se guarda el identificador de instancia en una variable común y
//        se crea y muestra la ventana principal del programa.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Almacenar identificador de instancia en una variable global

	HWND hWnd = CreateDialog(hInst,
		MAKEINTRESOURCE(IDD_LOGIN),
		NULL,
		WndProc);
	if (!hWnd) {
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCIÓN: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PROPÓSITO: Procesa mensajes de la ventana principal.
//
//  WM_COMMAND  - procesar el menú de aplicaciones
//  WM_PAINT    - Pintar la ventana principal
//  WM_DESTROY  - publicar un mensaje de salida y volver
//
//

// Controlador de sistema
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Antes de cargar nuevas citas, libera la memoria existente
	for (int i = 0; i < numPacientes; ++i) {
		delete listaPacientes[i];
	}

	cargarCitas();

	switch (message)
	{
	case WM_COMMAND: {
		int wmId = LOWORD(wParam);

		switch (wmId) {
		case BTN_INGRESA_IN: {
			WCHAR usuario[255], contrasena[255];
			GetDlgItemText(hWnd, TXT_USUARIO_IN, usuario, 255);
			GetDlgItemText(hWnd, TXT_PASSWORD_IN, contrasena, 255);

			// Cerrar el archivo de escritura
			archivoCredenciales.close();

			// Abrir el archivo en modo lectura
			ifstream archivoLectura("credenciales.bin", ios::binary);

			// Verificar si el archivo se abrió correctamente
			if (!archivoLectura.is_open()) {
				MessageBox(nullptr, L"No se pudo abrir el archivo para lectura.", L"Error", MB_OK | MB_ICONERROR);
				return 1; // Salir con código de error
			}

			// Verificar las credenciales
			if (verificarCredenciales(usuario, contrasena, archivoLectura)) {
				// Credenciales válidas, continuar con la ventana de Agenda

				HWND ventana = CreateDialog(hInst,
					MAKEINTRESOURCE(IDD_Agenda),
					NULL,
					ventanaAgenda);
				ShowWindow(ventana, SW_SHOW);
				EndDialog(hWnd, 0);
			}

			// Cerrar el archivo de lectura
			archivoLectura.close();

			// Reabrir el archivo en modo escritura
			archivoCredenciales.open("credenciales.bin", ios::out | ios::binary | ios::app);
		} break;
		case BTN_REGISTRO_IN: {
			HWND ventana =
				CreateDialog(hInst,
					MAKEINTRESOURCE(IDD_REGISTER),
					NULL,
					principalCallback);
			ShowWindow(ventana, SW_SHOW);
			EndDialog(hWnd, 0);
		}break;
		case WM_DESTROY: {
			PostQuitMessage(0);
		}break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Agregar cualquier código de dibujo que use hDC aquí...
		EndPaint(hWnd, &ps);
	}break;
	case WM_DESTROY: {
		MessageBox(hWnd,
			L"Adios",
			L"Saliendo",
			0);
		PostQuitMessage(0);
	}break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Controlador principal
LRESULT CALLBACK principalCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Declara un objeto ofstream en algún lugar accesible desde tu código
	ofstream archivoCredenciales;
	switch (message) {
	case WM_COMMAND: {
		int wmId = LOWORD(wParam);
		if (menu(wmId, hWnd)) {
			return FALSE;
		}
		switch (wmId)
		{
		case BTN_ATRAS_REGISTRO: {
			HWND ventana =
				CreateDialog(hInst,
					MAKEINTRESOURCE(IDD_LOGIN),
					NULL,
					WndProc);
			ShowWindow(ventana, SW_SHOW);
			EndDialog(hWnd, 0);
		}break;
		case BTN_SLC_IMG_REGISTRO: {
			// GUARDA LA FOTO EN LA ESTRUCTURA
			WCHAR ruta[300] = { 0 };
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFile = ruta;
			ofn.nMaxFile = 1000;
			ofn.lpstrFilter = L"Imagenes\0*.bmp";
			ofn.nFilterIndex = 1;
			ofn.lpstrTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if (GetOpenFileName(&ofn)) {
				SetDlgItemText(hWnd,
					TXT_RUTA,
					ruta);
				HBITMAP imagen = (HBITMAP)LoadImage(hInst, ruta, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE | LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
				if (imagen == NULL) {
					// Manejo de error, imprime el código de error
					DWORD error = GetLastError();
					wprintf(L"LoadImage failed with error %lu\n", error);
				}
				else {
					if (!SendMessage(GetDlgItem(hWnd, IMG_FPERFIL_REGISTRO), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)imagen)) {
						// Manejo de error, imprime el código de error
						DWORD error = GetLastError();
						wprintf(L"SendMessage failed with error %lu\n", error);
					}
				}
			}
		} break;

		case BTN_REGISTRAR_REGISTRO: {
			// GUARDA CADA CAMPO DE TEXTO DE VARIABLES
			WCHAR nombre[255], usuario[255], contrasena[255], cedula[100], contrasenaConfirmacion[255], ruta[300];
			GetDlgItemText(hWnd, TXT_NOMBRE_REGISTRO, nombre, 255);
			GetDlgItemText(hWnd, TXT_USUARIO_REGISTRO, usuario, 255);
			GetDlgItemText(hWnd, TXT_PASSWORD_REGISTRO, contrasena, 255);
			GetDlgItemText(hWnd, TXT_CEDULA_REGISTRO, cedula, 100);
			GetDlgItemText(hWnd, TXT_PASSWORD_CONFIRM_REGISTRO, contrasenaConfirmacion, 255);
			GetDlgItemText(hWnd, TXT_RUTA, ruta, 300);

			// VALIDA QUE SE HAYAN LLENADO TODOS LOS CAMPOS
			if (wcslen(nombre) == 0 || wcslen(usuario) == 0 || wcslen(contrasena) == 0 || wcslen(cedula) == 0 || wcslen(contrasenaConfirmacion) == 0 || wcslen(ruta) == 0) {
				MessageBox(hWnd, L"Por favor, complete todos los campos.", L"Error", MB_OK | MB_ICONERROR);
				break;
			}

			

			// Verificar que la contraseña y la confirmación coincidan
			if (wcscmp(contrasena, contrasenaConfirmacion) == 0) {

				// Crear un usuario
				USUARIO* nuevoUsuario = crearUsuario(nombre, usuario, contrasena, cedula, ruta);

				// Verificar si la creación del usuario fue exitosa
				if (nuevoUsuario != nullptr) {
					// Guardar las credenciales en el archivo binario
					guardarCredenciales(nuevoUsuario, hWnd);
					usuarioActual = *nuevoUsuario;
					// Liberar la memoria asignada para el usuario
					delete nuevoUsuario;
					archivoCredenciales.close();
					MessageBox(hWnd, L"Usuario registrado exitosamente.", L"Exito", MB_OK | MB_ICONINFORMATION);
					HWND ventana =
						CreateDialog(hInst,
							MAKEINTRESOURCE(IDD_Agenda),
							NULL,
							principalCallback);
					ShowWindow(ventana, SW_SHOW);
					EndDialog(hWnd, 0);
				}
				else {
					// Manejar el error al crear el usuario
					MessageBox(hWnd, L"No se pudo crear el usuario.", L"Error", MB_OK | MB_ICONERROR);
				}
			}
			else {
				MessageBox(hWnd, L"Las claves de acceso no coinciden.", L"Error", MB_OK | MB_ICONERROR);
			}
		} break;
		case BTN_CANCELAR_MODINFO: {
			HWND ventana = CreateDialog(hInst,
				MAKEINTRESOURCE(IDD_INFO),
				NULL,
				ventanaPerfil);
			ShowWindow(ventana, SW_SHOW);
			EndDialog(hWnd, 0);
		}break;
		case BTN_GUARDAR_MODINFO: {
			WCHAR usuario[255], nombre[255], cedula[100], contrasenaActual[255], contrasenaNueva[255];
			GetDlgItemText(hWnd, TXT_USUARIO_MODINFO, usuario, 255);
			GetDlgItemText(hWnd, TXT_NOMBRE_MODINFO, nombre, 255);
			GetDlgItemText(hWnd, TXT_CEDULA_MODINFO, cedula, 100);
			GetDlgItemText(hWnd, TXT_PASSWORD_MODINFO, contrasenaActual, 255);
			GetDlgItemText(hWnd, TXT_PASSWORD_CONFIRM_MODINFO, contrasenaNueva, 255);

			modificarInfoUsuario(usuario, nombre, cedula, contrasenaActual, contrasenaNueva, hWnd);
		} break;
		case WM_DESTROY: {
			PostQuitMessage(0);
		}break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

	}break;
	case WM_DESTROY: {
		PostQuitMessage(0);
	}break;
	}
	return FALSE;
}

// Controlador de ventana de citas
LRESULT CALLBACK ventanaCitas(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static HWND comboEspecie, btnEspecificar;
	switch (message) {

	case WM_INITDIALOG: {

		SetDlgItemText(hWnd,
			NOMBRE_CITAS,
			usuarioActual.nombreDoctor);

		// Obtener el handle del listbox (cambia el ID del listbox según tu recurso)
		HBITMAP imagen = (HBITMAP)LoadImage(hInst, rutaImagen, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);

		if (imagen != NULL) {
			// Obtener el control Picture
			HWND pictureControl = GetDlgItem(hWnd, IMG_FPERFIL_CITAS);

			// Establecer la imagen en el control Picture
			SendMessage(pictureControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)imagen);
		}

		// Obtener el handle del ListBox de especies
		HWND listBoxEspecie = GetDlgItem(hWnd, LIST_ESPECIE_CITAS);

		// Agregar especies al ListBox
		SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Perro"));
		SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Gato"));
		SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Pajaro"));
		SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Conejo"));
		SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Hamster"));


		HBITMAP foto = (HBITMAP)LoadImage(hInst, rutaImagen, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);

		if (foto != NULL) {
			// Obtener el control Picture
			HWND pictureControl = GetDlgItem(hWnd, IMG_FPERFIL_AGENDA);

			// Establecer la imagen en el control Picture
			SendMessage(pictureControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)foto);
		}

		// AQUI HACE QUE EL ESTATUS DE LA CITA QUEDE ACTIVA POR DEFECTO
		HWND comboEstatus = GetDlgItem(hWnd, LIST_ESTATUS);

		SendMessage(
			GetDlgItem(hWnd, LIST_ESTATUS),
			CB_INSERTSTRING,
			0,
			(LPARAM)L"ACTIVA");

		SendMessage(comboEstatus, CB_SETCURSEL, 0, 0);

	} break;
	case WM_COMMAND: {
		int wmId = LOWORD(wParam);
		//int wmEvent = HIWORD(wParam);

		if (menu(wmId, hWnd)) {
			return FALSE;
		}
		switch (wmId) {
		case BTN_REGISTRAR_CITAS: {
			// GUARDA CADA CAMPO DE TEXTO DE VARIABLES
			WCHAR nombreCliente[255], tel[50], especie[100], motivo[255], costo[50], estatus[50];
			WCHAR fecha[20], hora[20];
			GetDlgItemText(hWnd, TXT_NOMBRE_CITAS, nombreCliente, 255);
			GetDlgItemText(hWnd, TXT_TEL_CITAS, tel, 50);
			GetDlgItemText(hWnd, LIST_ESPECIE_CITAS, especie, 100);
			GetDlgItemText(hWnd, TXT_MASCOTA_CITAS, motivo, 255);
			GetDlgItemText(hWnd, TXT_COSTO_CITAS, costo, 50);
			GetDlgItemText(hWnd, LIST_ESTATUS, estatus, 50);
			GetDlgItemText(hWnd, FECHA_SELECT_CITAS, fecha, 20);
			GetDlgItemText(hWnd, HORA_SELECT_CITAS, hora, 20);
			
			if (wcslen(fecha) == 0 || wcslen(hora) == 0 || wcslen(nombreCliente) == 0 || wcslen(tel) != 10 || wcslen(especie) == 0
				|| wcslen(motivo) == 0 || wcslen(costo) == 0 || wcslen(estatus) == 0) {
				MessageBox(hWnd, L"Por favor, complete todos los campos.", L"Error", MB_OK | MB_ICONERROR);
				break;
			}

			SYSTEMTIME Tiempo;
			GetLocalTime(&Tiempo);

			// Convertir las cadenas de fecha y hora a valores numéricos
			int year = _wtoi(fecha + 6);
			int month = _wtoi(fecha + 3); 
			int day = _wtoi(fecha);   

			int hour = _wtoi(hora);
			int minute = _wtoi(hora + 3); 

			// Almacenar si es a.m. o p.m.
			bool PM = (hora[9] == L'p');

			// Convertir la hora al formato de 24 horas si es necesario
			if (PM && hour < 12) {
				// Si es PM y la hora es menor que 12, agregar 12 horas
				hour += 12;
			}
			else if (!PM && hour == 12) {
				// Si es AM y la hora es 12, cambiar a 0
				hour = 0;
			}

			if (year < Tiempo.wYear ||
				(year == Tiempo.wYear && (month < Tiempo.wMonth ||
					(month == Tiempo.wMonth && (day < Tiempo.wDay ||
						(day == Tiempo.wDay && (hour < Tiempo.wHour ||
							(hour == Tiempo.wHour && minute <= Tiempo.wMinute)))))))) {
				wchar_t mensaje[500]; // Ajusta el tamaño según tus necesidades

				swprintf(mensaje, L"No se pueden registrar citas con fechas o horas anteriores a la actual.\n"
					L"Ano actual: %d, Mes actual: %d, Dia actual: %d, Hora actual: %d, Minuto actual: %d\n"
					L"Ano ingresado: %d, Mes ingresado: %d, Dia ingresado: %d, Hora ingresada: %d, Minuto ingresado: %d",
					Tiempo.wYear, Tiempo.wMonth, Tiempo.wDay, Tiempo.wHour, Tiempo.wMinute,
					year, month, day, hour, minute);

				MessageBox(hWnd, mensaje, L"Error", MB_OK | MB_ICONERROR);

				break;
			}

			// Verificar si ya existe una cita con la misma fecha y hora
			for (int i = 0; i < numPacientes; ++i) {
				if (wcscmp(listaPacientes[i]->fecha, fecha) == 0 && wcscmp(listaPacientes[i]->hora, hora) == 0) {
					MessageBox(hWnd, L"Ya existe una cita para la misma fecha y hora.", L"Error", MB_OK | MB_ICONERROR);
					return FALSE;
				}
			}

			// Continuar con el registro de la cita
			listaPacientes[numPacientes++] = crearPaciente(nombreCliente, fecha, hora, tel, especie, motivo, costo, estatus, usuarioActual.nombreUsuario);

			// Mostrar mensaje de citas registradas
			MessageBox(hWnd, L"Cita registrada con exito.", L"Exito", MB_OK | MB_ICONINFORMATION);

			// Impresiones para verificar que los datos se están guardando correctamente
			wprintf(L"Fecha registrada: %s\n", fecha);
			wprintf(L"Hora registrada: %s\n", hora);

		} break;

		case LIST_ESPECIE_CITAS: {

		} break;
		case BTN_ESPECIFICAR: {
			// Obtener el handle del ComboBox
			HWND comboBox = GetDlgItem(hWnd, LIST_ESPECIE_CITAS);

			// Obtener la longitud del texto en el ComboBox
			int length = GetWindowTextLength(comboBox);

			// Asegurarse de que hay texto en el ComboBox
			if (length > 0) {
				// Reservar memoria para el texto
				WCHAR* nuevoElemento = new WCHAR[length + 1];

				// Obtener el texto del ComboBox
				GetWindowText(comboBox, nuevoElemento, length + 1);

				// Añadir el nuevo elemento solo si no está vacío
				if (wcslen(nuevoElemento) > 0) {
					int nuevoIndice = SendMessage(comboBox, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(nuevoElemento));

					// Seleccionar el nuevo elemento y establecer el foco en el ComboBox
					SendMessage(comboBox, CB_SETCURSEL, nuevoIndice, 0);
					SetFocus(comboBox);
				}

				// Liberar la memoria reservada
				delete[] nuevoElemento;
			}
		} break;
		case BTN_LIMPIAR_CITAS:
		{
			HWND ventana = CreateDialog(hInst,
				MAKEINTRESOURCE(IDD_CITAS),
				NULL,
				ventanaCitas);
			ShowWindow(ventana, SW_SHOW);
			EndDialog(hWnd, 0);
		}break;
		}
	} break;

	case WM_CLOSE:
	{
		DestroyWindow(hWnd);
		break;
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
	}break;
	}
	return FALSE;
}

// Controlador de ventana de perfil
LRESULT CALLBACK ventanaPerfil(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
	case WM_INITDIALOG: {
		// Mostrar datos del usuario no modificables
		SetDlgItemText(hWnd, TXT_NMB_DOC_INFO, usuarioActual.nombreDoctor);
		SetDlgItemText(hWnd, TXT_CEDULA_INFO, usuarioActual.cedula);
		SetDlgItemText(hWnd, TXT_USUARIO_INFO, usuarioActual.nombreUsuario);
		HBITMAP imagen = (HBITMAP)LoadImage(hInst, rutaImagen, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
		if (imagen != NULL) {
			// Obtener el control Picture
			HWND pictureControl = GetDlgItem(hWnd, IMG_FPERFIL_INFO);

			// Establecer la imagen en el control Picture
			SendMessage(pictureControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)imagen);
		}
	}break;
	case WM_COMMAND: {
		int id = LOWORD(wParam);
		if (menu(id, hWnd))
			return FALSE;
		switch (id) {
		case BTN_CAMBIAR_IMG_INFO: {
			WCHAR ruta[300] = { 0 };
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFile = ruta;
			ofn.nMaxFile = 1000;
			ofn.lpstrFilter = L"Imagenes\0*.bmp\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if (GetOpenFileName(&ofn)) {
				wcscpy_s(usuarioActual.ruta, ruta);

				HBITMAP imagen = (HBITMAP)LoadImage(hInst, ruta, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE | LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
				if (imagen != NULL)
					SendMessage(GetDlgItem(hWnd, IMG_FPERFIL_INFO), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)imagen);
			}
		} break;
		case BTN_MODIFICAR_INFO: {
			HWND ventana = CreateDialog(hInst,
				MAKEINTRESOURCE(IDD_MODINFO),
				NULL,
				principalCallback);
			ShowWindow(ventana, SW_SHOW);
			EndDialog(hWnd, 0);
		}break;
		case WM_DESTROY: {
			PostQuitMessage(0);
		}break;
		}
	}break;
	case WM_DESTROY: {
		PostQuitMessage(0);
	}break;
	}
	return FALSE;
}

// Controlador de ventana de agenda
LRESULT CALLBACK ventanaAgenda(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch (message) {
	case WM_CREATE: {
	}break;
	case WM_INITDIALOG: {
		FiltrarCitasPorUsuario(usuarioActual.nombreUsuario, hWnd);

		// Obtener el handle del ListBox
		HWND listBox = GetDlgItem(hWnd, LIST_AGENDA);
		
		// Borra todos los elementos actuales en el ListBox
		SendMessage(listBox, LB_RESETCONTENT, 0, 0);

		// Reinicia el contador
		contadorUsuarioActual = 1;

		SYSTEMTIME Tiempo;
		GetLocalTime(&Tiempo);

		for (int i = 0; i < numPacientes; ++i) {
			if (wcscmp(listaPacientes[i]->nombreUsuario, usuarioActual.nombreUsuario) == 0) {
				// Convertir las cadenas de fecha y hora a valores numéricos
				int year = _wtoi(listaPacientes[i]->fecha + 6);
				int month = _wtoi(listaPacientes[i]->fecha + 3);
				int day = _wtoi(listaPacientes[i]->fecha);

				int hour = _wtoi(listaPacientes[i]->hora);
				int minute = _wtoi(listaPacientes[i]->hora + 3);

				// Almacenar si es a.m. o p.m.
				bool PM = (listaPacientes[i]->hora[9] == L'p');

				// Convertir la hora al formato de 24 horas si es necesario
				if (PM && hour < 12) {
					// Si es PM y la hora es menor que 12, agregar 12 horas
					hour += 12;
				}
				else if (!PM && hour == 12) {
					// Si es AM y la hora es 12, cambiar a 0
					hour = 0;
				}

				if (year > Tiempo.wYear ||
					(year == Tiempo.wYear && (month > Tiempo.wMonth ||
						(month == Tiempo.wMonth && (day > Tiempo.wDay ||
							(day == Tiempo.wDay && (hour > Tiempo.wHour ||
								(hour == Tiempo.wHour && minute > Tiempo.wMinute)))))))) {

					// Verificar si la cita no está cancelada
					if (wcscmp(listaPacientes[i]->estatus, L"CANCELADA") != 0) {
						wstring infoCita = L"[" + to_wstring(contadorUsuarioActual) + L"] " + listaPacientes[i]->nombreCliente;
						SendMessage(listBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(infoCita.c_str()));
					}
				}

				// Incrementa el contador específico para el usuario actual
				++contadorUsuarioActual;
			}
		}
		SetDlgItemText(hWnd,
			NOMBRE_AGENDA,
			usuarioActual.nombreDoctor);

		// Obtener el handle del listbox (cambia el ID del listbox según tu recurso)
		HBITMAP imagen = (HBITMAP)LoadImage(hInst, rutaImagen, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);

		if (imagen != NULL) {
			// Obtener el control Picture
			HWND pictureControl = GetDlgItem(hWnd, IMG_FPERFIL_AGENDA);

			// Establecer la imagen en el control Picture
			SendMessage(pictureControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)imagen);
		}


		// Obtener el handle del ListBox de especies
		HWND listBoxEspecie = GetDlgItem(hWnd, LIST_ESPECIE_AGENDA);

		// Agregar especies al ListBox
		SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Perro"));
		SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Gato"));
		SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Pajaro"));
		SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Conejo"));
		SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Hamster"));


	}break;
	case WM_COMMAND: {
		int id = LOWORD(wParam);
		if (menu(id, hWnd))
			return FALSE;
		switch (id) {
		case LIST_AGENDA: {
			// Si se selecciona un elemento en el ListBox
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				// Obtener el handle del ListBox
				HWND listBox = GetDlgItem(hWnd, LIST_AGENDA);


				// Obtener el índice del elemento seleccionado
				int selectedIndex = SendMessage(listBox, LB_GETCURSEL, 0, 0);

				// Verificar si se seleccionó algún elemento
				if (selectedIndex != LB_ERR) {
					// Obtener el índice filtrado correspondiente al índice seleccionado en el ListBox
					int indiceFiltrado = indicesFiltrados[selectedIndex];

					// Obtener el paciente correspondiente al índice filtrado
					PACIENTES* pacienteSeleccionado = listaPacientes[indiceFiltrado];

					// Llenar los controles con los datos del paciente
					SetDlgItemText(hWnd, TXT_NOMBRE_AGENDA, pacienteSeleccionado->nombreCliente);
					SetDlgItemText(hWnd, TXT_TEL_AGENDA, pacienteSeleccionado->telefono);
					SetDlgItemText(hWnd, TXT_MASCOTA_AGENDA, pacienteSeleccionado->motivo);
					SetDlgItemText(hWnd, TXT_COSTO_AGENDA, pacienteSeleccionado->costo);
					SetDlgItemText(hWnd, IDC_FECHA, pacienteSeleccionado->fecha);
					SetDlgItemText(hWnd, IDC_HORA, pacienteSeleccionado->hora);
					

					// Configurar la especie seleccionada en el ComboBox
					HWND hComboBoxEspecie = GetDlgItem(hWnd, LIST_ESPECIE_AGENDA);
					SendMessage(hComboBoxEspecie, CB_RESETCONTENT, 0, 0);
					int nuevoIndiceEspecie = SendMessage(hComboBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pacienteSeleccionado->especie));
					SendMessage(hComboBoxEspecie, CB_SETCURSEL, nuevoIndiceEspecie, 0);

					// Configurar el estatus seleccionado en el ComboBox
					HWND hComboBoxEstatus = GetDlgItem(hWnd, LIST_ESTATUS_AGENDA);
					SendMessage(hComboBoxEstatus, CB_RESETCONTENT, 0, 0);
					int nuevoIndiceEstatus = SendMessage(hComboBoxEstatus, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pacienteSeleccionado->estatus));
					SendMessage(hComboBoxEstatus, CB_SETCURSEL, nuevoIndiceEstatus, 0);


				}
			}
		} break;

		case BTN_NUEVO: {
			
			// Obtener el handle del ComboBox
			HWND comboBox = GetDlgItem(hWnd, LIST_ESPECIE_AGENDA);

			// Obtener la longitud del texto en el ComboBox
			int length = GetWindowTextLength(comboBox);

			// Asegurarse de que hay texto en el ComboBox
			if (length > 0) {
				// Reservar memoria para el texto
				WCHAR* nuevoElemento = new WCHAR[length + 1];

				// Obtener el texto del ComboBox
				GetWindowText(comboBox, nuevoElemento, length + 1);

				// Añadir el nuevo elemento solo si no está vacío
				if (wcslen(nuevoElemento) > 0) {
					int nuevoIndice = SendMessage(comboBox, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(nuevoElemento));

					// Seleccionar el nuevo elemento y establecer el foco en el ComboBox
					SendMessage(comboBox, CB_SETCURSEL, nuevoIndice, 0);
					SetFocus(comboBox);
				}

				// Liberar la memoria reservada
				delete[] nuevoElemento;
			}
		} break;

		case BTN_MODIFICAR_AGENDA: {
			// Obtener el índice del elemento seleccionado en el ListBox
			HWND listBox = GetDlgItem(hWnd, LIST_AGENDA);
			int selectedIndex = SendMessage(listBox, LB_GETCURSEL, 0, 0);

			// Verificar si se seleccionó algún elemento en el ListBox
			if (selectedIndex != LB_ERR) {
				// Obtener el índice filtrado correspondiente al índice seleccionado en el ListBox
				int indiceFiltrado = indicesFiltrados[selectedIndex];
				// Obtener el paciente correspondiente al índice filtrado
				PACIENTES* pacienteSeleccionado = listaPacientes[indiceFiltrado];

				// Verificar el estatus del paciente antes de permitir la modificación
				if (wcscmp(pacienteSeleccionado->estatus, L"CANCELADO") == 0) {
					MessageBox(hWnd, L"No se puede modificar una cita cancelada.", L"Error", MB_OK | MB_ICONERROR);
				}
				else{
					// Obtén el handle de los controles que deseas habilitar
					HWND txtNombre = GetDlgItem(hWnd, TXT_NOMBRE_AGENDA);
					HWND txtTelefono = GetDlgItem(hWnd, TXT_TEL_AGENDA);
					HWND txtMotivo = GetDlgItem(hWnd, TXT_MASCOTA_AGENDA);
					HWND txtCosto = GetDlgItem(hWnd, TXT_COSTO_AGENDA);
					HWND listEspecie = GetDlgItem(hWnd, LIST_ESPECIE_AGENDA);
					HWND listEstatus = GetDlgItem(hWnd, LIST_ESTATUS_AGENDA);
					HWND btnguardar = GetDlgItem(hWnd, BTN_REGISTRAR_AGENDA);
					HWND fechaPicker = GetDlgItem(hWnd, IDC_DATETIMEPICKER1);
					HWND horaPicker = GetDlgItem(hWnd, IDC_DATETIMEPICKER2);

					// Agregar especies al ListBox
					// Obtener el handle del ListBox de especies
					HWND listBoxEspecie = GetDlgItem(hWnd, LIST_ESPECIE_AGENDA);
					// Limpiar las especies anteriores
					SendMessage(listBoxEspecie, CB_RESETCONTENT, 0, 0);
					// Agregar especies al ListBox
					SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Perro"));
					SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Gato"));
					SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Pajaro"));
					SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Conejo"));
					SendMessage(listBoxEspecie, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Hamster"));

					// Limpiar los estatus anteriores
					SendMessage(listEstatus, CB_RESETCONTENT, 0, 0);

					SendMessage(listEstatus, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"ACTIVA"));
					SendMessage(listEstatus, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"CANCELADO"));

					// Habilita los controles
					EnableWindow(txtNombre, TRUE);
					EnableWindow(txtTelefono, TRUE);
					EnableWindow(txtMotivo, TRUE);
					EnableWindow(txtCosto, TRUE);
					EnableWindow(listEspecie, TRUE);
					EnableWindow(listEstatus, TRUE);
					EnableWindow(btnguardar, TRUE);
					EnableWindow(fechaPicker, TRUE);
					EnableWindow(horaPicker, TRUE);
				}
			}
			else {
				MessageBox(hWnd, L"Seleccione un paciente de la lista.", L"Error", MB_OK | MB_ICONERROR);
				HWND ventana = CreateDialog(hInst,
					MAKEINTRESOURCE(IDD_Agenda),
					NULL,
					ventanaAgenda);
				ShowWindow(ventana, SW_SHOW);
				EndDialog(hWnd, 0);
				break;
			}
			
		}break;

		case BTN_REGISTRAR_AGENDA: {
			// Obtener el índice del elemento seleccionado en el ListBox
			HWND listBox = GetDlgItem(hWnd, LIST_AGENDA);
			int selectedIndex = SendMessage(listBox, LB_GETCURSEL, 0, 0);

			// Verificar si se seleccionó algún elemento en el ListBox
			if (selectedIndex != LB_ERR) {
				// Obtener el índice filtrado correspondiente al índice seleccionado en el ListBox
				int indiceFiltrado = indicesFiltrados[selectedIndex];
				WCHAR nombreCliente[255], tel[50], especie[100], motivo[255], costo[50], estatus[50];
				WCHAR fecha[20], hora[20];
				GetDlgItemText(hWnd, TXT_NOMBRE_AGENDA, nombreCliente, 255);
				GetDlgItemText(hWnd, TXT_TEL_AGENDA, tel, 50);
				GetDlgItemText(hWnd, LIST_ESPECIE_AGENDA, especie, 100);
				GetDlgItemText(hWnd, TXT_MASCOTA_AGENDA, motivo, 255);
				GetDlgItemText(hWnd, TXT_COSTO_AGENDA, costo, 50);
				GetDlgItemText(hWnd, LIST_ESTATUS_AGENDA, estatus, 50);
				GetDlgItemText(hWnd, IDC_DATETIMEPICKER1, fecha, 20);
				GetDlgItemText(hWnd, IDC_DATETIMEPICKER2, hora, 20);

				if (wcslen(fecha) == 0 || wcslen(hora) == 0 || wcslen(nombreCliente) == 0 || wcslen(tel) != 10 || wcslen(especie) == 0
					|| wcslen(motivo) == 0 || wcslen(costo) == 0 || wcslen(estatus) == 0) {
					MessageBox(hWnd, L"Por favor, complete todos los campos.", L"Error", MB_OK | MB_ICONERROR);
					HWND ventana = CreateDialog(hInst,
						MAKEINTRESOURCE(IDD_Agenda),
						NULL,
						ventanaAgenda);
					ShowWindow(ventana, SW_SHOW);
					EndDialog(hWnd, 0);
					break;
				}

				SYSTEMTIME Tiempo;
				GetLocalTime(&Tiempo);

				// Convertir las cadenas de fecha y hora a valores numéricos
				int year = _wtoi(fecha + 6);
				int month = _wtoi(fecha + 3);
				int day = _wtoi(fecha);

				int hour = _wtoi(hora);
				int minute = _wtoi(hora + 3);

				// Almacenar si es a.m. o p.m.
				bool PM = (hora[9] == L'p');

				// Convertir la hora al formato de 24 horas si es necesario
				if (PM && hour < 12) {
					// Si es PM y la hora es menor que 12, agregar 12 horas
					hour += 12;
				}
				else if (!PM && hour == 12) {
					// Si es AM y la hora es 12, cambiar a 0
					hour = 0;
				}

				if (year < Tiempo.wYear ||
					(year == Tiempo.wYear && (month < Tiempo.wMonth ||
						(month == Tiempo.wMonth && (day < Tiempo.wDay ||
							(day == Tiempo.wDay && (hour < Tiempo.wHour ||
								(hour == Tiempo.wHour && minute <= Tiempo.wMinute)))))))) {
					wchar_t mensaje[500]; // Ajusta el tamaño según tus necesidades

					swprintf(mensaje, L"No se pueden registrar citas con fechas o horas anteriores a la actual.\n"
						L"Ano actual: %d, Mes actual: %d, Dia actual: %d, Hora actual: %d, Minuto actual: %d\n"
						L"Ano ingresado: %d, Mes ingresado: %d, Dia ingresado: %d, Hora ingresada: %d, Minuto ingresado: %d",
						Tiempo.wYear, Tiempo.wMonth, Tiempo.wDay, Tiempo.wHour, Tiempo.wMinute,
						year, month, day, hour, minute);

					MessageBox(hWnd, mensaje, L"Error", MB_OK | MB_ICONERROR);

					HWND txtNombre = GetDlgItem(hWnd, TXT_NOMBRE_AGENDA);
					HWND txtTelefono = GetDlgItem(hWnd, TXT_TEL_AGENDA);
					HWND txtMotivo = GetDlgItem(hWnd, TXT_MASCOTA_AGENDA);
					HWND txtCosto = GetDlgItem(hWnd, TXT_COSTO_AGENDA);
					HWND listEspecie = GetDlgItem(hWnd, LIST_ESPECIE_AGENDA);
					HWND listEstatus = GetDlgItem(hWnd, LIST_ESTATUS_AGENDA);
					HWND fechaPicker = GetDlgItem(hWnd, IDC_DATETIMEPICKER1);
					HWND horaPicker = GetDlgItem(hWnd, IDC_DATETIMEPICKER2);
					HWND btnguardar = GetDlgItem(hWnd, BTN_REGISTRAR_AGENDA);

					EnableWindow(txtNombre, FALSE);
					EnableWindow(txtTelefono, FALSE);
					EnableWindow(txtMotivo, FALSE);
					EnableWindow(txtCosto, FALSE);
					EnableWindow(listEspecie, FALSE);
					EnableWindow(listEstatus, FALSE);
					EnableWindow(btnguardar, FALSE);
					EnableWindow(fechaPicker, FALSE);
					EnableWindow(horaPicker, FALSE);

					HWND ventana = CreateDialog(hInst,
						MAKEINTRESOURCE(IDD_Agenda),
						NULL,
						ventanaAgenda);
					ShowWindow(ventana, SW_SHOW);
					EndDialog(hWnd, 0);
					break;
				}

				// Verificar si ya existe una cita con la misma fecha y hora
				for (int i = 0; i < numPacientes; ++i) {
					if (wcscmp(listaPacientes[i]->fecha, fecha) == 0 && wcscmp(listaPacientes[i]->hora, hora) == 0) {
						MessageBox(hWnd, L"Ya existe una cita para la misma fecha y hora.", L"Error", MB_OK | MB_ICONERROR);
						return FALSE;
					}
				}

				// Verificar si se seleccionó algún elemento en el ListBox
				if (selectedIndex != LB_ERR) {
					// Obtener el índice filtrado correspondiente al índice seleccionado en el ListBox
					int indiceFiltrado = indicesFiltrados[selectedIndex];

					// Obtener el paciente correspondiente al índice filtrado
					PACIENTES* pacienteSeleccionado = listaPacientes[indiceFiltrado];

					// Actualizar los datos del paciente seleccionado con la nueva información
					wcscpy(pacienteSeleccionado->nombreCliente, nombreCliente);
					wcscpy(pacienteSeleccionado->telefono, tel);
					wcscpy(pacienteSeleccionado->especie, especie);
					wcscpy(pacienteSeleccionado->motivo, motivo);
					wcscpy(pacienteSeleccionado->costo, costo);
					wcscpy(pacienteSeleccionado->estatus, estatus);
					wcscpy(pacienteSeleccionado->fecha, fecha);
					wcscpy(pacienteSeleccionado->hora, hora);

					// Mostrar mensaje de citas registradas
					MessageBox(hWnd, L"Cita actualizada con exito.", L"Éxito", MB_OK | MB_ICONINFORMATION);
				}
				else {
					MessageBox(hWnd, L"Seleccione una cita de la lista para actualizar.", L"Error", MB_OK | MB_ICONERROR);
				}

				// Impresiones para verificar que los datos se están guardando correctamente
				wprintf(L"Fecha registrada: %s\n", fecha);
				wprintf(L"Hora registrada: %s\n", hora);

				HWND txtNombre = GetDlgItem(hWnd, TXT_NOMBRE_AGENDA);
				HWND txtTelefono = GetDlgItem(hWnd, TXT_TEL_AGENDA);
				HWND txtMotivo = GetDlgItem(hWnd, TXT_MASCOTA_AGENDA);
				HWND txtCosto = GetDlgItem(hWnd, TXT_COSTO_AGENDA);
				HWND listEspecie = GetDlgItem(hWnd, LIST_ESPECIE_AGENDA);
				HWND listEstatus = GetDlgItem(hWnd, LIST_ESTATUS_AGENDA);
				HWND fechaPicker = GetDlgItem(hWnd, IDC_DATETIMEPICKER1);
				HWND horaPicker = GetDlgItem(hWnd, IDC_DATETIMEPICKER2);

				// Deshabilita los controles
				EnableWindow(txtNombre, FALSE);
				EnableWindow(txtTelefono, FALSE);
				EnableWindow(txtMotivo, FALSE);
				EnableWindow(txtCosto, FALSE);
				EnableWindow(listEspecie, FALSE);
				EnableWindow(listEstatus, FALSE);
				EnableWindow(fechaPicker, FALSE);
				EnableWindow(horaPicker, FALSE);

				HWND ventana = CreateDialog(hInst,
					MAKEINTRESOURCE(IDD_Agenda),
					NULL,
					ventanaAgenda);
				ShowWindow(ventana, SW_SHOW);
				EndDialog(hWnd, 0);
				break;
			}
			else {
				MessageBox(hWnd, L"Seleccione un paciente de la lista.", L"Error", MB_OK | MB_ICONERROR);
				HWND ventana = CreateDialog(hInst,
					MAKEINTRESOURCE(IDD_Agenda),
					NULL,
					ventanaAgenda);
				ShowWindow(ventana, SW_SHOW);
				EndDialog(hWnd, 0);
				break;
			}

			
		}break;
		case BTN_CANCELAR_AGENDA:
		{
			// Obtener el índice del elemento seleccionado en el ListBox
			HWND listBox = GetDlgItem(hWnd, LIST_AGENDA);
			int selectedIndex = SendMessage(listBox, LB_GETCURSEL, 0, 0);

			// Verificar si se seleccionó algún elemento en el ListBox
			if (selectedIndex != LB_ERR) {
				// Obtener el índice filtrado correspondiente al índice seleccionado en el ListBox
				int indiceFiltrado = indicesFiltrados[selectedIndex];

				// Eliminar el paciente correspondiente al índice filtrado de la lista
				delete listaPacientes[indiceFiltrado];

				// Eliminar el índice filtrado del array de índices filtrados
				indicesFiltrados.erase(indicesFiltrados.begin() + selectedIndex);

				// Actualizar el ListBox
				SendMessage(listBox, LB_RESETCONTENT, 0, 0);

				// Reiniciar el contador
				contadorUsuarioActual = 1;

				// Llenar el ListBox con los pacientes restantes
				for (int i = 0; i < numPacientes; ++i) {
					if (wcscmp(listaPacientes[i]->nombreUsuario, usuarioActual.nombreUsuario) == 0) {
						wstring infoCita = L"[" + to_wstring(contadorUsuarioActual) + L"] " + listaPacientes[i]->nombreCliente;
						SendMessage(listBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(infoCita.c_str()));

						// Incrementar el contador específico para el usuario actual
						++contadorUsuarioActual;
					}
				}

				// Mostrar mensaje de paciente eliminado
				MessageBox(hWnd, L"Paciente eliminado con exito.", L"Exito", MB_OK | MB_ICONINFORMATION);
			}
			else {
				MessageBox(hWnd, L"Seleccione un paciente de la lista para eliminar.", L"Error", MB_OK | MB_ICONERROR);

			}
			HWND ventana = CreateDialog(hInst,
				MAKEINTRESOURCE(IDD_Agenda),
				NULL,
				ventanaAgenda);
			ShowWindow(ventana, SW_SHOW);
			EndDialog(hWnd, 0);
		}break;
		case BTN_RECIENTE:
		{

		}break;
		case BTN_VIEJO:
		{

		}break;
		case BTN_PASADAS: {
			// Obtener el handle del ListBox
			HWND listBox = GetDlgItem(hWnd, LIST_AGENDA);

			// Borra todos los elementos actuales en el ListBox
			SendMessage(listBox, LB_RESETCONTENT, 0, 0);

			// Reinicia el contador
			contadorUsuarioActual = 1;

			SYSTEMTIME Tiempo;
			GetLocalTime(&Tiempo);

			for (int i = 0; i < numPacientes; ++i) {
				if (wcscmp(listaPacientes[i]->nombreUsuario, usuarioActual.nombreUsuario) == 0) {
					// Convertir las cadenas de fecha y hora a valores numéricos
					int year = _wtoi(listaPacientes[i]->fecha + 6);
					int month = _wtoi(listaPacientes[i]->fecha + 3);
					int day = _wtoi(listaPacientes[i]->fecha);

					int hour = _wtoi(listaPacientes[i]->hora);
					int minute = _wtoi(listaPacientes[i]->hora + 3);

					// Almacenar si es a.m. o p.m.
					bool PM = (listaPacientes[i]->hora[9] == L'p');

					// Convertir la hora al formato de 24 horas si es necesario
					if (PM && hour < 12) {
						// Si es PM y la hora es menor que 12, agregar 12 horas
						hour += 12;
					}
					else if (!PM && hour == 12) {
						// Si es AM y la hora es 12, cambiar a 0
						hour = 0;
					}

					if (year < Tiempo.wYear ||
						(year == Tiempo.wYear && (month < Tiempo.wMonth ||
							(month == Tiempo.wMonth && (day < Tiempo.wDay ||
								(day == Tiempo.wDay && (hour < Tiempo.wHour ||
									(hour == Tiempo.wHour && minute <= Tiempo.wMinute)))))))) {

						// Verificar si la cita está cancelada
						if (wcscmp(listaPacientes[i]->estatus, L"CANCELADA") == 0) {
							wstring infoCita = L"[" + to_wstring(contadorUsuarioActual) + L"] " + listaPacientes[i]->nombreCliente;
							SendMessage(listBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(infoCita.c_str()));
						}
					}

					// Incrementa el contador específico para el usuario actual
					++contadorUsuarioActual;
				}
			}
		} break;
		case BTN_SIGUIENTES: {
			HWND ventana = CreateDialog(hInst,
				MAKEINTRESOURCE(IDD_Agenda),
				NULL,
				ventanaAgenda);
			ShowWindow(ventana, SW_SHOW);
			EndDialog(hWnd, 0);
		} break;
		case WM_DESTROY: {
			PostQuitMessage(0);
		}break;
		}
	}break;
	case WM_DESTROY: {
		PostQuitMessage(0);
	}break;

	}
	return FALSE;
}

// Opciones del menu
bool menu(int opcion, HWND hWnd) {
	switch (opcion) {
	case ID_Perfil: {
		HWND ventana = CreateDialog(hInst,
			MAKEINTRESOURCE(IDD_INFO),
			NULL,
			ventanaPerfil);
		ShowWindow(ventana, SW_SHOW);
		EndDialog(hWnd, 0);
	}break;
	case ID_Agenda: {
		HWND ventana = CreateDialog(hInst,
			MAKEINTRESOURCE(IDD_Agenda),
			NULL,
			ventanaAgenda);
		ShowWindow(ventana, SW_SHOW);
		EndDialog(hWnd, 0);
	}break;
	case ID_CITAS: {
		HWND ventana = CreateDialog(hInst,
			MAKEINTRESOURCE(IDD_CITAS),
			NULL,
			ventanaCitas);
		ShowWindow(ventana, SW_SHOW);
		EndDialog(hWnd, 0);
	}break;
	case IDM_EXIT: {
		PostQuitMessage(0);
	}break;
	case WM_DESTROY: {
		PostQuitMessage(0);
	}break;
	default: return false;
	}
	return true;
}

// Agregar paciente al inicio de la lista
void agregarPacienteInicio(PACIENTES* dato) {
	NODOLA* nodo = nuevoNodo(dato);
	if (ListaClientes.origen == NULL) {
		ListaClientes.origen = nodo;
		ListaClientes.fin = nodo;
		nodo->siguiente = NULL;
	}
	else {
		nodo->siguiente = ListaClientes.origen;
		ListaClientes.origen = nodo;
	}
}

// Agregar paciente en medio de la lista
void agregarPacienteMedio(const char* buscar, PACIENTES* dato) {
	NODOLA* busqueda = buscarNombre(buscar);
	if (busqueda == nullptr)
		return;
	if (busqueda == ListaClientes.fin)
		return agregarPacienteFinal(dato);
	NODOLA* nodo = nuevoNodo(dato);
	nodo->siguiente = busqueda->siguiente;
	busqueda->siguiente = nodo;
}

// Agregar paciente al final de la lista
void agregarPacienteFinal(PACIENTES* dato) {
	NODOLA* nodo = nuevoNodo(dato);
	if (ListaClientes.origen == NULL) {
		ListaClientes.origen = nodo;
		ListaClientes.fin = nodo;
		nodo->siguiente = NULL;
	}
	else {
		ListaClientes.fin->siguiente = nodo;
		ListaClientes.fin = nodo;
		nodo->siguiente = NULL;
	}
}

// Convierte de char a wchar
void ConvertirCharToWCHAR(const char* source, WCHAR* dest, int destSize) {
	MultiByteToWideChar(CP_UTF8, 0, source, -1, dest, destSize);
}
