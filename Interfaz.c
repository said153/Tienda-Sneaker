//********************************Proyecto final**********************************************
//********************************Interfaz Usuario********************************************
//********************************************************************************************
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <curses.h>
#define MAX_PRODUCTOS 100
typedef struct nodos{
    int idProducto;
    char nombre[50];
    char marca[50];
    int cantidad;
    double precio;
} Producto;

typedef struct nodos1{
    int idCliente;
    int idProducto;
    int idCarrito;
    int cantidad;
    double precio;
    double precioFinal;
    bool resultado;
}   Carrito;

typedef struct nodos2{
    int idCliente;
    char usuario[50];
    char password[50];
    bool resultado;
} Usuario;

typedef struct  nodos3{
    int opcionMenu;
    int cantidadMenu;
    int idProductoMenu;
    bool resultado;
} Menu;

void pantalla();
void Opciones();
int sesion(Usuario*, Menu*, Carrito*, Producto*);
void registrar();
void recuperar();
void encriptar(char *cadena);
void verificacion(char *, char *);
void color();
void visualizar();
void OPcionescatalogo();
void carrito();
void agregar(Carrito*);
void mostrarCatalogo(Menu *Opcion,Carrito *, Producto*, Usuario*);
void visualizarCarrito(Usuario* cliente);
void mostrarCatalogoInterfaz();

int main(){
    key_t key = 12;
    key_t key2 = 34;
    key_t key3 = 542;
    key_t key4 = 145;

//Inicializamos ncurses
    initscr();
//Creamos el borde
    box(stdscr, '*', '*');
//*************************************************************************************************
//*****************************MEMORIA COMPARTIDA**************************************************
    Menu *miMenu = NULL;
    int idMemoria1 = shmget(key,sizeof(Menu), 0666);
    if(idMemoria1 == -1){
        perror("\n\tError al obtener la memoria compartida de Menu");
        return 1;
    }
    
    Producto *productos = NULL;
    int idMemoria2 = shmget(key2, MAX_PRODUCTOS *sizeof(Producto), 0777);
    if(idMemoria2 ==-1){
        perror("\n\tError al obtener la memoria compartida de Producto");
        return 1;
    }
    
    Usuario *cliente = NULL;
    int idMemoria3 = shmget(key3, sizeof(Usuario), 0777);
    if(idMemoria3 ==-1){
        perror("\n\tError al obtener la memoria compartida de Usuario");
        return 1;
    }

    Carrito *miCarrito = NULL;
    int idMemoria4 = shmget(key4,sizeof(Carrito) * 10, 0777);
    if(idMemoria4 ==-1){
        perror("\n\tError al obtener la memoria compartida de Carrito");
        return 1;
    }
//*************************************************************************************************
//*****************************Menu de opciones**************************************************
    miMenu = (Menu*)shmat(idMemoria1,NULL,0);
    productos = (Producto*)shmat(idMemoria2, NULL,0);
    cliente = (Usuario*)shmat(idMemoria3, NULL, 0);
    miCarrito = (Carrito*)shmat(idMemoria4, NULL, 0);
    int numero;
    do{
        Opciones();
        move(23,43);
        printw("Ingresa la opcion: ");
        scanw("%d", &numero);
        switch(numero){
            case 1:
                miMenu->opcionMenu = 1;
                sesion(cliente, miMenu,miCarrito, productos);
                break;
        
            case 2:
                miMenu->opcionMenu = 2;
                refresh();
                registrar(cliente);
                break;
        
            case 3:
                refresh();
                miMenu->opcionMenu = 3;
                recuperar();
                break;
            case 4:
                miMenu->opcionMenu = 4;
                clear();
                move(15,55);
                printw("Finalizando interfaz....");
                break;
            default:
                printw("OPCION INVALIDA");
                break;
        }
    }while(numero != 4);
    shmdt(miMenu);
    shmdt(cliente);

    refresh();
    getch();
    endwin();
}

void pantalla(){
    clear();
    box(stdscr, '*', '*');
}

void Opciones(){
    pantalla();
    color();
    attron(COLOR_PAIR(2) | A_BOLD | A_UNDERLINE);
    move(5,60);
    printw("BIENVENIDO A LA TIENDA DE SNAKER");
    attroff(COLOR_PAIR(2) | A_BOLD | A_UNDERLINE);
    refresh();
    
//*************************************Lado izquierdo*************************************************
    attron(COLOR_PAIR(3) | A_BOLD | A_BLINK);
    move(12,7);
    printw("T");
    move(14,7);
    printw("e");
    move(16,7);
    printw("n");
    move(18,7);
    printw("n");
    move(20,7);
    printw("i");
    move(22,7);
    printw("s");
    attroff(COLOR_PAIR(3) | A_BOLD | A_BLINK);
//**************************************************************************************************

//*************************************Lado derecho*************************************************    
    attron(COLOR_PAIR(3) | A_BOLD | A_BLINK);
    move(12,140);
    printw("T");
    move(14,140);
    printw("e");
    move(16,140);
    printw("n");
    move(18,140);
    printw("n");
    move(20,140);
    printw("i");
    move(22,140);
    printw("s");
    attroff(COLOR_PAIR(3) | A_BOLD | A_BLINK);
    refresh();
//**************************************************************************************************

    attron(COLOR_PAIR(2) | A_BOLD | A_UNDERLINE);
    move(10,35);
    printw("Seleccione el numero de la opcion de su agrado");
    attroff(COLOR_PAIR(2) | A_BOLD | A_UNDERLINE);
    move(15,43);
    printw("1) Iniciar sesion");
    move(17,43);    
    printw("2) Registrarte");
    move(19,43);
    printw("3) Olvide la contraseña");
    move(21,43);
    printw("4) Salir");
}

int sesion(Usuario *cliente, Menu *Opcion,Carrito *resultado, Producto* productos){
    clear();
    color();
    box(stdscr, '*', '*');
//*************************************Titulo*************************************************
    attron(COLOR_PAIR(5) | A_BOLD);
    move(5,70);
    printw("TIENDA SNAKER");
    attroff(COLOR_PAIR(5) | A_BOLD);
//********************************************************************************************
//*****************************Ingresar datos*************************************************
    curs_set(0);
    attron(COLOR_PAIR(8));
    move(10, 48);
    printw("Usuario:");
    attroff(COLOR_PAIR(8));
    scanw("%s", cliente->usuario);
    noecho();
    move(13,48);    
    attron(COLOR_PAIR(8));
    printw("Contraseña:");
    attroff(COLOR_PAIR(8));
    scanw("%s",cliente->password);
    echo();
    curs_set(1);
//********************************************************************************************
//*****************************Comprobacion de los resultados*********************************
    if (Opcion->resultado)
    {
        pantalla();
        move(13,48);
        printw("Encontrado");
        mostrarCatalogo(Opcion,resultado, productos, cliente);
        getch();
    }
    
    else{
        pantalla();
        move(13,48);
        printw("No encontrado");
        getch();
    }
//********************************************************************************************
}

void registrar(Usuario *cliente){
//**************************************Inicializacion de la base*****************************
    MYSQL *conn;
    conn = mysql_init(NULL);
    if (conn == NULL)
        printf("Error al inicializar la conexión: %s\n", mysql_error(conn));
    if (mysql_real_connect(conn, "localhost", "root", "S@id153*", "Operativos", 0, NULL, 0) == NULL)
        printf("Error al conectarse a la base de datos: %s\n", mysql_error(conn));
//********************************************************************************************
//********************************************************************************************
    clear();
    int resultado;
    color();
    refresh();
    box(stdscr, '*', '*');
//*********************************************************************************************    
//***************************************Titulo************************************************    
    attron(COLOR_PAIR(2) | A_BOLD | A_UNDERLINE);
    move(5, 65);
    printw("REGISTRO DE USUARIOS");
    attroff(COLOR_PAIR(2) | A_BOLD | A_UNDERLINE);
//********************************************************************************************
//****************************Nombre de usuario***********************************************
    char nombre[20];
    move(10, 40);
    attron(COLOR_PAIR(9) | A_BOLD);
    curs_set(0);
    printw("Ingresa el nombre de usuario:");
    attroff(COLOR_PAIR(9) | A_BOLD);
    scanw("%s", cliente->usuario);
//********************************************************************************************
//**********************************Contraseña************************************************
    char contraseña[20];
    noecho();
    refresh();
    move(13,40);
    attron(COLOR_PAIR(9) | A_BOLD);
    printw("Ingresa la contraseña:");
    attroff(COLOR_PAIR(9) | A_BOLD);
    scanw("%s", cliente->password);
//********************************************************************************************
//************************************Confirmacion********************************************
    char confirmacion[20];
    refresh();
    move(16,40);
    attron(COLOR_PAIR(9) | A_BOLD);
    printw("Confirma la contraseña introducida:");
    attroff(COLOR_PAIR(9) | A_BOLD);
    scanw("%s", confirmacion);
    echo();
    curs_set(1);
//********************************************************************************************
//********************************Ingresar datos a la base************************************
    resultado = strcmp(cliente->password, confirmacion);
    if(resultado == 0){
        char consulta[100];
        //sprintf(consulta, "INSERT INTO cliente (usuario,contraseña) VALUES ('%s')", cliente->usuario);
        if(mysql_query(conn, consulta)){
            move(30,60);
            printw("Error al ejecutar la consulta: %s", mysql_error(conn));
        }
        else{
            refresh();
            move(30,60);
            printw("Usuario registrado");
        }
    }

    else{
        move(20,60);
        attron(COLOR_PAIR(10) | A_BOLD | A_BLINK);
        printw("La contraseña no coincide");
        attroff(COLOR_PAIR(10) | A_BOLD | A_BLINK);
    }
    getch();
    mysql_close(conn);
}

void recuperar(){
//**************************************Inicializacion de la base*****************************
    MYSQL *conn;
    conn = mysql_init(NULL);

    if (conn == NULL)
        printf("Error al inicializar la conexión: %s\n", mysql_error(conn));

    if (mysql_real_connect(conn, "localhost", "root", "S@id153*", "Operativos", 0, NULL, 0) == NULL)
        printf("Error al conectarse a la base de datos: %s\n", mysql_error(conn));
//********************************************************************************************
//******************************************TITULO********************************************
    pantalla();
    char nombre[10];
    color();
    attron(COLOR_PAIR(8) | A_BOLD);
    move(5,55);
    printw("BIENVENIDO A LA RECUPERACION DE CUENTA");
    attroff(COLOR_PAIR(8) | A_BOLD);//********************************************************************************************
//***********************************Comprobar el usuario*************************************
    attron(COLOR_PAIR(6) | A_BOLD);
    move(12,45);
    printw("Ingresa tu usuario: ");
    attroff(COLOR_PAIR(6) | A_BOLD);
    scanw("%s",nombre);

    char consulta[100];
    sprintf(consulta, "SELECT contraseña FROM cliente WHERE usuario = '%s' ", nombre);
    
    if (mysql_query(conn, consulta))
        printf("Error al ejecutar la consulta: %s", mysql_error(conn));
    
    MYSQL_RES *resultado = mysql_store_result(conn);
    if (resultado == NULL)
        printf("Error al obtener el resultado: %s", mysql_error(conn));
    
    MYSQL_ROW fila = mysql_fetch_row(resultado);
    if(fila != NULL){
        char dato[100];
        strcpy(dato, fila[0]);
        pantalla();
        move(12,59);
        printw("Usuario encontrado");
        encriptar(dato);
    }

    else{
        pantalla();
        move(15,57);
        printw("Usuario no encontrado");
    }

    move(22,45);
    printw("Da enter para regresar al menu principal");
    getch();
    mysql_free_result(resultado);
    mysql_close(conn);
}

void encriptar(char *cadena){
    int i, longitud = strlen(cadena);
	char pista[longitud + 1];

    for( i = 0 ; i < longitud - 3 ; i++){
    	if(isalpha(cadena[i]))
            pista[i] = '*';
        else
            pista[i] = cadena[i];
	}

	for( i = longitud - 3; i < longitud ; i++)
		pista[i] = cadena[i];
	
    pista[longitud] = '\0';
    move(15,55);
	printw("Pista de tu contraseña: %s",pista);
}

void color(){
    start_color();
    init_pair(1, COLOR_RED, COLOR_YELLOW);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);
    init_pair(7, COLOR_RED, COLOR_WHITE);
    init_pair(8, COLOR_WHITE, COLOR_RED);
    init_pair(9, COLOR_BLUE, COLOR_WHITE);
    init_pair(10, COLOR_RED, COLOR_BLACK);
}

void OPcionescatalogo(){
    color();
    pantalla();
    attron(COLOR_PAIR(2) | A_BOLD | A_UNDERLINE);
    move(10,35);
    printw("Seleccione la opcion de su agrado");
    attroff(COLOR_PAIR(2) | A_BOLD | A_UNDERLINE);
    move(15,43);
    printw("1) Mostrar catalogo");
    move(17,43);
    printw("2) Agregar al carrito");
    move(19,43);
    printw("3) Mostrar carrito");
    move(21,43);
    printw("4) Salir");
}

void carrito(){
    key_t key4 = 145;
    int idMemoria4 = shmget(key4,sizeof(Carrito)*100,0777);
    if(idMemoria4 == -1){
        perror("\n\tError al obtener la memoria compartida de Carrito");
    }
    Carrito *miCarrito = (Carrito*)shmat(idMemoria4,NULL,0);
}

void agregar(Carrito* resultado){
    if(resultado->resultado){
        pantalla();
        move(3,55);
        printw("Se agrego correctamente");
    }
    else{
        pantalla();
        move(3,55);
        printw("No se agrego");
    }
}

void visualizar() {
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);

    if (mysql_real_connect(conn, "localhost", "root", "S@id153*", "Operativos", 0, NULL, 0) == NULL) {
        printf("Error de conexión: %s\n", mysql_error(conn));
    }

    if (mysql_query(conn, "SELECT * FROM producto")) {
        printf("Error al ejecutar la consulta: %s\n", mysql_error(conn));
    }

    res = mysql_use_result(conn);
    pantalla();
    move(10,13);
    printw("ID");
    move(10,25);
    printw("Nombre");
    move(10,39);
    printw("Marca");
    move(10,47);
    printw("Cantidad");
    move(10,58);
    printw("Precio");
    refresh();

    int j =12;
    while ((row = mysql_fetch_row(res)) != NULL) {
        int id = atoi(row[0]);
        char* nombre = row[1];
        char* marca = row[2];
        int cantidad = atoi(row[3]);
        double precio = atof(row[4]);
        move(j+1,j+3);
        printw("%d\t%s\t%s\t%d\t%.2lf\n", id, nombre, marca, cantidad, precio);
        j++;
    }

    mysql_free_result(res);
    mysql_close(conn);
}

void mostrarCatalogo(Menu *Opcion,Carrito *resultado, Producto *productos, Usuario *cliente){
    pantalla(); 
    int opcion, idProductoMenu, cantidadProductoMenu;

    do{
        OPcionescatalogo();
        move(22, 55);
        printw("Cual es tu opcion: ");
        scanw("%d", &opcion);
        switch (opcion)
        {
            case 1:
                pantalla();
                mostrarCatalogoInterfaz();
                 //MOSTRAR CATALOG0
                break;
            case 2:
                //mostrar(productos,num_productos);
                visualizar();
                move(25, 50);
                printw("Digita el id del producto: ");
                scanw("%d",&idProductoMenu);
                Opcion->idProductoMenu = idProductoMenu;
                move(28,50);
                printw("Digita la canitdad que quieras agregar: ");
                scanw("%d",&cantidadProductoMenu);
                Opcion->cantidadMenu = cantidadProductoMenu;
                Opcion->opcionMenu = 3;
                agregar(resultado); //AGREGAR CARRITO ID_PRODUCTO CANTIDAD ID_CLIENTE
                visualizarCarrito(cliente);
                break;
            case 3:
                Opcion->opcionMenu = 4;
                pantalla();
                visualizarCarrito(cliente);
                break;

            default:
                printw("OPCION INVALIDA");
                break;
        }
    } while (opcion != 4);
    shmdt(productos);
}

void mostrar(Producto *productos,size_t limite){
    pantalla();
    for (size_t i = 0; i < limite; i++){
        printw("ID: %d\n", productos[i].idProducto);
        printw("Nombre: %s\n", productos[i].nombre);
        printw("Marca: %s\n", productos[i].marca);
        printw("Cantidad: %d\n", productos[i].cantidad);
        printw("Precio: %.2f\n", productos[i].precio);
        printw("----------------------------\n");
    }
}

void mostrarCatalogoInterfaz(){
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Error al inicializar la conexión: %s\n", mysql_error(conn));
        exit(1);
    }
    if (mysql_real_connect(conn, "localhost", "root", "S@id153*", "Operativos", 0, NULL, 0) == NULL) {
        fprintf(stderr, "Error al conectarse a la base de datos: %s\n", mysql_error(conn));
        exit(1);
    }
    if (mysql_query(conn, "SELECT * FROM producto")) {
        fprintf(stderr, "Error al ejecutar la consulta: %s\n", mysql_error(conn));
        exit(1);
    }
    res = mysql_use_result(conn);

    color();
    attron(COLOR_PAIR(2) | A_BOLD | A_UNDERLINE);
    move(5,60);
    printw("Catalogo de productos");
    attroff(COLOR_PAIR(2) | A_BOLD | A_UNDERLINE);

    move(10,19);
    printw("ID");
    move(10,26);
    printw("Nombre");
    move(10,39);
    printw("Marca");
    move(10,47);
    printw("Cantidad");
    move(10,58);
    printw("Precio");
    refresh();

    int j= 10;
    while ((row = mysql_fetch_row(res)) != NULL){
        move(j + 1, 20);
        for (int i = 0; i < 5; i++)
        {
            printw("%s\t", row[i]);
            refresh();
        }
        j++;
    }

    mysql_free_result(res);
    mysql_close(conn);

    getch(); // Esperar a que el usuario presione una tecla antes de salir
}

void visualizarCarrito(Usuario* cliente){
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
    
    // Establecer la conexión a la base de datos
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "root", "S@id153*", "Operativos", 0, NULL, 0)) {
        endwin();
        fprintf(stderr, "Error de conexión: %s\n", mysql_error(conn));
    }
    // Construir la consulta SQL
    char query[100];
    sprintf(query, "SELECT * FROM carrito WHERE id_cliente = %d", cliente->idCliente);
    // Ejecutar la consulta
    if (mysql_query(conn, query)) {
        endwin();
        fprintf(stderr, "Error al ejecutar la consulta: %s\n", mysql_error(conn));
    }
    // Obtener el resultado de la consulta
    result = mysql_use_result(conn);
    // Recorrer el resultado y mostrar las filas en la ventana
    color();
    attron(COLOR_PAIR(2) | A_BOLD | A_UNDERLINE);
    move(5,65);
    printw("CARRITO");
    attroff(COLOR_PAIR(2) | A_BOLD | A_UNDERLINE);

    int j = 10;
    while ((row = mysql_fetch_row(result)) != NULL) {
        move(j + 1, 20);
    
            printw("ID: %s, Producto: %s, Cantidad: %s", row[0], row[1], row[3]);
            refresh();
        j++;
    }

    // Actualizar y refrescar la ventana
    // Esperar la entrada del usuario antes de salir
    getch();

    // Liberar memoria y cerrar la conexión
    mysql_free_result(result);
    mysql_close(conn);
}